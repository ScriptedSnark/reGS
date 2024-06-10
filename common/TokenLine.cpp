#include "quakedef.h"

#include "TokenLine.h"

TokenLine::TokenLine( char* string )
{
	SetLine( string );
}

bool TokenLine::SetLine( const char* newLine )
{
	m_tokenNumber = 0;

	if( !newLine || strlen( newLine ) > MAX_TOKEN_BUFFER - 2 )
	{
		memset( m_fullLine, 0, sizeof( m_fullLine ) );
		memset( m_tokenBuffer, 0, sizeof( m_tokenBuffer ) );

		return false;
	}

	strncpy( m_fullLine, newLine, ARRAYSIZE( m_fullLine ) - 1 );
	m_fullLine[ ARRAYSIZE( m_fullLine ) - 1 ] = '\0';

	strncpy( m_tokenBuffer, newLine, ARRAYSIZE( m_tokenBuffer ) - 1 );
	m_tokenBuffer[ ARRAYSIZE( m_tokenBuffer ) - 1 ] = '\0';

	char* pszToken = m_tokenBuffer;

	while( *pszToken && m_tokenNumber < MAX_TOKENS )
	{
		//Skip spaces and invalid characters.
		while( *pszToken && ( *pszToken <= ' ' && *pszToken < 127 ) )
		{
			++pszToken;
		}

		if( !( *pszToken ) )
			break;

		m_token[ m_tokenNumber ] = pszToken;

		if( *pszToken == '"' )
		{
			//Find the end of the quoted token.
			m_token[ m_tokenNumber ] = pszToken + 1;

			while( *pszToken && *pszToken != '"' )
			{
				++pszToken;
			}
		}
		else
		{
			//Skip spaces and unprintable characters.
			for( m_token[ m_tokenNumber ] = pszToken; *pszToken && ( *pszToken <= ' ' && *pszToken < 127 ); ++pszToken )
			{
			}
		}
		++m_tokenNumber;

		if( !( *pszToken ) )
			break;

		*pszToken++ = '\0';
	}

	return m_tokenNumber != MAX_TOKENS;
}

char* TokenLine::GetLine()
{
	return m_fullLine;
}

char* TokenLine::GetToken( int i )
{
	if( i >= m_tokenNumber )
		return nullptr;

	return m_token[ i ];
}

char* TokenLine::CheckToken( char* parm )
{
	for( int i = 0; i < m_tokenNumber; ++i )
	{
		if( m_token[ i ] && !strcmp( parm, m_token[ i ] ) )
		{
			if( i + 1 < m_tokenNumber )
				return m_token[ i + 1 ];

			return "";
		}
	}

	return nullptr;
}

int TokenLine::CountToken()
{
	if( m_tokenNumber <= 0 )
		return 0;

	int count = 0;

	for( int i = 0; i < m_tokenNumber; ++i )
	{
		if( m_token[ i ] )
			++count;
	}

	return count;
}

char* TokenLine::GetRestOfLine( int i )
{
	if( i >= m_tokenNumber )
		return nullptr;

	return m_token[ i ] + MAX_TOKEN_BUFFER;
}
