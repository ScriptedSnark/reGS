#ifndef COMMON_TOKENLINE_H
#define COMMON_TOKENLINE_H

#include <cstdlib>

/**
*	Simple tokenizer that separates a string by spaces.
*	Quoted sections are parsed as one token.
*/
class TokenLine
{
private:
	static const size_t MAX_TOKEN_BUFFER = 2048;
	static const int MAX_TOKENS = 128;

public:
	TokenLine() = default;
	TokenLine( char* string );
	virtual ~TokenLine() = default;

	bool SetLine( const char* newLine );

	char* GetLine();

	char* GetToken( int i );

	char* CheckToken( char* parm );

	int CountToken();

	char* GetRestOfLine( int i );

private:
	char m_tokenBuffer[ MAX_TOKEN_BUFFER ];
	char m_fullLine[ MAX_TOKEN_BUFFER ];
	char* m_token[ MAX_TOKENS ];
	int m_tokenNumber;
};

#endif //COMMON_TOKENLINE_H
