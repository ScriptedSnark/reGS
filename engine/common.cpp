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
// common.c -- misc functions used in client and server
#include <cctype>
#include <cstdarg>
#include <cstdio>

#include "quakedef.h"
#include "bspfile.h"
#include "cdll_int.h"
#include "modinfo.h"

char gpszProductString[ 32 ] = {};
char gpszVersionString[ 32 ] = {};

void SZ_Alloc( const char* name, sizebuf_t* buf, int startsize )
{
	if( startsize < 256 )
		startsize = 256;

	buf->buffername = name;
	buf->data = reinterpret_cast<byte*>( Hunk_AllocName( startsize, "sizebuf" ) );
	buf->maxsize = startsize;
	buf->cursize = 0;
	buf->flags = 0;
}

void SZ_Clear( sizebuf_t* buf )
{
	buf->cursize = 0;
	buf->flags &= ~FSB_OVERFLOWED;
}

void* SZ_GetSpace( sizebuf_t* buf, int length )
{
	if( buf->cursize + length > buf->maxsize )
	{
		const char* name = buf->buffername;

		if( !name )
			name = "???";

		if( !( buf->flags & FSB_ALLOWOVERFLOW ) )
		{
			if( buf->maxsize == 0 )
			{
				Sys_Error( "SZ_GetSpace:  Tried to write to an uninitialized sizebuf_t: %s", name );
			}

			Sys_Error( "SZ_GetSpace: overflow without FSB_ALLOWOVERFLOW set on %s", name );
		}

		if( length > buf->maxsize )
		{
			if( !( buf->flags & FSB_ALLOWOVERFLOW ) )
			{
				Sys_Error( "SZ_GetSpace: %i is > full buffer size on %s", length, name );
			}

			Con_DPrintf( "SZ_GetSpace: %i is > full buffer size on %s, ignoring", length, name );
		}

		Con_Printf( "SZ_GetSpace: overflow on %s\n", name );

		buf->cursize = length;
		buf->flags |= FSB_OVERFLOWED;

		return buf->data;
	}

	void* data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write( sizebuf_t* buf, const void* data, int length )
{
	byte* pSpace = reinterpret_cast<byte*>( SZ_GetSpace( buf, length ) );

	if( buf->flags & FSB_OVERFLOWED )
		return;

	Q_memcpy( pSpace, data, length );
}

void SZ_Print( sizebuf_t* buf, const char* data )
{
	const int len = Q_strlen( data ) + 1;

	// byte* cast to keep VC++ happy
	byte* pSpace = buf->data[ buf->cursize - 1 ] ? 
		reinterpret_cast<byte*>( SZ_GetSpace( buf, len ) ) :		// no trailing 0
		reinterpret_cast<byte*>( SZ_GetSpace( buf, len - 1 ) ) - 1;	// write over trailing 0

	if( buf->flags & FSB_OVERFLOWED )
		return;

	Q_memcpy( pSpace, data, len );
}

#define NUM_SAFE_ARGVS  7

static const char* largv[ MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1 ];
static const char* argvdummy = " ";

static const char* safeargvs[ NUM_SAFE_ARGVS ] =
{ "-stdvid", "-nolan", "-nosound", "-nocdaudio", "-nojoy", "-nomouse", "-dibonly" };

int		com_argc = 0;
const char** com_argv = nullptr;

#define CMDLINE_LENGTH	256
char	com_cmdline[ CMDLINE_LENGTH ];

char com_clientfallback[ FILENAME_MAX ] = {};
char com_gamedir[ FILENAME_MAX ] = {};

//TODO: on Windows com_token seems to be 2048 characters large. - Solokiller
char com_token[ 1024 ] = {};

static bool s_com_token_unget = false;

bool com_ignorecolons = false;

void COM_UngetToken()
{
	s_com_token_unget = true;
}

//Updated version of COM_Parse from Quake:
//Allows retrieving the last token by calling COM_UngetToken to mark it
//Supports Unicode
//Has buffer overflow checks
//Allows colons to be treated as regular characters using com_ignorecolons
char *COM_Parse( char *data )
{
	if( s_com_token_unget )
	{
		s_com_token_unget = false;

		return data;
	}

	int             len;

	len = 0;
	com_token[ 0 ] = 0;

	if( !data )
		return NULL;

	uchar32 wchar;

	// skip whitespace
skipwhite:
	while( !V_UTF8ToUChar32( data, wchar ) && wchar <= ' ' )
	{
		if( wchar == 0 )
			return NULL;                    // end of file;
		data = Q_UnicodeAdvance( data, 1 );
	}

	// skip // comments
	if( *data == '/' && data[ 1 ] == '/' )
	{
		while( *data && *data != '\n' )
			data++;
		goto skipwhite;
	}

	// handle quoted strings specially
	if( *data == '\"' )
	{
		data++;
		char c;
		while( len != ( ARRAYSIZE( com_token ) - 1 ) )
		{
			c = *data++;
			if( c == '\"' || !c )
			{
				break;
			}
			com_token[ len ] = c;
			len++;
		}

		com_token[ len ] = 0;
		return data;
	}

	// parse single characters
	if( *data == '{' || 
		*data == '}' || 
		*data == ')' ||
		*data == '(' ||
		*data == '\'' || 
		( !com_ignorecolons && *data == ':' ) )
	{
		com_token[ len ] = *data;
		len++;
		com_token[ len ] = 0;
		return data + 1;
	}

	char c;
	// parse a regular word
	do
	{
		com_token[ len ] = *data;
		data++;
		len++;
		c = *data;
		if( c == '{' || 
			c == '}' || 
			c == ')' || 
			c == '(' || 
			c == '\'' || 
			( !com_ignorecolons && c == ':' ) ||
			len >= ( ARRAYSIZE( com_token ) - 1 ) )
			break;
	}
	while( c>' ' );

	com_token[ len ] = 0;
	return data;
}

char* COM_ParseLine( char* data )
{
	if( s_com_token_unget )
	{
		s_com_token_unget = false;

		return data;
	}

	com_token[ 0 ] = 0;

	if( !data )
		return nullptr;

	int len = 0;

	char c = *data;

	do
	{
		com_token[ len ] = c;
		data++;
		len++;
		c = *data;
	}
	while( c>=' ' && len < ( ARRAYSIZE( com_token ) - 1 ) );

	com_token[ len ] = 0;

	//Skip unprintable characters.
	while( *data && *data < ' ' )
	{
		++data;
	}

	//End of data.
	if( *data == '\0' )
		return nullptr;

	return data;
}

//From SharedTokenWaiting - Solokiller
bool COM_TokenWaiting( const char* buffer )
{
	for( auto p = buffer; *p && *p != '\n'; ++p )
	{
		if( !isspace( *p ) || isalnum( *p ) )
			return true;
	}

	return false;
}

int COM_CheckParm( const char* parm )
{
	for( int i = 1; i<com_argc; i++ )
	{
		if( !com_argv[ i ] )
			continue;               // NEXTSTEP sometimes clears appkit vars.
		if( !Q_strcmp( parm, com_argv[ i ] ) )
			return i;
	}

	return 0;
}

void COM_InitArgv( int argc, const char** argv )
{
	int             i, n;

	// reconstitute the command line for the cmdline externally visible cvar
	n = 0;

	for( int j = 0; ( j<MAX_NUM_ARGVS ) && ( j< argc ); j++ )
	{
		i = 0;

		while( ( n < ( CMDLINE_LENGTH - 1 ) ) && argv[ j ][ i ] )
		{
			com_cmdline[ n++ ] = argv[ j ][ i++ ];
		}

		if( n < ( CMDLINE_LENGTH - 1 ) )
			com_cmdline[ n++ ] = ' ';
		else
			break;
	}

	com_cmdline[ n ] = 0;

	bool safe = false;

	for( com_argc = 0; ( com_argc<MAX_NUM_ARGVS ) && ( com_argc < argc );
		 com_argc++ )
	{
		largv[ com_argc ] = argv[ com_argc ];
		if( !Q_strcmp( "-safe", argv[ com_argc ] ) )
			safe = true;
	}

	if( safe )
	{
		// force all the safe-mode switches. Note that we reserved extra space in
		// case we need to add these, so we don't need an overflow check
		for( i = 0; i<NUM_SAFE_ARGVS; i++ )
		{
			largv[ com_argc ] = safeargvs[ i ];
			com_argc++;
		}
	}

	largv[ com_argc ] = argvdummy;
	com_argv = largv;
}

void COM_Init()
{
	//TODO: implement - Solokiller
}

void COM_Shutdown()
{
	//Nothing
}

void COM_FileBase( const char *in, char *out )
{
	if( !in )
		return;

	if( !*in )
		return;

	size_t uiLength = strlen( in );

	// scan backward for '.'
	size_t end = uiLength - 1;
	while( end && in[ end ] != '.' && in[ end ] != '/' && in[ end ] != '\\' )
		--end;

	if( in[ end ] != '.' )	// no '.', copy to end
		end = uiLength - 1;
	else
		--end;				// Found ',', copy to left of '.'


	// Scan backward for '/'
	size_t start = uiLength;
	while( start > 0 && in[ start - 1 ] != '/' && in[ start - 1 ] != '\\' )
		--start;

	// Length of new string
	uiLength = end - start + 1;

	// Copy partial string
	strncpy( out, &in[ start ], uiLength );
	// Terminate it
	out[ uiLength ] = '\0';
}

bool COM_SetupDirectories()
{
	com_clientfallback[ 0 ] = '\0';
	com_gamedir[ 0 ] = '\0';

	char basedir[ 512 ];

	COM_ParseDirectoryFromCmd( "-basedir", basedir, "valve" );
	COM_ParseDirectoryFromCmd( "-game", com_gamedir, basedir );

	bool bResult = FileSystem_SetGameDirectory( basedir, com_gamedir[ 0 ] ? com_gamedir : nullptr );

	if( bResult )
	{
		//TODO: serverinfo is only 256 characters large, but 512 is passed in. - Solokiller
		Info_SetValueForStarKey( serverinfo, "*gamedir", com_gamedir, 512 );
	}

	return bResult;
}

void COM_ParseDirectoryFromCmd( const char *pCmdName, char *pDirName, const char *pDefault )
{
	const char* pszResult = nullptr;

	if( com_argc > 1 )
	{
		for( int arg = 1; arg < com_argc; ++arg )
		{
			auto pszArg = com_argv[ arg ];

			if( pszArg )
			{
				if( pCmdName && *pCmdName == *pszArg )
				{
					if( *pCmdName && !strcmp( pCmdName, pszArg ) )
					{
						if( arg < com_argc - 1 )
						{
							auto pszValue = com_argv[ arg + 1 ];

							if( *pszValue != '+' && *pszValue != '-' )
							{
								strcpy( pDirName, pszValue );
								pszResult = pDirName;
								break;
							}
						}
					}
				}
			}
		}
	}

	if( !pszResult )
	{
		if( pDefault )
		{
			strcpy( pDirName, pDefault );
			pszResult = pDirName;
		}
		else
		{
			pszResult = pDirName;
			*pDirName = '\0';
		}
	}

	const auto uiLength = strlen( pszResult );

	if( uiLength > 0 )
	{
		auto pszEnd = &pDirName[ uiLength - 1 ];
		if( *pszEnd == '/' || *pszEnd == '\\' )
			*pszEnd = '\0';
	}
}

void COM_FixSlashes( char *pname )
{
	for( char* pszNext = pname; *pszNext; ++pszNext )
	{
		if( *pszNext == '\\' )
			*pszNext = '/';
	}
}

void COM_AddDefaultDir( const char* pszDir )
{
	if( pszDir && *pszDir )
	{
		FileSystem_AddFallbackGameDir( pszDir );
	}
}

void COM_AddAppDirectory( const char* pszBaseDir )
{
	FS_AddSearchPath( pszBaseDir, "PLATFORM" );
}

const char* COM_FileExtension( const char* in )
{
	static char exten[ 8 ];

	for( const char* pszExt = in; *pszExt; ++pszExt )
	{
		if( *pszExt == '.' )
		{
			//Skip the '.'
			++pszExt;

			size_t uiIndex;

			for( uiIndex = 0; *pszExt && uiIndex < ARRAYSIZE( exten ) - 1; ++pszExt, ++uiIndex )
			{
				exten[ uiIndex ] = *pszExt;
			}

			exten[ uiIndex ] = '\0';
			return exten;
		}
	}

	return "";
}

void COM_DefaultExtension( char* path, char* extension )
{
	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	char* src = path + strlen( path ) - 1;

	while( *src != '/' && src != path )
	{
		if( *src == '.' )
			return;                 // it has an extension
		src--;
	}

	//TODO: define this constant, it's 260 on Linux as well - Solokiller
	strncat( path, extension, 260 - strlen( src ) );
}

void COM_StripExtension( char* in, char* out )
{
	if( !in || !( *in ) )
		return;

	const auto uiLength = strlen( in );

	auto pszSrc = in + uiLength - 1;
	auto pszDest = out + uiLength - 1;

	bool bHandledExt = false;

	for( ; in <= pszSrc; --pszSrc, --pszDest )
	{
		if( bHandledExt || *pszSrc != '.' )
		{
			if( *pszSrc == '\\' || *pszSrc == '/' )
				bHandledExt = true;

			*pszSrc = *pszDest;
		}
		else
		{
			*pszSrc = '\0';
			bHandledExt = true;
		}
	}
}

void COM_StripTrailingSlash( char* ppath )
{
	const auto uiLength = strlen( ppath );

	if( uiLength > 0 )
	{
		auto pszEnd = &ppath[ uiLength - 1 ];

		if( *pszEnd == '/' || *pszEnd == '\\' )
		{
			*pszEnd = '\0';
		}
	}
}

void COM_CreatePath( char* path )
{
	//TODO: check if null or empty - Solokiller
	//TODO: copy into temp buffer - Solokiller
	auto pszNext = path + 1;

	while( *pszNext )
	{
		if( *pszNext == '\\' || *pszNext == '/' )
		{
			const auto cSave = *pszNext;
			*pszNext = '\0';

			FS_CreateDirHierarchy( path, nullptr );

			*pszNext = cSave;
		}

		++pszNext;
	}
}

unsigned int COM_GetApproxWavePlayLength( const char* filepath )
{
	//TODO: implement - Solokiller
	return 0;
}

char* Info_Serverinfo()
{
	return serverinfo;
}

char* va( const char* format, ... )
{
	static char string[ 1024 ];

	va_list argptr;

	va_start( argptr, format );
	vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );

	return string;
}

