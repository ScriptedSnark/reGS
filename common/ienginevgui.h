#ifndef COMMON_IENGINEVGUI_H
#define COMMON_IENGINEVGUI_H

#include "interface.h"

#include <vgui/VGUI2.h>

#include "steam/steam2compat.h"

enum VGUIPANEL
{
	PANEL_ROOT = 0,
	PANEL_CLIENTDLL,
	PANEL_GAMEUIDLL
};

/**
*	Interface from the engine that exposes data and actions for VGUI
*/
class IEngineVGui : public IBaseInterface
{
public:
	virtual vgui2::VPANEL GetPanel( VGUIPANEL type ) = 0;
	virtual bool SteamRefreshLogin( const char* password, bool isSecure ) = 0;
	virtual bool SteamProcessCall( bool* finished, TSteamProgress* progress, TSteamError* steamError ) = 0;
};

#define ENGINEVGUI_INTERFACE_VERSION "VEngineVGui001"

#endif //COMMON_IENGINEVGUI_H
