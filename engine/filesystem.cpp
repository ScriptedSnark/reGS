#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "steam/steam_api.h"

#include "quakedef.h"

#include "client.h"
#include "FilePaths.h"
#include "IRegistry.h"
#include "sv_main.h"

static CSysModule* g_pFileSystemModule = nullptr;
static CreateInterfaceFn g_FileSystemFactory = nullptr;

static char s_pBaseDir[ 512 ] = {};

IFileSystem* g_pFileSystem = nullptr;

bool bLowViolenceBuild = false;

CUtlVector<char*> g_fallbackLocalizationFiles;

CreateInterfaceFn GetFileSystemFactory()
{
	return g_FileSystemFactory;
}

const char* GetBaseDirectory()
{
	return s_pBaseDir;
}

bool FileSystem_Init( char *basedir, void *voidfilesystemFactory )
{
	g_FileSystemFactory = reinterpret_cast<CreateInterfaceFn>( voidfilesystemFactory );

	strcpy( s_pBaseDir, basedir );
	host_parms.basedir = s_pBaseDir;

	if( !g_FileSystemFactory )
	{
		g_pFileSystemModule = Sys_LoadModule( filepath::FILESYSTEM_STDIO );

		if( !g_pFileSystemModule )
			return false;

		g_FileSystemFactory = Sys_GetFactory( g_pFileSystemModule );

		if( !g_FileSystemFactory )
			return false;
	}

	g_pFileSystem = static_cast<IFileSystem*>( g_FileSystemFactory( FILESYSTEM_INTERFACE_VERSION, nullptr ) );

	if( !g_pFileSystem )
	{
		return false;
	}

	return COM_SetupDirectories();
}

void FileSystem_Shutdown()
{
	g_pFileSystem->RemoveAllSearchPaths();

	if( g_pFileSystemModule )
	{
		Sys_UnloadModule( g_pFileSystemModule );
		g_pFileSystemModule = nullptr;
	}
}

bool BEnableAddonsFolder()
{
	if( COM_CheckParm( "-addons" ) )
		return true;

	return registry->ReadInt( "addons_folder", 0 ) > 0;
}

bool BEnabledHDAddon()
{
	if( COM_CheckParm( "-nohdmodels" ) )
		return false;

	return registry->ReadInt( "hdmodels", 1 ) > 0;
}

#define FS_FILENAME_MAX 128

void FileSystem_AddDirectory( bool bReadOnly, const char* pszPathID, const char* pszFormat, ... )
{
	char temp[ FS_FILENAME_MAX ];

	va_list list;

	va_start( list, pszFormat );
	vsnprintf( temp, sizeof( temp ) - 1, pszFormat, list );
	va_end( list );

	temp[ sizeof( temp ) - 1 ] = '\0';
	COM_FixSlashes( temp );

	if( bReadOnly )
		g_pFileSystem->AddSearchPathNoWrite( temp, pszPathID );
	else
		g_pFileSystem->AddSearchPath( temp, pszPathID );
}

