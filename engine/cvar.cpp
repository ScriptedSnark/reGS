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
*	dynamic variable tracking
*/

#include "quakedef.h"
#include "cdll_int.h"

cvar_t* cvar_vars = nullptr;
const char* cvar_null_string = "";

void Cvar_Init()
{
}

void Cvar_Shutdown()
{
	cvar_vars = nullptr;
}

cvar_t* Cvar_FindVar( const char* var_name )
{
	g_engdstAddrs.pfnGetCvarPointer( &var_name );

	for( cvar_t* var = cvar_vars; var; var = var->next )
	{
		if( !Q_strcmp( var_name, var->name ) )
		{
			return var;
		}
	}

	return nullptr;
}

cvar_t* Cvar_FindPrevVar( const char* var_name )
{
	cvar_t* pPrev;
	cvar_t* pNext;

	for( pPrev = cvar_vars, pNext = cvar_vars->next;
		 pNext && !Q_stricmp( var_name, pNext->name );
		 pPrev = pNext, pNext = pNext->next )
	{
	}

	if( !pNext )
		return nullptr;

	return pPrev;
}

float Cvar_VariableValue( const char* var_name )
{
	cvar_t* var = Cvar_FindVar( var_name );

	if( !var )
		return 0;

	return Q_atof( var->string );
}

int Cvar_VariableInt( const char* var_name )
{
	cvar_t* var = Cvar_FindVar( var_name );

	if( !var )
		return 0;

	return Q_atoi( var->string );
}

const char* Cvar_VariableString( const char* var_name )
{
	cvar_t* var = Cvar_FindVar( var_name );

	if( !var )
		return cvar_null_string;

	return var->string;
}

const char* Cvar_CompleteVariable( const char* partial, bool forward )
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

	cvar_t* pCvar = cvar_vars;

	//User continuing search, continue where we left off.
	if( !Q_stricmp( search, lastpartial ) )
	{
		for( ; pCvar; pCvar = pCvar->next )
		{
			if( !Q_strcmp( search, pCvar->name ) )
			{
				bool bFound = false;

				if( forward )
				{
					pCvar = pCvar->next;
					bFound = true;
				}
				else
				{
					cvar_t* pPrev = Cvar_FindPrevVar( pCvar->name );

					bFound = pPrev != nullptr;

					if( pPrev )
						pCvar = pPrev;
					else
						pCvar = cvar_vars;
				}

				if( bFound )
				{
					Q_strncpy( lastpartial, pCvar->name, ARRAYSIZE( lastpartial ) );
					return pCvar->name;
				}
			}
		}
	}

	// check functions
	for( ; pCvar; pCvar = pCvar->next )
	{
		if( !Q_strncmp( search, pCvar->name, len ) )
		{
			Q_strncpy( lastpartial, pCvar->name, ARRAYSIZE( lastpartial ) );
			return pCvar->name;
		}
	}

	return nullptr;
}

