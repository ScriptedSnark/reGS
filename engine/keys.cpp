/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "quakedef.h"
#include "cdll_int.h"
#include "dll_state.h"
#include "sound.h"
#include "vgui_int.h"

/*

key up events are sent even if in console mode

*/

//TODO: this code assumes 256 keys max all over the place. Cleanup - Solokiller
//TODO: define key lines - Solokiller

#define		MAXCMDLINE	256
char	key_lines[ 32 ][ MAXCMDLINE ];
int		key_linepos;
int		shift_down = false;

int		edit_line = 0;
int		history_line = 0;

keydest_t	key_dest;

char	*keybindings[ 256 ];
bool	consolekeys[ 256 ];	// if true, can't be rebound while in console
bool	menubound[ 256 ];	// if true, can't be rebound while in menu
int		keyshift[ 256 ];		// key to map to if shift held down in console
int		key_repeats[ 256 ];	// if > 1, it is autorepeating
bool	keydown[ 256 ];
bool	keyGameUIdown[ 256 ];

int toggleconsole_key = 0;

typedef struct
{
	const char* name;
	int keynum;
} keyname_t;

const keyname_t keynames[] =
{
	{ "TAB", K_TAB },
	{ "ENTER", K_ENTER },
	{ "ESCAPE", K_ESCAPE },
	{ "SPACE", K_SPACE },
	{ "BACKSPACE", K_BACKSPACE },
	{ "UPARROW", K_UPARROW },
	{ "DOWNARROW", K_DOWNARROW },
	{ "LEFTARROW", K_LEFTARROW },
	{ "RIGHTARROW", K_RIGHTARROW },

	{ "ALT", K_ALT },
	{ "CTRL", K_CTRL },
	{ "SHIFT", K_SHIFT },

	{ "F1", K_F1 },
	{ "F2", K_F2 },
	{ "F3", K_F3 },
	{ "F4", K_F4 },
	{ "F5", K_F5 },
	{ "F6", K_F6 },
	{ "F7", K_F7 },
	{ "F8", K_F8 },
	{ "F9", K_F9 },
	{ "F10", K_F10 },
	{ "F11", K_F11 },
	{ "F12", K_F12 },

	{ "INS", K_INS },
	{ "DEL", K_DEL },
	{ "PGDN", K_PGDN },
	{ "PGUP", K_PGUP },
	{ "HOME", K_HOME },
	{ "END", K_END },

	{ "MOUSE1", K_MOUSE1 },
	{ "MOUSE2", K_MOUSE2 },
	{ "MOUSE3", K_MOUSE3 },
	{ "MOUSE4", K_MOUSE4 },
	{ "MOUSE5", K_MOUSE5 },

	{ "JOY1", K_JOY1 },
	{ "JOY2", K_JOY2 },
	{ "JOY3", K_JOY3 },
	{ "JOY4", K_JOY4 },

	{ "AUX1", K_AUX1 },
	{ "AUX2", K_AUX2 },
	{ "AUX3", K_AUX3 },
	{ "AUX4", K_AUX4 },
	{ "AUX5", K_AUX5 },
	{ "AUX6", K_AUX6 },
	{ "AUX7", K_AUX7 },
	{ "AUX8", K_AUX8 },
	{ "AUX9", K_AUX9 },
	{ "AUX10", K_AUX10 },
	{ "AUX11", K_AUX11 },
	{ "AUX12", K_AUX12 },
	{ "AUX13", K_AUX13 },
	{ "AUX14", K_AUX14 },
	{ "AUX15", K_AUX15 },
	{ "AUX16", K_AUX16 },
	{ "AUX17", K_AUX17 },
	{ "AUX18", K_AUX18 },
	{ "AUX19", K_AUX19 },
	{ "AUX20", K_AUX20 },
	{ "AUX21", K_AUX21 },
	{ "AUX22", K_AUX22 },
	{ "AUX23", K_AUX23 },
	{ "AUX24", K_AUX24 },
	{ "AUX25", K_AUX25 },
	{ "AUX26", K_AUX26 },
	{ "AUX27", K_AUX27 },
	{ "AUX28", K_AUX28 },
	{ "AUX29", K_AUX29 },
	{ "AUX30", K_AUX30 },
	{ "AUX31", K_AUX31 },
	{ "AUX32", K_AUX32 },

	{ "KP_HOME", K_KP_HOME },
	{ "KP_UPARROW", K_KP_UPARROW },
	{ "KP_PGUP", K_KP_PGUP },
	{ "KP_LEFTARROW", K_KP_LEFTARROW },
	{ "KP_5", K_KP_5 },
	{ "KP_RIGHTARROW", K_KP_RIGHTARROW },
	{ "KP_END", K_KP_END },
	{ "KP_DOWNARROW", K_KP_DOWNARROW },
	{ "KP_PGDN", K_KP_PGDN },
	{ "KP_ENTER", K_KP_ENTER },
	{ "KP_INS", K_KP_INS },
	{ "KP_DEL", K_KP_DEL },
	{ "KP_SLASH", K_KP_SLASH },
	{ "KP_MINUS", K_KP_MINUS },
	{ "KP_PLUS", K_KP_PLUS },
	{ "CAPSLOCK", K_CAPSLOCK },
	{ "MWHEELUP", K_MWHEELUP },
	{ "MWHEELDOWN", K_MWHEELDOWN },

	{ "PAUSE", K_PAUSE },

	{ "SEMICOLON", ';' },	// because a raw semicolon seperates commands
	{ "KP_MUL", K_KP_MUL },
	{ "WIN", K_WIN },

	{ NULL,0 }
};

