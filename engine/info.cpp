#include <cctype>

#include "quakedef.h"

const int MAX_INFO_TOKEN = MAX_INFO_STRING / 2;
const int MAX_VALUE_BUFFERS = 4;

char serverinfo[ MAX_INFO_STRING ] = {};

//TODO: these functions repeat the same code a lot, refactor using helpers to extract key & value ranges - Solokiller

bool Info_IsKeyImportant( const char* key )
{
	if( *key != '*'
		&& Q_strcmp( key, "name" )
		&& Q_strcmp( key, "model" )
		&& Q_strcmp( key, "rate" )
		&& Q_strcmp( key, "topcolor" )
		&& Q_strcmp( key, "bottomcolor" )
		&& Q_strcmp( key, "cl_updaterate" )
		&& Q_strcmp( key, "cl_lw" )
		&& Q_strcmp( key, "cl_lc" )
		&& Q_strcmp( key, "*hltv" )
		&& Q_strcmp( key, "*sid" ) )
	{
		return false;
	}

	return true;
}

static char largest_key[ MAX_INFO_TOKEN + 1 ] = {};

char* Info_FindLargestKey( char* s )
{
	char key[ MAX_INFO_TOKEN ];
	char value[ MAX_INFO_TOKEN ];

	largest_key[ 0 ] = '\0';
	int largest_size = 0;

	for( auto pszNext = s; *pszNext; )
	{
		if( *pszNext == '\\' )
		{
			++pszNext;

			if( !( *pszNext ) )
				break;
		}

		int i;
		for( i = 0; i < ( MAX_INFO_TOKEN - 1 ); ++i )
		{
			key[ i ] = *pszNext++;

			if( key[ i ] == '\\' )
			{
				break;
			}
		}

		key[ i ] = '\0';

		//Key with no value?
		if( !( *pszNext ) )
			return largest_key;

		//Skip the '\\'
		++pszNext;

		//Extract the value.
		for( i = 0; i < ( MAX_INFO_TOKEN - 1 ) && *pszNext; ++i )
		{
			char c = *pszNext++;

			if( c == '\\' )
			{
				break;
			}

			value[ i ] = c;
		}

		value[ i ] = '\0';

		const auto iTotalLength = Q_strlen( key ) + Q_strlen( value );

		if( largest_size < iTotalLength )
		{
			if( !Info_IsKeyImportant( key ) )
			{
				Q_strncpy( largest_key, key, sizeof( largest_key ) - 1 );
				largest_key[ sizeof( largest_key ) - 1 ] = '\0';
				largest_size = iTotalLength;
			}
		}
	}

	return largest_key;
}

void Info_SetValueForStarKey( char* s, const char* key, const char* value, int maxsize )
{
	if( strstr( key, "\\" ) || strstr( value, "\\" ) )
	{
		Con_Printf( "Can't use keys or values with a \\\n" );
		return;
	}

	if( Q_strstr( key, ".." ) || Q_strstr( value, ".." ) )
	{
		return;
	}

	if( strstr( key, "\"" ) || strstr( value, "\"" ) )
	{
		Con_Printf( "Can't use keys or values with a \"\n" );
		return;
	}

	if( Q_strlen( key ) <= 0 || Q_strlen( key ) >= MAX_INFO_TOKEN || Q_strlen( value ) >= MAX_INFO_TOKEN )
	{
		Con_Printf( "Keys and values must be < %i characters and > 0.\n", MAX_INFO_TOKEN );
		return;
	}

	if( !Q_UnicodeValidate( value ) )
	{
		Con_Printf( "Values must be valid utf8 text\n" );
		return;
	}

	Info_RemoveKey( s, key );
	if( !value || !Q_strlen( value ) )
		return;

	char newVal[ 1024 ];
	snprintf( newVal, ARRAYSIZE( newVal ), "\\%s\\%s", key, value );

	if( Q_strlen( newVal ) + Q_strlen( s ) > maxsize )
	{
		Con_Printf( "Info string length exceeded\n" );
		return;
	}

	if( ( Q_strlen( s ) + Q_strlen( newVal ) ) >= maxsize )
	{
		bool bCanFit = !Info_IsKeyImportant( key );

		if( bCanFit )
		{
			while( true )
			{
				auto pszKeyRemove = Info_FindLargestKey( s );

				Info_RemoveKey( s, pszKeyRemove );

				if( maxsize > ( Q_strlen( s ) + Q_strlen( newVal ) ) )
				{
					bCanFit = true;
					break;
				}

				if( !*pszKeyRemove )
					break;
			}
		}

		if( !bCanFit )
		{
			Con_Printf( "Info string length exceeded\n" );
			return;
		}
	}

	char* pszDest = &s[ Q_strlen( s ) ];

	for( auto pszSource = newVal; *pszSource; ++pszSource )
	{
		int c = *pszSource;

		//Convert team names to lower case.
		//Only ASCII characters.
		if( !Q_stricmp( key, "team" ) && c <= 255 )
			c = tolower( c );

		*pszDest++ = c;
	}

	*pszDest = '\0';
}

void Info_SetValueForKey( char* s, const char* key, const char* value, int maxsize )
{
	if( *key == '*' )
		Con_Printf( "Can't set * keys\n" );
	else
		Info_SetValueForStarKey( s, key, value, maxsize );
}