void Cvar_DirectSet( cvar_t* var, const char* value )
{
	if( !value || !var )
		return;

	char szNew[ 1024 ];

	const char* pszNewValue = value;

	if( var->flags & FCVAR_PRINTABLEONLY )
	{
		szNew[ 0 ] = '\0';

		char* i = szNew;

		if( Q_UnicodeValidate( value ) )
		{
			Q_strncpy( szNew, value, ARRAYSIZE( szNew ) );
		}
		else
		{
			for( const char* pszNext = value; *pszNext; ++pszNext )
			{
				if( ( unsigned char ) ( *pszNext - ' ' ) <= 0x5Eu )
					*i++ = *pszNext;
			}

			*i = '\0';
		}

		if( !Q_UnicodeValidate( i ) )
			Q_UnicodeRepair( i );

		if( !Q_strlen( szNew ) )
			Q_strcpy( szNew, "empty" );

		pszNewValue = szNew;
	}

	if( var->flags & FCVAR_NOEXTRAWHITEPACE )
	{
		if( pszNewValue != szNew )
			Q_strncpy( szNew, pszNewValue, ARRAYSIZE( szNew ) );

		Q_StripUnprintableAndSpace( szNew );
	}

	const bool bChanged = Q_strcmp( var->string, pszNewValue ) != 0;

	if( var->flags & FCVAR_USERINFO )
	{
		//TODO: implement - Solokiller
		if( false /*cls.state
			|| ( Info_SetValueForKey( Info_Serverinfo(), var->name, pszNewValue, 512 ),
				 SV_BroadcastCommand( "fullserverinfo \"%s\"\n", Info_Serverinfo() ),
				 ( _DWORD ) cls.state )*/ )
		{
			//TODO: implement - Solokiller
			//Info_SetValueForKey( cls.userinfo, var->name, pszNewValue, 256 );

			//TODO: implement - Solokiller
			if( bChanged /*&& ( unsigned int ) cls.state > 2*/ )
			{
				//TODO: implement - Solokiller
				//MSG_WriteByte( &cls.netchan.message, clc_stringcmd );
				//SZ_Print( &cls.netchan.message, va( "setinfo \"%s\" \"%s\"\n", var->name, pszNewValue ) );
			}
		}
	}

	if( ( var->flags & FCVAR_SERVER ) && bChanged )
	{
		if( !( var->flags & FCVAR_UNLOGGED ) )
		{
			if( var->flags & FCVAR_PROTECTED )
			{
				Log_Printf( "Server cvar \"%s\" = \"%s\"\n", var->name, "***PROTECTED***" );
				//TODO: implement - Solokiller
				//SV_BroadcastPrintf( "\"%s\" changed to \"%s\"\n", var->name, "***PROTECTED***" );
			}
			else
			{
				Log_Printf( "Server cvar \"%s\" = \"%s\"\n", var->name, pszNewValue );
				//TODO: implement - Solokiller
				//SV_BroadcastPrintf( "\"%s\" changed to \"%s\"\n", var->name, v3 );
			}
		}

		const char* pszPrintValue = pszNewValue;

		if( var->flags & FCVAR_PROTECTED )
		{
			pszPrintValue = "0";

			if( Q_strlen( pszNewValue ) > 0 )
			{
				if( Q_stricmp( pszNewValue, "none" ) )
					pszPrintValue = "1";
			}
		}

		Steam_SetCVar( var->name, pszPrintValue );
	}
	
	Z_Free( var->string );
	var->string = reinterpret_cast<char*>( Z_Malloc( Q_strlen( pszNewValue ) + 1 ) );
	Q_strcpy( var->string, pszNewValue );
	var->value = Q_atof( var->string );
}

void Cvar_Set( const char* var_name, const char* value )
{
	cvar_t* var = Cvar_FindVar( var_name );

	if( !var )
	{	// there is an error in C code if this happens
		Con_Printf( "Cvar_Set: variable %s not found\n", var_name );
		return;
	}

	Cvar_DirectSet( var, value );
}

void Cvar_SetValue( const char* var_name, float value )
{
	char val[ 32 ];

	const int intValue = static_cast<int>( floor( value ) );

	//Set as integer if it's a whole number, to within a small margin of error.
	if( fabs( value - intValue ) < 0.000001 )
	{
		snprintf( val, ARRAYSIZE( val ), "%d", intValue );
	}
	else
	{
		snprintf( val, ARRAYSIZE( val ), "%f", value );
	}

	Cvar_Set( var_name, val );
}

void Cvar_RegisterVariable( cvar_t* variable )
{
	// first check to see if it has allready been defined
	if( Cvar_FindVar( variable->name ) )
	{
		Con_Printf( "Can't register variable %s, already defined\n", variable->name );
		return;
	}

	// check for overlap with a command
	if( Cmd_Exists( variable->name ) )
	{
		Con_Printf( "Cvar_RegisterVariable: %s is a command\n", variable->name );
		return;
	}

	// copy the value off, because future sets will Z_Free it
	char* oldstr = variable->string;
	variable->string = reinterpret_cast<char*>( Z_Malloc( Q_strlen( variable->string ) + 1 ) );
	Q_strcpy( variable->string, oldstr );
	variable->value = Q_atof( variable->string );

	//Insert into list in alphabetical order.
	cvar_t dummyvar;

	dummyvar.name = " ";
	dummyvar.next = cvar_vars;

	cvar_t* pPrev = &dummyvar;
	cvar_t* pNext = cvar_vars;

	while( pNext && stricmp( pNext->name, variable->name ) <= 0 )
	{
		pPrev = pNext;
		pNext = pNext->next;
	}

	// link the variable in
	pPrev->next = variable;
	variable->next = pNext;
	cvar_vars = dummyvar.next;
}