const int MAX_VEC_STRINGS = 16;

char* vstr( vec_t* v )
{
	static char string[ MAX_VEC_STRINGS ][ 1024 ];
	static int idx = 0;

	idx = ( idx + 1 ) % MAX_VEC_STRINGS;

	snprintf( string[ idx ], ARRAYSIZE( string[ idx ] ), "%.4f %.4f %.4f", v[ 0 ], v[ 1 ], v[ 2 ] );

	return string[ idx ];
}

int memsearch( byte* start, int count, int search )
{
	if( count <= 0 )
		return -1;

	int result = 0;

	if( *start != search )
	{
		while( 1 )
		{
			++result;
			if( result == count )
				break;

			if( start[ result ] == search )
				return result;
		}

		result = -1;
	}

	return result;
}

int Q_FileNameCmp( const char* file1, const char* file2 )
{
	for( auto pszLhs = file1, pszRhs = file2; *pszLhs && *pszRhs; ++pszLhs, ++pszRhs )
	{
		if( ( *pszLhs != '/' || *pszRhs != '\\' ) &&
			( *pszLhs != '\\' || *pszRhs != '/' ) )
		{
			if( tolower( *pszLhs ) != tolower( *pszRhs ) )
				return -1;
		}
	}

	return 0;
}

static cache_user_t* loadcache = nullptr;
static byte* loadbuf = nullptr;
static int loadsize = 0;

