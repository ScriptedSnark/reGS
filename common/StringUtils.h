#ifndef STDLIB_STRINGUTILS_H
#define STDLIB_STRINGUTILS_H

#include <algorithm>
#include <cctype>
#include <cstring>

#include "tier0/platform.h"

/**
*	Checks if a printf operation was successful
*/
inline bool PrintfSuccess( const int iRet, const size_t uiBufferSize )
{
	return iRet >= 0 && static_cast<size_t>( iRet ) < uiBufferSize;
}

/**
*	Taken from MSVC string hash.
*/
inline size_t StringHash( const char* const pszString )
{
#if defined( _WIN64 ) || ( defined( __GNUC__ ) && ( __x86_64__ || __ppc64__ ) )
	static_assert( sizeof( size_t ) == 8, "This code is for 64-bit size_t." );
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert( sizeof( size_t ) == 4, "This code is for 32-bit size_t." );
	const size_t _FNV_offset_basis = 2166136261U;
	const size_t _FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	const size_t _Count = strlen( pszString );

	size_t _Val = _FNV_offset_basis;
	for( size_t _Next = 0; _Next < _Count; ++_Next )
	{	// fold in another byte
		_Val ^= ( size_t ) pszString[ _Next ];
		_Val *= _FNV_prime;
	}
	return ( _Val );
}

/**
*	String hashing function
*	Ripped from the MSVC std::hash<std::string> implementation
*	Case insensitive
*/
inline size_t StringHashI( const char* pszString )
{
	if( !pszString )
		pszString = "";

	size_t _Val = 2166136261U;
	size_t _First = 0;
	size_t _Last = strlen( pszString );
	size_t _Stride = 1 + _Last / 10;

	for( ; _First < _Last; _First += _Stride )
		_Val = 16777619U * _Val ^ ( size_t ) tolower( pszString[ _First ] );
	return ( _Val );
}

/**
*	Functor for char* hashing.
*/
template<size_t( *HASHER )( const char* )>
struct BaseRawCharHash
{
	typedef const char* argument_type;
	typedef size_t result_type;

	size_t operator()( const char* pszString ) const
	{
		return HASHER( pszString );
	}
};

typedef BaseRawCharHash<StringHash> RawCharHash;
typedef BaseRawCharHash<StringHashI> RawCharHashI;

template<int( *COMPARE )( const char*, const char* )>
struct BaseRawCharEqualTo
{
	typedef const char* first_argument_type;
	typedef const char* second_argument_type;
	typedef bool result_type;

	bool operator()( const char* pszLHS, const char* pszRHS ) const
	{
		return COMPARE( pszLHS, pszRHS ) == 0;
	}
};

typedef BaseRawCharEqualTo<strcmp> RawCharEqualTo;
typedef BaseRawCharEqualTo<stricmp> RawCharEqualToI;

template<typename STR, int( *COMPARE )( STR lhs, STR rhs ) = strcmp>
struct Less_C_String final
{
	constexpr bool operator()( STR lhs, STR rhs ) const
	{
		return COMPARE( lhs, rhs ) < 0;
	}
};

/**
*	Works like strstr, but the substring length is given.
*/
const char* strnstr( const char* pszString, const char* pszSubString, const size_t uiLength );

/**
*	Works like strrstr, but the substring length is given.
*/
const char* strnrstr( const char* pszString, const char* pszSubString, const size_t uiLength );

/**
*	Checks whether a token matches a string.
*	The token can have '*' characters to signal 0 or more characters that can span the space between given characters.
*	@param pszString String to match against.
*	@param pszToken Token to match.
*	@return Whether the token matches.
*/
bool UTIL_TokenMatches( const char* pszString, const char* pszToken );

/**
*	Fixes slashes to use the correct platform specific slashes.
*/
char* UTIL_FixSlashes( char* pszPath );

char* UTIL_SafeStrncpy( char* pszDest, const char* pszSource, const size_t uiSizeInCharacters );

#endif //STDLIB_STRINGUTILS_H