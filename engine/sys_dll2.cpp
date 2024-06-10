#include <clocale>
#include <cstdint>
#include <cstring>

#include "winheaders.h"

#ifdef WIN32
#include <Winsock2.h>
#else
#error "Implement me"
#endif

#include "steam/steam_api.h"

#include "quakedef.h"

#include "cdll_int.h"

#include "vgui2/src/vgui_key_translation.h"

#include "dll_state.h"
#include "client.h"
#include "common.h"
#include "buildnum.h"
#include "engine_hlds_api.h"
#include "engine_launcher_api.h"
#include "filesystem.h"
#include "host.h"
#include "host_cmd.h"
#include "idedicatedexports.h"
#include "IEngine.h"
#include "IGame.h"
#include "igameuifuncs.h"
#include "IRegistry.h"
#include "kbutton.h"
#include "keys.h"
#include "modinfo.h"
#include "pr_cmds.h"
#include "qgl.h"
#include "strtools.h"
#include "sv_steam3.h"
#include "sys.h"
#include "sys_getmodes.h"
#include "traceinit.h"
#include "vgui_int.h"

char* g_pPostRestartCmdLineArgs = nullptr;

bool g_bIsDedicatedServer = false;

const char* szReslistsBaseDir = "reslists";
const char* szCommonPreloads = "multiplayer_preloads";
const char* szReslistsExt = ".lst";

const char* argv[ MAX_NUM_ARGVS ];

void Sys_InitArgv( char* lpCmdLine );
void Sys_ShutdownArgv();

void SetRateRegistrySetting( const char* pchRate )
{
	registry->WriteString( "rate", pchRate );
}

const char* GetRateRegistrySetting( const char* pchDef )
{
	return registry->ReadString( "rate", pchDef );
}

const char* GetCurrentSteamAppName()
{
	if( !stricmp( com_gamedir, "cstrike" ) ||
		!stricmp( com_gamedir, "cstrike_beta" ) )
	{
		return "Counter-Strike";
	}
	else if( !stricmp( com_gamedir, "valve" ) )
	{
		return "Half-Life";
	}
	else if( !stricmp( com_gamedir, "ricochet" ) )
	{
		return "Ricochet";
	}
	else if( !stricmp( com_gamedir, "dod" ) )
	{
		return "Day of Defeat";
	}
	else if( !stricmp( com_gamedir, "tfc" ) )
	{
		return "Team Fortress Classic";
	}
	else if( !stricmp( com_gamedir, "dmc" ) )
	{
		return "Deathmatch Classic";
	}
	else if( !stricmp( com_gamedir, "czero" ) )
	{
		return "Condition Zero";
	}

	return "Half-Life";
}

int RunListenServer( void *instance, char *basedir, char *cmdline, char *postRestartCmdLineArgs, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory );

class CEngineAPI final : public IEngineAPI
{
public:
	int Run( void* instance, char* basedir, char* cmdline, char* postRestartCmdLineArgs, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory ) override;
};

EXPOSE_SINGLE_INTERFACE( CEngineAPI, IEngineAPI, ENGINE_LAUNCHER_INTERFACE_VERSION );

int CEngineAPI::Run( void* instance, char* basedir, char* cmdline, char* postRestartCmdLineArgs, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory )
{
	if( !strstr( cmdline, "-nobreakpad" ) )
	{
		SteamAPI_UseBreakpadCrashHandler( va( "%d", build_number() ), __DATE__, __TIME__, 0, 0, 0 );
	}

	return RunListenServer( instance, basedir, cmdline, postRestartCmdLineArgs, launcherFactory, filesystemFactory );
}

//Legacy factory function used to acquire the client launcher interface
DLL_EXPORT void F( void* pv )
{
	auto factory = Sys_GetFactoryThis();

	auto api = reinterpret_cast<IEngineAPI**>( pv );

	*api = static_cast<IEngineAPI*>( factory( ENGINE_LAUNCHER_INTERFACE_VERSION, nullptr ) );
}

