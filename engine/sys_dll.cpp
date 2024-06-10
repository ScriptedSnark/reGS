#include <csetjmp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <SDL2/SDL.h>

#include "quakedef.h"
#include "client.h"
#include "dll_state.h"
#include "server.h"
#include "host.h"

#include "FilePaths.h"
#include "qgl.h"
#include "modinfo.h"
#include "sys_getmodes.h"

#ifdef WIN32
#include "winheaders.h"
#else
#error
#include <dlfcn.h>
#include <unistd.h>
#endif

bool gHasMMXTechnology = false;

FlipScreenFn VID_FlipScreen = nullptr;

bool gfBackground = false;

//TODO: define flags for these - Solokiller
int giActive = DLL_INACTIVE;
int giStateInfo = 0;
int giSubState = 0;

bool gfExtendedError = false;
char gszDisconnectReason[ 256 ] = {};
char gszExtendedDisconnectReason[ 256 ] = {};

DLL_FUNCTIONS gEntityInterface = {};
NEW_DLL_FUNCTIONS gNewDLLFunctions = {};

PrintfFunc Launcher_ConsolePrintf = nullptr;

int g_iextdllMac = 0;
extensiondll_t g_rgextdll[ MAX_EXT_DLLS ] = {};

void Sys_Printf( const char* fmt, ... )
{
	char text[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsnprintf( text, sizeof( text ), fmt, va );
	va_end( va );

	if( g_bIsDedicatedServer && Launcher_ConsolePrintf )
	{
		Launcher_ConsolePrintf( "%s", text );
	}

	fprintf( stderr, "%s\n", text );
}

void Sys_Error( const char* error, ... )
{
	static bool bReentry = false;

	char text[ 1024 ];

	va_list va;

	va_start( va, error );
	vsnprintf( text, sizeof( text ), error, va );
	va_end( va );

	if( bReentry == false )
	{
		bReentry = true;

		if( svs.dll_initialized )
		{
			if( gEntityInterface.pfnSys_Error )
				gEntityInterface.pfnSys_Error( text );
		}

		Log_Printf( "FATAL ERROR (shutting down): %s\n", text );

		if( g_bIsDedicatedServer )
		{
			if( Launcher_ConsolePrintf )
				Launcher_ConsolePrintf( "FATAL ERROR (shutting down): %s\n", text );
			else
				printf( "FATAL ERROR (shutting down): %s\n", text );
		}
		else
		{
			Sys_Printf( text );
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Fatal Error", text, pmainwindow );
			//TODO: Don't know why this is here - Solokiller
			VideoMode_IsWindowed();
		}
		exit( -1 );
	}

	fprintf( stderr, "%s\n", text );

	longjmp( host_abortserver, 2 );
}

void Sys_Warning( const char* pszWarning, ... )
{
	char text[ 1024 ];
	va_list va;

	va_start( va, pszWarning );
	vsnprintf( text, ARRAYSIZE( text ), pszWarning, va );
	va_end( va );

	Sys_Printf( text );
}

void Sys_DebugOutStraight( const char* pStr )
{
	//TODO: verify that calls to fprintf use this function if needed - Solokiller
	fprintf( stderr, "%s\n", pStr );
}

static FileFindHandle_t g_hfind = FILESYSTEM_INVALID_FIND_HANDLE;

const char* Sys_FindFirstPathID( const char* path, char* pathid )
{
	if( g_hfind != FILESYSTEM_INVALID_FIND_HANDLE )
		Sys_Error( "Sys_FindFirst without close" );

	return FS_FindFirst( path, &g_hfind, pathid );
}

const char* Sys_FindFirst( const char *path, char* basename )
{
	if( g_hfind != FILESYSTEM_INVALID_FIND_HANDLE )
		Sys_Error( "Sys_FindFirst without close" );

	const char* result = FS_FindFirst( path, &g_hfind, nullptr );

	if( result && basename )
	{
		COM_FileBase( result, basename );
	}

	return result;
}

const char* Sys_FindNext( char* basename )
{
	const char* result = FS_FindNext( g_hfind );

	if( result && basename )
	{
		COM_FileBase( result, basename );
	}

	return result;
}

void Sys_FindClose()
{
	if( g_hfind != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		FS_FindClose( g_hfind );
		g_hfind = FILESYSTEM_INVALID_FIND_HANDLE;
	}
}

void ForceReloadProfile()
{
	Cbuf_AddText( "exec config.cfg\n" );
	Cbuf_AddText( "+mlook\n" );
	Cbuf_Execute();

	if( COM_CheckParm( "-nomousegrab" ) )
		Cvar_Set( "cl_mousegrab", "0" );

	Key_SetBinding( '~', "toggleconsole" );
	Key_SetBinding( '`', "toggleconsole" );
	Key_SetBinding( K_ESCAPE, "cancelselect" );

	SDL_GL_SetSwapInterval( ( gl_vsync.value <= 0.0 ) - 1 );

	if( cls.state != ca_dedicated )
	{
		char szRate[ 32 ];
		strncpy( szRate, GetRateRegistrySetting( rate.string ), ARRAYSIZE( szRate ) );
		Cvar_DirectSet( &rate, szRate );
	}
}

FIELDIOFUNCTION GetIOFunction( const char* pName )
{
	FIELDIOFUNCTION result = nullptr;

	for( int i = 0; i < g_iextdllMac; ++i )
	{
		result = reinterpret_cast<FIELDIOFUNCTION>( Sys_GetProcAddress( g_rgextdll[ i ].pDLLHandle, pName ) );

		if( result )
			break;
	}

	return result;
}

DISPATCHFUNCTION GetDispatch( const char* pname )
{
	DISPATCHFUNCTION result = nullptr;

	for( int i = 0; i < g_iextdllMac; ++i )
	{
		result = reinterpret_cast<DISPATCHFUNCTION>( Sys_GetProcAddress( g_rgextdll[ i ].pDLLHandle, pname ) );

		if( result )
			break;
	}

	return result;
}

ENTITYINIT GetEntityInit( const char* pClassName )
{
	ENTITYINIT result = nullptr;

	for( int i = 0; i < g_iextdllMac; ++i )
	{
		result = reinterpret_cast<ENTITYINIT>( Sys_GetProcAddress( g_rgextdll[ i ].pDLLHandle, pClassName ) );

		if( result )
			break;
	}

	return result;
}

void DLL_SetModKey( modinfo_t *pinfo, char *pkey, char *pvalue )
{
	if( !Q_stricmp( pkey, "url_info" ) )
	{
		pinfo->bIsMod = true;
		Q_strncpy( pinfo->szInfo, pvalue, sizeof( pinfo->szInfo ) - 1 );
		pinfo->szInfo[ sizeof( pinfo->szInfo ) - 1 ] = '\0';
	}
	else if( !Q_stricmp( pkey, "url_dl" ) )
	{
		pinfo->bIsMod = true;
		Q_strncpy( pinfo->szDL, pvalue, sizeof( pinfo->szDL ) - 1 );
		pinfo->szDL[ sizeof( pinfo->szDL ) - 1 ] = 0;
	}
	else if( !Q_stricmp( pkey, "version" ) )
	{
		pinfo->bIsMod = true;
		pinfo->version = strtol( pvalue, nullptr, 10 );
	}
	else if( !Q_stricmp( pkey, "size" ) )
	{
		pinfo->bIsMod = true;
		pinfo->size = strtol( pvalue, nullptr, 10 );
	}
	else if( !Q_stricmp( pkey, "svonly" ) )
	{
		pinfo->bIsMod = true;
		pinfo->svonly = strtol( pvalue, nullptr, 10 ) != 0;
	}
	else if( !Q_stricmp( pkey, "cldll" ) )
	{
		pinfo->bIsMod = true;
		pinfo->cldll = strtol( pvalue, nullptr, 10 ) != 0;
	}
	else if( !Q_stricmp( pkey, "secure" ) )
	{
		pinfo->bIsMod = true;
		pinfo->secure = strtol( pvalue, nullptr, 10 ) != 0;
	}
	else if( !Q_stricmp( pkey, "hlversion" ) )
	{
		Q_strncpy( pinfo->szHLVersion, pvalue, sizeof( pinfo->szHLVersion ) - 1 );
		pinfo->szHLVersion[ sizeof( pinfo->szHLVersion ) - 1 ] = '\0';
	}
	else if( !Q_stricmp( pkey, "edicts" ) )
	{
		int iEdicts = strtol( pvalue, nullptr, 10 );

		if( iEdicts < MAX_EDICTS )
			iEdicts = MAX_EDICTS;

		pinfo->num_edicts = iEdicts;
	}
	else if( !Q_stricmp( pkey, "fallback_dir" ) )
	{
		COM_AddDefaultDir( pvalue );
	}
	if( !Q_stricmp( pkey, "type" ) )
	{
		if( !Q_stricmp( pvalue, "singleplayer_only" ) )
		{
			pinfo->type = SINGLEPLAYER_ONLY;
		}
		else if( !Q_stricmp( pvalue, "multiplayer_only" ) )
		{
			pinfo->type = MULTIPLAYER_ONLY;
		}
		else
		{
			pinfo->type = BOTH;
		}
	}
}

//TODO: implement all functions and add here - Solokiller
static enginefuncs_t g_engfuncsExportedToDlls;

#ifdef WIN32
#define GIVEFNPTRSTODLL_CALLCONV __stdcall
#else
#define GIVEFNPTRSTODLL_CALLCONV
#endif

using GiveFnptrsToDllFn = void( GIVEFNPTRSTODLL_CALLCONV* )( enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals );

/**
*	Loads an entity DLL and passes the engine functions and global variables to it.
*/
static void LoadThisDll( const char* szDllFilename )
{
	CSysModule* pModule = Sys_LoadModule( szDllFilename );

	if( pModule )
	{
		auto pFn = reinterpret_cast<GiveFnptrsToDllFn>( Sys_GetProcAddress( pModule, "GiveFnptrsToDll" ) );

		if( pFn )
		{
			pFn( &g_engfuncsExportedToDlls, &gGlobalVariables );

			if( g_iextdllMac == MAX_EXT_DLLS )
			{
				Con_Printf( "Too many DLLs, ignoring remainder\n" );
				Sys_UnloadModule( pModule );
			}
			else
			{
				auto& extdll = g_rgextdll[ g_iextdllMac++ ];

				Q_memset( &extdll, 0, sizeof( extdll ) );
				extdll.pDLLHandle = pModule;
			}
		}
		else
		{
			Con_Printf( "Couldn't get GiveFnptrsToDll in %s\n", szDllFilename );
			Sys_UnloadModule( pModule );
		}
	}
	else
	{
#ifdef WIN32
		Con_Printf( "LoadLibrary failed on %s (%d)\n", szDllFilename, GetLastError() );
#else
		Con_Printf( "LoadLibrary failed on %s: %s\n", szDllFilename, dlerror() );
#endif
	}
}

//TODO: separate suffix for OSX? - Solokiller
#ifdef WIN32
#define GAMEDLL_KEY "gamedll"
#else
#define GAMEDLL_KEY "gamedll_linux"
#endif

void LoadEntityDLLs( const char* szBaseDir )
{
	SV_ResetModInfo();

	g_iextdllMac = 0;
	Q_memset( g_rgextdll, 0, sizeof( g_rgextdll ) );

	char szGameDir[ 64 ];
	Q_strncpy( szGameDir, com_gamedir, ARRAYSIZE( szGameDir ) );

	if( Q_stricmp( szGameDir, "valve" ) )
		gmodinfo.bIsMod = true;

	char szDllFilename[ 8192 ];

	//TODO: most of this code is copy pasted from SV_ResetModInfo - Solokiller
	char szDllListFile[ FILENAME_MAX ];
	snprintf( szDllListFile, ARRAYSIZE( szDllListFile ), "%s", "liblist.gam" );

	FileHandle_t hLibListFile = FS_Open( szDllListFile, "rb" );

	if( hLibListFile )
	{
		char szKey[ 64 ];
		char szValue[ 256 ];

		const int iSize = FS_Size( hLibListFile );

		if( iSize > ( 512 * 512 ) || !iSize )
			Sys_Error( "Game listing file size is bogus [%s: size %i]", "liblist.gam", iSize );

		byte* pFileData = reinterpret_cast<byte*>( Mem_Malloc( iSize + 1 ) );

		if( !pFileData )
			Sys_Error( "Could not allocate space for game listing file of %i bytes", iSize + 1 );

		const int iRead = FS_Read( pFileData, iSize, hLibListFile );

		if( iRead != iSize )
			Sys_Error( "Error reading in game listing file, expected %i bytes, read %i", iSize, iRead );

		pFileData[ iSize ] = '\0';

		char* pBuffer = ( char* ) pFileData;

		com_ignorecolons = true;

		while( 1 )
		{
			pBuffer = COM_Parse( pBuffer );

			if( Q_strlen( com_token ) <= 0 )
				break;

			Q_strncpy( szKey, com_token, ARRAYSIZE( szKey ) );

			pBuffer = COM_Parse( pBuffer );

			Q_strncpy( szValue, com_token, ARRAYSIZE( szValue ) );

			if( Q_stricmp( szKey, GAMEDLL_KEY ) )
			{
				DLL_SetModKey( &gmodinfo, szKey, szValue );
			}
			else
			{
				const int iDllOverride = COM_CheckParm( "-dll" );

				if( iDllOverride && iDllOverride < com_argc - 1 )
				{
					Q_strncpy( szValue, com_argv[ iDllOverride + 1 ], ARRAYSIZE( szValue ) );
				}

#ifndef WIN32
				//Find and remove architecture extensions from the filename.
				char* pszUnderscore = strchr( szValue, '_' );

				if( pszUnderscore )
				{
					*pszUnderscore = '\0';

					//Append correct extension.
					Q_strcat( szValue, DEFAULT_SO_EXT, ARRAYSIZE( szValue ) );
				}
#endif

				if( Q_strstr( szValue, DEFAULT_SO_EXT ) )
				{
					FS_GetLocalPath( szValue, szDllFilename, ARRAYSIZE( szDllFilename ) );
					Con_DPrintf( "\nAdding:  %s/%s\n", szGameDir, szValue );
					LoadThisDll( szDllFilename );
				}
				else
				{
					Con_DPrintf( "Skipping non-shared library:  %s\n", szValue );
				}
			}
		}
		com_ignorecolons = false;
		Mem_Free( pFileData );
		FS_Close( hLibListFile );
	}
	else
	{
		char szDllWildcard[ FILENAME_MAX ];

		snprintf( szDllWildcard, ARRAYSIZE( szDllFilename ), "%s/*" DEFAULT_SO_EXT, "valve/dlls" );

		for( const char* i = Sys_FindFirst( szDllWildcard, nullptr ); i; i = Sys_FindNext( nullptr ) )
		{
			snprintf( szDllFilename, ARRAYSIZE( szDllFilename ), "%s/%s/%s", szBaseDir, "valve/dlls", i );
			LoadThisDll( szDllFilename );
		}

		Sys_FindClose();
	}

	//Initialize the newest functions in case the dll doesn't provide it.
	gNewDLLFunctions.pfnGameShutdown = nullptr;
	gNewDLLFunctions.pfnShouldCollide = nullptr;
	gNewDLLFunctions.pfnCvarValue = nullptr;
	gNewDLLFunctions.pfnCvarValue2 = nullptr;
	gNewDLLFunctions.pfnOnFreeEntPrivateData = nullptr;

	int interface_version;

	//First check if the dll supports the newest functions.
	auto pGetNewDLLFunctions = reinterpret_cast<NEW_DLL_FUNCTIONS_FN>( GetIOFunction( "GetNewDLLFunctions" ) );

	if( pGetNewDLLFunctions )
	{
		interface_version = NEW_DLL_FUNCTIONS_VERSION;

		pGetNewDLLFunctions( &gNewDLLFunctions, &interface_version );
	}

	//Check if it supports the newer dll functions.
	auto pGetEntityAPI2 = reinterpret_cast<APIFUNCTION2>( GetIOFunction( "GetEntityAPI2" ) );

	if( pGetEntityAPI2 )
	{
		interface_version = INTERFACE_VERSION;

		if( !pGetEntityAPI2( &gEntityInterface, &interface_version ) )
		{
			Con_Printf( "==================\n" );
			Con_Printf( "Game DLL version mismatch\n" );
			Con_Printf( "DLL version is %i, engine version is %i\n", interface_version, INTERFACE_VERSION );

			if( interface_version > INTERFACE_VERSION )
				Con_Printf( "Engine appears to be outdated, check for updates\n" );
			else
				Con_Printf( "The game DLL for %s appears to be outdated, check for updates\n", szGameDir );

			Con_Printf( "==================\n" );
			Host_Error( "\n" );
		}
	}
	else
	{
		//Check if it provides the original dll functions.
		auto pGetEntityAPI = reinterpret_cast<APIFUNCTION>( GetIOFunction( "GetEntityAPI" ) );

		if( !pGetEntityAPI )
			Host_Error( "Couldn't get DLL API from %s!", szDllFilename );

		interface_version = INTERFACE_VERSION;

		if( !pGetEntityAPI( &gEntityInterface, interface_version ) )
		{
			Con_Printf( "==================\n" );
			Con_Printf( "Game DLL version mismatch\n" );
			Con_Printf( "The game DLL for %s appears to be outdated, check for updates\n", szGameDir );
			Con_Printf( "==================\n" );
			Host_Error( "\n" );
		}
	}

	const char* pszDescription = gEntityInterface.pfnGetGameDescription();

	const char* pszType = "mod";

	if( !gmodinfo.bIsMod )
		pszType = "game";

	Con_DPrintf( "Dll loaded for %s %s\n", pszType, pszDescription );
}

void ReleaseEntityDlls()
{
	if( svs.dll_initialized )
	{
		//TODO: implement - Solokiller
		//FreeAllEntPrivateData();

		if( gNewDLLFunctions.pfnGameShutdown )
			gNewDLLFunctions.pfnGameShutdown();

		Cvar_UnlinkExternals();

		for( int i = 0; i < g_iextdllMac; ++i )
		{
			auto& extdll = g_rgextdll[ i ];

			Sys_UnloadModule( extdll.pDLLHandle );
			extdll.pDLLHandle = nullptr;

			if( extdll.functionTable )
				Mem_Free( extdll.functionTable );

			extdll.functionTable = nullptr;
		}

		//TODO: this doesn't reset the extdll count, so it will eventually fill up all slots. Change? - Solokiller

		svs.dll_initialized = false;
	}
}

static bool s_bTimeInitialized = false;
static double curtime = 0.0;
static double lastcurtime = 0.0;

#ifdef WIN32
static double pfreq = 0;
static int lowshift = 0;
static CRITICAL_SECTION s_Time_CriticalSection;

#ifdef _M_IX86
static long ceil_cw = 0;
static long single_cw = 0;
static long full_cw = 0;
static long cw = 0;
static long pushed_cw = 0;
#endif
#endif

//TODO: determine if this needs changes for other platforms - Solokiller
void Sys_SetFPCW()
{
}

void Sys_PushFPCW_SetHigh()
{
}

void Sys_PopFPCW()
{
}

void MaskExceptions()
{
}

double Sys_FloatTime()
{
#ifdef WIN32
	static int			sametimecount;
	static unsigned int	oldtime;
	static bool			first = true;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp;

	if( !s_bTimeInitialized )
		return 1.0;

	EnterCriticalSection( &s_Time_CriticalSection );

	Sys_PushFPCW_SetHigh();

	QueryPerformanceCounter( &PerformanceCount );

	if( lowshift )
	{
		temp = ( ( unsigned int ) PerformanceCount.LowPart >> lowshift ) |
		( ( unsigned int ) PerformanceCount.HighPart << ( 32 - lowshift ) );
	}
	else
	{
		temp = ( unsigned int ) PerformanceCount.LowPart;
	}

	if( first )
	{
		oldtime = temp;
		first = false;
	}
	else
	{
		// check for turnover or backward time
		if( ( temp <= oldtime ) && ( ( oldtime - temp ) < 0x10000000 ) )
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			unsigned int t2 = temp - oldtime;

			double time = ( double ) t2 * pfreq;
			oldtime = temp;

			curtime += time;

			if( curtime == lastcurtime )
			{
				sametimecount++;

				if( sametimecount > 100000 )
				{
					curtime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			lastcurtime = curtime;
		}
	}

	Sys_PopFPCW();

	LeaveCriticalSection( &s_Time_CriticalSection );

	return curtime;
#else
	//TODO: verify implementation - Solokiller
	static timespec start_time;

	timespec now;

	if( !s_bTimeInitialized )
	{
		s_bTimeInitialized = true;
		clock_gettime( 1, &start_time );
	}

	clock_gettime( 1, &now );

	return ( double ) ( now.tv_sec - start_time.tv_sec ) + now.tv_nsec / 1000000.0;
#endif
}

void Sys_InitFloatTime()
{
	Sys_FloatTime();

	int j = COM_CheckParm( "-starttime" );

	if( j )
	{
		curtime = Q_atof( com_argv[ j + 1 ] );
	}
	else
	{
		curtime = 0;
	}

	lastcurtime = curtime;
}

void Sys_ShutdownFloatTime()
{
	lastcurtime = 0.0;
	curtime = 0.0;
}

//TODO: should refactor references to the globals to use these functions instead - Solokiller
void GameSetSubState( int iSubState )
{
	//TODO: define constants - Solokiller
	if( iSubState & 2 )
	{
		giSubState = 1;
	}
	else if( iSubState != 1 )
	{
		giSubState = iSubState;
	}
}

void GameSetState( int iState )
{
	giActive = iState;
}

void Dispatch_Substate( int iSubState )
{
	giSubState = iSubState;
}

void GameSetBackground( bool bNewSetting )
{
	gfBackground = bNewSetting;
}

void Sys_Init()
{
#ifdef WIN32
	if( !s_bTimeInitialized )
	{
		InitializeCriticalSection( &s_Time_CriticalSection );
		s_bTimeInitialized = true;
	}

	MaskExceptions();
	Sys_SetFPCW();

	LARGE_INTEGER	PerformanceFreq;

	if( !QueryPerformanceFrequency( &PerformanceFreq ) )
		Sys_Error( "No hardware timer available" );

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	unsigned int lowpart = ( unsigned int ) PerformanceFreq.LowPart;
	unsigned int highpart = ( unsigned int ) PerformanceFreq.HighPart;
	lowshift = 0;

	while( highpart || ( lowpart > 2000000.0 ) )
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= ( highpart & 1 ) << 31;
		highpart >>= 1;
	}

	pfreq = 1.0 / ( double ) lowpart;

	Sys_CheckOSVersion();
#endif

	Sys_InitFloatTime();
}

void Sys_Quit()
{
	giActive = DLL_CLOSE;
}

void Sys_SplitPath( const char* path, char* drive, char* dir, char* fname, char* ext )
{
	if( drive )
		*drive = '\0';

	if( dir )
		*dir = '\0';

	if( ext )
		*ext = '\0';

	auto pszNameStart = path;

	//On Windows, paths start with a drive letter and ':'
	if( *path && path[ 1 ] == ':' )
	{
		if( drive )
		{
			strncpy( drive, path, 2 );
			drive[ 2 ] = '\0';
		}

		pszNameStart = path + 2;
	}

	const char* pszDot = nullptr;
	auto pszEnd = pszNameStart;

	if( *pszNameStart )
	{
		const char* pszLastSlash = nullptr;

		while( *pszEnd )
		{
			if( *pszEnd == '\\' || *pszEnd == '/' )
			{
				pszLastSlash = pszEnd + 1;
			}
			else if( *pszEnd == '.' )
			{
				pszDot = pszEnd;
			}

			++pszEnd;
		}

		if( pszLastSlash )
		{
			if( dir )
			{
				auto iSize = min( MAX_SPLITPATH_BUF, pszLastSlash - pszNameStart );
				strncpy( dir, pszNameStart, iSize );
				dir[ iSize ] = '\0';
			}

			pszNameStart = pszLastSlash;
		}
	}

	if( pszDot && pszNameStart <= pszDot )
	{
		if( fname )
		{
			auto iSize = min( MAX_SPLITPATH_BUF, pszDot - pszNameStart );
			strncpy( fname, pszNameStart, iSize );
			fname[ iSize ] = '\0';
		}
		if( ext )
		{
			auto iSize = min( MAX_SPLITPATH_BUF, pszEnd - pszDot );
			strncpy( ext, pszDot, iSize );
			ext[ iSize ] = '\0';
		}
	}
	else
	{
		if( fname )
		{
			auto iSize = min( MAX_SPLITPATH_BUF, pszEnd - pszNameStart );
			strncpy( fname, pszNameStart, iSize );
			fname[ iSize ] = '\0';
		}
	}
}

void Sys_Sleep( int msec )
{
#ifdef WIN32
	Sleep( msec );
#else
	usleep( 1000 * msec );
#endif
}

void Sys_PageIn( void* ptr, int size )
{
	//Obsolete due to vastly increased available memory & I/O speeds
}

void Sys_MakeCodeWriteable( unsigned long startaddr, unsigned long length )
{
#ifdef WIN32
	DWORD  flOldProtect;

	if( !VirtualProtect( ( LPVOID ) startaddr, length, PAGE_READWRITE, &flOldProtect ) )
		Sys_Error( "Protection change failed\n" );
#endif
}

void AlertMessage( ALERT_TYPE atype, const char* szFmt, ... )
{
	static char szOut[ 1024 ];

	va_list va;

	va_start( va, szFmt );

	if( atype != at_logged || svs.maxclients <= 1 )
	{
		if( developer.value != 0.0 )
		{
			if( at_notice <= atype && atype <= at_error )
			{
				switch( atype )
				{
				case at_aiconsole:
					if( developer.value < 2.0 )
					{
						va_end( va );
						return;
					}

				case at_console:
					szOut[ 0 ] = '\0';
					break;

				case at_warning:
					Q_strcpy( szOut, "WARNING:  " );
					break;

				case at_notice:
					Q_strcpy( szOut, "NOTE:  " );
					break;

				case at_error:
					Q_strcpy( szOut, "ERROR:  " );
					break;

				default:
					break;
				}
			}

			vsnprintf( &szOut[ Q_strlen( szOut ) ], ARRAYSIZE( szOut ) - Q_strlen( szOut ), szFmt, va );
			Con_Printf( "%s", szOut );
		}
	}
	else
	{
		vsnprintf( szOut, ARRAYSIZE( szOut ), szFmt, va );
		Log_Printf( "%s", szOut );
	}

	va_end( va );
}

void EngineFprintf()
{
	AlertMessage( at_console, "EngineFprintf:  Obsolete API\n" );
}

//Used to fix up function names for saved games saved on another platform
const char* ConvertNameToLocalPlatform( const char* pchInName )
{
	static char s_szNewName[ 512 ];

	char szTempName[ 512 ];

#ifdef WIN32
	if( strstr( pchInName, "@" ) )
		return pchInName;

	if( *pchInName != '_' || pchInName[ 1 ] != 'Z' )
	{
		return "unknown";
	}

	strncpy( szTempName, pchInName + 3, ARRAYSIZE( szTempName ) );
	szTempName[ ARRAYSIZE( szTempName ) - 1 ] = '\0';

	const char* pszEnd = &szTempName[ strlen( szTempName ) ];

	const auto iParamBytes = atoi( szTempName );

	auto pszParams = szTempName;

	while( pszParams < pszEnd && isdigit( *pszParams ) )
	{
		++pszParams;
	}

	auto pszParamEnd = &pszParams[ iParamBytes ];

	const auto iNameBytes = atoi( pszParamEnd );

	*pszParamEnd = '\0';

	auto pszName = pszParamEnd;

	while( pszName < pszEnd && isdigit( *pszName ) )
	{
		++pszName;
	}

	snprintf( s_szNewName, ARRAYSIZE( s_szNewName ), "%s@%s", pszName, pszParams );

	return s_szNewName;
#else
	if( *pchInName == '_' && pchInName[ 1 ] == 'Z' )
		return pchInName;

	if( strchr( pchInName, '@' ) )
	{
		strncpy( szTempName, pchInName, ARRAYSIZE( szTempName ) );
		szTempName[ ARRAYSIZE( szTempName ) - 1 ] = '\0';

		auto pszAt = strchr( szTempName, '@' );
		auto pszParams = pszAt + 1;
		*pszAt = '0';

		//Think functions
		snprintf( s_szNewName, ARRAYSIZE( s_szNewName ), "_ZN%d%s%d%sEv", strlen( pszParams ), pszParams, strlen( szTempName ), szTempName );
		if( Sys_GetProcAddress( g_rgextdll[ 0 ].pDLLHandle, s_szNewName ) )
			return s_szNewName;

		//Touch/Blocked functions
		snprintf( s_szNewName, ARRAYSIZE( s_szNewName ), "_ZN%d%s%d%sEP11CBaseEntity", strlen( pszParams ), pszParams, strlen( szTempName ), szTempName );
		if( Sys_GetProcAddress( g_rgextdll[ 0 ].pDLLHandle, s_szNewName ) )
			return s_szNewName;

		//Use functions
		snprintf( s_szNewName, ARRAYSIZE( s_szNewName ), "_ZN%d%s%d%sEP11CBaseEntityS1_8USE_TYPEf", strlen( pszParams ), pszParams, strlen( szTempName ), szTempName );
		if( Sys_GetProcAddress( g_rgextdll[ 0 ].pDLLHandle, s_szNewName ) )
			return s_szNewName;
	}

	return "unknown";
#endif
}

const char* FindAddressInTable( extensiondll_t* pDll, uint32 function )
{
#ifdef WIN32
	for( int i = 0; i < pDll->functionCount; ++i )
	{
		if( pDll->functionTable[ i ].pFunction == function )
			return pDll->functionTable[ i ].pFunctionName;
	}
#else
	Dl_info addrInfo;

	if( dladdr( function, &addrInfo ) )
		return addrInfo.dli_sname;
#endif

	return nullptr;
}

uint32 FindNameInTable( extensiondll_t* pDll, const char* pName )
{
#ifdef WIN32
	for( int i = 0; i < pDll->functionCount; ++i )
	{
		if( !Q_strcmp( pName, pDll->functionTable[ i ].pFunctionName ) )
			return pDll->functionTable[ i ].pFunction;
	}

	return 0;
#else
	return reinterpret_cast<uint32>( dlsym( pDll->pDLLHandle, pName ) );
#endif
}

const char* NameForFunction( uint32 function )
{
	const char* pszName = nullptr;

	for( int i = 0; i < g_iextdllMac; ++i )
	{
		pszName = FindAddressInTable( &g_rgextdll[ i ], function );

		if( pszName )
			return pszName;
	}

	Con_Printf( "Can't find address: %08lx\n", function );

	return nullptr;
}

uint32 FunctionFromName( const char* pName )
{
	const char* pszName = ConvertNameToLocalPlatform( pName );

	uint32 function = 0;

	for( int i = 0; i < g_iextdllMac; ++i )
	{
		function = FindNameInTable( &g_rgextdll[ i ], pszName );

		if( function )
			return function;
	}

	Con_Printf( "Can't find proc: %s\n", pszName );

	return 0;
}

//TODO: check if any of these need to be called somewhere - Solokiller
void IN_ActivateMouse()
{
	ClientDLL_ActivateMouse();
}

void IN_DeactivateMouse()
{
	ClientDLL_DeactivateMouse();
}

void IN_ClearStates()
{
	ClientDLL_ClearStates();
}

void IN_MouseEvent( int mstate )
{
	ClientDLL_MouseEvent( mstate );
}

qboolean Voice_GetClientListening( int iReceiver, int iSender )
{
	const int iSenderIdx = iSender - 1;
	const int iReceiverIdx = iReceiver - 1;

	if( 0 <= iReceiverIdx && iReceiverIdx < svs.maxclients &&
		0 <= iSenderIdx && iSenderIdx < svs.maxclients )
	{
		return ( ( 1 << iReceiverIdx ) & ( svs.clients[ iSenderIdx ].m_VoiceStreams[ iReceiverIdx / MAX_CLIENTS ] ) ) != 0;
	}

	return false;
}

qboolean Voice_SetClientListening( int iReceiver, int iSender, qboolean bListen )
{
	const int iSenderIdx = iSender - 1;
	const int iReceiverIdx = iReceiver - 1;

	if( 0 <= iReceiverIdx && iReceiverIdx < svs.maxclients &&
		0 <= iSenderIdx && iSenderIdx < svs.maxclients )
	{
		auto pStreams = svs.clients[ iSenderIdx ].m_VoiceStreams;

		const auto stream = iReceiverIdx / MAX_CLIENTS;

		const auto bit = 1 << iReceiverIdx;

		if( bListen )
		{
			pStreams[ stream ] |= bit;
		}
		else
		{
			pStreams[ stream ] &= ~bit;
		}

		return true;
	}

	return false;
}
