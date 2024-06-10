#ifndef COMMON_FILEPATHS_H
#define COMMON_FILEPATHS_H

/**
*	@file
*
*	File paths used in multiple projects.
*/

#ifdef WIN32
#define DEFAULT_SO_EXT ".dll"
#elif defined( LINUX )
#define DEFAULT_SO_EXT ".so"
#elif defined( OSX )
#define DEFAULT_SO_EXT ".dylib"
#else
#error "Unsupported platform"
#endif

namespace filepath
{
extern const char FILESYSTEM_STDIO[];
}

#endif //COMMON_FILEPATHS_H