class CDedicatedServerAPI : IDedicatedServerAPI
{
public:
	bool Init( char* basedir, char* cmdline, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory ) override;

	int Shutdown() override;

	bool RunFrame() override;

	void AddConsoleText( char* text ) override;

	void UpdateStatus( float* fps, int* nActive, int* nMaxPlayers, char* pszMap ) override;

private:
	char m_OrigCmd[ 1024 ];
};

IDedicatedExports* dedicated = nullptr;

EXPOSE_SINGLE_INTERFACE( CDedicatedServerAPI, IDedicatedServerAPI, ENGINE_HLDS_INTERFACE_VERSION );

bool CDedicatedServerAPI::Init( char* basedir, char* cmdline, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory )
{
	dedicated = static_cast<IDedicatedExports*>( launcherFactory( "VENGINE_DEDICATEDEXPORTS_API_VERSION001", nullptr ) );
	
	if( dedicated )
	{
		strcpy( m_OrigCmd, cmdline );

		if( !strstr( cmdline, "-nobreakpad" ) )
		{
			SteamAPI_UseBreakpadCrashHandler( va( "%d", build_number() ), __DATE__, __TIME__, false, nullptr, nullptr );
		}

		TraceInit( "Sys_InitArgv( m_OrigCmd )", "Sys_ShutdownArgv()", 0 );
		Sys_InitArgv( m_OrigCmd );

		eng->SetQuitting( IEngine::QUIT_NOTQUITTING );

		registry->Init();

		g_bIsDedicatedServer = true;

		TraceInit( "FileSystem_Init(basedir, (void *)filesystemFactory)", "FileSystem_Shutdown()", 0 );
		if( FileSystem_Init( basedir, filesystemFactory ) &&
			game->Init( nullptr ) &&
			eng->Load( true, basedir, cmdline ) )
		{
			char szCommand[ 256 ];
			snprintf( szCommand, ARRAYSIZE( szCommand ), "exec %s\n", servercfgfile.string );
			szCommand[ ARRAYSIZE( szCommand ) - 1 ] = '\0';
			Cbuf_InsertText( szCommand );

			return true;
		}
	}

	return false;
}

int CDedicatedServerAPI::Shutdown()
{
	eng->Unload();
	game->Shutdown();

	TraceShutdown( "FileSystem_Shutdown()", 0 );
	FileSystem_Shutdown();

	registry->Shutdown();

	TraceShutdown( "Sys_ShutdownArgv()", 0 );
	Sys_ShutdownArgv();

	dedicated = nullptr;

	return giActive;
}

bool CDedicatedServerAPI::RunFrame()
{
	if( !eng->GetQuitting() )
	{
		eng->Frame();
		return true;
	}

	return false;
}

void CDedicatedServerAPI::AddConsoleText( char* text )
{
	Cbuf_AddText( text );
}

void CDedicatedServerAPI::UpdateStatus( float* fps, int* nActive, int* nMaxPlayers, char* pszMap )
{
	Host_GetHostInfo( fps, nActive, nullptr, nMaxPlayers, pszMap );
}

vgui2::KeyCode GetVGUI2KeyCodeForBind( const char* bind )
{
	const char* pszName = Key_NameForBinding( bind );

	if( pszName )
	{
		auto sdlKey = GetSDLKeycodeFromEngineKey( Key_StringToKeynum( pszName ) );

		if( sdlKey > SDL_SCANCODE_UNKNOWN )
			return KeyCode_VirtualKeyToVGUI( sdlKey );
	}

	return vgui2::KEY_NONE;
}

class CGameUIFuncs : public IGameUIFuncs
{
public:
	CGameUIFuncs() = default;
	~CGameUIFuncs() = default;

