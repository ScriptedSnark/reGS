#include <cstdlib>
#include <cstring>

#include "mem.h"

void *Mem_Malloc( size_t size )
{
	return malloc( size );
}

void *Mem_ZeroMalloc( size_t size )
{
	void *result = malloc( size );

	memset( result, 0, size );

	return result;
}

void *Mem_Realloc( void *ptr, size_t size )
{
	return realloc( ptr, size );
}

void *Mem_Calloc( size_t nmemb, size_t size )
{
	return calloc( nmemb, size );
}

char *Mem_Strdup( const char *strSource )
{
	return _strdup( strSource );
}

void Mem_Free( void *ptr )
{
	free( ptr );
}