void Cvar_RemoveHudCvars()
{
	if( !cvar_vars )
		return;

	cvar_t* pPrev = nullptr;
	cvar_t* pCvar = cvar_vars;
	cvar_t* pNext;

	while( pCvar )
	{
		pNext = pCvar->next;

		if( pCvar->flags & FCVAR_CLIENTDLL )
		{
			Z_Free( pCvar->string );
			Z_Free( pCvar );

			if( pPrev )
				pPrev->next = pNext;
			else
				cvar_vars = pNext;
		}
		else
		{
			pPrev = pCvar;
		}

		pCvar = pNext;
	}

	//TODO: The reverse engineered code does some weird thing here
	//where it clears cvar_vars and sets it to the last cvar in the list defined by
	//pPrev. Investigate. - Solokiller
}

const char* Cvar_IsMultipleTokens( const char* varname )
{
	//TODO: why is this such an odd size? - Solokiller
	static char firstToken[ 516 ];

	firstToken[ 0 ] = '\0';

	char* pszData = const_cast<char*>( varname );

	int count = 0;

	while( 1 )
	{
		pszData = COM_Parse( pszData );

		if( Q_strlen( com_token ) <= 0 )
			break;

		if( count )
		{
			++count;
		}
		else
		{
			count = 1;
			Q_strncpy( firstToken, com_token, ARRAYSIZE( firstToken ) );
		}
	}

	if( count != 1 )
		return firstToken;

	return nullptr;
}

bool Cvar_Command()
{
	const char* pszToken = Cvar_IsMultipleTokens( Cmd_Argv( 0 ) );

	const bool bIsMultipleTokens = pszToken != nullptr;

	if( !pszToken )
		pszToken = Cmd_Argv( 0 );

	cvar_t* pCvar = Cvar_FindVar( pszToken );

	if( !pCvar )
		return false;

	// perform a variable print or set
	if( bIsMultipleTokens || Cmd_Argc() == 1 )
	{
		Con_Printf( "\"%s\" is \"%s\"\n", pCvar->name, pCvar->string );
		return true;
	}

	//TODO: implement - Solokiller
	if( ( pCvar->flags & FCVAR_SPONLY ) /*&& ( unsigned int ) cls.state > 1 && ei.maxclients > 1*/ )
	{
		Con_Printf( "Can't set %s in multiplayer\n", pCvar->name );
		return true;
	}

	//As ridiculous as this seems, this is what the engine does.
	//TODO: figure out if this can be replaced with DirectSet. - Solokiller
	Cvar_Set( pCvar->name, Cmd_Argv( 1 ) );
	return true;
}

void Cvar_WriteVariables( FileHandle_t f )
{
	for( cvar_t* var = cvar_vars; var; var = var->next )
	{
		if( ( var->flags & FCVAR_ARCHIVE ) )
		{
			FS_FPrintf( f, "%s \"%s\"\n", var->name, var->string );
		}
	}
}

void Cmd_CvarListPrintCvar( cvar_t *var, FileHandle_t f )
{
	char szOutstr[ 256 ];

	const int intValue = static_cast<int>( floor( var->value ) );

	if( var->value == intValue )
	{
		snprintf( szOutstr, ARRAYSIZE( szOutstr ) - 11, "%-15s : %8i", var->name, intValue );
	}
	else
	{
		snprintf( szOutstr, ARRAYSIZE( szOutstr ) - 11, "%-15s : %8.3f", var->name, var->value );
	}

	if( var->flags & FCVAR_ARCHIVE )
	{
		Q_strcat( szOutstr, ", a", ARRAYSIZE( szOutstr ) );
	}

	if( var->flags & FCVAR_SERVER )
	{
		Q_strcat( szOutstr, ", sv", ARRAYSIZE( szOutstr ) );
	}

	if( var->flags & FCVAR_USERINFO )
	{
		Q_strcat( szOutstr, ", i", ARRAYSIZE( szOutstr ) );
	}

	Q_strcat( szOutstr, "\n", ARRAYSIZE( szOutstr ) );

	Con_Printf( "%s", szOutstr );

	if( f != FILESYSTEM_INVALID_HANDLE )
		FS_FPrintf( f, "%s", szOutstr );
}

