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
#ifndef ENGINE_CMD_H
#define ENGINE_CMD_H

/**
*	@file
*
*	Command buffer and command execution
*/

//===========================================================================

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

/**
*	allocates an initial text buffer that will grow as needed
*/
void Cbuf_Init();

/**
*	as new commands are generated from the console or keybindings,
*	the text is added to the end of the command buffer.
*/
void Cbuf_AddText( const char* text );

/**
*	when a command wants to issue other commands immediately, the text is
*	inserted at the beginning of the buffer, before any remaining unexecuted
*	commands.
*/
void Cbuf_InsertText( const char* text );

/**
*	Inserts text at the beginning of the buffer, adding newlines between the text and the remainder of the buffer.
*	@see Cbuf_InsertText
*/
void Cbuf_InsertTextLines( const char* text );

/**
*	Pulls off \n terminated lines of text from the command buffer and sends
*	them through Cmd_ExecuteString.  Stops when the buffer is empty.
*	Normally called once per frame, but may be explicitly invoked.
*	Do not call inside a command function!
*/
void Cbuf_Execute();

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

Commands can come from three sources, but the handler functions may choose
to dissallow the action or forward it to a remote server if the source is
not apropriate.

*/

typedef void ( *xcommand_t )();

typedef struct cmd_function_s
{
	cmd_function_s* next;
	const char* name;
	xcommand_t function;
	int flags;
} cmd_function_t;

enum cmd_source_t
{
	src_client,		// came in over a net connection as a clc_stringcmd
					// host_client will be valid during this state.
	src_command		// from the command buffer
};

extern cmd_source_t cmd_source;

/**
*	Flags for commands
*/
enum
{
	/**
	*	This is a Hud command (Client dll)
	*/
	CMD_HUD = 1 << 0,

	/**
	*	This is a game command (Server dll)
	*/
	CMD_GAME = 1 << 1,

	/**
	*	This is a wrapper command (GameUI dll)
	*/
	CMD_WRAPPER = 1 << 2,
};

typedef struct cmdalias_s cmdalias_t;

cmdalias_t* Cmd_GetAliasesList();

cmd_function_t* Cmd_GetFirstCmd();

cmd_function_t** Cmd_GetFunctions();

cmd_function_t* Cmd_FindCmd( const char* cmd_name );
cmd_function_t* Cmd_FindCmdPrev( const char* cmd_name );

void Cmd_Init();
void Cmd_Shutdown();

/**
*	called by the init functions of other parts of the program to
*	register commands and functions to call for them.
*	The cmd_name is referenced later, so it should not be in temp memory
*/
void Cmd_AddCommand( const char* cmd_name, xcommand_t function );

void Cmd_AddMallocCommand( const char* cmd_name, xcommand_t function, int flag );

void Cmd_AddHUDCommand( const char* cmd_name, xcommand_t function );

void Cmd_AddWrapperCommand( const char* cmd_name, xcommand_t function );

void Cmd_AddGameCommand( const char* cmd_name, xcommand_t function );

void Cmd_RemoveMallocedCmds( int flag );

void Cmd_RemoveHudCmds();

void Cmd_RemoveGameCmds();

void Cmd_RemoveWrapperCmds();

/**
*	used by the cvar code to check for cvar / command name overlap
*/
bool Cmd_Exists( const char* cmd_name );

/**
*	attempts to match a partial command for automatic command line completion
*	returns nullptr if nothing fits
*/
const char* Cmd_CompleteCommand( const char* partial, bool forward );

int	 Cmd_Argc();
const char* Cmd_Argv( int arg );
const char* Cmd_Args();
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

/**
*	Returns the position (1 to argc-1) in the command's argument list
*	where the given parameter apears, or 0 if not present
*/
int Cmd_CheckParm( const char* parm );

/**
*	Takes a null terminated string.  Does not need to be /n terminated.
*	breaks the string up into arg tokens.
*/
void Cmd_TokenizeString( const char* text );

/**
*	Parses a single line of text into arguments and tries to execute it.
*	The text can come from the command buffer, a remote client, or stdin.
*/
void Cmd_ExecuteString( const char* text, cmd_source_t src );

/**
*	adds the current command line as a clc_stringcmd to the client message.
*	things like godmode, noclip, etc, are commands directed to the server,
*	so when they are typed in at the console, they will need to be forwarded.
*/
void Cmd_ForwardToServer();

bool Cmd_ForwardToServerUnreliable();

#endif //ENGINE_CMD_H
