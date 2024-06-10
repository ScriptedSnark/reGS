//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#if defined( WIN32 ) && !defined( _X360 )
#define NOMINMAX
#include <wtypes.h>
#include <winuser.h>
#include "xbox/xboxstubs.h"
#endif
#include "tier0/dbg.h"
#include "vgui_key_translation.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif
#ifdef POSIX
#define VK_RETURN -1
#endif

#include <SDL2/SDL.h>

#include "vgui/ISystem.h"
#include "vgui_controls/Controls.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

struct vgui_to_virtual_t
{
	vgui2::KeyCode vguiKeyCode;
	//Obsolete
	//int32 windowsKeyCode;
	int32 sdlKeyCode;
};

static const vgui_to_virtual_t keyMap[] = 
{
	{ vgui2::KEY_0, SDLK_0 },
	{ vgui2::KEY_1, SDLK_1 },
	{ vgui2::KEY_2, SDLK_2 },
	{ vgui2::KEY_3, SDLK_3 },
	{ vgui2::KEY_4, SDLK_4 },
	{ vgui2::KEY_5, SDLK_5 },
	{ vgui2::KEY_6, SDLK_6 },
	{ vgui2::KEY_7, SDLK_7 },
	{ vgui2::KEY_8, SDLK_8 },
	{ vgui2::KEY_9, SDLK_9 },

	{ vgui2::KEY_A, SDLK_a },
	{ vgui2::KEY_B, SDLK_b },
	{ vgui2::KEY_C, SDLK_c },
	{ vgui2::KEY_D, SDLK_d },
	{ vgui2::KEY_E, SDLK_e },
	{ vgui2::KEY_F, SDLK_f },
	{ vgui2::KEY_G, SDLK_g },
	{ vgui2::KEY_H, SDLK_h },
	{ vgui2::KEY_I, SDLK_i },
	{ vgui2::KEY_J, SDLK_j },
	{ vgui2::KEY_K, SDLK_k },
	{ vgui2::KEY_L, SDLK_l },
	{ vgui2::KEY_M, SDLK_m },
	{ vgui2::KEY_N, SDLK_n },
	{ vgui2::KEY_O, SDLK_o },
	{ vgui2::KEY_P, SDLK_p },
	{ vgui2::KEY_Q, SDLK_q },
	{ vgui2::KEY_R, SDLK_r },
	{ vgui2::KEY_S, SDLK_s },
	{ vgui2::KEY_T, SDLK_t },
	{ vgui2::KEY_U, SDLK_u },
	{ vgui2::KEY_V, SDLK_v },
	{ vgui2::KEY_W, SDLK_q },
	{ vgui2::KEY_X, SDLK_x },
	{ vgui2::KEY_Y, SDLK_y },
	{ vgui2::KEY_Z, SDLK_z },

	{ vgui2::KEY_PAD_0, SDLK_KP_0 },
	{ vgui2::KEY_PAD_1, SDLK_KP_1 },
	{ vgui2::KEY_PAD_2, SDLK_KP_2 },
	{ vgui2::KEY_PAD_3, SDLK_KP_3 },
	{ vgui2::KEY_PAD_4, SDLK_KP_4 },
	{ vgui2::KEY_PAD_5, SDLK_KP_5 },
	{ vgui2::KEY_PAD_6, SDLK_KP_6 },
	{ vgui2::KEY_PAD_7, SDLK_KP_7 },
	{ vgui2::KEY_PAD_8, SDLK_KP_8 },
	{ vgui2::KEY_PAD_9, SDLK_KP_9 },

	{ vgui2::KEY_PAD_DIVIDE, SDLK_KP_DIVIDE },
	{ vgui2::KEY_PAD_MULTIPLY, SDLK_KP_MULTIPLY },
	{ vgui2::KEY_PAD_MINUS, SDLK_KP_MINUS },
	{ vgui2::KEY_PAD_PLUS, SDLK_KP_PLUS },
	{ vgui2::KEY_PAD_ENTER, SDLK_KP_ENTER },
	{ vgui2::KEY_PAD_DECIMAL, SDLK_KP_DECIMAL },

	{ vgui2::KEY_LBRACKET, SDLK_LEFTBRACKET },
	{ vgui2::KEY_RBRACKET, SDLK_RIGHTBRACKET },
	{ vgui2::KEY_SEMICOLON, SDLK_SEMICOLON },
	{ vgui2::KEY_APOSTROPHE, SDLK_QUOTE },
	{ vgui2::KEY_BACKQUOTE, SDLK_BACKQUOTE },
	{ vgui2::KEY_COMMA, SDLK_COMMA },
	{ vgui2::KEY_PERIOD, SDLK_PERIOD },
	{ vgui2::KEY_SLASH, SDLK_SLASH },
	{ vgui2::KEY_BACKSLASH, SDLK_BACKSLASH },
	{ vgui2::KEY_MINUS, SDLK_MINUS },
	{ vgui2::KEY_EQUAL, SDLK_EQUALS },
	{ vgui2::KEY_ENTER, SDLK_RETURN },
	{ vgui2::KEY_SPACE, SDLK_SPACE },
	{ vgui2::KEY_BACKSPACE, SDLK_BACKSPACE },
	{ vgui2::KEY_TAB, SDLK_TAB },
	{ vgui2::KEY_CAPSLOCK, SDLK_CAPSLOCK },
	{ vgui2::KEY_NUMLOCK, SDLK_NUMLOCKCLEAR },
	{ vgui2::KEY_ESCAPE, SDLK_ESCAPE },
	{ vgui2::KEY_SCROLLLOCK, SDLK_SCROLLLOCK },
	{ vgui2::KEY_INSERT, SDLK_INSERT },
	{ vgui2::KEY_DELETE, SDLK_DELETE },
	{ vgui2::KEY_HOME, SDLK_HOME },
	{ vgui2::KEY_END, SDLK_END },
	{ vgui2::KEY_PAGEUP, SDLK_PAGEUP },
	{ vgui2::KEY_PAGEDOWN, SDLK_PAGEDOWN },
	{ vgui2::KEY_BREAK, SDLK_PAUSE },
	{ vgui2::KEY_RSHIFT, SDLK_RSHIFT },
	{ vgui2::KEY_LSHIFT, SDLK_LSHIFT },
	{ vgui2::KEY_RALT, SDLK_RALT },
	{ vgui2::KEY_LALT, SDLK_LALT },
	{ vgui2::KEY_RCONTROL, SDLK_RCTRL },
	{ vgui2::KEY_LCONTROL, SDLK_LCTRL },
	{ vgui2::KEY_LWIN, SDLK_LGUI },
	{ vgui2::KEY_RWIN, SDLK_RGUI },
	{ vgui2::KEY_APP, SDLK_APPLICATION },

	{ vgui2::KEY_UP, SDLK_UP },
	{ vgui2::KEY_LEFT, SDLK_LEFT },
	{ vgui2::KEY_DOWN, SDLK_DOWN },
	{ vgui2::KEY_RIGHT, SDLK_RIGHT },

	{ vgui2::KEY_F1, SDLK_F1 },
	{ vgui2::KEY_F2, SDLK_F2 },
	{ vgui2::KEY_F3, SDLK_F3 },
	{ vgui2::KEY_F4, SDLK_F4 },
	{ vgui2::KEY_F5, SDLK_F5 },
	{ vgui2::KEY_F6, SDLK_F6 },
	{ vgui2::KEY_F7, SDLK_F7 },
	{ vgui2::KEY_F8, SDLK_F8 },
	{ vgui2::KEY_F9, SDLK_F9 },
	{ vgui2::KEY_F10, SDLK_F10 },
	{ vgui2::KEY_F11, SDLK_F11 },
	{ vgui2::KEY_F12, SDLK_F12 },
};

