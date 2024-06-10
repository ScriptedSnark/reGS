#include "quakedef.h"
#include "client.h"
#include "cl_demo.h"
#include "gl_draw.h"
#include "tmessage.h"
#include "vgui2/text_draw.h"

client_textmessage_t gMessageParms = {};

client_textmessage_t* gMessageTable = nullptr;
int gMessageTableCount = 0;

const char* gNetworkMessageNames[ TMSG_MAX_MSGS ] =
{
	"__NETMESSAGE__1",
	"__NETMESSAGE__2",
	"__NETMESSAGE__3",
	"__NETMESSAGE__4"
};

char gNetworkTextMessageBuffer[ TMSG_MAX_MSGS ][ TMSG_MAX_MESSAGE_LENGTH ] = {};

client_textmessage_t gNetworkTextMessage[ TMSG_MAX_MSGS ] =
{
	{
		0,
		255, 255, 255, 255,
		255, 255, 255, 255,
		-1, -1,
		0, 0,
		0, 0,
		"__NETMESSAGE__1",
		gNetworkTextMessageBuffer[ 0 ]
	}
};

//From HL1 SDK
char* memfgets( byte* pMemFile, int fileSize, int& filePos, char* pBuffer, int bufferSize )
{
	// Bullet-proofing
	if( !pMemFile || !pBuffer )
		return NULL;

	if( filePos >= fileSize )
		return NULL;

	int i = filePos;
	int last = fileSize;

	// fgets always NULL terminates, so only read bufferSize-1 characters
	if( last - filePos > ( bufferSize - 1 ) )
		last = filePos + ( bufferSize - 1 );

	int stop = 0;

	// Stop at the next newline (inclusive) or end of buffer
	while( i < last && !stop )
	{
		if( pMemFile[ i ] == '\n' )
			stop = 1;
		i++;
	}


	// If we actually advanced the pointer, copy it over
	if( i != filePos )
	{
		// We read in size bytes
		int size = i - filePos;
		// copy it out
		Q_memcpy( pBuffer, pMemFile + filePos, sizeof( byte )*size );

		// If the buffer isn't full, terminate (this is always true)
		if( size < bufferSize )
			pBuffer[ size ] = 0;

		// Update file pointer
		filePos = i;
		return pBuffer;
	}

	// No data read, bail
	return NULL;
}

bool IsComment( const char* pText )
{
	if( !pText )
		return true;

	const auto v2 = Q_strlen( pText );

	if( v2 <= 1 )
	{
		return v2 != 1;
	}

	return pText[ 0 ] == '/' && pText[ 1 ] == '/';
}

bool IsStartOfText( char* pText )
{
	if( pText )
		return *pText == '{';

	return false;
}

bool IsEndOfText( const char* pText )
{
	if( pText )
		return *pText == '}';

	return false;
}

bool IsWhiteSpace( char space )
{
	return space == '\t' || space == ' ' || space == '\r' || space == '\n';
}

const char* SkipSpace( const char* pText )
{
	if( !pText )
		return nullptr;

	const char* i;

	for( i = pText; *i; ++i )
	{
		if( !IsWhiteSpace( *i ) )
			break;
	}

	return i;
}

const char* SkipText( const char* pText )
{
	if( !pText )
		return nullptr;

	const char* i;

	for( i = pText; *i; ++i )
	{
		if( IsWhiteSpace( *i ) )
			break;
	}

	return i;
}

bool ParseFloats( const char* pText, float* pFloat, int count )
{
	if( pText )
	{
		auto i = pText;

		auto pflNext = pFloat;

		while( count > 0 )
		{
			if( *i )
			{
				if( !IsWhiteSpace( *i ) )
					i = SkipText( i );

				i = SkipSpace( i );
			}

			--count;

			*pflNext = Q_atof( i );
			++pflNext;
		}
	}

	return count == 0;
}

void TrimSpace( const char* source, char* dest )
{
	int i;

	for( i = 0; source[ i ]; ++i )
	{
		if( !IsWhiteSpace( source[ i ] ) )
			break;
	}

	auto iLast = Q_strlen( source ) - 1;

	while( iLast > 0 )
	{
		if( !IsWhiteSpace( source[ iLast ] ) )
			break;

		--iLast;
	}

	const size_t uiLength = ( iLast + 1 ) - i;

	//This cast is needed so iLast == 0 is treated as < 0 here
	if( static_cast<int>( uiLength ) > 0 )
	{
		Q_strncpy( dest, &source[ i ], uiLength + 1 );
	}
}

bool IsToken( const char* pText, const char* pTokenName )
{
	if( pTokenName && pText )
	{
		return !Q_strnicmp( pText + 1, pTokenName, Q_strlen( pTokenName ) );
	}

	return false;
}

