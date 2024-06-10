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
#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#include <cstdio>

#include "tier0/platform.h"

/**
*	@file
*
*	general definitions
*/

enum FSB
{
	/**
	*	If not set, do a Sys_Error
	*/
	FSB_ALLOWOVERFLOW	=	1	<<	0,

	/**
	*	set if the buffer size failed
	*/
	FSB_OVERFLOWED		=	1	<<	1
};

typedef struct sizebuf_s
{
	const char* buffername;
	unsigned short flags;
	byte* data;
	int maxsize;
	int cursize;
} sizebuf_t;

void SZ_Alloc( const char* name, sizebuf_t* buf, int startsize );
void SZ_Clear( sizebuf_t* buf );
void* SZ_GetSpace( sizebuf_t* buf, int length );
void SZ_Write( sizebuf_t* buf, const void* data, int length );

/**
*	strcats onto the sizebuf
*/
void SZ_Print( sizebuf_t* buf, const char* data );

extern char gpszProductString[ 32 ];
extern char gpszVersionString[ 32 ];

extern int com_argc;
extern const char** com_argv;
extern char com_gamedir[ FILENAME_MAX ];

extern char com_token[ 1024 ];

/**
*	If true, colons are treated as regular characters, instead of being parsed as single characters.
*/
extern bool com_ignorecolons;

void COM_UngetToken();

/**
*	Parse a token out of a string
*/
char *COM_Parse( char *data );

/**
*	Parse a line out of a string. Used to parse out lines out of cfg files
*/
char* COM_ParseLine( char* data );

bool COM_TokenWaiting( const char* buffer );

/**
*	Returns the position (1 to argc-1) in the program's argument list
*	where the given parameter apears, or 0 if not present
*/
int COM_CheckParm( const char* parm );
void COM_InitArgv( int argc, const char** argv );
void COM_Init();
void COM_Shutdown();

void COM_FileBase( const char *in, char *out );

bool COM_SetupDirectories();

void COM_ParseDirectoryFromCmd( const char *pCmdName, char *pDirName, const char *pDefault );

void COM_FixSlashes( char *pname );

void COM_AddDefaultDir( const char* pszDir );

void COM_AddAppDirectory( const char* pszBaseDir );

const char* COM_FileExtension( const char* in );

void COM_DefaultExtension( char* path, char* extension );

void COM_StripExtension( char* in, char* out );

void COM_StripTrailingSlash( char* ppath );

/**
*	Creates a hierarchy of directories specified by path
*	Modifies the given string while performing this operation, but restores it to its original state
*/
void COM_CreatePath( char* path );

unsigned int COM_GetApproxWavePlayLength( const char* filepath );

char* Info_Serverinfo();

/**
*	does a varargs printf into a temp buffer, so I don't need to have
*	varargs versions of all text functions.
*	FIXME: make this buffer size safe someday
*/
char* va( const char* format, ... );

/**
*	Converts a vector to a string representation.
*/
char* vstr( vec_t* v );

/**
*	Searches for a byte of data in a binary buffer
*/
int memsearch( byte* start, int count, int search );

/**
*	Compares filenames
*	@return -1 if file1 is not equal to file2, 0 otherwise
*/
int Q_FileNameCmp( const char* file1, const char* file2 );

byte* COM_LoadFile( const char* path, int usehunk, int* pLength );

void COM_FreeFile( void *buffer );

byte* COM_LoadHunkFile( const char* path );

byte* COM_LoadTempFile( const char* path, int* pLength );

void COM_LoadCacheFile( const char* path, cache_user_t* cu );

byte* COM_LoadStackFile( const char* path, void* buffer, int bufsize, int* length );

byte* COM_LoadFileForMe( const char* filename, int* pLength );

byte* COM_LoadFileLimit( const char* path, int pos, int cbmax, int* pcbread, FileHandle_t* phFile );

void COM_WriteFile( char* filename, void* data, int len );

int COM_FileSize( const char* filename );

bool COM_ExpandFilename( char* filename );

int COM_CompareFileTime( const char* filename1, const char* filename2, int* iCompare );

/**
*	@param cachepath Modified by the function but restored before returning
*/
void COM_CopyFile( const char* netpath, char* cachepath );

void COM_CopyFileChunk( FileHandle_t dst, FileHandle_t src, int nSize );

void COM_Log( const char* pszFile, const char* fmt, ... );

void COM_ListMaps( const char* pszSubString );

void COM_GetGameDir( char* szGameDir );

const char* COM_SkipPath( const char* pathname );

char* COM_BinPrintf( byte* buf, int nLen );

unsigned char COM_Nibble( char c );

void COM_HexConvert( const char* pszInput, int nInputLength, byte* pOutput );

/**
*	Normalizes the angles to a range of [ -180, 180 ]
*/
void COM_NormalizeAngles( vec_t* angles );

int COM_EntsForPlayerSlots( int nPlayers );

/**
*	Set explanation for disconnection
*	@param bPrint Whether to print the explanation to the console
*/
void COM_ExplainDisconnection( bool bPrint, const char* fmt, ... );

/**
*	Set extended explanation for disconnection
*	Only used if COM_ExplainDisconnection has been called as well
*	@param bPrint Whether to print the explanation to the console
*/
void COM_ExtendedExplainDisconnection( bool bPrint, const char* fmt, ... );

void MSG_WriteChar( sizebuf_t *sb, int c );
void MSG_WriteByte( sizebuf_t *sb, int c );
void MSG_WriteShort( sizebuf_t *sb, int c );
void MSG_WriteLong( sizebuf_t *sb, int c );
void MSG_WriteFloat( sizebuf_t *sb, float f );
void MSG_WriteString( sizebuf_t *sb, char *s );
void MSG_WriteCoord( sizebuf_t *sb, float f );
void MSG_WriteAngle( sizebuf_t *sb, float f );

extern int msg_readcount;

/**
*	set if a read goes beyond end of message
*/
extern bool msg_badread;

void MSG_BeginReading();
int MSG_ReadChar();
int MSG_ReadByte();
int MSG_ReadShort();
int MSG_ReadLong();
float MSG_ReadFloat();
char *MSG_ReadString();

float MSG_ReadCoord();
float MSG_ReadAngle();

#endif //ENGINE_COMMON_H
