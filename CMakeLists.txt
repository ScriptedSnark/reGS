###################################################
#                                                 #
#                                                 #
#   reGS (GSR fork) CMake build file              #
#                                                 #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.6 )

include( cmake/InputFilesList.cmake )
include( cmake/MSVCRuntime.cmake )

project( reGS )

include( cmake/WinXPSupport.cmake )

set(CMAKE_CXX_STANRARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Needs static CRT to avoid alloc issues.
set( MSVC_RUNTIME "static" )

configure_msvc_runtime()

set(variables
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
message(STATUS "Initial build flags:")
foreach(variable ${variables})
  message(STATUS "  '${variable}': ${${variable}}")
endforeach()
message(STATUS "")

if( UNIX )
	#Additional debug info for GDB.
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g" )
endif()

set( STEAMCOMMON_DIR_NAME Half-Life )
set( GAME_NAME valve )

#Base dir for engine outputs
set( ENGINE_BASE_PATH "${CMAKE_SOURCE_DIR}/test_game" CACHE PATH "Path to deploy binaries to. Equivalent to common/Half-Life, but defaults to test_game for testing only" )
set( GAME_BASE_PATH "${ENGINE_BASE_PATH}/${GAME_NAME}" )

message( STATUS "Base Engine Path: ${ENGINE_BASE_PATH}" )
message( STATUS "Base Game Path: ${GAME_BASE_PATH}" )

#Get the steam/steamapps/common directory
set( STEAMCOMMON "$ENV{SteamCommon}" CACHE PATH "Steam common directory path" )

if( "${STEAMCOMMON}" STREQUAL "" )
	message( FATAL_ERROR "Unable to get Steam common directory path" )
endif()

set( HL_DIR ${STEAMCOMMON}/${STEAMCOMMON_DIR_NAME} )
set( GAME_DIR ${HL_DIR}/${GAME_NAME} )

set( COPY_RESOURCES OFF CACHE BOOL "Flag used to track whether resources should be copied. Set to ON to force update, set to OFF to skip." )

#To avoid including copyrighted content, we copy all resources from a Half-Life installation to the test directory.
if( COPY_RESOURCES )
	message( STATUS "Copying resources from game installation..." )
	
	set( FILES_TO_COPY
		chromehtml.dll
		libcef.dll
		icudt.dll
		vgui.dll
		vstdlib.dll
		${GAME_NAME}/config.cfg
		${GAME_NAME}/fonts.wad
		${GAME_NAME}/game.ico
		${GAME_NAME}/game.tga
		${GAME_NAME}/gfx.wad
		${GAME_NAME}/language.cfg
		${GAME_NAME}/liblist.gam
		${GAME_NAME}/settings.scr
		${GAME_NAME}/titles.txt
		${GAME_NAME}/user.scr
		${GAME_NAME}/valve.rc
		${GAME_NAME}/cl_dlls/client.dll
		${GAME_NAME}/cl_dlls/particleman.dll
	)
	
	foreach( filename IN LISTS FILES_TO_COPY )
		message( STATUS "Copying file \"${filename}\"" )
		get_filename_component( FILE_DIR ${filename} DIRECTORY )
		get_filename_component( FILE_NAME ${filename} NAME )
		file( COPY ${HL_DIR}/${FILE_DIR}/${FILE_NAME} DESTINATION ${ENGINE_BASE_PATH}/${FILE_DIR} )
	endforeach()
	
	set( DIRS_TO_COPY 
		platform
		${GAME_NAME}/dlls
		${GAME_NAME}/gfx
		${GAME_NAME}/hw
		${GAME_NAME}/logos
		${GAME_NAME}/models/player
		${GAME_NAME}/resource
	)
	
	foreach( dir IN LISTS DIRS_TO_COPY )
		message( STATUS "Copying directory \"${dir}\"" )
		#Go up one directory so nested paths end up where they belong.
		file( COPY ${HL_DIR}/${dir} DESTINATION ${ENGINE_BASE_PATH}/${dir}/.. )
	endforeach()
	
	message( STATUS "Done copying resources" )
endif()

set( SRC_DIR ${CMAKE_SOURCE_DIR} )
set( EXTERNAL_DIR ${SRC_DIR}/external )

if( WIN32 )
	set( LIB_DIR ${SRC_DIR}/lib/public )
	set( BIN_DIR ${SRC_DIR}/bin/public )
elseif( APPLE )
	set( LIB_DIR ${SRC_DIR}/lib/osx32 )
	set( BIN_DIR ${LIB_DIR} )
elseif( UNIX )
	set( LIB_DIR ${SRC_DIR}/lib/linux32 )
	set( BIN_DIR ${LIB_DIR} )
else()
	set( LIB_DIR )
	set( BIN_DIR ${LIB_DIR} )
	message( WARNING "Unknown target platform; cannot determine library directory" )
endif()

#Shared preprocessor definitions
set( SHARED_DEFS 
	NO_MALLOC_OVERRIDE
)

if( MSVC )
	set( SHARED_DEFS
		${SHARED_DEFS}
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
	
	set( LINUX_32BIT_FLAG "" )
else()
	#All non-MSVC targets need this. That includes Cygwin and Mingw.
	#Match Source 2013 in defining GNUC for UNIX platforms.
	#Match Source 2013 in defining POSIX for UNIX platforms.
	#Enable 64 bit fopen functions for use in the filesystem.
	set( SHARED_DEFS
		${SHARED_DEFS}
		GNUC
		POSIX
		_LARGEFILE_SOURCE
		_LARGEFILE64_SOURCE
	)

	if( APPLE )
		#Define OSX so code can check for it.
		set( SHARED_DEFS
			${SHARED_DEFS}
			OSX
		)
	endif()

	if( UNIX )
		#Define LINUX so code can check for it.
		#Define _LINUX for Source SDK dependencies.
		set( SHARED_DEFS
			${SHARED_DEFS}
			LINUX
			_LINUX
		)
	endif()

	set( LINUX_32BIT_FLAG "-m32" )
endif()

if( MSVC )
	#Set Windows subsystem
	set( SHARED_ENGINE_LINKER_FLAGS
		${SHARED_ENGINE_LINKER_FLAGS} "/SUBSYSTEM:WINDOWS "
)
elseif( UNIX )
	#From the Github 2013 Makefile, match the settings for Unix environments.
	# force 387 for FP math so the precision between win32 and linux and osx match
	# Trigger an error if any code tries to use an implicit return type
	# Default visibility is hidden unless explicitly altered with __attribute__( visibility() )
	set( SHARED_ENGINE_LINKER_FLAGS
		${SHARED_ENGINE_LINKER_FLAGS} "-mfpmath=387 -Werror=return-type -fvisibility=hidden "
	)

	if( APPLE )
		set( SHARED_ENGINE_LINKER_FLAGS
			${SHARED_ENGINE_LINKER_FLAGS} "-momit-leaf-frame-pointer -mtune=core2 "
		)
	else()
		#Linux, Cygwin, etc.
	endif()
endif()

#Shared directories
#Include paths for external dependencies. Should always come after all project paths.
set( SHARED_EXTERNAL_INCLUDE_PATHS
)

set( SHARED_INCLUDE_PATHS
)

#Find shared dependencies
set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

find_package( OpenGL REQUIRED )

if( NOT OPENGL_FOUND )
	MESSAGE( FATAL_ERROR "Could not locate OpenGL library" )
endif()

find_library( GLEW2 glew32s ${CMAKE_SHARED_LIBRARY_PREFIX}GLEW${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${EXTERNAL_DIR}/GLEW/lib/ NO_DEFAULT_PATH )

if( UNIX )
	set( UNIX_FS_LIB stdc++fs )
else()
	set( UNIX_FS_LIB )
endif()

set( STEAM_API_SO ${BIN_DIR}/steam_api${CMAKE_SHARED_LIBRARY_SUFFIX} )

if( WIN32 )
	set( SDL2_NAME SDL2${CMAKE_STATIC_LIBRARY_SUFFIX} )
	set( SDL2_SO_NAME SDL2${CMAKE_SHARED_LIBRARY_SUFFIX} )
elseif( APPLE )
	set( SDL2_NAME libSDL2-2.0.0.dylib )
	set( SDL2_SO_NAME ${SDL2_NAME} )
elseif( UNIX )
	set( SDL2_NAME libSDL2-2.0.so.0 )
	set( SDL2_SO_NAME ${SDL2_NAME} )
else()
	set( SDL2_NAME )
	set( SDL2_SO_NAME ${SDL2_NAME} )
endif()

set( SDL2_SO ${EXTERNAL_DIR}/SDL2/lib/${SDL2_SO_NAME} )

#Copy libraries so they can be linked properly.
set( LIBS_TO_COPY ${STEAM_API_SO} ${SDL2_SO} )

foreach( lib IN LISTS LIBS_TO_COPY )
	MESSAGE( STATUS "Copying ${lib}" )
	file( COPY ${lib} DESTINATION ${ENGINE_BASE_PATH} )
endforeach()

#Link with the library in the deployment directory if present, this is needed to ensure that the correct library file is loaded.
find_library( STEAM_API steam_api PATHS ${ENGINE_BASE_PATH} ${LIB_DIR} NO_DEFAULT_PATH )
find_library( SDL2 ${SDL2_NAME} PATHS ${ENGINE_BASE_PATH} ${EXTERNAL_DIR}/SDL2/lib NO_DEFAULT_PATH )

#TODO: temporary until we have our own vstdlib - Solokiller
add_library( vstdlib SHARED IMPORTED )
set_property( TARGET vstdlib PROPERTY IMPORTED_LOCATION ${ENGINE_BASE_PATH}/vstdlib.dll )
set_property( TARGET vstdlib PROPERTY IMPORTED_IMPLIB ${SRC_DIR}/lib/public/vstdlib.lib )

add_subdirectory( vgui2 )
add_subdirectory( engine )
add_subdirectory( GameUI )
add_subdirectory( launcher )
add_subdirectory( tier0 )
add_subdirectory( tier1 )
