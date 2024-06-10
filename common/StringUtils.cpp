#include <cassert>

#include "tier0/platform.h"

#include "StringUtils.h"

const char* strnstr( const char* pszString, const char* pszSubString, const size_t uiLength )
{
	assert( pszString );
	assert( pszSubString );

	while( *pszString )
	{
		if( strncmp( pszString, pszSubString, uiLength ) == 0 )
			return pszString;

		++pszString;
	}

	return nullptr;
}

const char* strnrstr( const char* pszString, const char* pszSubString, const size_t uiLength )
{
	assert( pszString );
	assert( pszSubString );

	size_t uiStrLength = strlen( pszString );

	pszString += uiStrLength - 1;

	while( uiStrLength > 0 )
	{
		if( strncmp( pszString, pszSubString, uiLength ) == 0 )
			return pszString;

		--pszString;
		--uiStrLength;
	}

	return nullptr;
}

bool UTIL_TokenMatches( const char* pszString, const char* pszToken )
{
	assert( pszToken );
	assert( pszString );

	//Empty tokens never match anything other than empty strings.
	if( !( *pszToken ) )
		return !( *pszString );

	const size_t uiLength = strlen( pszToken );

	const char* pszPrevPos = nullptr;
	const char* pszPos = pszToken;

	while( *pszPos )
	{
		pszPos = strchr( pszPos, '*' );

		if( !pszPos )
		{
			//No * found.
			pszPos = pszToken + uiLength;
		}

		//There was a previous * character, so match characters inbetween.
		if( pszPrevPos )
		{
			if( ( pszPos - pszPrevPos ) > 0 )
			{
				//This uses reverse search since * covers any number of characters, so matching the last instance is most valid.
				if( !( pszString = strnrstr( pszString, pszPrevPos, pszPos - pszPrevPos ) ) )
					return false;

				pszString += ( pszPos - pszPrevPos );
			}
		}
		else
		{
			//No previous * character, so match from the start of the string.
			if( strncmp( pszString, pszToken, pszPos - pszToken ) )
				return false;

			pszString += ( pszPos - pszToken );
		}

		if( !( *pszPos ) )
		{
			if( *pszString && *( pszPos - 1 ) != '*' )
				return false;

			break;
		}

		pszPrevPos = pszPos + 1;
		++pszPos;
	}

	return true;
}

char* UTIL_FixSlashes( char* pszPath )
{
	if( !pszPath )
		return nullptr;

	for( char* pszPos = pszPath; *pszPos; ++pszPos )
	{
		if( *pszPos == '\\' )
			*pszPos = '/';
	}

	return pszPath;
}

char* UTIL_SafeStrncpy( char* pszDest, const char* pszSource, const size_t uiSizeInCharacters )
{
	strncpy( pszDest, pszSource, uiSizeInCharacters );

	pszDest[ uiSizeInCharacters - 1 ] = '\0';

	return pszDest;
}
