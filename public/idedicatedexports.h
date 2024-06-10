#ifndef PUBLIC_IDEDICATEDEXPORTS_H
#define PUBLIC_IDEDICATEDEXPORTS_H

#include "interface.h"

/**
*	Interface exported by the dedicated server launcher
*/
class IDedicatedExports : public IBaseInterface
{
public:
	virtual void Sys_Printf( char* text ) = 0;
};

#endif //PUBLIC_IDEDICATEDEXPORTS_H
