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
/**
*	@file
*
*	Quake script command processing module
*/
#include <cstdarg>

#include "quakedef.h"
#include "client.h"

cmdalias_t* cmd_alias = nullptr;

cmdalias_t* Cmd_GetAliasesList()
{
	return cmd_alias;
}

int trashtest;
int *trashspot;

bool cmd_wait = false;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f()
{
	cmd_wait = true;
}

/*
=============================================================================

COMMAND BUFFER

=============================================================================
*/

sizebuf_t cmd_text;

/*
============
Cbuf_Init
============
*/
void Cbuf_Init()
{
	SZ_Alloc( "cmd_text", &cmd_text, 8192 );	// space for commands and script files
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText( const char* text )
{
	const int l = Q_strlen( text );

	if( cmd_text.cursize + l >= cmd_text.maxsize )
	{
		Con_Printf( "Cbuf_AddText: overflow\n" );
		return;
	}

	SZ_Write( &cmd_text, text, l );
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText( const char* text )
{
	char* temp;

	// copy off any commands still remaining in the exec buffer
	const int templen = cmd_text.cursize;

	if( templen )
	{
		temp = reinterpret_cast<char*>( Z_Malloc( templen ) );
		Q_memcpy( temp, cmd_text.data, templen );
		SZ_Clear( &cmd_text );
	}
	else
		temp = nullptr;	// shut up compiler

	// add the entire text of the file
	Cbuf_AddText( text );

	// add the copied off data
	if( templen )
	{
		SZ_Write( &cmd_text, temp, templen );
		Z_Free( temp );
	}
}

void Cbuf_InsertTextLines( const char* text )
{
	const int oldBufSize = cmd_text.cursize;

	if( oldBufSize + Q_strlen( text ) + 2 >= cmd_text.maxsize )
	{
		Con_Printf( "Cbuf_InsertTextLines: overflow\n" );
		return;
	}

	void* pBuffer = nullptr;

	if( oldBufSize )
	{
		pBuffer = Z_Malloc( oldBufSize );
		Q_memcpy( pBuffer, cmd_text.data, oldBufSize );
		SZ_Clear( &cmd_text );
	}

	if( cmd_text.cursize + Q_strlen( "\n" ) >= cmd_text.maxsize )
	{
		Con_Printf( "Cbuf_AddText: overflow\n" );
	}
	else
	{
		SZ_Write( &cmd_text, "\n", Q_strlen( "\n" ) );
	}

	if( cmd_text.cursize + Q_strlen( text ) >= cmd_text.maxsize )
	{
		Con_Printf( "Cbuf_AddText: overflow\n" );
	}
	else
	{
		SZ_Write( &cmd_text, text, Q_strlen( text ) );
	}

	if( cmd_text.cursize + Q_strlen( "\n" ) < cmd_text.maxsize )
	{
		SZ_Write( &cmd_text, "\n", Q_strlen( "\n" ) );
	}
	else
	{
		Con_Printf( "Cbuf_AddText: overflow\n" );
	}

	if( !oldBufSize )
		return;

	SZ_Write( &cmd_text, pBuffer, oldBufSize );
	Z_Free( pBuffer );
}

char* CopyString( const char* in )
{
	char* out = reinterpret_cast<char*>( Z_Malloc( Q_strlen( in ) + 1 ) );

	Q_strcpy( out, in );

	return out;
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute()
{
	int i;
	char* text;
	char line[ 1024 ];
	bool quotes;

	while( cmd_text.cursize )
	{
		// find a \n or ; line break
		text = reinterpret_cast<char*>( cmd_text.data );

		quotes = false;

		for( i = 0; i < cmd_text.cursize; ++i )
		{
			if( text[ i ] == '"' )
				quotes = !quotes;

			if( !quotes && text[ i ] == ';' )
				break;	// don't break if inside a quoted string

			if( text[ i ] == '\n' )
				break;
		}

		const size_t uiLength = i < ARRAYSIZE( line ) ? i : ( ARRAYSIZE( line ) - 1 );

		Q_memcpy( line, text, uiLength );

		line[ uiLength ] = '\0';

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer

		if( i == cmd_text.cursize )
			cmd_text.cursize = 0;
		else
		{
			++i;
			cmd_text.cursize -= i;
			Q_memcpy( text, text + i, cmd_text.cursize );
		}

		// execute the command line
		Cmd_ExecuteString( line, src_command );

		if( cmd_wait )
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
}

/*
==============================================================================

SCRIPT COMMANDS

==============================================================================
*/

/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f()
{
	for( int i = 1; i < Cmd_Argc(); ++i )
	{
		Con_Printf( "%s ", Cmd_Argv( i ) );
	}

	Con_Printf( "\n" );
}

void Cmd_CmdList_f()
{
	const char* pszStartsWith = nullptr;
	bool bLogging = false;
	FileHandle_t f = FILESYSTEM_INVALID_HANDLE;

	if( Cmd_Argc() > 1 )
	{
		const char* pszCommand = Cmd_Argv( 1 );

		if( !Q_strcasecmp( pszCommand, "?" ) )
		{
			Con_Printf( "CmdList           : List all commands\nCmdList [Partial] : List commands starting with 'Partial'\nCmdList log [Partial] : Logs commands to file \"cmdlist.txt\" in the gamedir.\n" );
			return;
		}

		int matchIndex = 1;

		if( !Q_strcasecmp( pszCommand, "log" ) )
		{
			matchIndex = 2;

			char szTemp[ 256 ];
			snprintf( szTemp, sizeof( szTemp ), "cmdlist.txt" );

			f = FS_Open( szTemp, "wt" );

			if( f == FILESYSTEM_INVALID_HANDLE )
			{
				Con_Printf( "Couldn't open [%s] for writing!\n", szTemp );
				return;
			}

			bLogging = true;
		}

		if( Cmd_Argc() == matchIndex + 1 )
		{
			pszStartsWith = Cmd_Argv( matchIndex );
		}
	}

	int iStartsWithLength = 0;

	if( pszStartsWith )
	{
		iStartsWithLength = Q_strlen( pszStartsWith );
	}

	int count = 0;
	Con_Printf( "Command List\n--------------\n" );

	for( auto pCmd = Cmd_GetFirstCmd(); pCmd; pCmd = pCmd->next, ++count )
	{
		if( pszStartsWith &&
			Q_strncasecmp( pCmd->name, pszStartsWith, iStartsWithLength ) )
			continue;

		Con_Printf( "%-16.16s\n", pCmd->name );

		if( bLogging )
		{
			FS_FPrintf( f, "%-16.16s\n", pCmd->name );
		}
	}

	if( pszStartsWith && *pszStartsWith )
		Con_Printf( "--------------\n%3i Commands for [%s]\nCmdList ? for syntax\n", count, pszStartsWith );
	else
		Con_Printf( "--------------\n%3i Total Commands\nCmdList ? for syntax\n", count );

	if( bLogging )
		FS_Close( f );
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/
void Cmd_Alias_f()
{
	if( Cmd_Argc() == 1 )
	{
		Con_Printf( "Current alias commands:\n" );
		for( cmdalias_t* a = cmd_alias; a; a = a->next )
		{
			Con_Printf( "%s : %s\n", a->name, a->value );
		}
		return;
	}

	const char* s = Cmd_Argv( 1 );

	if( strlen( s ) >= MAX_ALIAS_NAME )
	{
		Con_Printf( "Alias name is too long\n" );
		return;
	}

	SetCStrikeFlags();

	if( Cvar_FindVar( s ) ||
		( g_bIsCStrike || g_bIsCZero ) &&
		!stricmp( s, "cl_autobuy" ) ||
		!stricmp( s, "cl_rebuy" ) ||
		!stricmp( s, "gl_ztrick" ) ||
		!stricmp( s, "gl_ztrick_old" ) ||
		!stricmp( s, "gl_d3dflip" ) ||
		!stricmp( s, "_special" ) ||
		!stricmp( s, "special" ) )
	{
		Con_Printf( "Alias name is invalid\n" );
		return;
	}

	// copy the rest of the command line
	char cmd[ 1024 ];
	cmd[ 0 ] = '\0';		// start out with a null string

	const int c = Cmd_Argc();

	for( int i = 2; i < c; ++i )
	{
		Q_strncat( cmd, Cmd_Argv( i ), ARRAYSIZE( cmd ) );
		if( i != c )
			strcat( cmd, " " );
	}

	Q_strcat( cmd, "\n", ARRAYSIZE( cmd ) );

	// if the alias already exists, reuse it
	cmdalias_t* a;
	for( a = cmd_alias; a; a = a->next )
	{
		if( !strcmp( s, a->name ) )
		{
			Z_Free( a->value );
			break;
		}
	}

	const bool bExists = a != nullptr;

	if( !bExists )
	{
		a = reinterpret_cast<cmdalias_t*>( Z_Malloc( sizeof( cmdalias_t ) ) );
		a->next = cmd_alias;
		cmd_alias = a;
	}

	if( !bExists || Q_strcmp( a->value, cmd ) )
	{
		Q_strncpy( a->name, s, ARRAYSIZE( a->name ) );
		a->value = CopyString( cmd );
	}
}

/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
void Cmd_StuffCmds_f()
{
	if( Cmd_Argc() != 1 )
	{
		Con_Printf( "stuffcmds : execute command line parameters\n" );
		return;
	}

	// build the combined string to parse from
	int s = 0;
	for( int i = 1; i<com_argc; i++ )
	{
		if( !com_argv[ i ] )
			continue;		// NEXTSTEP nulls out -NXHost
		s += Q_strlen( com_argv[ i ] ) + 1;
	}

	if( !s )
		return;

	const size_t uiTextSize = s + 1;

	char* text = reinterpret_cast<char*>( Z_Malloc( uiTextSize ) );
	text[ 0 ] = '\0';

	for( int i = 1; i<com_argc; i++ )
	{
		if( !com_argv[ i ] )
			continue;		// NEXTSTEP nulls out -NXHost
		Q_strcat( text, com_argv[ i ], uiTextSize );
		if( i != com_argc - 1 )
			Q_strcat( text, " ", uiTextSize );
	}

	// pull out the commands
	char* build = reinterpret_cast<char*>( Z_Malloc( uiTextSize ) );
	build[ 0 ] = '\0';

	char c;
	int j;

	for( int i = 0; i<s - 1; i++ )
	{
		if( text[ i ] == '+' )
		{
			++i;

			for( j = i; ( text[ j ] != '+' ) && ( text[ j ] != '-' ) && ( text[ j ] != '\0' ); ++j )
			{
			}

			c = text[ j ];
			text[ j ] = '\0';

			Q_strcat( build, text + i, uiTextSize );
			Q_strcat( build, "\n", uiTextSize );
			text[ j ] = c;
			i = j - 1;
		}
	}

	if( build[ 0 ] )
		Cbuf_InsertText( build );

	Z_Free( text );
	Z_Free( build );
}

/*
=============================================================================

COMMAND EXECUTION

=============================================================================
*/

#define	MAX_ARGS		80

static int cmd_argc;
static char* cmd_argv[ MAX_ARGS ];
static const char* cmd_null_string = "";
static const char* cmd_args = nullptr;

cmd_source_t cmd_source;


static cmd_function_t* cmd_functions = nullptr;		// possible commands to execute

cmd_function_t* Cmd_GetFirstCmd()
{
	return cmd_functions;
}

cmd_function_t** Cmd_GetFunctions()
{
	return &cmd_functions;
}

void Cmd_Exec_f();

/*
============
Cmd_Init
============
*/
void Cmd_Init()
{
	//
	// register our commands
	//
	Cmd_AddCommand( "stuffcmds", Cmd_StuffCmds_f );
	Cmd_AddCommand( "exec", Cmd_Exec_f );
	Cmd_AddCommand( "echo", Cmd_Echo_f );
	Cmd_AddCommand( "alias", Cmd_Alias_f );
	Cmd_AddCommand( "cmd", Cmd_ForwardToServer );
	Cmd_AddCommand( "wait", Cmd_Wait_f );
	Cmd_AddCommand( "cmdlist", Cmd_CmdList_f );
}

void Cmd_Shutdown()
{
	cmd_functions = nullptr;
	cmd_argc = 0;

	Q_memset( cmd_argv, 0, sizeof( cmd_argv ) );

	cmd_args = nullptr;
}

/*
============
Cmd_Argc
============
*/
int Cmd_Argc()
{
	g_engdstAddrs.Cmd_Argc();

	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
const char* Cmd_Argv( int arg )
{
	g_engdstAddrs.Cmd_Argv( &arg );

	if( ( unsigned ) arg >= ( unsigned ) cmd_argc )
		return cmd_null_string;
	return cmd_argv[ arg ];
}

/*
============
Cmd_Args
============
*/
const char* Cmd_Args()
{
	return cmd_args;
}

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString( const char* text )
{
	// clear the args from the last string
	for( int i = 0; i < cmd_argc; ++i )
		Z_Free( cmd_argv[ i ] );

	cmd_argc = 0;
	cmd_args = nullptr;

	char* pszData = const_cast<char*>( text );

	while( 1 )
	{
		// skip whitespace up to a /n
		while( *pszData && *pszData <= ' ' && *pszData != '\n' )
		{
			++pszData;
		}

		if( *pszData == '\n' )
		{	// a newline seperates commands in the buffer
			++pszData;
			break;
		}

		if( !*pszData )
			return;

		if( cmd_argc == 1 )
			cmd_args = pszData;

		pszData = COM_Parse( pszData );
		if( !pszData )
			return;

		//TODO: why is this using such an arbitrary limit? - Solokiller
		if( Q_strlen( com_token ) + 1 > 515 )
			return;

		if( cmd_argc < MAX_ARGS )
		{
			cmd_argv[ cmd_argc ] = reinterpret_cast<char*>( Z_Malloc( Q_strlen( com_token ) + 1 ) );
			Q_strcpy( cmd_argv[ cmd_argc ], com_token );
			++cmd_argc;
		}
	}

}

cmd_function_t* Cmd_FindCmd( const char* cmd_name )
{
	for( cmd_function_t* i = cmd_functions; i; i = i->next )
	{
		if( !Q_strcmp( cmd_name, i->name ) )
			return i;
	}

	return nullptr;
}

cmd_function_t* Cmd_FindCmdPrev( const char* cmd_name )
{
	cmd_function_t* pPrev = cmd_functions;
	cmd_function_t* pCvar = cmd_functions->next;

	while( pCvar )
	{
		if( !Q_strcmp( cmd_name, pCvar->name ) )
			return pPrev;

		pPrev = pCvar;
		pCvar = pCvar->next;
	}

	return nullptr;
}

/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand( const char* cmd_name, xcommand_t function )
{
	if( host_initialized )	// because hunk allocation would get stomped
		Sys_Error( "Cmd_AddCommand after host_initialized" );

	// fail if the command is a variable name
	if( Cvar_VariableString( cmd_name )[ 0 ] )
	{
		Con_Printf( "Cmd_AddCommand: %s already defined as a var\n", cmd_name );
		return;
	}

	// fail if the command already exists
	for( cmd_function_t* cmd = cmd_functions; cmd; cmd = cmd->next )
	{
		if( !Q_strcmp( cmd_name, cmd->name ) )
		{
			Con_Printf( "Cmd_AddCommand: %s already defined\n", cmd_name );
			return;
		}
	}

	cmd_function_t* cmd = reinterpret_cast<cmd_function_t*>( Hunk_Alloc( sizeof( cmd_function_t ) ) );
	cmd->name = cmd_name;

	if( !function )
		function = &Cmd_ForwardToServer;

	cmd->function = function;
	cmd->flags = 0;

	//Insert into list in alphabetical order.
	cmd_function_t dummycmd;

	dummycmd.name = " ";
	dummycmd.next = cmd_functions;

	cmd_function_t* pPrev = &dummycmd;
	cmd_function_t* pNext = cmd_functions;

	while( pNext && stricmp( pNext->name, cmd->name ) <= 0 )
	{
		pPrev = pNext;
		pNext = pNext->next;
	}

	// link the variable in
	pPrev->next = cmd;
	cmd->next = pNext;
	cmd_functions = dummycmd.next;
}

void Cmd_AddMallocCommand( const char* cmd_name, xcommand_t function, int flag )
{
	// fail if the command is a variable name
	if( *Cvar_VariableString( cmd_name ) )
	{
		Con_Printf( "Cmd_AddCommand: %s already defined as a var\n", cmd_name );
		return;
	}

	// fail if the command already exists
	for( cmd_function_t* cmd = cmd_functions; cmd; cmd = cmd->next )
	{
		if( !Q_strcmp( cmd_name, cmd->name ) )
		{
			Con_Printf( "Cmd_AddCommand: %s already defined\n", cmd_name );
			return;
		}
	}

	cmd_function_t* cmd = ( cmd_function_t * ) Mem_ZeroMalloc( sizeof( cmd_function_t ) );

	if( !function )
		function = Cmd_ForwardToServer;

	//TODO: doesn't use sorted insertion like above? - Solokiller
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->flags = flag;

	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

void Cmd_AddHUDCommand( const char* cmd_name, xcommand_t function )
{
	Cmd_AddMallocCommand( cmd_name, function, CMD_HUD );
}

void Cmd_AddWrapperCommand( const char* cmd_name, xcommand_t function )
{
	Cmd_AddMallocCommand( cmd_name, function, CMD_WRAPPER );
}

void Cmd_AddGameCommand( const char* cmd_name, xcommand_t function )
{
	Cmd_AddMallocCommand( cmd_name, function, CMD_GAME );
}

void Cmd_RemoveMallocedCmds( int flag )
{
	cmd_function_t* pPrev = nullptr;
	cmd_function_t* pCmd = cmd_functions;
	cmd_function_t* pNext;

	while( pCmd )
	{
		pNext = pCmd->next;

		if( pCmd->flags & flag )
		{
			Mem_Free( pCmd );

			if( pPrev )
				pPrev->next = pNext;
			else
				cmd_functions = pNext;
		}
		else
		{
			pPrev = pCmd;
		}

		pCmd = pNext;
	}
}

void Cmd_RemoveHudCmds()
{
	Cmd_RemoveMallocedCmds( CMD_HUD );
}

void Cmd_RemoveGameCmds()
{
	Cmd_RemoveMallocedCmds( CMD_GAME );
}

void Cmd_RemoveWrapperCmds()
{
	Cmd_RemoveMallocedCmds( CMD_WRAPPER );
}

/*
============
Cmd_Exists
============
*/
bool Cmd_Exists( const char* cmd_name )
{
	for( cmd_function_t* cmd = cmd_functions; cmd; cmd = cmd->next )
	{
		if( !Q_strcmp( cmd_name, cmd->name ) )
			return true;
	}

	return false;
}

/*
============
Cmd_CompleteCommand
============
*/
const char* Cmd_CompleteCommand( const char* partial, bool forward )
{
	static char lastpartial[ 256 ] = {};

	int len = Q_strlen( partial );

	if( !len )
		return nullptr;

	char search[ 256 ];

	Q_strncpy( search, partial, ARRAYSIZE( search ) );

	//Strip trailing whitespace.
	for( int i = len - 1; search[ len - 1 ] == ' '; --i )
	{
		search[ i ] = '\0';
		len = i;
	}

	cmd_function_t* pCmd = cmd_functions;

	//User continuing search, continue where we left off.
	if( !Q_stricmp( search, lastpartial ) )
	{
		for( ; pCmd; pCmd = pCmd->next )
		{
			if( !Q_strcmp( search, pCmd->name ) )
			{
				bool bFound = false;

				if( forward )
				{
					pCmd = pCmd->next;
					bFound = true;
				}
				else
				{
					if( cmd_functions->next )
					{
						const char* pszName = pCmd->name;

						pCmd = cmd_functions;

						while( true )
						{
							if( !Q_strcmp( pszName, pCmd->next->name ) )
							{
								bFound = true;
								break;
							}

							if( !pCmd->next )
							{
								pCmd = cmd_functions;
								break;
							}

							pCmd = pCmd->next;
						}
					}
				}

				if( bFound )
				{
					Q_strncpy( lastpartial, pCmd->name, ARRAYSIZE( lastpartial ) );
					return pCmd->name;
				}
			}
		}
	}

	// check functions
	for( ; pCmd; pCmd = pCmd->next )
	{
		if( !Q_strncmp( search, pCmd->name, len ) )
		{
			Q_strncpy( lastpartial, pCmd->name, ARRAYSIZE( lastpartial ) );
			return pCmd->name;
		}
	}

	return nullptr;
}

/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
bool Cmd_ForwardToServerInternal( sizebuf_t* pBuf )
{
	char tempData[ 4096 ];
	sizebuf_t tempBuf;

	tempBuf.flags = FSB_ALLOWOVERFLOW;
	tempBuf.data = ( byte * ) tempData;
	tempBuf.maxsize = sizeof( tempData );
	tempBuf.cursize = 0;
	tempBuf.buffername = "Cmd_ForwardToServerInternal::tempBuf";

	if( cls.state != ca_connected )
	{
		if( Q_stricmp( Cmd_Argv( 0 ), "setinfo" ) )
		{
			Con_Printf( "Can't \"%s\", not connected\n", Cmd_Argv( 0 ) );
		}

		return false;
	}
	else
	{
		if( !cls.demoplayback && !g_bIsDedicatedServer )
		{
			MSG_WriteByte( &tempBuf, clc_stringcmd );
			if( Q_strcasecmp( Cmd_Argv( 0 ), "cmd" ) != 0 )
			{
				SZ_Print( &tempBuf, Cmd_Argv( 0 ) );
				SZ_Print( &tempBuf, " " );
			}

			if( Cmd_Argc() > 1 )
				SZ_Print( &tempBuf, cmd_args );
			else
				SZ_Print( &tempBuf, "\n" );

			if( !( tempBuf.flags & FSB_OVERFLOWED ) && tempBuf.cursize + pBuf->cursize <= pBuf->maxsize )
			{
				SZ_Write( pBuf, tempBuf.data, tempBuf.cursize );
				return true;
			}
		}
	}

	return false;
}

void Cmd_ForwardToServer()
{
	if( Q_stricmp( Cmd_Argv( 0 ), "cmd" ) || Q_stricmp( Cmd_Argv( 1 ), "dlfile" ) )
	{
		//TODO: implement - Solokiller
		//Cmd_ForwardToServerInternal( &cls.netchan.message );
	}
}

/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void Cmd_ExecuteString( const char* text, cmd_source_t src )
{
	cmd_source = src;
	Cmd_TokenizeString( text );

	// execute the command line
	if( !Cmd_Argc() )
		return;		// no tokens

	// check functions
	for( cmd_function_t* cmd = cmd_functions; cmd; cmd = cmd->next )
	{
		if( !Q_strcasecmp( cmd_argv[ 0 ], cmd->name ) )
		{
			cmd->function();
			//TODO: implement - Solokiller
			/*
			if( cls.demorecording && cmd->flags & 1 && cls.spectator == false )
				CL_RecordHUDCommand( cmd->name );
				*/
			return;
		}
	}

	// check alias
	for( cmdalias_t* a = cmd_alias; a; a = a->next )
	{
		if( !Q_strcasecmp( cmd_argv[ 0 ], a->name ) )
		{
			Cbuf_InsertText( a->value );
			return;
		}
	}

	// check cvars
	if( !Cvar_Command() && ( cls.state == ca_connected || cls.state == ca_active || cls.state == ca_uninitialized ) )
		Cmd_ForwardToServer();
}

/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f()
{
	if( Cmd_Argc() != 2 )
	{
		Con_Printf( "exec <filename> : execute a script file\n" );
		return;
	}

	const char* pszFilename = Cmd_Argv( 1 );

	if( !pszFilename )
		return;

	if( Q_strstr( pszFilename, "\\" ) ||
		Q_strstr( pszFilename, ":" ) ||
		Q_strstr( pszFilename, "~" ) ||
		Q_strstr( pszFilename, ".." ) ||
		*pszFilename == '/' )
	{
		Con_Printf( "exec %s: invalid path.\n", pszFilename );
		return;
	}

	if( strchr( pszFilename, '.' ) != strrchr( pszFilename, '.' ) )
	{
		Con_Printf( "exec %s: invalid filename.\n", pszFilename );
		return;
	}

	const char* pszExt = COM_FileExtension( pszFilename );

	if( Q_strcmp( pszExt, "cfg" ) && Q_strcmp( pszExt, "rc" ) )
	{
		Con_Printf( "exec %s: not a .cfg or .rc file\n", pszFilename );
		return;
	}

	FileHandle_t hFile = FS_OpenPathID( pszFilename, "rb", "GAMECONFIG" );

	if( hFile == FILESYSTEM_INVALID_HANDLE )
		hFile = FS_OpenPathID( pszFilename, "rb", "GAME" );

	if( hFile == FILESYSTEM_INVALID_HANDLE )
		hFile = FS_Open( pszFilename, "rb" );

	if( hFile != FILESYSTEM_INVALID_HANDLE )
	{
		const int iSize = FS_Size( hFile );

		char* pszBuffer = reinterpret_cast<char*>( Mem_Malloc( iSize + 1 ) );

		if( pszBuffer )
		{
			FS_Read( pszBuffer, iSize, hFile );
			pszBuffer[ iSize ] = '\0';

			Con_DPrintf( "execing %s\n", pszFilename );

			if( cmd_text.cursize + iSize + 2 >= cmd_text.maxsize )
			{
				char* pszText = pszBuffer;

				while( 1 )
				{
					Cbuf_Execute();

					pszText = COM_ParseLine( pszText );

					if( Q_strlen( com_token ) <= 0 )
						break;

					Cbuf_InsertTextLines( com_token );
				}
			}
			else
			{
				Cbuf_InsertTextLines( pszBuffer );
			}

			Mem_Free( pszBuffer );
		}
		else
		{
			Con_Printf( "exec: not enough space for %s", pszFilename );
		}

		FS_Close( hFile );
	}
	else if( !Q_strstr( pszFilename, "autoexec.cfg" ) &&
			 !Q_strstr( pszFilename, "userconfig.cfg" ) &&
			 !Q_strstr( pszFilename, "hw/opengl.cfg" ) &&
			 !Q_strstr( pszFilename, "joystick.cfg" ) &&
			 !Q_strstr( pszFilename, "game.cfg" ) )
	{
		Con_Printf( "couldn't exec %s\n", pszFilename );
	}
}

bool Cmd_ForwardToServerUnreliable()
{
	//TODO: implement - Solokiller
	return false/*Cmd_ForwardToServerInternal( &cls.datagram )*/;
}

/*
================
Cmd_CheckParm

Returns the position (1 to argc-1) in the command's argument list
where the given parameter apears, or 0 if not present
================
*/

int Cmd_CheckParm( const char* parm )
{
	if( !parm )
		Sys_Error( "Cmd_CheckParm: NULL" );

	for( int i = 1; i < Cmd_Argc(); ++i )
	{
		if( !Q_strcasecmp( parm, Cmd_Argv( i ) ) )
			return i;
	}

	return 0;
}