byte* COM_LoadFile( const char* path, int usehunk, int* pLength )
{
	g_engdstAddrs.COM_LoadFile( const_cast<char**>( &path ), &usehunk, &pLength );

	if( pLength )
		*pLength = 0;

	FileHandle_t hFile = FS_Open( path, "rb" );

	if( hFile == FILESYSTEM_INVALID_HANDLE )
		return nullptr;

	const int iSize = FS_Size( hFile );

	char base[ 4096 ];
	COM_FileBase( path, base );
	base[ 32 ] = '\0';

	void* pBuffer = 0;

	if( usehunk == 0 )
	{
		pBuffer = Z_Malloc( iSize + 1 );
	}
	else if( usehunk == 1 )
	{
		pBuffer = Hunk_AllocName( iSize + 1, base );
	}
	else if( usehunk == 2 )
	{
		pBuffer = Hunk_TempAlloc( iSize + 1 );
	}
	else if( usehunk == 3 )
	{
		pBuffer = Cache_Alloc( loadcache, iSize + 1, base );
	}
	else if( usehunk == 4 )
	{
		pBuffer = loadbuf;

		if( iSize >= loadsize )
		{
			pBuffer = Hunk_TempAlloc( iSize + 1 );
		}
	}
	else if( usehunk == 5 )
	{
		pBuffer = Mem_Malloc( iSize + 1 );
	}
	else
	{
		Sys_Error( "COM_LoadFile: bad usehunk" );
	}

	if( !pBuffer )
		Sys_Error( "COM_LoadFile: not enough space for %s", path );

	*( ( byte* ) pBuffer + iSize ) = '\0';

	FS_Read( pBuffer, iSize, hFile );
	FS_Close( hFile );

	if( pLength )
		*pLength = iSize;

	return ( byte * ) pBuffer;
}