	bool IsKeyDown( const char* keyname, bool& isdown ) override
	{
		auto pButton = ClientDLL_FindKey( keyname );

		if( !pButton )
			return false;

		//TODO: define constants for state - Solokiller
		isdown = ( pButton->state & 1 ) != 0;

		return true;
	}

	const char* Key_NameForKey( int keynum ) override
	{
		return Key_KeynumToString( keynum );
	}

	const char* Key_BindingForKey( int keynum ) override
	{
		return ::Key_BindingForKey( keynum );
	}

	vgui2::KeyCode GetVGUI2KeyCodeForBind( const char* bind ) override
	{
		return ::GetVGUI2KeyCodeForBind( bind );
	}

	void GetVideoModes( vmode_t** liststart, int* count ) override
	{
		VideoMode_GetVideoModes( liststart, count );
	}

	void GetCurrentVideoMode( int* width, int* height, int* bpp ) override
	{
		VideoMode_GetCurrentVideoMode( width, height, bpp );
	}

	void GetCurrentRenderer( char* name, int namelen,
							 int* windowed, int* hdmodels,
							 int* addons_folder, int* vid_level ) override
	{
		VideoMode_GetCurrentRenderer(
			name, namelen,
			windowed,
			hdmodels, addons_folder,
			vid_level
		);
	}

	bool IsConnectedToVACSecureServer() override
	{
		if( cls.state == ca_active || cls.state == ca_connected )
			return cls.isVAC2Secure;

		return false;
	}

	int Key_KeyStringToKeyNum( const char* keyname ) override
	{
		return Key_StringToKeynum( keyname );
	}

private:
	CGameUIFuncs( const CGameUIFuncs& ) = delete;
	CGameUIFuncs& operator=( const CGameUIFuncs& ) = delete;
};

EXPOSE_SINGLE_INTERFACE( CGameUIFuncs, IGameUIFuncs, GAMEUIFUNCS_INTERFACE_VERSION );

void Sys_InitArgv( char* lpCmdLine )
{
	host_parms.argc = 1;
	argv[ 0 ] = "";

	while( *lpCmdLine && ( host_parms.argc < MAX_NUM_ARGVS ) )
	{
		while( *lpCmdLine && ( ( *lpCmdLine <= ' ' ) || ( *lpCmdLine > '~' ) ) )
			lpCmdLine++;

		if( *lpCmdLine )
		{
			argv[ host_parms.argc ] = lpCmdLine;
			host_parms.argc++;

			while( *lpCmdLine && ( ( *lpCmdLine > ' ' ) && ( *lpCmdLine <= '~' ) ) )
				lpCmdLine++;

			if( *lpCmdLine )
			{
				*lpCmdLine = '\0';
				lpCmdLine++;
			}

		}
	}

	host_parms.argv = argv;

	COM_InitArgv( host_parms.argc, host_parms.argv );

	host_parms.argc = com_argc;
	host_parms.argv = com_argv;
}

void Sys_ShutdownArgv()
{
	//Nothing
}