void Cmd_CvarList_f()
{
	FileHandle_t f = FILESYSTEM_INVALID_HANDLE;
	bool bLogging = false;	//Output to file
	bool bArchive = false;	//Only print archived cvars
	bool bSOnly = false;	//Only print server cvars
	char szTemp[ 256 ];

	const char* pszPartial = nullptr;
	int iPartialLength = 0;
	bool bPartialMatch = false;	//Whether this is a partial match. If logging, this is false

	const int iArgC = Cmd_Argc();

	if( iArgC >= 2 )
	{
		const char* pszCommand = Cmd_Argv( 1 );

		if( !Q_stricmp( pszCommand, "?" ) )
		{
			Con_Printf( "CvarList           : List all cvars\nCvarList [Partial] : List cvars starting with 'Partial'\nCvarList log [Partial] : Logs cvars to file \"cvarlist.txt\" in the gamedir.\n" );
			return;
		}

		if( !Q_stricmp( pszCommand, "log" ) )
		{
			int count = 0;

			FileHandle_t hTestFile;

			while( 1 )
			{
				snprintf( szTemp, ARRAYSIZE( szTemp ), "cvarlist%02d.txt", count );
				COM_FixSlashes( szTemp );

				hTestFile = FS_Open( szTemp, "r" );

				if( hTestFile == FILESYSTEM_INVALID_HANDLE )
					break;

				++count;

				FS_Close( hTestFile );

				if( count == 100 )
				{
					Con_Printf( "Can't cvarlist! Too many existing cvarlist output files in the gamedir!\n" );
					return;
				}
			}

			f = FS_Open( szTemp, "wt" );

			if( f == FILESYSTEM_INVALID_HANDLE )
			{
				Con_Printf( "Couldn't open [%s] for writing!\n", szTemp );
				return;
			}

			bLogging = true;

			if( iArgC == 3 )
			{
				pszPartial = Cmd_Argv( 2 );
				iPartialLength = Q_strlen( pszPartial );
			}
		}
		else if( !Q_stricmp( pszCommand, "-a" ) )
		{
			bArchive = true;
		}
		else if( !Q_stricmp( pszCommand, "-s" ) )
		{
			bSOnly = true;
		}
		else
		{
			pszPartial = pszCommand;
			iPartialLength = Q_strlen( pszPartial );
			bPartialMatch = iArgC == 2 && pszPartial != nullptr;
		}
	}

	int count = 0;
	Con_Printf( "CVar List\n--------------\n" );

	for( cvar_t* pCvar = cvar_vars; pCvar; pCvar = pCvar->next )
	{
		if( bArchive && !( pCvar->flags & FCVAR_ARCHIVE ) )
			continue;

		if( bSOnly && !( pCvar->flags & FCVAR_SERVER ) )
			continue;

		if( pszPartial && Q_strnicmp( pCvar->name, pszPartial, iPartialLength ) )
			continue;

		++count;
		Cmd_CvarListPrintCvar( pCvar, f );
	}

	if( bPartialMatch && *pszPartial )
		Con_Printf( "--------------\n%3i CVars for [%s]\nCvarList ? for syntax\n", count, pszPartial );
	else
		Con_Printf( "--------------\n%3i Total CVars\nCvarList ? for syntax\n", count );

	if( bLogging )
	{
		FS_Close( f );
		Con_Printf( "cvarlist logged to %s\n", szTemp );
	}
}

int Cvar_CountServerVariables()
{
	int count = 0;

	for( cvar_t* pCvar = cvar_vars; pCvar; pCvar = pCvar->next )
	{
		if( pCvar->flags & FCVAR_SERVER )
			++count;
	}

	return count;
}

void Cvar_UnlinkExternals()
{
	if( !cvar_vars )
		return;

	cvar_t* pCvar = cvar_vars;
	cvar_t** ppNext = &cvar_vars;

	while( pCvar )
	{
		if( pCvar->flags & FCVAR_EXTDLL )
		{
			*ppNext = pCvar->next;
			pCvar = pCvar->next;
		}
		else
		{
			ppNext = &pCvar->next;
			pCvar = pCvar->next;
		}
	}
}

void Cvar_CmdInit()
{
	Cmd_AddCommand( "cvarlist", Cmd_CvarList_f );
}
