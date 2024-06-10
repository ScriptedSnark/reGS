#include <cwctype>

#include "UnicodeFileHelpers.h"

uchar16* AdvanceOverWhitespace( uchar16* Start )
{
	uchar16* pszString;

	for( pszString = Start; *pszString && iswspace( *pszString ); ++pszString )
	{
	}

	return pszString;
}

uchar16* ReadToEndOfLine( uchar16* start )
{
	uchar16* result = start;

	if( *result )
	{
		//Advance to EOL tokens
		while( *result != '\n' && *result != '\r' )
		{
			++result;

			if( !*result )
				return result;
		}

		//Advance past EOL tokens
		while( *result == '\n' || *result == '\r' )
		{
			++result;
		}
	}

	return result;
}

uchar16* ReadUnicodeToken( uchar16* start, uchar16* token, int tokenBufferSize, bool& quoted )
{
	auto pszNext = AdvanceOverWhitespace( start );

	quoted = false;
	token[ 0 ] = '\0';

	if( !( *pszNext ) )
		return pszNext;

	auto pszDest = token;

	//Copy whitespace delimited token
	if( *pszNext != '"' )
	{
		//TODO: will attempt to write to one past the end of the buffer if i >= tokenBufferSize - Solokiller
		for( int i = 0;
			( i < tokenBufferSize ) && *pszNext && !iswspace( *pszNext );
			 ++i )
		{
			*pszDest++ = *pszNext++;
		}

		*pszDest = '\0';
		return pszNext;
	}

	//Copy token between quotes, advance past closing quote
	quoted = true;

	++pszNext;

	//Empty token
	if( *pszNext == '"' || !( *pszNext ) )
	{
		if( *pszNext == '"' )
			++pszNext;

		*pszDest = '\0';
		return pszNext;
	}

	if( tokenBufferSize <= 0 )
	{
		*pszDest = '\0';
		return pszNext;
	}

	while( true )
	{
		//Encountered a possible escape character, see if it's one we want to convert.		
		if( *pszNext == '\\' )
		{
			if( pszNext[ 1 ] == 'n' )
			{
				++pszNext;
				*pszDest = '\n';
			}
			else if( pszNext[ 1 ] == '"' )
			{
				++pszNext;
				*pszDest = '"';
			}
			else
			{
				*pszDest = *pszNext;
			}
		}
		else
		{
			*pszDest = *pszNext;
		}

		++pszDest;
		++pszNext;

		if( *pszNext == '"' || !( *pszNext ) )
		{
			if( *pszNext == '"' )
				++pszNext;

			*pszDest = '\0';
			return pszNext;
		}

		if( pszDest == &token[ tokenBufferSize ] )
		{
			*pszDest = '\0';
			return pszNext;
		}
	}

	return pszNext;
}
