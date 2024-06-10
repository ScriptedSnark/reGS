#ifndef ENGINE_TMESSAGE_H
#define ENGINE_TMESSAGE_H

/**
*	Maximum number of game_text messages that can be displayed at any one time.
*/
const int TMSG_MAX_MSGS = 4;
const int TMSG_MAX_MESSAGE_LENGTH = 512;
const int MAX_MESSAGES = 1000;

char* memfgets( byte* pMemFile, int fileSize, int& filePos, char* pBuffer, int bufferSize );

/**
*	Trims leading and trailing whitespace
*/
void TrimSpace( const char* source, char* dest );

void TextMessageShutdown();

void TextMessageInit();

void SetDemoMessage( const char* pszMessage, float fFadeInTime, float fFadeOutTime, float fHoldTime );

client_textmessage_t* TextMessageGet( const char* pName );

int TextMessageDrawCharacter( int x, int y, int number, int r, int g, int b );

#endif //ENGINE_TMESSAGE_H