bool ParseDirective( const char *pText )
{
	if( !pText || *pText != '$' )
		return false;

	float tempFloat[ 8 ];

	if( !Q_strnicmp( pText + 1, "position", Q_strlen( "position" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 2 ) )
		{
			gMessageParms.x = tempFloat[ 0 ];
			gMessageParms.y = tempFloat[ 1 ];
		}

		return true;
	}
	else if( !Q_strnicmp( pText + 1, "effect", Q_strlen( "effect" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 1 ) )
			gMessageParms.effect = static_cast<int>( tempFloat[ 0 ] );

		return true;
	}
	else if( !Q_strnicmp( pText + 1, "fxtime", Q_strlen( "fxtime" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 1 ) )
			gMessageParms.fxtime = tempFloat[ 0 ];

		return true;
	}
	else if( !Q_strnicmp( pText + 1, "color2", Q_strlen( "color2" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 3 ) )
		{
			gMessageParms.r2 = static_cast<int>( tempFloat[ 0 ] );
			gMessageParms.g2 = static_cast<int>( tempFloat[ 1 ] );
			gMessageParms.b2 = static_cast<int>( tempFloat[ 2 ] );
		}

		return true;
	}
	else if( !Q_strnicmp( pText + 1, "color", Q_strlen( "color" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 3 ) )
		{
			gMessageParms.r1 = static_cast<int>( tempFloat[ 0 ] );
			gMessageParms.g1 = static_cast<int>( tempFloat[ 1 ] );
			gMessageParms.b1 = static_cast<int>( tempFloat[ 2 ] );
		}

		return true;
	}
	else if( !Q_strnicmp( pText + 1, "fadein", Q_strlen( "fadein" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 1 ) )
			gMessageParms.fadein = tempFloat[ 0 ];

		return true;
	}
	else if( !Q_strnicmp( pText + 1, "fadeout", Q_strlen( "fadeout" ) ) )
	{
		if( ParseFloats( pText, tempFloat, 3 ) )
		{
			gMessageParms.fadeout = tempFloat[ 0 ];
		}

		return true;
	}
	else if( IsToken( pText, "holdtime" ) )
	{
		if( ParseFloats( pText, tempFloat, 3 ) )
			gMessageParms.holdtime = tempFloat[ 0 ];

		return true;
	}
	else
	{
		Con_DPrintf( "Unknown token: %s\n", pText );
		return true;
	}

	return false;
}

const int TMSG_NAMEHEAP_SIZE = 16384;

