#ifndef COMMON_GAMEUI_IGAMECONSOLE_H
#define COMMON_GAMEUI_IGAMECONSOLE_H

#include "interface.h"

#include <vgui/VGUI2.h>

/**
*	GameUI console interface
*/
class IGameConsole : public IBaseInterface
{
public:
	virtual void Activate() = 0;
	virtual void Initialize() = 0;
	virtual void Hide() = 0;
	virtual void Clear() = 0;

	virtual bool IsConsoleVisible() = 0;

	virtual void Printf( const char* format, ... ) = 0;
	virtual void DPrintf( const char* format, ... ) = 0;

	virtual void SetParent( vgui2::VPANEL parent ) = 0;
};

#define GAMECONSOLE_INTERFACE_VERSION "GameConsole003"

#endif //COMMON_GAMEUI_IGAMECONSOLE_H
