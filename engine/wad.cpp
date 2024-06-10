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
#include "wad.h"

static wadlist_t wads[ 2 ];

void SwapPic( qpic_t* pic );

/*
==================
W_CleanupName

Lowercases name and pads with spaces and a terminating 0 to the length of
lumpinfo_t->name.
Used so lumpname lookups can proceed rapidly by comparing 4 chars at a time
Space padding is so names can be printed nicely in tables.
Can safely be performed in place.
==================
*/
void W_CleanupName( const char* in, char* out )
{
	int		i;
	int		c;

	for( i = 0; i<16; i++ )
	{
		c = in[ i ];
		if( !c )
			break;

		if( c >= 'A' && c <= 'Z' )
			c += ( 'a' - 'A' );
		out[ i ] = c;
	}

	for( ; i< 16; i++ )
		out[ i ] = '\0';
}



/*
====================
W_LoadWadFile
====================
*/
int W_LoadWadFile( const char* filename )
{
	lumpinfo_t		*lump_p;
	wadinfo_t		*header;
	unsigned		i;
	int				infotableofs;

	int iWadIndex;

	if( !wads[ 0 ].loaded )
		iWadIndex = 0;
	else if( !wads[ 1 ].loaded )
		iWadIndex = 1;
	else
	{
		Con_Printf( "No room for wad %s\n", filename );
		return -1;
	}

	wadlist_t* pList = &wads[ iWadIndex ];

	pList->wad_base = COM_LoadHunkFile( filename );
	if( !pList->wad_base )
	{
		if( iWadIndex == 0 )
			Sys_Error( "W_LoadWadFile: couldn't load %s", filename );

		Con_Printf( "WARNING:  W_LoadWadFile, couldn't load %s\n", filename );
		return -1;
	}

	Q_strncpy( pList->wadname, filename, ARRAYSIZE( pList->wadname ) );

	pList->loaded = true;

	header = ( wadinfo_t * ) pList->wad_base;

	if( header->identification[ 0 ] != 'W'
		|| header->identification[ 1 ] != 'A'
		|| header->identification[ 2 ] != 'D'
		|| header->identification[ 3 ] != '3' )
		Sys_Error( "Wad file %s doesn't have WAD3 id\n", filename );

	pList->wad_numlumps = LittleLong( header->numlumps );
	infotableofs = LittleLong( header->infotableofs );
	pList->wad_lumps = ( lumpinfo_t * ) ( pList->wad_base + infotableofs );

	for( i = 0, lump_p = pList->wad_lumps; i<static_cast<unsigned int>( pList->wad_numlumps ); i++, lump_p++ )
	{
		lump_p->filepos = LittleLong( lump_p->filepos );
		lump_p->size = LittleLong( lump_p->size );
		W_CleanupName( lump_p->name, lump_p->name );
		if( lump_p->type == TYP_QPIC )
			SwapPic( ( qpic_t * ) ( pList->wad_base + lump_p->filepos ) );
	}

	return iWadIndex;
}


/*
=============
W_GetLumpinfo
=============
*/
lumpinfo_t* W_GetLumpinfo( int wad, const char* name, bool doerror )
{
	int		i;
	lumpinfo_t	*lump_p;
	char	clean[ 16 ];

	W_CleanupName( name, clean );

	for( lump_p = wads[ wad ].wad_lumps, i = 0; i<wads[ wad ].wad_numlumps; i++, lump_p++ )
	{
		if( !strcmp( clean, lump_p->name ) )
			return lump_p;
	}

	if( doerror )
		Sys_Error( "W_GetLumpinfo: %s not found", name );

	return nullptr;
}

void* W_GetLumpName( int wad, const char* name )
{
	lumpinfo_t	*lump;

	lump = W_GetLumpinfo( wad, name, true );

	return ( void * ) ( wads[ wad ].wad_base + lump->filepos );
}

void* W_GetLumpNum( int wad, int num )
{
	lumpinfo_t	*lump;

	if( num < 0 || num > wads[ wad ].wad_numlumps )
		Sys_Error( "W_GetLumpNum: bad number: %i", num );

	lump = wads[ wad ].wad_lumps + num;

	return ( void * ) ( wads[ wad ].wad_base + lump->filepos );
}

/*
=============================================================================

automatic byte swapping

=============================================================================
*/

void SwapPic( qpic_t* pic )
{
	pic->width = LittleLong( pic->width );
	pic->height = LittleLong( pic->height );
}

void W_Shutdown()
{
	if( wads[ 0 ].loaded )
	{
		memset( &wads[ 0 ], 0, sizeof( wads[ 0 ] ) );

		if( wads[ 1 ].loaded )
		{
			memset( &wads[ 1 ], 0, sizeof( wads[ 1 ] ) );
		}
	}
}