static void TextMessageParse( byte* pMemFile, int fileSize )
{
	client_textmessage_t textMessages[ MAX_MESSAGES ];
	char nameHeap[ TMSG_NAMEHEAP_SIZE ];

	int messageCount = 0;
	int nameHeapSize = 0;

	if( pMemFile && fileSize > 0 )
	{
		enum Mode
		{
			MODE_PARSE_DIRECTIVES = 0,
			MODE_PARSE_TEXT = 1
		};

		int iNextRead = 0;
		Mode mode = MODE_PARSE_DIRECTIVES;
		int lineNumber = 0;

		char trim[ TMSG_MAX_MESSAGE_LENGTH ];
		char buf[ TMSG_MAX_MESSAGE_LENGTH ];
		char currentName[ TMSG_MAX_MESSAGE_LENGTH ];

		char* pCurrentText;

		while( true )
		{
			int iEndPos = iNextRead;

			const int iMaxLength = ( fileSize - iNextRead < ARRAYSIZE( buf ) ) ? fileSize : ( iNextRead + ARRAYSIZE( buf ) - 1 );

			//Find the end of the token
			while( pMemFile[ iEndPos++ ] != '\n' && iEndPos < iMaxLength )
			{
			}

			if( iNextRead == iEndPos )
				break;

			Q_memcpy( buf, &pMemFile[ iNextRead ], iEndPos - iNextRead );

			if( iEndPos - iNextRead < ARRAYSIZE( buf ) )
				buf[ iEndPos - iNextRead ] = '\0';

			if( messageCount >= MAX_MESSAGES )
				Sys_Error( "tmessage::TextMessageParse : messageCount>=MAX_MESSAGES" );

			TrimSpace( buf, trim );

			if( mode == MODE_PARSE_TEXT )
			{
				if( IsEndOfText( trim ) )
				{
					if( nameHeapSize + Q_strlen( currentName ) > TMSG_NAMEHEAP_SIZE )
					{
						Con_DPrintf( "Error parsing file!  length > %i bytes\n", TMSG_NAMEHEAP_SIZE );
						return;
					}

					Q_strcpy( &nameHeap[ nameHeapSize ], currentName );
					pMemFile[ iNextRead - 1 ] = '\0';

					memcpy( &textMessages[ messageCount ], &gMessageParms, sizeof( gMessageParms ) );

					textMessages[ messageCount ].pName = &nameHeap[ nameHeapSize ];
					nameHeapSize += Q_strlen( currentName ) + 1;

					textMessages[ messageCount ].pMessage = pCurrentText;

					++messageCount;

					mode = MODE_PARSE_DIRECTIVES;
				}
				else if( IsStartOfText( trim ) )
				{
					Con_DPrintf( "Unexpected '{' found, line %d\n", lineNumber );
					return;
				}
			}
			else
			{
				mode = MODE_PARSE_DIRECTIVES;

				if( !IsComment( trim ) )
				{
					if( !ParseDirective( trim ) )
					{
						if( IsStartOfText( trim ) )
						{
							pCurrentText = reinterpret_cast<char*>( &pMemFile[ iEndPos ] );
							mode = MODE_PARSE_TEXT;
						}
						else
						{
							if( IsEndOfText( trim ) )
							{
								Con_DPrintf( "Unexpected '}' found, line %d\n", lineNumber );
								return;
							}

							Q_strncpy( currentName, trim, ARRAYSIZE( currentName ) );
						}
					}
				}
			}

			iNextRead = iEndPos;

			++lineNumber;

			if( iEndPos >= fileSize )
				break;
		}
	}

	Con_DPrintf( "Parsed %d text messages\n", messageCount );

	int msgHeapSize = 0;

	for( int i = 0; i < messageCount; ++i )
	{
		msgHeapSize += Q_strlen( textMessages[ i ].pMessage ) + 1;
	}

	//Allocate space for the messages, plus a heap for names and messages
	gMessageTable = reinterpret_cast<client_textmessage_t*>( Mem_Malloc( sizeof( client_textmessage_t ) * messageCount + msgHeapSize + nameHeapSize ) );
	
	Q_memcpy( gMessageTable, textMessages, sizeof( client_textmessage_t ) * messageCount );

	byte* pNameHeap = reinterpret_cast<byte*>( &gMessageTable[ messageCount ] );
	Q_memcpy( pNameHeap, nameHeap, nameHeapSize );

	char* pszMsgDest = reinterpret_cast<char*>( pNameHeap ) + nameHeapSize;

	//Offset between the stack based name heap and the allocated one
	const int iNameOffset = pNameHeap - reinterpret_cast<const byte*>( gMessageTable->pName );

	//Update string pointers, copy messages to heap
	for( int i = 0; i < messageCount; ++i )
	{
		auto& msg = gMessageTable[ i ];

		msg.pName += iNameOffset;

		Q_strcpy( pszMsgDest, msg.pMessage );
		msg.pMessage = pszMsgDest;
		pszMsgDest += Q_strlen( pszMsgDest ) + 1;
	}

	gMessageTableCount = messageCount;
}

void TextMessageShutdown()
{
	if( gMessageTable )
	{
		Mem_Free( gMessageTable );
		gMessageTable = nullptr;
	}
}

void TextMessageInit()
{
	TextMessageShutdown();

	int fileSize;
	byte* pFileData = COM_LoadTempFile( "titles.txt", &fileSize );

	if( pFileData )
		TextMessageParse( pFileData, fileSize );
}

void SetDemoMessage( const char* pszMessage, float fFadeInTime, float fFadeOutTime, float fHoldTime )
{
	if( pszMessage && *pszMessage )
	{
		Q_strcpy( const_cast<char*>( tm_demomessage.pMessage ), pszMessage );
		tm_demomessage.fadein = fFadeInTime;
		tm_demomessage.fadeout = fFadeOutTime;
		tm_demomessage.holdtime = fHoldTime;
	}
}

client_textmessage_t* TextMessageGet( const char* pName )
{
	g_engdstAddrs.pfnTextMessageGet( &pName );

	if( !Q_stricmp( pName, "__DEMOMESSAGE__" ) )
		return &tm_demomessage;

	for( int i = 0; i < ARRAYSIZE( gNetworkMessageNames ); ++i )
	{
		if( !Q_stricmp( pName, gNetworkMessageNames[ i ] ) )
			return &gNetworkTextMessage[ i ];
	}

	if( gMessageTable )
	{
		for( int i = 0; i < gMessageTableCount; ++i )
		{
			if( !Q_stricmp( pName, gMessageTable[ i ].pName ) )
				return &gMessageTable[ i ];
		}
	}
	
	return nullptr;
}

int TextMessageDrawCharacter( int x, int y, int number, int r, int g, int b )
{
	g_engdstAddrs.pfnDrawCharacter( &x, &y, &number, &r, &g, &b );

	if( r || g || b )
	{
		return Draw_MessageCharacterAdd( x, y, number, r, g, b, VGUI2_GetCreditsFont() );
	}

	return 0;
}