/*
==============================================================================

LINE TYPING INTO THE CONSOLE

==============================================================================
*/

void Key_Unbindall_f()
{
	for( int i = 0; i < 256; ++i )
	{
		if( keybindings[ i ] && i != K_ESCAPE )
		{
			Key_SetBinding( i, "" );
		}
	}

	Key_SetBinding( '~', "toggleconsole" );
	Key_SetBinding( '`', "toggleconsole" );
	Key_SetBinding( K_ESCAPE, "cancelselect" );
}

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f()
{
	const int c = Cmd_Argc();

	if( c != 2 && c != 3 )
	{
		Con_Printf( "bind <key> [command] : attach a command to a key\n" );
		return;
	}

	const int b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if( b == -1 )
	{
		Con_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	if( c == 2 )
	{
		if( keybindings[ b ] )
			Con_Printf( "\"%s\" = \"%s\"\n", Cmd_Argv( 1 ), keybindings[ b ] );
		else
			Con_Printf( "\"%s\" is not bound\n", Cmd_Argv( 1 ) );
		return;
	}

	char cmd[ 1024 ];

	// copy the rest of the command line
	cmd[ 0 ] = 0;		// start out with a null string

	if( b == K_ESCAPE )
	{
		Q_strcpy( cmd, "cancelselect" );
	}
	else
	{
		for( int i = 2; i < c; ++i )
		{
			if( i > 2 )
				Q_strncat( cmd, " ", ARRAYSIZE( cmd ) );
			Q_strncat( cmd, Cmd_Argv( i ), ARRAYSIZE( cmd ) );
		}
	}

	Key_SetBinding( b, cmd );
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f()
{
	if( Cmd_Argc() != 2 )
	{
		Con_Printf( "unbind <key> : remove commands from a key\n" );
		return;
	}

	const int b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if( b == -1 )
	{
		Con_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	if( b == K_ESCAPE )
	{
		Con_Printf( "Can't unbind ESCAPE key\n" );
		return;
	}

	Key_SetBinding( b, "" );
}

bool CheckForCommand()
{
	char command[ 128 ];

	int i;

	for( i = 0; i < ARRAYSIZE( command ) - 1; ++i )
	{
		if( key_lines[ edit_line + 1 ][ i ] <= ' ' )
			break;

		command[ i ] = key_lines[ edit_line + 1 ][ i ];
	}

	command[ i ] = '\0';

	const char* pszCompleted = Cmd_CompleteCommand( command, !shift_down );

	if( !pszCompleted )
		pszCompleted = Cvar_CompleteVariable( command, !shift_down );

	if( pszCompleted && !Q_strcmp( pszCompleted, command ) )
		return true;

	return false;
}

void CompleteCommand()
{
	char* pszBuffer = key_lines[ edit_line + 1 ];

	char c = *key_lines[ edit_line + 1 ];

	if( c == '/' || c == '\\' )
		++pszBuffer;

	const char* pszCompleted = Cmd_CompleteCommand( pszBuffer, !shift_down );

	if( !pszCompleted )
		pszCompleted = Cvar_CompleteVariable( pszBuffer, !shift_down );

	if( pszCompleted )
	{
		*key_lines[ edit_line + 1 ] = '/';
		snprintf( key_lines[ edit_line + 2 ], ARRAYSIZE( key_lines[ edit_line + 2 ] ) - 2, "%s", pszCompleted );
		key_linepos = Q_strlen( pszCompleted );

		key_lines[ edit_line ][ key_linepos + 2 ] = ' ';
		key_linepos += 3;
		key_lines[ edit_line ][ key_linepos ] = '\0';
	}
}

void Key_UpLine()
{
	while( 1 )
	{
		history_line = ( ( byte ) history_line - 1 ) % 32;

		if( history_line == edit_line )
		{
			history_line = ( ( byte ) history_line + 1 ) % 32;
			break;
		}

		if( *key_lines[ history_line + 1 ] )
			break;
	}

	snprintf( key_lines[ edit_line ], ARRAYSIZE( key_lines[ edit_line ] ), "%s", key_lines[ history_line ] );
	key_linepos = Q_strlen( key_lines[ edit_line ] );
}

/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console( int key )
{
	//Obsolete
}

//============================================================================

char chat_buffer[ MAX_CHAT_BUFFER ];

/**
*	Size of the string in chat_buffer, in bytes.
*/
int chat_bufferlen = 0;

/**
*	messagemode type.
*/
char message_type[ 32 ] = "say";

void Cbuf_ReplaceSemiColons()
{
	//Replace semicolons with commas,
	//because the cbuf tokenizer treats those as command delimiters.
	for( char* pszBuf = chat_buffer; *pszBuf; ++pszBuf )
	{
		if( *pszBuf == ';' )
		{
			*pszBuf = ',';
		}
	}
}

SDL_Keycode GetSDLKeycodeFromEngineKey( int iKey )
{
	SDL_Scancode code = SDL_SCANCODE_UNKNOWN;

	switch( iKey )
	{
	case K_TAB: code = SDL_SCANCODE_TAB; break;
	case K_ENTER: code = SDL_SCANCODE_RETURN; break;
	case K_ESCAPE: code = SDL_SCANCODE_ESCAPE; break;
	case K_SPACE: code = SDL_SCANCODE_SPACE; break;

	case '\'': code = SDL_SCANCODE_APOSTROPHE; break;
	case ',': code = SDL_SCANCODE_COMMA; break;
	case '-': code = SDL_SCANCODE_MINUS; break;
	case '.': code = SDL_SCANCODE_PERIOD; break;
	case '/': code = SDL_SCANCODE_SLASH; break;
	case '0': code = SDL_SCANCODE_0; break;
	case '1': code = SDL_SCANCODE_1; break;
	case '2': code = SDL_SCANCODE_2; break;
	case '3': code = SDL_SCANCODE_3; break;
	case '4': code = SDL_SCANCODE_4; break;
	case '5': code = SDL_SCANCODE_5; break;
	case '6': code = SDL_SCANCODE_6; break;
	case '7': code = SDL_SCANCODE_7; break;
	case '8': code = SDL_SCANCODE_8; break;
	case '9': code = SDL_SCANCODE_9; break;
	case ';': code = SDL_SCANCODE_SEMICOLON; break;
	case '=': code = SDL_SCANCODE_EQUALS; break;
	case '[': code = SDL_SCANCODE_LEFTBRACKET; break;
	case ']': code = SDL_SCANCODE_RIGHTBRACKET; break;
	case '\\': code = SDL_SCANCODE_BACKSLASH; break;
	case '^': code = SDL_SCANCODE_GRAVE; break;
	case 'a': code = SDL_SCANCODE_A; break;
	case 'b': code = SDL_SCANCODE_B; break;
	case 'c': code = SDL_SCANCODE_C; break;
	case 'd': code = SDL_SCANCODE_D; break;
	case 'e': code = SDL_SCANCODE_E; break;
	case 'f': code = SDL_SCANCODE_F; break;
	case 'g': code = SDL_SCANCODE_G; break;
	case 'h': code = SDL_SCANCODE_H; break;
	case 'i': code = SDL_SCANCODE_I; break;
	case 'j': code = SDL_SCANCODE_J; break;
	case 'k': code = SDL_SCANCODE_K; break;
	case 'l': code = SDL_SCANCODE_L; break;
	case 'm': code = SDL_SCANCODE_M; break;
	case 'n': code = SDL_SCANCODE_N; break;
	case 'o': code = SDL_SCANCODE_O; break;
	case 'p': code = SDL_SCANCODE_P; break;
	case 'q': code = SDL_SCANCODE_Q; break;
	case 'r': code = SDL_SCANCODE_R; break;
	case 's': code = SDL_SCANCODE_S; break;
	case 't': code = SDL_SCANCODE_T; break;
	case 'u': code = SDL_SCANCODE_U; break;
	case 'v': code = SDL_SCANCODE_V; break;
	case 'w': code = SDL_SCANCODE_W; break;
	case 'x': code = SDL_SCANCODE_X; break;
	case 'y': code = SDL_SCANCODE_Y; break;
	case 'z': code = SDL_SCANCODE_Z; break;

	case K_BACKSPACE: code = SDL_SCANCODE_BACKSPACE; break;
	case K_UPARROW: code = SDL_SCANCODE_UP; break;
	case K_DOWNARROW: code = SDL_SCANCODE_DOWN; break;
	case K_LEFTARROW: code = SDL_SCANCODE_LEFT; break;
	case K_RIGHTARROW: code = SDL_SCANCODE_RIGHT; break;

	case K_ALT: code = SDL_SCANCODE_LALT; break;
	case K_CTRL: code = SDL_SCANCODE_LCTRL; break;
	case K_SHIFT: code = SDL_SCANCODE_LSHIFT; break;

	case K_F1: code = SDL_SCANCODE_F1; break;
	case K_F2: code = SDL_SCANCODE_F2; break;
	case K_F3: code = SDL_SCANCODE_F3; break;
	case K_F4: code = SDL_SCANCODE_F4; break;
	case K_F5: code = SDL_SCANCODE_F5; break;
	case K_F6: code = SDL_SCANCODE_F6; break;
	case K_F7: code = SDL_SCANCODE_F7; break;
	case K_F8: code = SDL_SCANCODE_F8; break;
	case K_F9: code = SDL_SCANCODE_F9; break;
	case K_F10: code = SDL_SCANCODE_F10; break;
	case K_F11: code = SDL_SCANCODE_F11; break;
	case K_F12: code = SDL_SCANCODE_F12; break;

	case K_INS: code = SDL_SCANCODE_INSERT; break;
	case K_HOME: code = SDL_SCANCODE_HOME; break;
	case K_PGUP: code = SDL_SCANCODE_PAGEUP; break;
	case K_DEL: code = SDL_SCANCODE_DELETE; break;
	case K_END: code = SDL_SCANCODE_END; break;
	case K_PGDN: code = SDL_SCANCODE_PAGEDOWN; break;

	case K_KP_HOME: code = SDL_SCANCODE_KP_7; break;
	case K_KP_UPARROW: code = SDL_SCANCODE_KP_8; break;
	case K_KP_PGUP: code = SDL_SCANCODE_KP_9; break;
	case K_KP_LEFTARROW: code = SDL_SCANCODE_KP_4; break;
	case K_KP_5: code = SDL_SCANCODE_KP_5; break;
	case K_KP_RIGHTARROW: code = SDL_SCANCODE_KP_6; break;
	case K_KP_END: code = SDL_SCANCODE_KP_1; break;
	case K_KP_DOWNARROW: code = SDL_SCANCODE_KP_2; break;
	case K_KP_PGDN: code = SDL_SCANCODE_KP_3; break;
	case K_KP_ENTER: code = SDL_SCANCODE_KP_ENTER; break;
	case K_KP_INS: code = SDL_SCANCODE_KP_0; break;
	case K_KP_DEL: code = SDL_SCANCODE_KP_PERIOD; break;
	case K_KP_SLASH: code = SDL_SCANCODE_KP_DIVIDE; break;
	case K_KP_MINUS: code = SDL_SCANCODE_KP_MINUS; break;
	case K_KP_PLUS: code = SDL_SCANCODE_KP_PLUS; break;
	case K_CAPSLOCK: code = SDL_SCANCODE_CAPSLOCK; break;
	case K_KP_MUL: code = SDL_SCANCODE_KP_MULTIPLY; break;
	case K_WIN: code = SDL_SCANCODE_LGUI; break;

	case K_PAUSE: code = SDL_SCANCODE_PAUSE; break;

	default: break;
	}

	return SDL_GetKeyFromScancode( code );
}

int GetEngineKeyFromSDLScancode( SDL_Scancode code )
{
	switch( code )
	{
	case SDL_SCANCODE_RETURN: return K_ENTER;
	case SDL_SCANCODE_ESCAPE: return K_ESCAPE;
	case SDL_SCANCODE_BACKSPACE: return K_BACKSPACE;
	case SDL_SCANCODE_TAB: return K_TAB;
	case SDL_SCANCODE_SPACE: return K_SPACE;

	case SDL_SCANCODE_APOSTROPHE: return '\'';
	case SDL_SCANCODE_COMMA: return ',';
	case SDL_SCANCODE_MINUS: return '-';
	case SDL_SCANCODE_PERIOD: return '.';
	case SDL_SCANCODE_SLASH: return '/';
	case SDL_SCANCODE_0: return '0';
	case SDL_SCANCODE_1: return '1';
	case SDL_SCANCODE_2: return '2';
	case SDL_SCANCODE_3: return '3';
	case SDL_SCANCODE_4: return '4';
	case SDL_SCANCODE_5: return '5';
	case SDL_SCANCODE_6: return '6';
	case SDL_SCANCODE_7: return '7';
	case SDL_SCANCODE_8: return '8';
	case SDL_SCANCODE_9: return '9';
	case SDL_SCANCODE_SEMICOLON: return ';';
	case SDL_SCANCODE_EQUALS: return '=';
	case SDL_SCANCODE_LEFTBRACKET: return '[';
	case SDL_SCANCODE_RIGHTBRACKET: return ']';
	case SDL_SCANCODE_BACKSLASH: return '\\';
	case SDL_SCANCODE_GRAVE: return '^';
	case SDL_SCANCODE_A: return 'a';
	case SDL_SCANCODE_B: return 'b';
	case SDL_SCANCODE_C: return 'c';
	case SDL_SCANCODE_D: return 'd';
	case SDL_SCANCODE_E: return 'e';
	case SDL_SCANCODE_F: return 'f';
	case SDL_SCANCODE_G: return 'g';
	case SDL_SCANCODE_H: return 'h';
	case SDL_SCANCODE_I: return 'i';
	case SDL_SCANCODE_J: return 'j';
	case SDL_SCANCODE_K: return 'k';
	case SDL_SCANCODE_L: return 'l';
	case SDL_SCANCODE_M: return 'm';
	case SDL_SCANCODE_N: return 'n';
	case SDL_SCANCODE_O: return 'o';
	case SDL_SCANCODE_P: return 'p';
	case SDL_SCANCODE_Q: return 'q';
	case SDL_SCANCODE_R: return 'r';
	case SDL_SCANCODE_S: return 's';
	case SDL_SCANCODE_T: return 't';
	case SDL_SCANCODE_U: return 'u';
	case SDL_SCANCODE_V: return 'v';
	case SDL_SCANCODE_W: return 'w';
	case SDL_SCANCODE_X: return 'x';
	case SDL_SCANCODE_Y: return 'y';
	case SDL_SCANCODE_Z: return 'z';

	case SDL_SCANCODE_CAPSLOCK: return K_CAPSLOCK;

	case SDL_SCANCODE_F1: return K_F1;
	case SDL_SCANCODE_F2: return K_F2;
	case SDL_SCANCODE_F3: return K_F3;
	case SDL_SCANCODE_F4: return K_F4;
	case SDL_SCANCODE_F5: return K_F5;
	case SDL_SCANCODE_F6: return K_F6;
	case SDL_SCANCODE_F7: return K_F7;
	case SDL_SCANCODE_F8: return K_F8;
	case SDL_SCANCODE_F9: return K_F9;
	case SDL_SCANCODE_F10: return K_F10;
	case SDL_SCANCODE_F11: return K_F11;
	case SDL_SCANCODE_F12: return K_F12;

	case SDL_SCANCODE_PAUSE: return K_PAUSE;
	case SDL_SCANCODE_INSERT: return K_INS;
	case SDL_SCANCODE_HOME: return K_HOME;
	case SDL_SCANCODE_PAGEUP: return K_PGUP;
	case SDL_SCANCODE_DELETE: return K_DEL;
	case SDL_SCANCODE_END: return K_END;
	case SDL_SCANCODE_PAGEDOWN: return K_PGDN;

	case SDL_SCANCODE_RIGHT: return K_RIGHTARROW;
	case SDL_SCANCODE_LEFT: return K_LEFTARROW;
	case SDL_SCANCODE_DOWN: return K_DOWNARROW;
	case SDL_SCANCODE_UP: K_UPARROW;

	case SDL_SCANCODE_KP_DIVIDE: return K_KP_SLASH;
	case SDL_SCANCODE_KP_MULTIPLY: return K_KP_MUL;
	case SDL_SCANCODE_KP_MINUS: return K_KP_MINUS;
	case SDL_SCANCODE_KP_PLUS: return K_KP_PLUS;
	case SDL_SCANCODE_KP_ENTER: return K_KP_ENTER;
	case SDL_SCANCODE_KP_1: return K_KP_END;
	case SDL_SCANCODE_KP_2: return K_KP_DOWNARROW;
	case SDL_SCANCODE_KP_3: return K_KP_PGDN;
	case SDL_SCANCODE_KP_4: return K_KP_LEFTARROW;
	case SDL_SCANCODE_KP_5: return K_KP_5;
	case SDL_SCANCODE_KP_6: return K_KP_RIGHTARROW;
	case SDL_SCANCODE_KP_7: return K_KP_HOME;
	case SDL_SCANCODE_KP_8: return K_KP_UPARROW;
	case SDL_SCANCODE_KP_9: return K_KP_PGUP;
	case SDL_SCANCODE_KP_0: return K_KP_INS;
	case SDL_SCANCODE_KP_PERIOD: return K_KP_DEL;

	case SDL_SCANCODE_LCTRL:
	case SDL_SCANCODE_RCTRL: return K_CTRL;

	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT: return K_SHIFT;

	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RALT: return K_ALT;

	case SDL_SCANCODE_LGUI:
	case SDL_SCANCODE_RGUI: return K_WIN;
	default: return 0;
	}
}

void Key_Message( int key )
{
	if( key == K_KP_ENTER || key == K_ENTER )
	{
		Cbuf_ReplaceSemiColons();

		Cbuf_AddText( message_type );
		Cbuf_AddText( " \"" );
		Cbuf_AddText( chat_buffer );
		Cbuf_AddText( "\"\n" );

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[ 0 ] = '\0';
		Q_strcpy( message_type, "say" );
		return;
	}

	if( key == K_ESCAPE )
	{
		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[ 0 ] = '\0';
		return;
	}

	if( key < 32 || key > 127 )
		return;	// non printable

	if( key == K_BACKSPACE )
	{
		if( chat_bufferlen )
		{
			const int length = Q_UnicodeLength( chat_buffer );
			char* pszEnd = Q_UnicodeAdvance( chat_buffer, length - 1 );

			if( pszEnd )
				*pszEnd = '\0';

			if( !Q_UnicodeValidate( chat_buffer ) )
				*chat_buffer = '\0';

			chat_bufferlen = Q_strlen( chat_buffer );
		}
		return;
	}
}

//============================================================================

void Key_Escape_f()
{
	if( giSubState & 0x10 )
	{
		Cbuf_AddText( "disconnect\n" );
		giActive = DLL_PAUSED;
		giStateInfo = 2;
		giSubState = 1;
		Cbuf_Execute();
	}
	else if( key_dest != key_game )
	{
		if( key_dest != key_message )
			Sys_Error( "Bad key_dest" );
		Key_Message( K_ESCAPE );
	}
	else
	{
		VGuiWrap2_HideGameUI();
		giActive = DLL_PAUSED;
	}
}

/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum( const char* str )
{
	if( !str || !str[ 0 ] )
		return -1;
	if( !str[ 1 ] )
		return str[ 0 ];

	for( const keyname_t* kn = keynames; kn->name; ++kn )
	{
		if( !Q_strcasecmp( str, kn->name ) )
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
const char* Key_KeynumToString( int keynum )
{
	static char tinystr[ 2 ];

	if( keynum == -1 )
		return "<KEY NOT FOUND>";
	if( keynum > 32 && keynum < 127 )
	{	// printable ascii
		tinystr[ 0 ] = keynum;
		tinystr[ 1 ] = '\0';
		return tinystr;
	}

	for( const keyname_t* kn = keynames; kn->name; ++kn )
	{
		if( keynum == kn->keynum )
			return kn->name;
	}

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding( int keynum, const char* binding )
{
	if( keynum == -1 || keynum > 255 || !binding )
		return;

	// free old bindings
	if( keybindings[ keynum ] )
	{
		//Don't bother if it's the same binding.
		if( !Q_strcmp( keybindings[ keynum ], binding ) )
			return;

		Z_Free( keybindings[ keynum ] );
		keybindings[ keynum ] = nullptr;
	}

	// allocate memory for new binding
	const int l = Q_strlen( binding );
	char* newBinding = reinterpret_cast<char*>( Z_Malloc( l + 1 ) );
	Q_strcpy( newBinding, binding );
	newBinding[ l ] = '\0';
	keybindings[ keynum ] = newBinding;

	if( !Q_stricmp( binding, "toggleconsole" ) )
		toggleconsole_key = keynum;
}

int Key_CountBindings()
{
	int count = 0;

	for( int i = 0; i < 256; ++i )
	{
		if( keybindings[ i ] && *keybindings[ i ] )
			++count;
	}

	return count;
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings( FileHandle_t f )
{
	for( int i = 0; i < 256; ++i )
	{
		if( keybindings[ i ] )
		{
			if( *keybindings[ i ] )
			{
				FS_FPrintf( f, "bind \"%s\" \"%s\"\n", 
							Key_KeynumToString( i ), keybindings[ i ] );
			}
		}
	}
}

const char* Key_NameForBinding( const char* pBinding )
{
	g_engdstAddrs.Key_LookupBinding( &pBinding );

	const int iLength = Q_strlen( pBinding );

	for( int i = 0; i < 256; ++i )
	{
		if( !keybindings[ i ] || !( *keybindings[ i ] ) )
			continue;

		if( ( *keybindings[ i ] == '+' && !Q_strnicmp( keybindings[ i ] + 1, pBinding, iLength ) ) ||
			!Q_strnicmp( keybindings[ i ], pBinding, iLength ) )
		{
			return Key_KeynumToString( i );
		}
	}

	return nullptr;
}

/*
===================
Key_Init
===================
*/
void Key_Init()
{
	for( int i = 0; i < 32; i++ )
	{
		key_lines[ i ][ 0 ] = ']';
		key_lines[ i ][ 1 ] = '\0';
	}
	key_linepos = 1;

	//
	// init ascii characters in console mode
	//
	for( int i = 32; i < 128; ++i )
		consolekeys[ i ] = true;

	consolekeys[ K_ENTER ] = true;
	consolekeys[ K_KP_ENTER ] = true;
	consolekeys[ K_TAB ] = true;
	consolekeys[ K_LEFTARROW ] = true;
	consolekeys[ K_KP_LEFTARROW ] = true;
	consolekeys[ K_RIGHTARROW ] = true;
	consolekeys[ K_KP_RIGHTARROW ] = true;
	consolekeys[ K_UPARROW ] = true;
	consolekeys[ K_KP_UPARROW ] = true;
	consolekeys[ K_DOWNARROW ] = true;
	consolekeys[ K_KP_DOWNARROW ] = true;
	consolekeys[ K_BACKSPACE ] = true;
	consolekeys[ K_PGUP ] = true;
	consolekeys[ K_KP_PGUP ] = true;
	consolekeys[ K_PGDN ] = true;
	consolekeys[ K_KP_PGDN ] = true;
	consolekeys[ K_HOME ] = true;
	consolekeys[ K_KP_HOME ] = true;
	consolekeys[ K_END ] = true;
	consolekeys[ K_KP_END ] = true;
	consolekeys[ K_ALT ] = true;
	consolekeys[ ' ' ] = true;
	consolekeys[ K_CTRL ] = true;
	consolekeys[ K_SHIFT ] = true;
	consolekeys[ K_KP_SLASH ] = true;
	consolekeys[ K_KP_MINUS ] = true;
	consolekeys[ K_KP_PLUS ] = true;
	consolekeys[ K_KP_MUL ] = true;
	consolekeys[ K_MWHEELUP ] = true;
	consolekeys[ K_MWHEELDOWN ] = true;
	consolekeys[ K_WIN ] = false;
	consolekeys[ '`' ] = false;
	consolekeys[ '~' ] = false;

	for( int i = 0; i < 256; ++i )
		keyshift[ i ] = i;
	for( int i = 'a'; i <= 'z'; ++i )
		keyshift[ i ] = i - 'a' + 'A';
	keyshift[ '1' ] = '!';
	keyshift[ '2' ] = '@';
	keyshift[ '3' ] = '#';
	keyshift[ '4' ] = '$';
	keyshift[ '5' ] = '%';
	keyshift[ '6' ] = '^';
	keyshift[ '7' ] = '&';
	keyshift[ '8' ] = '*';
	keyshift[ '9' ] = '(';
	keyshift[ '0' ] = ')';
	keyshift[ '-' ] = '_';
	keyshift[ '=' ] = '+';
	keyshift[ ',' ] = '<';
	keyshift[ '.' ] = '>';
	keyshift[ '/' ] = '?';
	keyshift[ ';' ] = ':';
	keyshift[ '\'' ] = '"';
	keyshift[ '[' ] = '{';
	keyshift[ ']' ] = '}';
	keyshift[ '`' ] = '~';
	keyshift[ '\\' ] = '|';

	menubound[ K_ESCAPE ] = true;
	for( int i = 0; i < 12; ++i )
		menubound[ K_F1 + i ] = true;

	//
	// register our functions
	//
	Cmd_AddCommand( "bind", Key_Bind_f );
	Cmd_AddCommand( "unbind", Key_Unbind_f );
	Cmd_AddCommand( "unbindall", Key_Unbindall_f );
	Cmd_AddCommand( "escape", Key_Escape_f );
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
void Key_Event( int key, bool down )
{
	if( key > 255 )
		return;

	{
		int iDown = down;
		g_engdstAddrs.Key_Event( &key, &iDown );
		down = iDown != 0;
	}

	keydown[ key ] = down;

	if( keydown[ K_CTRL ] && keydown[ K_ALT ] && keydown[ K_DEL ] )
		Sys_Error( "ctrl-alt-del pressed" );

	if( key_dest == key_game )
	{
		if( down || keyGameUIdown[ key ] )
		{
			if( !ClientDLL_Key_Event( down != false, key, keybindings[ key ] ) )
			{
				if( key_repeats[ key ] )
				{
					if( down )
						return;
				}
				else if( down )
				{
					keyGameUIdown[ key ] = true;
					return;
				}

				key_repeats[ key ] = false;
				keyGameUIdown[ key ] = false;
				return;
			}
		}
	}

	bool bSkipKeyChecks = false;

	if( !down )
	{
		key_repeats[ key ] = 0;
		keyGameUIdown[ key ] = false;
	}
	else
	{
		// update auto-repeat status
		++key_repeats[ key ];

		if( key != K_PGUP &&
			key != K_KP_PGUP &&
			key != K_PGDN &&
			key != K_KP_PGDN )
		{
			if( key != K_BACKSPACE && key != K_PAUSE && key_repeats[ key ] > 1 )
			{
				return;	// ignore most autorepeats
			}

			if( key >= 200 )
			{
				if( !keybindings[ key ] )
				{
					Con_Printf( "%s is unbound.\n", Key_KeynumToString( key ) );
				}
				else
				{
					bSkipKeyChecks = true;
				}
			}
		}
		else
		{
			bSkipKeyChecks = true;
		}
	}

	char cmd[ 1024 ];

	if( !bSkipKeyChecks )
	{
		if( key == K_SHIFT )
			shift_down = down;

		//
		// key up events only generate commands if the game key binding is
		// a button command (leading + sign).  These will occur even in console mode,
		// to keep the character from continuing an action started before a console
		// switch.  Button commands include the kenum as a parameter, so multiple
		// downs can be matched with ups
		//
		if( !down )
		{
			const char* kb = keybindings[ key ];
			if( kb && kb[ 0 ] == '+' )
			{
				snprintf( cmd, ARRAYSIZE( cmd ), "-%s %i\n", kb + 1, key );
				Cbuf_AddText( cmd );
			}
			if( keyshift[ key ] != key )
			{
				kb = keybindings[ keyshift[ key ] ];
				if( kb && kb[ 0 ] == '+' )
				{
					snprintf( cmd, ARRAYSIZE( cmd ), "-%s %i\n", kb + 1, key );
					Cbuf_AddText( cmd );
				}
			}
			return;
		}
	}

	//TODO: define flags for these - Solokiller
	if( giSubState & 0x10 && key != K_ESCAPE )
		return;

	if( key_dest == key_game && key == K_ESCAPE )
	{
		if( VGui_GameUIKeyPressed() )
			return;
	}

	//
	// during demo playback, most keys bring up the main menu
	//
	//TODO: implement - Solokiller
	if( /*cls.demoplayback && && cls.spectator &&*/ down && consolekeys[ key ] && key_dest == key_game )
	{
		Con_ToggleConsole_f();
		return;
	}

	//
	// if not a consolekey, send to the interpreter no matter what mode is
	//
	if( ( key_dest == key_menu && menubound[ key ] )
		|| ( key_dest == key_game && ( !con_forcedup || !consolekeys[ key ] ) ) )
	{
		const char* kb = keybindings[ key ];
		if( kb )
		{
			if( kb[ 0 ] == '+' )
			{	// button commands add keynum as a parm
				snprintf( cmd, ARRAYSIZE( cmd ), "%s %i\n", kb, key );
				Cbuf_AddText( cmd );
			}
			else if( Q_strnicmp( kb, "toggleconsole", 4 ) || !keydown[ K_SHIFT ] || con_shifttoggleconsole.value )
			{
				Cbuf_AddText( kb );
				Cbuf_AddText( "\n" );
			}
		}
		return;
	}

	if( !down )
		return;		// other systems only care about key down events

	if( shift_down )
	{
		key = keyshift[ key ];
	}

	switch( key_dest )
	{
	case key_message:
		Key_Message( key );
		break;
	default:
		Sys_Error( "Bad key_dest" );
	}
}

void Key_EventStub( int key, bool down )
{
	if( !g_fUseDInput )
		Key_Event( key, down );
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates()
{
	for( int i = 0; i < 256; ++i )
	{
		keydown[ i ] = false;
		keyGameUIdown[ i ] = false;
		key_repeats[ i ] = 0;
	}
}

const char* Key_BindingForKey( int keynum )
{
	if( 0 <= keynum && keynum < 256 )
		return keybindings[ keynum ];

	return nullptr;
}

void Key_Shutdown()
{
	Q_memset( keybindings, 0, sizeof( keybindings ) );
}
