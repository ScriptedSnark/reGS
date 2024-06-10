#ifndef PUBLIC_UNICODEFILEHELPERS_H
#define PUBLIC_UNICODEFILEHELPERS_H

#include <tier0/platform.h>

uchar16* AdvanceOverWhitespace( uchar16* Start );

uchar16* ReadToEndOfLine( uchar16* start );

uchar16* ReadUnicodeToken( uchar16* start, uchar16* token, int tokenBufferSize, bool& quoted );

#endif //PUBLIC_UNICODEFILEHELPERS_H
