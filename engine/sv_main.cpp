#include "quakedef.h"

#include "client.h"
#include "modinfo.h"
#include "pr_edict.h"
#include "sv_main.h"
#include "sv_phys.h"
#include "server.h"

server_static_t svs;
server_t sv;

//TODO: implement functions and add them - Solokiller
playermove_t g_svmove;
globalvars_t gGlobalVariables = {};

bool allow_cheats = false;

cvar_t sv_allow_upload = { "sv_allowupload", "1", FCVAR_SERVER };
cvar_t mapcyclefile = { "mapcyclefile", "mapcycle.txt" };
cvar_t servercfgfile = { "servercfgfile", "server.cfg" };
cvar_t max_queries_sec = { "max_queries_sec", "3.0", FCVAR_PROTECTED | FCVAR_SERVER };
cvar_t max_queries_sec_global = { "max_queries_sec_global", "30", FCVAR_PROTECTED | FCVAR_SERVER };
cvar_t max_queries_window = { "max_queries_window", "60", FCVAR_PROTECTED | FCVAR_SERVER };

int SV_UPDATE_BACKUP = 1 << 3;
int SV_UPDATE_MASK = SV_UPDATE_BACKUP - 1;

struct GameToAppIDMapItem_t
{
	AppId_t iAppID;
	const char *pGameDir;
};

const AppId_t HALF_LIFE_APPID = 70;

const GameToAppIDMapItem_t g_GameToAppIDMap[] =
{
	{ 10, "cstrike" },
	{ 20, "tfc" },
	{ 30, "dod" },
	{ 40, "dmc" },
	{ 50, "gearbox" },
	{ 60, "ricochet" },
	{ HALF_LIFE_APPID, "valve" },
	{ 80, "czero" },
	{ 100, "czeror" },
	{ 130, "bshift" },
	{ 150, "cstrike_beta" }
};

modinfo_t gmodinfo = {};

AppId_t GetGameAppID()
{
	char gd[ FILENAME_MAX ];
	char arg[ FILENAME_MAX ];

	COM_ParseDirectoryFromCmd( "-game", arg, "valve" );
	COM_FileBase( arg, gd );

	for( const auto& data : g_GameToAppIDMap )
	{
		if( !stricmp( data.pGameDir, gd ) )
		{
			return data.iAppID;
		}
	}

	return HALF_LIFE_APPID;
}

bool IsGameSubscribed( const char *game )
{
	AppId_t appId = HALF_LIFE_APPID;

	for( const auto& data : g_GameToAppIDMap )
	{
		if( !stricmp( data.pGameDir, game ) )
		{
			appId = data.iAppID;
		}
	}

	return ISteamApps_BIsSubscribedApp( appId );
}

bool BIsValveGame()
{
	for( const auto& data : g_GameToAppIDMap )
	{
		if( !stricmp( data.pGameDir, com_gamedir ) )
		{
			return false;
		}
	}

	return true;
}

static bool g_bCS_CZ_Flags_Initialized = false;

bool g_bIsCStrike = false;
bool g_bIsCZero = false;
bool g_bIsCZeroRitual = false;
bool g_bIsTerrorStrike = false;
bool g_bIsTFC = false;

void SetCStrikeFlags()
{
	if( !g_bCS_CZ_Flags_Initialized )
	{
		if( !stricmp( com_gamedir, "cstrike" ) || 
			!stricmp( com_gamedir, "cstrike_beta" ) )
		{
			g_bIsCStrike = true;
		}
		else if( !stricmp( com_gamedir, "czero" ) )
		{
			g_bIsCZero = true;
		}
		else if( !stricmp( com_gamedir, "czeror" ) )
		{
			g_bIsCZeroRitual = true;
		}
		else if( !stricmp( com_gamedir, "terror" ) )
		{
			g_bIsTerrorStrike = true;
		}
		else if( !stricmp( com_gamedir, "tfc" ) )
		{
			g_bIsTFC = true;
		}

		g_bCS_CZ_Flags_Initialized = true;
	}
}

void SV_DeallocateDynamicData()
{
	//TODO: implement - Solokiller
}

void SV_AllocClientFrames()
{
	for( int i = 0; i < svs.maxclientslimit; ++i )
	{
		if( svs.clients[ i ].frames )
			Con_DPrintf( "Allocating over frame pointer?\n" );

		svs.clients[ i ].frames = reinterpret_cast<client_frame_t*>( Mem_ZeroMalloc( sizeof( client_frame_t ) * SV_UPDATE_BACKUP ) );
	}
}

void SV_ClearFrames( client_frame_t** frames )
{
	if( *frames )
	{
		auto pFrame = *frames;

		for( int i = 0; i < SV_UPDATE_BACKUP; ++i, ++pFrame )
		{
			if( pFrame->entities.entities )
			{
				Mem_Free( pFrame->entities.entities );
			}

			pFrame->entities.entities = nullptr;
			pFrame->entities.num_entities = 0;

			pFrame->senttime = 0;
			pFrame->ping_time = -1;
		}

		Mem_Free( *frames );
		*frames = nullptr;
	}
}