void COM_FreeFile( void *buffer )
{
	g_engdstAddrs.COM_FreeFile( &buffer );

	if( buffer )
		Mem_Free( buffer );
}

byte* COM_LoadHunkFile( const char* path )
{
	return COM_LoadFile( path, 1, nullptr );
}

byte* COM_LoadTempFile( const char* path, int* pLength )
{
	return COM_LoadFile( path, 2, pLength );
}

void COM_LoadCacheFile( const char* path, cache_user_t* cu )
{
	loadcache = cu;
	COM_LoadFile( path, 3, nullptr );
}

byte* COM_LoadStackFile( const char* path, void* buffer, int bufsize, int* length )
{
	loadbuf = reinterpret_cast<byte*>( buffer );
	loadsize = bufsize;

	return COM_LoadFile( path, 4, length );
}

byte* COM_LoadFileForMe( const char* filename, int* pLength )
{
	return COM_LoadFile( filename, 5, pLength );
}

byte* COM_LoadFileLimit( const char* path, int pos, int cbmax, int* pcbread, FileHandle_t* phFile )
{
	auto hFile = *phFile;

	if( FILESYSTEM_INVALID_HANDLE == *phFile )
	{
		hFile = FS_Open( path, "rb" );
	}

	byte* pData = nullptr;

	if( FILESYSTEM_INVALID_HANDLE != hFile )
	{
		const auto uiSize = FS_Size( hFile );

		if( uiSize < static_cast<decltype( uiSize )>( pos ) )
			Sys_Error( "COM_LoadFileLimit: invalid seek position for %s", path );

		FS_Seek( hFile, pos, FILESYSTEM_SEEK_HEAD );

		*pcbread = min( cbmax, static_cast<int>( uiSize ) );

		char base[ 32 ];

		if( path )
		{
			COM_FileBase( path, base );
		}

		pData = reinterpret_cast<byte*>( Hunk_TempAlloc( *pcbread + 1 ) );

		if( pData )
		{
			pData[ uiSize ] = '\0';

			FS_Read( pData, uiSize, hFile );
		}
		else
		{
			if( path )
				Sys_Error( "COM_LoadFileLimit: not enough space for %s", path );

			FS_Close( hFile );
		}
	}

	*phFile = hFile;

	return pData;
}