void CheckLiblistForFallbackDir( const char *pGameDir, bool bLanguage, const char *pLanguage, bool bLowViolenceBuild )
{
	char szTemp[ 512 ];

	snprintf( szTemp, sizeof( szTemp ) - 1, "%s/liblist.gam", pGameDir );
	COM_FixSlashes( szTemp );
	g_pFileSystem->GetLocalCopy( szTemp );

	FileHandle_t hFile;

	if( !stricmp( com_gamedir, pGameDir ) )
	{
		hFile = FS_Open( "liblist.gam", "rt" );
	}
	else
	{
		snprintf( szTemp, sizeof( szTemp ) - 1, "../%s/liblist.gam", pGameDir );
		COM_FixSlashes( szTemp );
		hFile = FS_Open( szTemp, "rt" );
	}

	if( hFile == FILESYSTEM_INVALID_HANDLE )
		return;

	char szLine[ 512 ];

	const char* pszDir;

	size_t uiLength;

	bool bFound = false;

	while( 1 )
	{
		if( FS_EndOfFile( hFile ) )
			break;

		szLine[ 0 ] = '\0';
		FS_ReadLine( szLine, sizeof( szLine ) - 1, hFile );
		szLine[ sizeof( szLine ) - 1 ] = '\0';

		if( !strnicmp( szLine, "fallback_dir", 12 ) )
		{
			const char* pszStartQuote = strchr( szLine, '"' );

			if( !pszStartQuote )
				break;

			pszDir = pszStartQuote + 1;

			const char* pszEndQuote = strchr( pszStartQuote + 1, '"' );

			if( !pszEndQuote )
				break;

			uiLength = pszEndQuote - pszStartQuote - 1;

			if( uiLength >= ( FS_FILENAME_MAX - 1 ) )
				break;

			if( uiLength )
			{
				bFound = true;
				break;
			}
		}
	}

	if( bFound )
	{
		char szFallback[ FS_FILENAME_MAX ];
		strncpy( szFallback, pszDir, uiLength );
		szFallback[ uiLength ] = '\0';

		if( stricmp( pGameDir, szFallback ) )
		{
			if( bLowViolenceBuild )
			{
				FileSystem_AddDirectory( true, "GAME_FALLBACK", "%s/%s_lv", s_pBaseDir, szFallback );
			}

			if( BEnableAddonsFolder() )
			{
				FileSystem_AddDirectory( true, "GAME_FALLBACK", "%s/%s_addon", s_pBaseDir, szFallback );
			}

			if( bLanguage )
			{
				if( pLanguage )
				{
					FileSystem_AddDirectory( false, "GAME_FALLBACK", "%s/%s_%s", s_pBaseDir, szFallback, pLanguage );
					if( !COM_CheckParm( "-steam" ) )
					{
						char baseDir[ 4096 ];
						Q_strncpy( baseDir, s_pBaseDir, sizeof( baseDir ) );
						baseDir[ sizeof( baseDir ) - 1 ] = '\0';

						char* pszGamePath = Q_strstr( baseDir, "\\game" );

						if( pszGamePath )
						{
							*pszGamePath = '\0';
							FileSystem_AddDirectory( false, "GAME_FALLBACK", "%s\\localization\\%s_%s", baseDir, szFallback, pLanguage );
						}
					}
				}
			}

			if( BEnabledHDAddon() )
			{
				FileSystem_AddDirectory( true, "GAME_FALLBACK", "%s/%s_hd", s_pBaseDir, szFallback );
			}

			FileSystem_AddDirectory( false, "GAME_FALLBACK", "%s/%s", s_pBaseDir, szFallback );

			if( strcmp( "valve", szFallback ) )
			{
				char* pszLangFilename = new char[ FS_FILENAME_MAX ];
				snprintf( pszLangFilename, FS_FILENAME_MAX - 1, "Resource/%s_%%language%%.txt", szFallback );
				pszLangFilename[ FS_FILENAME_MAX - 1 ] = '\0';
				g_fallbackLocalizationFiles.AddToTail( pszLangFilename );
				//Recursively include fallback directories for the games that are being used as a fallback.
				//TODO: can result in infinite recursion.
				CheckLiblistForFallbackDir( szFallback, bLanguage, pLanguage, bLowViolenceBuild );
			}
		}
	}

	FS_Close( hFile );
}

bool FileSystem_AddFallbackGameDir( const char *pGameDir )
{
	char language[ FS_FILENAME_MAX ];

	language[ 0 ] = '\0';

	{
		const char* pszCurrentLanguage;

		if( SteamApps() && ( ( pszCurrentLanguage = SteamApps()->GetCurrentGameLanguage() ) != nullptr ) )
		{
			strncpy( language, pszCurrentLanguage, sizeof( language ) );
		}
		else
		{
			strncpy( language, "english", sizeof( language ) );
		}
	}

	if( language[ 0 ] && stricmp( language, "english" ) )
	{
		char temp[ 4096 ];
		snprintf( temp, sizeof( temp ), "%s/%s_%s", s_pBaseDir, pGameDir, language );
		g_pFileSystem->AddSearchPath( temp, "GAME" );
	}

	g_pFileSystem->AddSearchPath( pGameDir, "GAME" );

	return true;
}