void Info_RemoveKey( char* s, const char* key )
{
	if( strstr( key, "\\" ) )
	{
		Con_Printf( "Can't use a key with a \\\n" );
		return;
	}

	char	*start;
	char	pkey[ MAX_INFO_TOKEN ];
	char	value[ MAX_INFO_TOKEN ];
	char	*o;

	auto uiLength = min( strlen( key ), static_cast<size_t>( MAX_INFO_TOKEN - 1 ) );

	unsigned int uiSkipped;
	const char* pszEnd;

	while( true )
	{
		start = s;
		if( *s == '\\' )
			s++;
		o = pkey;
		uiSkipped = 0;
		while( uiSkipped < ( ARRAYSIZE( pkey ) - 2 ) && *s != '\\' )
		{
			++uiSkipped;
			if( !*s )
				return;
			*o++ = *s++;
		}
		*o = '\0';
		s++;

		o = value;
		pszEnd = s + MAX_INFO_TOKEN;
		while( *s != '\\' && *s && s != pszEnd )
		{
			if( !*s )
				return;
			*o++ = *s++;
		}
		*o = '\0';

		if( !Q_strncmp( key, pkey, uiLength ) )
		{
			Q_strcpy( start, s );	// remove this part
			return;
		}

		if( !*s )
			return;
	}
}

void Info_RemovePrefixedKeys( char* start, char prefix )
{
	char key[ MAX_INFO_TOKEN ];
	char value[ MAX_INFO_TOKEN ];

	for( auto pszNext = start; *pszNext; )
	{
		if( *pszNext == '\\' )
		{
			++pszNext;

			if( !( *pszNext ) )
				break;
		}

		int i = 0;
		while( true )
		{
			key[ i ] = *pszNext++;

			++i;

			if( *pszNext == '\\' )
			{
				break;
			}

			if( !( *pszNext ) )
				return;

			if( i == ( MAX_INFO_TOKEN - 1 ) )
				break;
		}

		key[ i ] = '\0';

		//Skip the '\\'
		++pszNext;

		//Extract the value.
		i = 0;
		while( true )
		{
			char c = *pszNext++;

			value[ i ] = c;

			++i;

			if( c == '\\' || !c )
			{
				break;
			}

			if( i == ( MAX_INFO_TOKEN - 1 ) )
				break;
		}

		value[ i ] = '\0';

		if( *key == prefix )
		{
			Info_RemoveKey( start, key );
		}
	}
}

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const char* Info_ValueForKey( const char* s, const char* key )
{
	char	pkey[ MAX_INFO_TOKEN ];
	static	char value[ MAX_VALUE_BUFFERS ][ MAX_INFO_TOKEN ];	// use 4 buffers so compares
									// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;
	const char* pszEnd;

	valueindex = ( valueindex + 1 ) % MAX_VALUE_BUFFERS;

	if( *s == '\\' )
		s++;

	int i;

	while( true )
	{
		i = 0;
		o = pkey;
		while( i < ARRAYSIZE( pkey ) - 2 && *s != '\\' )
		{
			++i;
			if( !*s )
				return "";
			*o++ = *s++;
		}
		*o = '\0';
		s++;

		o = value[ valueindex ];

		pszEnd = s + MAX_INFO_TOKEN;

		while( *s != '\\' && *s && s != pszEnd )
		{
			if( !*s )
				return "";
			*o++ = *s++;
		}
		*o = '\0';

		if( !strcmp( key, pkey ) )
			return value[ valueindex ];

		if( !*s )
			return "";
		s++;
	}
}

void Info_Print( const char* s )
{
	char key[ MAX_INFO_TOKEN ];
	char value[ MAX_INFO_TOKEN ];

	for( auto pszNext = s; *pszNext; )
	{
		if( *pszNext == '\\' )
		{
			++pszNext;

			if( !( *pszNext ) )
				break;
		}

		int i = 0;
		while( true )
		{
			key[ i ] = *pszNext++;

			++i;

			if( *pszNext == '\\' || !( *pszNext ) )
			{
				break;
			}

			if( i == ( MAX_INFO_TOKEN - 1 ) )
				break;
		}

		key[ i ] = '\0';

		//Pad to 20 characters minimum
		if( i < 20 )
		{
			Q_memset( key + i, ' ', 20 - i );
			key[ 20 ] = '\0';
		}

		Con_Printf( "%s", key );

		if( !( *pszNext ) )
		{
			Con_Printf( "MISSING VALUE\n" );
			return;
		}

		//Skip the '\\'
		++pszNext;

		//Extract the value.
		i = 0;
		while( true )
		{
			char c = *pszNext++;

			value[ i ] = c;

			++i;

			if( c == '\\' || !c )
			{
				break;
			}

			if( i == ( MAX_INFO_TOKEN - 1 ) )
				break;
		}

		value[ i ] = '\0';

		Con_Printf( "%s\n", value );
	}
}

bool Info_IsValid( const char* s )
{
	char key[ MAX_INFO_TOKEN ];
	char value[ MAX_INFO_TOKEN ];

	for( auto pszNext = s; *pszNext; )
	{
		if( *pszNext == '\\' )
		{
			++pszNext;

			if( !( *pszNext ) )
				break;
		}

		int i = 0;
		while( true )
		{
			key[ i ] = *pszNext++;

			++i;

			if( *pszNext == '\\' || !( *pszNext ) )
			{
				break;
			}

			if( i == ( MAX_INFO_TOKEN - 1 ) )
				return false;
		}

		key[ i ] = '\0';

		if( !( *pszNext ) )
			return false;

		//Skip the '\\'
		++pszNext;

		//Extract the value.
		i = 0;
		while( true )
		{
			char c = *pszNext++;

			value[ i ] = c;

			++i;

			if( c == '\\' || !c )
			{
				break;
			}

			if( i == ( MAX_INFO_TOKEN - 1 ) )
				return false;
		}

		value[ i ] = '\0';

		if( !Q_strlen( value ) )
			return false;
	}

	return true;
}