void COM_WriteFile( char* filename, void* data, int len )
{
	char name[ MAX_PATH ];
	snprintf( name, ARRAYSIZE( name ), "%s", filename );

	COM_FixSlashes( name );
	COM_CreatePath( name );

	auto hFile = FS_Open( name, "wb" );

	if( FILESYSTEM_INVALID_HANDLE != hFile )
	{
		Sys_Printf( "COM_WriteFile: %s\n", name );
		FS_Write( data, len, hFile );
		FS_Close( hFile );
	}
	else
	{
		Sys_Printf( "COM_WriteFile: failed on %s\n", name );
	}
}

int COM_FileSize( const char* filename )
{
	int result;

	auto hFile = FS_Open( filename, "rb" );

	if( FILESYSTEM_INVALID_HANDLE != hFile )
	{
		result = FS_Size( hFile );
		FS_Close( hFile );
	}
	else
	{
		result = -1;
	}

	return result;
}

bool COM_ExpandFilename( char* filename )
{
	char netpath[ MAX_PATH ];

	FS_GetLocalPath( filename, netpath, ARRAYSIZE( netpath ) );
	strcpy( filename, netpath );

	return *filename != '\0';
}

int COM_CompareFileTime( const char* filename1, const char* filename2, int* iCompare )
{
	*iCompare = 0;

	if( filename1 && filename2 )
	{
		const auto iLhs = FS_GetFileTime( filename1 );
		const auto iRhs = FS_GetFileTime( filename2 );

		if( iLhs < iRhs )
		{
			*iCompare = -1;
		}
		else if( iLhs > iRhs )
		{
			*iCompare = 1;
		}

		return true;
	}

	return false;
}

