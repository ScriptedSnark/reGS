#include <cstdio>
#include <cstdlib>

#include "quakedef.h"

#include "steam/steam_gameserver.h"

static CSteam3Client s_Steam3Client;

CSteam3Client::CSteam3Client()
	: m_CallbackClientGameServerDeny( this, &CSteam3Client::OnClientGameServerDeny )
	, m_CallbackGameServerChangeRequested( this, &CSteam3Client::OnGameServerChangeRequested )
	, m_CallbackGameOverlayActivated( this, &CSteam3Client::OnGameOverlayActivated )
{
}

void CSteam3Client::InitClient()
{
	if( !m_bLoggedOn )
	{
		m_bLoggedOn = true;
		_unlink( "steam_appid.txt" );
		if( !getenv( "SteamAppId" ) )
		{
			AppId_t appId = GetGameAppID();
			if( appId > 0 )
			{
				FILE* pFile = fopen( "steam_appid.txt", "w+" );
				if( pFile )
				{
					fprintf( pFile, "%d\n", static_cast<int>( appId ) );
					fclose( pFile );
				}
			}
		}
		if( !SteamAPI_Init() )
			Sys_Error( "Failed to initalize authentication interface. Exiting...\n" );
		m_bLogOnResult = false;
	}
}

void CSteam3Client::Shutdown()
{
	if( m_bLoggedOn )
	{
		SteamAPI_Shutdown();
		m_bLoggedOn = false;
	}
}

int CSteam3Client::InitiateGameConnection( void *pData, int cbMaxData, 
										   uint64 steamID, 
										   uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
	//TODO: Linux version might require by swapping for IP & port. - Solokiller
	return SteamUser()->InitiateGameConnection( pData, cbMaxData, 
												steamID, 
												unIPServer, usPortServer, bSecure );
}

void CSteam3Client::TerminateConnection( uint32 unIPServer, uint16 usPortServer )
{
	//TODO: Linux version might require by swapping for IP & port. - Solokiller
	SteamUser()->TerminateGameConnection( unIPServer, usPortServer );
}

void CSteam3Client::RunFrame()
{
	SteamAPI_RunCallbacks();
}

void CSteam3Client::OnClientGameServerDeny( ClientGameServerDeny_t* pParam )
{
	//TODO: implement - Solokiller
	/*
	COM_ExplainDisconnection( true, "Invalid server version, unable to connect." );
	CL_Disconnect();
	*/
}

void CSteam3Client::OnGameServerChangeRequested( GameServerChangeRequested_t *pGameServerChangeRequested )
{
	//TODO: implement - Solokiller
	/*
	Cvar_DirectSet( &password, pGameServerChangeRequested->m_rgchPassword );
	Con_Printf( "Connecting to %s\n", pGameServerChangeRequested );
	Cbuf_AddText( va( "connect %s\n", pGameServerChangeRequested ) );
	*/
}

void CSteam3Client::OnGameOverlayActivated( GameOverlayActivated_t *pGameOverlayActivated )
{
	//TODO: implement - Solokiller
	/*
	if( Host_IsSinglePlayerGame() )
	{
		if( pGameOverlayActivated->m_bActive )
		{
			Cbuf_AddText( "setpause;" );
		}
		else if( !( unsigned __int8 ) ( *( int( __cdecl ** )( _DWORD ) )( *( _DWORD * ) &staticGameUIFuncs->dummy[ 0 ] + 48 ) )( staticGameUIFuncs ) )
		{
			Cbuf_AddText( "unpause;" );
		}
	}
	*/
}

void Steam_InitClient()
{
	s_Steam3Client.InitClient();
}

void Steam_ShutdownClient()
{
	s_Steam3Client.Shutdown();
}

void Steam_ClientRunFrame()
{
	s_Steam3Client.RunFrame();
}

bool ISteamApps_BIsSubscribedApp( AppId_t appid )
{
	if( !SteamApps() )
		return false;

	return SteamApps()->BIsSubscribedApp( appid );
}

void Steam_SetCVar( const char* pchKey, const char* pchValue )
{
	//TODO: implement - Solokiller
	/*
	if( s_Steam3Server )
	{
		SteamGameServer()->SetKeyValue( pchKey, pchValue );
	}
	*/
}
