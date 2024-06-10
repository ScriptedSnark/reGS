#ifndef ENGINE_SV_STEAM3_H
#define ENGINE_SV_STEAM3_H

#include "steam/steam_api.h"

/**
*	Base class for Steam3 systems.
*/
class CSteam3
{
public:
	CSteam3() = default;
	virtual ~CSteam3() {}

	virtual void Shutdown() = 0;

protected:
	bool m_bLoggedOn = false;
	bool m_bLogOnResult = false;
	HSteamPipe m_hSteamPipe = 0;
};

class CSteam3Client final : public CSteam3
{
public:
	CSteam3Client();

	void InitClient();
	void Shutdown() override;

	int InitiateGameConnection( void *pData, int cbMaxData, uint64 steamID, uint32 unIPServer, uint16 usPortServer, bool bSecure );

	void TerminateConnection( uint32 unIPServer, uint16 usPortServer );

	void RunFrame();

private:
	STEAM_CALLBACK( CSteam3Client, OnClientGameServerDeny, ClientGameServerDeny_t, m_CallbackClientGameServerDeny );
	STEAM_CALLBACK( CSteam3Client, OnGameServerChangeRequested, GameServerChangeRequested_t, m_CallbackGameServerChangeRequested );
	STEAM_CALLBACK( CSteam3Client, OnGameOverlayActivated, GameOverlayActivated_t, m_CallbackGameOverlayActivated );
};

void Steam_InitClient();
void Steam_ShutdownClient();

void Steam_ClientRunFrame();

bool ISteamApps_BIsSubscribedApp( AppId_t appid );

void Steam_SetCVar( const char* pchKey, const char* pchValue );

#endif //ENGINE_SV_STEAM3_H
