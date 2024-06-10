//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef VGUI_KEY_TRANSLATION_H
#define VGUI_KEY_TRANSLATION_H
#ifdef _WIN32
#pragma once
#endif

#include <tier0/platform.h>

#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>

// Convert from Windows scan codes to VGUI key codes.
vgui2::KeyCode KeyCode_VirtualKeyToVGUI( int key );
int			  KeyCode_VGUIToVirtualKey( vgui2::KeyCode keycode );

int32 MouseCode_VGUIToVirtualKey( vgui2::MouseCode keyCode );
vgui2::MouseCode MouseCode_VirtualKeyToVGUI( int vk_code );

bool IsKeyCode( int vk_code );
bool IsMouseCode( int vk_code );

#endif // VGUI_KEY_TRANSLATION_H