static vgui2::KeyCode s_pVirtualKeyToVGUI[ 256 ];
static int s_pVGUIToVirtualKey[ 256 ];

namespace vgui2
{
void InitKeyTranslationTable()
{
	static bool bInitted = false;

	if( !bInitted )
	{
		bInitted = true;

		memset( s_pVirtualKeyToVGUI, 0, sizeof( s_pVirtualKeyToVGUI ) );

		for( int i = 0; i < ARRAYSIZE( keyMap ); ++i )
		{
			s_pVirtualKeyToVGUI[ keyMap[ i ].sdlKeyCode ] = keyMap[ i ].vguiKeyCode;
		}
	}
}

void InitVGUIToVKTranslationTable()
{
	static bool bInitted = false;

	if( !bInitted )
	{
		bInitted = true;

		memset( s_pVGUIToVirtualKey, 0, sizeof( s_pVGUIToVirtualKey ) );

		for( int i = 0; i < ARRAYSIZE( keyMap ); ++i )
		{
			s_pVGUIToVirtualKey[ keyMap[ i ].vguiKeyCode ] = keyMap[ i ].sdlKeyCode;
		}
	}
}
}

vgui2::KeyCode KeyCode_VirtualKeyToVGUI( int key )
{
	vgui2::InitKeyTranslationTable();

	if( key >= 0 && key <= 255 )
		return s_pVirtualKeyToVGUI[ key ];

	return vgui2::KEY_NONE;
}

int KeyCode_VGUIToVirtualKey( vgui2::KeyCode code )
{
	vgui2::InitVGUIToVKTranslationTable();

	if( code >= 0 && code <= 255 )
		return s_pVGUIToVirtualKey[ code ];

	return -1;
}

int32 MouseCode_VGUIToVirtualKey( vgui2::MouseCode keyCode )
{
	//Nothing
	return 0;
}

vgui2::MouseCode MouseCode_VirtualKeyToVGUI( int vk_code )
{
	//Nothing
	return vgui2::MOUSE_LAST;
}

bool IsKeyCode( int vk_code )
{
	return vk_code <= 255 && vk_code > 0;
}

bool IsMouseCode( int vk_code )
{
	return false;
}
