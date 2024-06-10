#ifndef COMMON_WINHEADERS_H
#define COMMON_WINHEADERS_H

/**
*	@file
*
*	Header that includes the base Windows headers
*/

#ifdef WIN32
#include "winsani_in.h"
#include "winlite.h"
#include "winsani_out.h"

#undef GetCurrentTime
#endif

#endif //COMMON_WINHEADERS_H