int RunListenServer( void *instance, char *basedir, char *cmdline, char *postRestartCmdLineArgs, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory )
{
	static char OrigCmd[ 1024 ];

	g_pPostRestartCmdLineArgs = postRestartCmdLineArgs;
	strcpy( OrigCmd, cmdline );

	TraceInit( "Sys_InitArgv( OrigCmd )", "Sys_ShutdownArgv()", 0 );
	Sys_InitArgv( OrigCmd );

	eng->SetQuitting( IEngine::QUIT_NOTQUITTING );
	registry->Init();
	Steam_InitClient();
	int result = ENGRUN_QUITTING;

	TraceInit( "FileSystem_Init(basedir, (void *)filesystemFactory)", "FileSystem_Shutdown()", 0 );

	if( FileSystem_Init( basedir, filesystemFactory ) )
	{
		VideoMode_Create();

		result = ENGRUN_UNSUPPORTED_VIDEOMODE;
		registry->WriteInt( "CrashInitializingVideoMode", 1 );

		if( videomode->Init( instance ) )
		{
			result = ENGRUN_CHANGED_VIDEOMODE;
			registry->WriteInt( "CrashInitializingVideoMode", 0 );

			if( game->Init(	instance ) )
			{
				result = ENGRUN_UNSUPPORTED_VIDEOMODE;

				if( eng->Load( false, basedir, cmdline ) )
				{
					while( true )
					{
						game->SleepUntilInput( 0 );

						if( eng->GetQuitting() != IEngine::QUIT_NOTQUITTING )
							break;

						eng->Frame();
					}

					result = eng->GetQuitting() != IEngine::QUIT_TODESKTOP ? ENGRUN_CHANGED_VIDEOMODE : ENGRUN_QUITTING;

					eng->Unload();
				}

				game->Shutdown();
			}

			videomode->Shutdown();
		}

		TraceShutdown( "FileSystem_Shutdown()", 0 );
		FileSystem_Shutdown();

		registry->Shutdown();

		TraceShutdown( "Sys_ShutdownArgv()", 0 );
		Sys_ShutdownArgv();
	}

	return result;
}

//TODO: from d_local.h - Solokiller
const int SURFCACHE_SIZE_AT_320X200 = 3072 * 1024;

int Sys_GetSurfaceCacheSize( int width, int height )
{
	const int iParm = COM_CheckParm( "-surfcachesize" );

	if( iParm )
	{
		//TODO: no range check? - Solokiller
		return Q_atoi( com_argv[ iParm + 1 ] ) * 1024;
	}

	int size = SURFCACHE_SIZE_AT_320X200;

	const int pix = width*height;
	if( pix > 64000 )
		size += ( pix - 64000 ) * 3;

	return size;
}

void Legacy_MP3subsys_Resume_Audio()
{
	//Nothing
}

void Legacy_MP3subsys_Suspend_Audio()
{
	//Nothing
}

void Legacy_ErrorMessage( int nLevel, const char* pszErrorMessage )
{
	//Nothing
}