void COM_CopyFile( const char* netpath, char* cachepath )
{
	auto hSrcFile = FS_Open( netpath, "rb" );

	if( FILESYSTEM_INVALID_HANDLE != hSrcFile )
	{
		auto uiSize = FS_Size( hSrcFile );

		//TODO: copy path instead of modifying original - Solokiller

		COM_CreatePath( cachepath );

		auto hDestFile = FS_Open( cachepath, "wb" );

		//TODO: check if file failed to open - Solokiller

		char buf[ 4096 ];

		while( uiSize >= sizeof( buf ) )
		{
			FS_Read( buf, sizeof( buf ), hSrcFile );
			FS_Write( buf, sizeof( buf ), hDestFile );

			uiSize -= sizeof( buf );
		}

		if( uiSize )
		{
			FS_Read( buf, uiSize, hSrcFile );
			FS_Write( buf, uiSize, hDestFile );
		}

		FS_Close( hSrcFile );
		FS_Close( hDestFile );
	}
}

void COM_CopyFileChunk( FileHandle_t dst, FileHandle_t src, int nSize )
{
	char copybuf[ 1024 ];

	auto iSizeLeft = nSize;

	if( iSizeLeft > sizeof( copybuf ) )
	{
		while( iSizeLeft > sizeof( copybuf ) )
		{
			FS_Read( copybuf, sizeof( copybuf ), src );
			FS_Write( copybuf, sizeof( copybuf ), dst );
			iSizeLeft -= sizeof( copybuf );
		}

		//Compute size left
		iSizeLeft = nSize - ( ( nSize - ( sizeof( copybuf ) + 1 ) ) & ~( sizeof( copybuf ) - 1 ) ) - sizeof( copybuf );
	}

	FS_Read( copybuf, iSizeLeft, src );
	FS_Write( copybuf, iSizeLeft, dst );
	FS_Flush( src );
	FS_Flush( dst );
}

void COM_Log( const char* pszFile, const char* fmt, ... )
{
	char string[ 1024 ];
	va_list va;

	va_start( va, fmt );

	auto pszFileName = pszFile ? pszFile : "c:\\hllog.txt";

	vsnprintf( string, ARRAYSIZE( string ), fmt, va );
	va_end( va );

	auto hFile = FS_Open( pszFileName, "a+t" );

	if( FILESYSTEM_INVALID_HANDLE != hFile )
	{
		FS_FPrintf( hFile, "%s", string );
		FS_Close( hFile );
	}
}

void COM_ListMaps( const char* pszSubString )
{
	const size_t uiSubStrLength = pszSubString && *pszSubString ? strlen( pszSubString ) : 0;

	Con_Printf( "-------------\n" );
	int showOutdated = 1;

	char curDir[ MAX_PATH ];
	char mapwild[ 64 ];
	char sz[ 64 ];

	dheader_t header;

	do
	{
		strcpy( mapwild, "maps/*.bsp" );
		for( auto i = Sys_FindFirst( mapwild, nullptr ); i; i = Sys_FindNext( nullptr ) )
		{
			snprintf( curDir, ARRAYSIZE( curDir ), "maps/%s", i );
			FS_GetLocalPath( curDir, curDir, ARRAYSIZE( curDir ) );
			if( strstr( curDir, com_gamedir ) && ( !uiSubStrLength || !strnicmp( i, pszSubString, uiSubStrLength ) ) )
			{
				memset( &header, 0, sizeof( header ) );

				sprintf( sz, "maps/%s", i );

				auto hFile = FS_Open( sz, "rb" );

				if( hFile )
				{
					FS_Read( &header, sizeof( header ), hFile );
					FS_Close( hFile );
				}

				//TODO: shouldn't this be calling LittleLong? - Solokiller
				if( header.version == BSPVERSION )
				{
					if( !showOutdated )
						Con_Printf( "%s\n", i );
				}
				else if( showOutdated )
				{
					Con_Printf( "OUTDATED:  %s\n", i );
				}
			}
		}
		Sys_FindClose();
		--showOutdated;
	}
	while( showOutdated != -1 );
}

void COM_GetGameDir( char* szGameDir )
{
	//TODO: define this particular limit. It's 260 for Linux as well - Solokiller
	if( szGameDir )
		snprintf( szGameDir, 259U, "%s", com_gamedir );
}

const char* COM_SkipPath( const char* pathname )
{
	auto pszLast = pathname;

	for( auto pszPath = pathname; *pszPath; ++pszPath )
	{
		if( *pszPath == '\\' || *pszPath == '/' )
		{
			pszLast = pszPath;
		}
	}

	return pszLast;
}