bool FileSystem_SetGameDirectory( const char *pDefaultDir, const char *pGameDir )
{
	g_pFileSystem->RemoveAllSearchPaths();

	char language[ FS_FILENAME_MAX ];
	language[ 0 ] = '\0';

	if( !bLowViolenceBuild && SteamApps() && GetGameAppID() == 70 )
	{
		bLowViolenceBuild = SteamApps()->BIsLowViolence();
	}

	{
		const char* pszCurrentLanguage;

		if( SteamApps() && ( ( pszCurrentLanguage = SteamApps()->GetCurrentGameLanguage() ) != 0 ) )
		{
			strncpy( language, pszCurrentLanguage, sizeof( language ) );
		}
		else
		{
			strncpy( language, "english", sizeof( language ) );
		}
	}

	if( !g_bIsDedicatedServer && !IsGameSubscribed( pGameDir ) )
		return false;

	SteamAPI_SetBreakpadAppID( GetGameAppID() );

	const bool bLanguage = language[ 0 ] && stricmp( language, "english" ) != 0;

	char baseDir[ 4096 ];

	//Mod directories, if specified.
	if( pGameDir || pDefaultDir )
	{
		//Mod dir can be null if this is the default directory ("valve" in GoldSource)
		const char* const pszModDir = pGameDir ? pGameDir : pDefaultDir;

		if( bLowViolenceBuild )
		{
			FileSystem_AddDirectory( true, "GAME", "%s/%s_lv", s_pBaseDir, pszModDir );
		}

		if( BEnableAddonsFolder() )
		{
			FileSystem_AddDirectory( true, "GAME", "%s/%s_addon", s_pBaseDir, pszModDir );
		}

		if( bLanguage )
		{
			FileSystem_AddDirectory( true, "GAME", "%s/%s_%s", s_pBaseDir, pszModDir, language );

			if( !COM_CheckParm( "-steam" ) )
			{
				Q_strncpy( baseDir, s_pBaseDir, sizeof( baseDir ) );
				baseDir[ sizeof( baseDir ) - 1 ] = '\0';

				char* pszGamePath = Q_strstr( baseDir, "\\game" );

				if( pszGamePath )
				{
					*pszGamePath = '\0';
					FileSystem_AddDirectory( true, "GAME", "%s\\localization\\%s_%s", baseDir, pszModDir, language );
				}
			}
		}
		if( BEnabledHDAddon() )
		{
			FileSystem_AddDirectory( true, "GAME", "%s/%s_hd", s_pBaseDir, pszModDir );
		}

		FileSystem_AddDirectory( false, "GAME", "%s/%s", s_pBaseDir, pszModDir );
		FileSystem_AddDirectory( false, "GAMECONFIG", "%s/%s", s_pBaseDir, pszModDir );
		FileSystem_AddDirectory( false, "GAMEDOWNLOAD", "%s/%s_downloads", s_pBaseDir, pszModDir );

		CheckLiblistForFallbackDir( pszModDir, bLanguage, language, bLowViolenceBuild );
	}

	//Default directories
	if( bLanguage )
	{
		if( bLowViolenceBuild )
		{
			FileSystem_AddDirectory( true, "DEFAULTGAME", "%s/%s_lv", s_pBaseDir, pDefaultDir );
		}

		if( COM_CheckParm( "-addons" ) || registry->ReadInt( "addons_folder", 0 ) > 0 )
		{
			FileSystem_AddDirectory( true, "DEFAULTGAME", "%s/%s_addon", s_pBaseDir, pDefaultDir );
		}

		FileSystem_AddDirectory( true, "DEFAULTGAME", "%s/%s_%s", s_pBaseDir, pDefaultDir, language );

		if( !COM_CheckParm( "-steam" ) )
		{
			Q_strncpy( baseDir, s_pBaseDir, sizeof( baseDir ) );
			baseDir[ sizeof( baseDir ) - 1 ] = '\0';

			char* pszGamePath = Q_strstr( baseDir, "\\game" );

			if( pszGamePath )
			{
				*pszGamePath = '\0';
				FileSystem_AddDirectory( true, "DEFAULTGAME", "%s\\localization\\%s_%s", baseDir, pDefaultDir, language );
			}
		}
	}

	if( BEnabledHDAddon() )
	{
		FileSystem_AddDirectory( true, "DEFAULTGAME", "%s/%s_hd", s_pBaseDir, pDefaultDir );
	}

	FileSystem_AddDirectory( false, "BASE", "%s", s_pBaseDir );
	FileSystem_AddDirectory( true, "DEFAULTGAME", "%s/%s", s_pBaseDir, pDefaultDir );
	FileSystem_AddDirectory( false, "PLATFORM", "%s/platform", s_pBaseDir );
	return true;
}

int Host_GetVideoLevel()
{
	return registry->ReadInt( "vid_level", 0 );
}

void Host_SetAddonsFolder_f()
{
	if( cls.state != ca_dedicated && Cmd_Argc() == 2 )
	{
		const auto bOldState = registry->ReadInt( "addons_folder", 0 ) > 0;

		registry->WriteInt( "addons_folder", !stricmp( Cmd_Argv( 1 ), "1" ) ? 1 : 0 );

		const auto bNewState =
			COM_CheckParm( "-addons" ) ||
			( registry->ReadInt( "addons_folder", 0 ) > 0 );

		if( bOldState != bNewState )
			COM_SetupDirectories();
	}
}

void Host_SetHDModels_f()
{
	if( cls.state != ca_dedicated && Cmd_Argc() == 2 )
	{
		const bool bOldState = registry->ReadInt( "hdmodels", 1 ) > 0;

		registry->WriteInt( "hdmodels", !stricmp( Cmd_Argv( 1 ), "1" ) ? 1 : 0 );

		const bool bNewState =
			!COM_CheckParm( "-nohdmodels" ) &&
			( registry->ReadInt( "hdmodels", 1 ) > 0 );

		if( bOldState != bNewState )
			COM_SetupDirectories();
	}
}

void Host_SetVideoLevel_f()
{
	if( cls.state != ca_dedicated && Cmd_Argc() == 2 )
	{
		registry->WriteInt( "vid_level", !stricmp( Cmd_Argv( 1 ), "1" ) ? 1 : 0 );
	}
}
