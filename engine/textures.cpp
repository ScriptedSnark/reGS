#include <cctype>

#include "quakedef.h"
#include "textures.h"
#include "wad.h"

struct texlumpinfo_t
{
	lumpinfo_t lump;
	int iTexFile;
};

texlumpinfo_t* lumpinfo = nullptr;

FileHandle_t texfiles[ TEX_MAX_WADS ] = {};

int nTexFiles = 0;
int nTexLumps = 0;

char miptex[ TEX_MAP_MIPTEX ][ TEX_MAX_MIPTEX_NAME ] = {};

int nummiptex = 0;

int lump_sorter( const void* lump1, const void* lump2 )
{
	return strcmp( reinterpret_cast<const lumpinfo_t*>( lump1 )->name, reinterpret_cast<const lumpinfo_t*>( lump2 )->name );
}

//TODO: the opposite of safe, crash on failure... - Solokiller
void SafeRead( FileHandle_t f, void* buffer, int count )
{
	if( FS_Read( buffer, count, f ) != count )
		Sys_Error( "File read failure" );
}

/*
==================
CleanupName

Lowercases name and pads with spaces and a terminating 0 to the length of
lumpinfo_t->name.
Used so lumpname lookups can proceed rapidly by comparing 4 chars at a time
Space padding is so names can be printed nicely in tables.
Can safely be performed in place.
//TODO: duplicate of W_CleanupName - Solokiller
==================
*/
void CleanupName( const char* in, char* out )
{
	int		i;
	int		c;

	for( i = 0; i<16; i++ )
	{
		c = in[ i ];
		if( !c )
			break;

		out[ i ] = toupper( c );
	}

	for( ; i< 16; i++ )
		out[ i ] = '\0';
}

//TODO: duplicate of COM_FixSlashes - Solokiller
void ForwardSlashes( char* pname )
{
	for( auto i = pname; *i; ++i )
	{
		if( *i == '\\' )
			*i = '/';
	}
}

bool TEX_InitFromWad( const char* path )
{
	char szTmpPath[ 1024 ];
	Q_strncpy( szTmpPath, path, ARRAYSIZE( szTmpPath ) );

	//Append if missing so tokenization is simpler
	if( !strchr( szTmpPath, ';' ) )
		Q_strcat( szTmpPath, ";", ARRAYSIZE( szTmpPath ) );

	char wadName[ MAX_PATH ];
	char wadPath[ MAX_PATH ];
	FileHandle_t texfile;
	wadinfo_t wadinfo;
	texlumpinfo_t* pLump;

	auto pszToken = strtok( szTmpPath, ";" );

	while( pszToken )
	{
		ForwardSlashes( pszToken );
		COM_FileBase( pszToken, wadName );
		//TODO: should be ARRAYSIZE( wadPath ) - Solokiller
		snprintf( wadPath, 256, "%s", wadName );
		COM_DefaultExtension( wadPath, ".wad" );

		if( !Q_strstr( wadName, "pldecal" ) && !Q_strstr( wadName, "tempdecal" ) )
		{
			texfiles[ nTexFiles ] = FS_Open( wadPath, "rb" );
			texfile = texfiles[ nTexFiles ];

			if( !texfiles[ nTexFiles ] )
				Sys_Error( "WARNING: couldn't open %s\n", wadPath );

			++nTexFiles;

			Con_DPrintf( "Using WAD File: %s\n", wadPath );
			SafeRead( texfile, &wadinfo, sizeof( wadinfo ) );

			if( Q_strncmp( wadinfo.identification, "WAD2", 4 ) && Q_strncmp( wadinfo.identification, "WAD3", 4 ) )
				Sys_Error( "TEX_InitFromWad: %s isn't a wadfile", wadPath );

			wadinfo.numlumps = LittleLong( wadinfo.numlumps );
			wadinfo.infotableofs = LittleLong( wadinfo.infotableofs );

			FS_Seek( texfile, wadinfo.infotableofs, FILESYSTEM_SEEK_HEAD );

			lumpinfo = reinterpret_cast<texlumpinfo_t*>( Mem_Realloc( lumpinfo, sizeof( texlumpinfo_t ) * ( nTexLumps + wadinfo.numlumps ) ) );
				
			for( int i = 0; i < wadinfo.numlumps; ++i )
			{
				SafeRead( texfile, &lumpinfo[ nTexLumps ].lump, sizeof( lumpinfo[ nTexLumps ].lump ) );

				pLump = &lumpinfo[ nTexLumps ];
				CleanupName( pLump->lump.name, pLump->lump.name );

				pLump->lump.filepos = LittleLong( pLump->lump.filepos );
				pLump->lump.disksize = LittleLong( pLump->lump.disksize );

				++nTexLumps;

				pLump->iTexFile = nTexFiles - 1;
			}
		}

		pszToken = strtok( nullptr, ";" );
	}

	qsort( lumpinfo, nTexLumps, sizeof( texlumpinfo_t ), lump_sorter );

	return true;
}

void TEX_CleanupWadInfo()
{
	if( lumpinfo )
	{
		Mem_Free( lumpinfo );
		lumpinfo = nullptr;
	}

	for( int i = 0; i < nTexFiles; ++i )
	{
		FS_Close( texfiles[ i ] );
		texfiles[ i ] = nullptr;
	}

	nTexLumps = 0;
	nTexFiles = 0;
}

int TEX_LoadLump( char* name, byte* dest )
{
	texlumpinfo_t key;

	CleanupName( name, key.lump.name );

	auto pLump = reinterpret_cast<texlumpinfo_t*>( bsearch( &key, lumpinfo, nTexLumps, sizeof( key ), lump_sorter ) );
	
	if( pLump )
	{
		FS_Seek( texfiles[ pLump->iTexFile ], pLump->lump.filepos, FILESYSTEM_SEEK_HEAD );
		
		SafeRead( texfiles[ pLump->iTexFile ], dest, pLump->lump.disksize );

		return pLump->lump.disksize;
	}

	Con_SafePrintf( "WARNING: texture lump \"%s\" not found\n", name );

	return 0;
}

int FindMiptex( const char* name )
{
	for( int i = 0; i < nummiptex; ++i )
	{
		if( !Q_stricmp( name, miptex[ i ] ) )
			return i;
	}

	if( nummiptex == TEX_MAP_MIPTEX )
		Sys_Error( "Exceeded MAX_MAP_TEXTURES" );

	Q_strncpy( miptex[ nummiptex ], name, ARRAYSIZE( miptex[ nummiptex ] ) );

	return nummiptex++;
}

void TEX_AddAnimatingTextures()
{
	const int base = nummiptex;

	if( nummiptex > 0 )
	{
		char name[ 32 ];

		for( int i = 0; i < base; ++i )
		{
			const char* pszTexture = miptex[ i ];

			if( *pszTexture == '-' || *pszTexture == '+' )
			{
				Q_strncpy( name, pszTexture, ARRAYSIZE( name ) );

				//Add +0..+9, then +A..+J
				for( int iTex = 0; iTex < 20; ++iTex )
				{
					name[ 1 ] = iTex < 10 ? ( '0' + iTex ) : ( 'A' + ( iTex - 10 ) );

					//Check if it exists
					for( int iSearch = 0; iSearch < nTexLumps; ++iSearch )
					{
						if( !Q_strcmp( name, lumpinfo[ iSearch ].lump.name ) )
						{
							//Add it
							FindMiptex( name );
							break;
						}
					}
				}
			}
		}

		if( nummiptex != base )
			Con_DPrintf( "added %i texture frames\n", nummiptex - base );
	}
}