char* COM_BinPrintf( byte* buf, int nLen )
{
	static char szReturn[ 4096 ];

	memset( szReturn, 0, sizeof( szReturn ) );

	char szChunk[ 10 ];
	for( int i = 0; i < nLen; ++i )
	{
		snprintf( szChunk, ARRAYSIZE( szChunk ), "%02x", buf[ i ] );

		strncat( szReturn, szChunk, ARRAYSIZE( szReturn ) - 1 - strlen( szReturn ) );
	}

	return szReturn;
}

unsigned char COM_Nibble( char c )
{
	if( ( c >= '0' ) &&
		( c <= '9' ) )
	{
		return ( unsigned char ) ( c - '0' );
	}

	if( ( c >= 'A' ) &&
		( c <= 'F' ) )
	{
		return ( unsigned char ) ( c - 'A' + 0x0a );
	}

	if( ( c >= 'a' ) &&
		( c <= 'f' ) )
	{
		return ( unsigned char ) ( c - 'a' + 0x0a );
	}

	return '0';
}

void COM_HexConvert( const char* pszInput, int nInputLength, byte* pOutput )
{
	const auto iBytes = ( ( max( nInputLength - 1, 0 ) ) / 2 ) + 1;

	auto p = pszInput;

	for( int i = 0; i < iBytes; ++i, ++p )
	{
		pOutput[ i ] = ( 0x10 * COM_Nibble( p[ 0 ] ) ) | COM_Nibble( p[ 1 ] );
	}
}

void COM_NormalizeAngles( vec_t* angles )
{
	for( int i = 0; i < 3; ++i )
	{
		if( angles[ i ] < -180.0 )
		{
			angles[ i ] = fmod( angles[ i ], 360.0 ) + 360.0;
		}
		else if( angles[ i ] > 180.0 )
		{
			angles[ i ] = fmod( angles[ i ], 360.0 ) - 360.0;
		}
	}
}

int COM_EntsForPlayerSlots( int nPlayers )
{
	int num_edicts = gmodinfo.num_edicts;

	const auto parm = COM_CheckParm( "-num_edicts" );

	if( parm != 0 )
	{
		const auto iSetting = atoi( com_argv[ parm + 1 ] );

		if( num_edicts <= iSetting )
			num_edicts = iSetting;
	}

	//TODO: this can exceed MAX_EDICTS - Solokiller
	return num_edicts + 15 * ( nPlayers - 1 );
}

void COM_ExplainDisconnection( bool bPrint, const char* fmt, ... )
{
	static char string[ 1024 ];

	va_list va;

	va_start( va, fmt );
	vsnprintf( string, ARRAYSIZE( string ), fmt, va );
	va_end( va );

	strncpy( gszDisconnectReason, string, ARRAYSIZE( gszDisconnectReason ) - 1 );
	gszDisconnectReason[ ARRAYSIZE( gszDisconnectReason ) - 1 ] = '\0';

	gfExtendedError = true;

	if( bPrint )
	{
		if( gszDisconnectReason[ 0 ] != '#' )
			Con_Printf( "%s\n", gszDisconnectReason );
	}
}

void COM_ExtendedExplainDisconnection( bool bPrint, const char* fmt, ... )
{
	static char string[ 1024 ];

	va_list va;

	va_start( va, fmt );
	vsnprintf( string, ARRAYSIZE( string ), fmt, va );
	va_end( va );

	strncpy( gszExtendedDisconnectReason, string, ARRAYSIZE( gszExtendedDisconnectReason ) - 1 );
	gszExtendedDisconnectReason[ ARRAYSIZE( gszExtendedDisconnectReason ) - 1 ] = '\0';

	if( bPrint )
	{
		if( gszExtendedDisconnectReason[ 0 ] != '#' )
			Con_Printf( "%s\n", gszExtendedDisconnectReason );
	}
}

