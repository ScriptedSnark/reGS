#include "quakedef.h"
#include "gl_screen.h"
#include "HUD.h"

void HudSizeUp()
{
	if( scr_viewsize.value < 120.0 )
	{
		float value = scr_viewsize.value + 10.0;
		Cvar_SetValue( "viewsize", value );
	}
}

void HudSizeDown()
{
	if( scr_viewsize.value > 30.0 )
	{
		float value = scr_viewsize.value - 10.0;
		Cvar_SetValue( "viewsize", value );
	}
}
