#include "quakedef.h"
#include "gl_screen.h"
#include "vid.h"
#include "HUD.h"

void HudSizeUp()
{
	float value;

	if (scr_viewsize.value < 120.0f)
	{
		value = scr_viewsize.value + 10.0f;
		Cvar_SetValue("viewsize", value);
	}
}

void HudSizeDown()
{
	float value;

	if (scr_viewsize.value > 30.0f)
	{
		value = scr_viewsize.value - 10.0f;
		Cvar_SetValue("viewsize", value);
	}
}

// UNUSED
void Sbar_Geiger()
{
}

void Sbar_Draw()
{
	// CAN'T BE IMPLEMENTED AT THIS MOMENT
}