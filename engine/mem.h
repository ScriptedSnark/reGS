#ifndef ENGINE_MEM_H
#define ENGINE_MEM_H

/**
*	@file
*
*	Heap allocation functions
*/

void *Mem_Malloc( size_t size );
void *Mem_ZeroMalloc( size_t size );
void *Mem_Realloc( void *ptr, size_t size );
void *Mem_Calloc( size_t nmemb, size_t size );
char *Mem_Strdup( const char *strSource );
void Mem_Free( void *ptr );

#endif //ENGINE_MEM_H
