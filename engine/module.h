#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H

//Some functions declared here
#include "cdll_int.h"

const size_t MODULE_MAX_SIZE = 0x10000U;

void* GetModuleHandle( char* name );

#endif //ENGINE_MODULE_H