/*
==============================================================================

MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

//
// writing functions
//

void MSG_WriteChar( sizebuf_t *sb, int c )
{
	byte    *buf;

#ifdef PARANOID
	if( c < -128 || c > 127 )
		Sys_Error( "MSG_WriteChar: range error" );
#endif

	buf = reinterpret_cast<byte*>( SZ_GetSpace( sb, 1 ) );
	buf[ 0 ] = c;
}

void MSG_WriteByte( sizebuf_t *sb, int c )
{
	byte    *buf;

#ifdef PARANOID
	if( c < 0 || c > 255 )
		Sys_Error( "MSG_WriteByte: range error" );
#endif

	buf = reinterpret_cast<byte*>( SZ_GetSpace( sb, 1 ) );
	buf[ 0 ] = c;
}

void MSG_WriteShort( sizebuf_t *sb, int c )
{
	byte    *buf;

#ifdef PARANOID
	if( c < ( ( short ) 0x8000 ) || c >( short )0x7fff )
		Sys_Error( "MSG_WriteShort: range error" );
#endif

	buf = reinterpret_cast<byte*>( SZ_GetSpace( sb, 2 ) );
	buf[ 0 ] = c & 0xff;
	buf[ 1 ] = c >> 8;
}

void MSG_WriteLong( sizebuf_t *sb, int c )
{
	byte    *buf;

	buf = reinterpret_cast<byte*>( SZ_GetSpace( sb, 4 ) );
	buf[ 0 ] = c & 0xff;
	buf[ 1 ] = ( c >> 8 ) & 0xff;
	buf[ 2 ] = ( c >> 16 ) & 0xff;
	buf[ 3 ] = c >> 24;
}

void MSG_WriteFloat( sizebuf_t *sb, float f )
{
	union
	{
		float   f;
		int     l;
	} dat;


	dat.f = f;
	dat.l = LittleLong( dat.l );

	SZ_Write( sb, &dat.l, 4 );
}

void MSG_WriteString( sizebuf_t *sb, char *s )
{
	if( !s )
		SZ_Write( sb, "", 1 );
	else
		SZ_Write( sb, s, Q_strlen( s ) + 1 );
}

void MSG_WriteCoord( sizebuf_t *sb, float f )
{
	MSG_WriteShort( sb, ( int ) ( f * 8 ) );
}

void MSG_WriteAngle( sizebuf_t *sb, float f )
{
	MSG_WriteByte( sb, ( ( int ) f * 256 / 360 ) & 255 );
}

//
// reading functions
//
int msg_readcount = 0;
bool msg_badread = false;

void MSG_BeginReading()
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar()
{
	int     c;

	if( msg_readcount + 1 > net_message.cursize )
	{
		msg_badread = true;
		return -1;
	}

	c = ( signed char ) net_message.data[ msg_readcount ];
	msg_readcount++;

	return c;
}

int MSG_ReadByte()
{
	int     c;

	if( msg_readcount + 1 > net_message.cursize )
	{
		msg_badread = true;
		return -1;
	}

	c = ( unsigned char ) net_message.data[ msg_readcount ];
	msg_readcount++;

	return c;
}

int MSG_ReadShort()
{
	int     c;

	if( msg_readcount + 2 > net_message.cursize )
	{
		msg_badread = true;
		return -1;
	}

	c = ( short ) ( net_message.data[ msg_readcount ]
					+ ( net_message.data[ msg_readcount + 1 ] << 8 ) );

	msg_readcount += 2;

	return c;
}

int MSG_ReadLong()
{
	int     c;

	if( msg_readcount + 4 > net_message.cursize )
	{
		msg_badread = true;
		return -1;
	}

	c = net_message.data[ msg_readcount ]
		+ ( net_message.data[ msg_readcount + 1 ] << 8 )
		+ ( net_message.data[ msg_readcount + 2 ] << 16 )
		+ ( net_message.data[ msg_readcount + 3 ] << 24 );

	msg_readcount += 4;

	return c;
}

float MSG_ReadFloat()
{
	union
	{
		byte    b[ 4 ];
		float   f;
		int     l;
	} dat;

	dat.b[ 0 ] = net_message.data[ msg_readcount ];
	dat.b[ 1 ] = net_message.data[ msg_readcount + 1 ];
	dat.b[ 2 ] = net_message.data[ msg_readcount + 2 ];
	dat.b[ 3 ] = net_message.data[ msg_readcount + 3 ];
	msg_readcount += 4;

	dat.l = LittleLong( dat.l );

	return dat.f;
}

char* MSG_ReadString()
{
	static char     string[ 2048 ];
	int             l, c;

	l = 0;
	do
	{
		c = MSG_ReadChar();
		if( c == -1 || c == 0 )
			break;
		string[ l ] = c;
		l++;
	}
	while( l < sizeof( string ) - 1 );

	string[ l ] = 0;

	return string;
}

float MSG_ReadCoord()
{
	return MSG_ReadShort() * ( 1.0 / 8 );
}

float MSG_ReadAngle()
{
	return MSG_ReadChar() * ( 360.0 / 256 );
}