void Legacy_Sys_Printf( const char* fmt, ... )
{
	char text[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsnprintf( text, ARRAYSIZE( text ), fmt, va );
	va_end( va );

	if( dedicated )
		dedicated->Sys_Printf( text );
}

void Sys_VID_FlipScreen()
{
	if( pmainwindow )
		SDL_GL_SwapWindow( pmainwindow );
}

//TODO: probably only used by software mode - Solokiller
using SurfaceCacheForResFn = int ( * )( int, int );

static SurfaceCacheForResFn D_SurfaceCacheForRes = nullptr;

void Sys_SetupLegacyAPIs()
{
	VID_FlipScreen = &Sys_VID_FlipScreen;
	D_SurfaceCacheForRes = &Sys_GetSurfaceCacheSize;
	Launcher_ConsolePrintf = &Legacy_Sys_Printf;
}

static bool Win32AtLeastV4 = false;
static bool g_bIsWin95 = false;
static bool g_bIsWin98 = false;

bool Sys_IsWin95()
{
	return g_bIsWin95;
}

bool Sys_IsWin98()
{
	return g_bIsWin98;
}

/**
*	Determine Windows OS version, set globals.
*	Information for fields retrieved from: https://www.go4expert.com/articles/os-version-detection-32-64-bit-os-t1472/
*/
void Sys_CheckOSVersion()
{
#ifdef WIN32
	OSVERSIONINFO vinfo;

	vinfo.dwOSVersionInfoSize = sizeof( vinfo );

	if( !GetVersionEx( &vinfo ) )
		Sys_Error( "Couldn't get OS info" );

	Win32AtLeastV4 = vinfo.dwMajorVersion >= 4;

	if( vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
		vinfo.dwMajorVersion == 4 )
	{
		if( vinfo.dwMinorVersion == 0 )
		{
			g_bIsWin95 = true;
		}
		else
		{
			if( vinfo.dwMinorVersion < 90 )
			{
				g_bIsWin98 = true;
			}
		}
	}
#endif
}

void Sys_InitLauncherInterface()
{
	gHasMMXTechnology = true;
	Sys_SetupLegacyAPIs();
}

void Sys_ShutdownLauncherInterface()
{
	//Nothing
}

void Sys_InitAuthentication()
{
	Sys_Printf( "STEAM Auth Server\r\n" );
}

void Sys_ShutdownAuthentication()
{
	//Nothing
}

static HDC maindc;
static HGLRC baseRC;

void Sys_Shutdown()
{
	Sys_ShutdownFloatTime();
	Steam_ShutdownClient();
	GL_Shutdown( pmainwindow, maindc, baseRC );
}

void Sys_InitMemory()
{
	// Take at least 14 Mb and no more than 134 Mb, unless they explicitly
	// request otherwise
	const int heapsizeIndex = COM_CheckParm( "-heapsize" );

	if( heapsizeIndex && heapsizeIndex < com_argc - 1 )
	{
		host_parms.memsize = strtol( com_argv[ heapsizeIndex + 1 ], nullptr, 10 ) * 1024;
	}

	if( host_parms.memsize >= MINIMUM_MEMORY )
	{
		if( host_parms.memsize > MAXIMUM_MEMORY )
		{
			host_parms.memsize = MAXIMUM_MEMORY;
		}
	}
	else
	{
		host_parms.memsize = DEFAULT_MEMORY;
	}

	if( COM_CheckParm( "-minmemory" ) )
	{
		host_parms.memsize = MINIMUM_MEMORY;
	}

	host_parms.membase = malloc( host_parms.memsize );

	if( !host_parms.membase )
		Sys_Error( "Unable to allocate %.2f MB\n", host_parms.memsize / ( 1024.0 * 1024.0 ) );
}

void Sys_ShutdownMemory()
{
	free( host_parms.membase );
	host_parms.membase = 0;
	host_parms.memsize = 0;
}

bool Sys_InitGame( char *lpOrgCmdLine, char *pBaseDir, void *pwnd, bool bIsDedicated )
{
	host_initialized = false;

	if( !bIsDedicated )
	{
		pmainwindow = reinterpret_cast<SDL_Window*>( pwnd );
		videomode->UpdateWindowPosition();
	}

	g_bIsDedicatedServer = bIsDedicated;

	Q_memset( &gmodinfo, 0, sizeof( gmodinfo ) );

	bool bSuccess = false;

	SV_ResetModInfo();

	TraceInit( "Sys_Init()", "Sys_Shutdown()", 0 );

	Sys_Init();

	FS_LogLevelLoadStarted( "Launcher" );

	SeedRandomNumberGenerator();

	TraceInit( "Sys_InitMemory()", "Sys_ShutdownMemory()", 0 );

	Sys_InitMemory();

	TraceInit( "Sys_InitLauncherInterface()", "Sys_ShutdownLauncherInterface()", 0 );
	
	Sys_InitLauncherInterface();

	if( GL_SetMode( pmainwindow, &maindc, &baseRC, "opengl32.dll", lpOrgCmdLine ) )
	{
		TraceInit( "Host_Init( &host_parms )", "Host_Shutdown()", 0 );
		
		Host_Init( &host_parms );

		if( host_initialized )
		{
			TraceInit( "Sys_InitAuthentication()", "Sys_ShutdownAuthentication()", 0 );
			
			Sys_InitAuthentication();

			if( g_bIsDedicatedServer )
			{
				Host_InitializeGameDLL();
				NET_Config( true );
			}
			else
			{
				ClientDLL_ActivateMouse();
			}

			bSuccess = true;

#ifndef WIN32
			char en_US[ 12 ];

			strcpy( en_US, "en_US.UTF-8" );

			//TODO: there is no category 6 - Solokiller
			const char* pszLocale = setlocale( 6, nullptr );

			if( !pszLocale )
				pszLocale = "c";

			if( stricmp( pszLocale, en_US ) )
			{
				char MessageText[ 512 ];

				snprintf(
					MessageText,
					sizeof( MessageText ),
					"SetLocale('%s') failed. Using '%s'.\nYou may have limited glyph support.\nPlease install '%s' locale.",
					en_US,
					pszLocale,
					en_US );

				SDL_ShowSimpleMessageBox( 0, "Warning", MessageText, pmainwindow );
			}

#endif
		}
	}

	return bSuccess;
}

void Sys_ShutdownGame()
{
	if( !g_bIsDedicatedServer )
	{
		ClientDLL_DeactivateMouse();
	}

	TraceShutdown( "Host_Shutdown()", 0 );
	Host_Shutdown();

	if( g_bIsDedicatedServer )
	{
		NET_Config( false );
	}

	TraceShutdown( "Sys_ShutdownLauncherInterface()", 0 );
	Sys_ShutdownLauncherInterface();

	TraceShutdown( "Sys_ShutdownAuthentication()", 0 );
	Sys_ShutdownAuthentication();

	TraceShutdown( "Sys_ShutdownMemory()", 0 );
	Sys_ShutdownMemory();

	TraceShutdown( "Sys_Shutdown()", 0 );
	Sys_Shutdown();
}

void ClearIOStates()
{
	for( int key = 0; key < 256; ++key )
	{
		Key_Event( key, false );
	}

	Key_ClearStates();
	ClientDLL_ClearStates();
}

//TODO: obsolete - Solokiller
static double g_flLastSteamProgressUpdateTime = 0;
static bool g_bPrintingKeepAliveDots = false;

void Sys_ShowProgressTicks()
{
	static bool recursionGuard = false;
	static int32 numTics = 0;

	if( !recursionGuard )
	{
		recursionGuard = true;

		if( COM_CheckParm( "-steam" ) )
		{
			const auto flTime = Sys_FloatTime();
			if( g_flLastSteamProgressUpdateTime + 2.0 <= flTime )
			{
				g_flLastSteamProgressUpdateTime = flTime;

				const auto iTics = ++numTics;

				if( g_bIsDedicatedServer )
				{
					if( g_bMajorMapChange )
					{
						g_bPrintingKeepAliveDots = 1;
						Sys_Printf( "." );
					}
				}
				else
				{
					char msg[ 128 ] = "Updating game resources";

					if( iTics % 5 >= 0 )
					{
						const auto iNumTics = iTics % 5 + 2;

						//TODO: not necessarily null terminated - Solokiller
						for( int i = 1; i < iNumTics; ++i )
						{
							msg[ strlen( msg ) ] = '.';
						}
					}

					SetLoadingProgressBarStatusText( msg );
				}
			}
		}

		recursionGuard = false;
	}
}

#ifdef WIN32
#define CDKEY_RANDOM_MAX INT16_MAX
#else
#define CDKEY_RANDOM_MAX INT32_MAX
#endif

void Sys_GetCDKey( char* pszCDKey, int* nLength, int* bDedicated )
{
	char hostname[ MAX_PATH ];
	char key[ 65 ];

	if( 0 != gethostname( hostname, ARRAYSIZE( hostname ) ) )
	{
		snprintf( key, ARRAYSIZE( key ), "%u", RandomLong( 0, CDKEY_RANDOM_MAX ) );
	}
	else
	{
		auto pHost = gethostbyname( hostname );

		if( pHost && pHost->h_length == 4 && pHost->h_addr_list[ 0 ] )
		{
			auto pszAddr = pHost->h_addr_list[ 0 ];

			snprintf(
				key, ARRAYSIZE( key ),
				"%u.%u.%u.%u",
				static_cast<unsigned int>( pszAddr[ 0 ] ),
				static_cast<unsigned int>( pszAddr[ 1 ] ),
				static_cast<unsigned int>( pszAddr[ 2 ] ),
				static_cast<unsigned int>( pszAddr[ 3 ] )
			);
		}
		else
		{
			CRC32_t crc;
			CRC32_ProcessBuffer( &crc, hostname, strlen( hostname ) );
			snprintf( key, ARRAYSIZE( key ), "%u", static_cast<unsigned int>( crc ) );
		}
	}

	key[ ARRAYSIZE( key ) - 1 ] = '\0';

	strcpy( pszCDKey, key );

	if( nLength )
		*nLength = strlen( key );

	if( bDedicated )
		*bDedicated = false;
}

int BuildMapCycleListHints( char** hints )
{
	char szMod[ MAX_PATH ];
	COM_FileBase( com_gamedir, szMod );

	char cszMapCycleTxtFile[ MAX_PATH ];
	sprintf( cszMapCycleTxtFile, "%s/%s", szMod, mapcyclefile.string );

	auto pFile = FS_Open( cszMapCycleTxtFile, "rb" );

	if( FILESYSTEM_INVALID_HANDLE == pFile )
	{
		Con_Printf( "Unable to open %s\n", cszMapCycleTxtFile );
		return false;
	}

	char szMap[ MAX_PATH + 2 ];
	sprintf(
		szMap, "%s\\%s\\%s%s\r\n",
		szReslistsBaseDir,
		GetCurrentSteamAppName(),
		szCommonPreloads,
		szReslistsExt
	);

	*hints = ( char * ) malloc( strlen( szMap ) + 1 );

	if( !*hints )
	{
		//TODO: file needs closing - Solokiller
		Con_Printf( "Unable to allocate memory for map cycle hints list" );
		return false;
	}

	strcpy( *hints, szMap );

	const auto uiSize = FS_Size( pFile );

	if( uiSize )
	{
		auto pszData = reinterpret_cast<char*>( malloc( uiSize ) );

		if( pszData )
		{
			auto pszParseToken = pszData;

			//TODO: this code will never work properly because FS_Read returns size read, not number of elements - Solokiller
			if( FS_Read( pszData, uiSize, pFile ) == 1 )
			{
				char mapLine[ MAX_PATH + 2 ];

				while( true )
				{
					pszParseToken = COM_Parse( pszParseToken );

					if( !com_token[ 0 ] )
					{
						break;
					}

					strncpy( szMap, com_token, ARRAYSIZE( szMap ) - 1 );
					szMap[ ARRAYSIZE( szMap ) - 1 ] = '\0';

					if( COM_TokenWaiting( pszParseToken ) )
						pszParseToken = COM_Parse( pszParseToken );

					snprintf(
						mapLine, ARRAYSIZE( mapLine ),
						"%s\\%s\\%s%s\r\n",
						szReslistsBaseDir,
						GetCurrentSteamAppName(),
						szMap,
						szReslistsExt
					);

					*hints = reinterpret_cast<char*>( realloc( *hints, strlen( *hints ) + strlen( mapLine ) + 2 ) );

					if( !*hints )
					{
						//TODO: file needs closing - Solokiller
						Con_Printf( "Unable to reallocate memory for map cycle hints list" );
						return 0;
					}

					strcat( *hints, mapLine );
				}
			}

			//TODO: this is the wrong address, it should be pszData - Solokiller
			free( pszParseToken );
		}
	}

	FS_Close( pFile );

	sprintf( szMap, "%s\\%s\\mp_maps.txt\r\n", szReslistsBaseDir, GetCurrentSteamAppName() );

	*hints = reinterpret_cast<char*>( realloc( *hints, strlen( *hints ) + strlen( szMap ) + 2 ) );
	strcat( *hints, szMap );

	return true;
}