void SV_ResetModInfo()
{
	Q_memset( &gmodinfo, 0, sizeof( gmodinfo ) );

	gmodinfo.version = 1;
	gmodinfo.svonly = true;
	gmodinfo.num_edicts = MAX_EDICTS;

	char szDllListFile[ FILENAME_MAX ];
	snprintf( szDllListFile, sizeof( szDllListFile ), "%s", "liblist.gam" );

	FileHandle_t hLibListFile = FS_Open( szDllListFile, "rb" );

	if( hLibListFile != FILESYSTEM_INVALID_HANDLE )
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

			Q_strncpy( szKey, com_token, sizeof( szKey ) - 1 );
			szKey[ sizeof( szKey ) - 1 ] = '\0';

			pBuffer = COM_Parse( pBuffer );

			Q_strncpy( szValue, com_token, sizeof( szValue ) - 1 );
			szValue[ sizeof( szValue ) - 1 ] = '\0';

			if( Q_stricmp( szKey, "gamedll_linux" ) )
				DLL_SetModKey( &gmodinfo, szKey, szValue );
		}

		com_ignorecolons = false;
		Mem_Free( pFileData );
		FS_Close( hLibListFile );
	}
}

void SV_ServerShutdown()
{
	//TODO: implement - Solokiller
}

void SV_Init()
{
	//TODO: implement - Solokiller
	Cvar_RegisterVariable( &sv_gravity );
	//TODO: implement - Solokiller
	Cvar_RegisterVariable( &sv_allow_upload );
	//TODO: implement - Solokiller
	Cvar_RegisterVariable( &mapcyclefile );
	//TODO: implement - Solokiller
	Cvar_RegisterVariable( &servercfgfile );
	//TODO: implement - Solokiller
	Cvar_RegisterVariable( &max_queries_sec );
	Cvar_RegisterVariable( &max_queries_sec_global );
	Cvar_RegisterVariable( &max_queries_window );
	//TODO: implement - Solokiller
}

void SV_Shutdown()
{
	//TODO: implement - Solokiller
}

void SV_SetMaxclients()
{
	for( int i = 0; i < svs.maxclientslimit; ++i )
	{
		SV_ClearFrames( &svs.clients[ i ].frames );
	}

	svs.maxclients = 1;

	const int iCmdMaxPlayers = COM_CheckParm( "-maxplayers" );

	if( iCmdMaxPlayers )
	{
		svs.maxclients = Q_atoi( com_argv[ iCmdMaxPlayers + 1 ] );
	}

	cls.state = g_bIsDedicatedServer ? ca_dedicated : ca_disconnected;

	if( svs.maxclients <= 0 )
	{
		svs.maxclients = MP_MIN_CLIENTS;
	}
	else if( svs.maxclients > MAX_CLIENTS )
	{
		svs.maxclients = MAX_CLIENTS;
	}

	svs.maxclientslimit = MAX_CLIENTS;

	//If we're a listen server and we're low on memory, reduce maximum player limit.
	if( !g_bIsDedicatedServer && host_parms.memsize < LISTENSERVER_SAFE_MINIMUM_MEMORY )
		svs.maxclientslimit = 4;

	//Increase the number of updates available for multiplayer.
	SV_UPDATE_BACKUP = 8;

	if( svs.maxclients != 1 )
		SV_UPDATE_BACKUP = 64;

	SV_UPDATE_MASK = SV_UPDATE_BACKUP - 1;

	svs.clients = reinterpret_cast<client_t*>( Hunk_AllocName( sizeof( client_t ) * svs.maxclientslimit, "clients" ) );

	for( int i = 0; i < svs.maxclientslimit; ++i )
	{
		auto& client = svs.clients[ i ];

		Q_memset( &client, 0, sizeof( client ) );
		client.resourcesneeded.pPrev = &client.resourcesneeded;
		client.resourcesneeded.pNext = &client.resourcesneeded;
		client.resourcesonhand.pPrev = &client.resourcesonhand;
		client.resourcesonhand.pNext = &client.resourcesonhand;
	}

	Cvar_SetValue( "deathmatch", svs.maxclients > 1 ? 1 : 0 );

	SV_AllocClientFrames();

	int maxclients = svs.maxclientslimit;

	if( maxclients > svs.maxclients )
		maxclients = svs.maxclients;

	svs.maxclients = maxclients;
}

void SV_CountPlayers( int* clients )
{
	*clients = 0;

	for( int i = 0; i < svs.maxclients; ++i )
	{
		auto& client = svs.clients[ i ];

		if( client.active || client.spawned || client.connected )
		{
			++*clients;
		}
	}
}

void SV_KickPlayer( int nPlayerSlot, int nReason )
{
	//TODO: implement - Solokiller
}

void SV_ClearEntities()
{
	for( int i = 0; i < sv.num_edicts; ++i )
	{
		if( !sv.edicts[ i ].free )
		{
			//TODO: need to increment the serial number so EHANDLE works properly - Solokiller
			ReleaseEntityDLLFields( &sv.edicts[ i ] );
		}
	}
}

void SV_ClearCaches()
{
	for( int i = 1; i < ARRAYSIZE( sv.event_precache ) && sv.event_precache[ i ].filename; ++i )
	{
		auto& event = sv.event_precache[ i ];

		event.filename = nullptr;

		if( event.pszScript )
			Mem_Free( const_cast<char*>( event.pszScript ) );

		event.pszScript = nullptr;
	}
}

void SV_PropagateCustomizations()
{
	//TODO: implement - Solokiller
}
