#ifndef ENGINE_INFO_H
#define ENGINE_INFO_H

/**
*	@file
*
*	Handles info key buffer keyvalues
*
*	Info key buffers store key value pairs, separated by '\\'.
*	Keys start with '\\' as well, meaning a buffer that has at least one keyvalue has the form "\\key\\value".
*/

extern char serverinfo[ MAX_INFO_STRING ];

bool Info_IsKeyImportant( const char* key );

void Info_SetValueForStarKey( char* s, const char* key, const char* value, int maxsize );

void Info_SetValueForKey( char* s, const char* key, const char* value, int maxsize );

void Info_RemoveKey( char* s, const char* key );

void Info_RemovePrefixedKeys( char* start, char prefix );

/**
*	Searches the string for the given
*	key and returns the associated value, or an empty string.
*/
const char* Info_ValueForKey( const char* s, const char* key );

void Info_Print( const char* s );

bool Info_IsValid( const char* s );

#endif //ENGINE_INFO_H
