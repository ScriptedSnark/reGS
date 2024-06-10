#ifndef ENGINE_CONSOLE_H
#define ENGINE_CONSOLE_H

#include "con_nprint.h"

#define CON_MAX_NOTIFY_STRING 80

enum redirect_t
{
	RD_NONE = 0,
	RD_CLIENT = 1,
	RD_PACKET = 2,
};

extern bool con_forcedup;	// because no entities to refresh
extern bool con_initialized;

extern cvar_t con_fastmode;
extern cvar_t con_notifytime;
extern cvar_t con_color;
extern cvar_t con_shifttoggleconsole;
extern cvar_t con_mono;

void Con_Shutdown();

void Con_Printf( const char* fmt, ... );
void Con_DPrintf( const char* fmt, ... );

void Con_ClearNotify();
void Con_CheckResize();

void Con_Init();

int Con_IsVisible();

void Con_ToggleConsole_f();

void Con_DrawNotify();

void Con_SafePrintf( const char* fmt, ... );

#endif //ENGINE_CONSOLE_H
