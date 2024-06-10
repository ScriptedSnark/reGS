
#
#	VGUI2 framework CMake build file
#

set( PROJECT_NAME vgui2 )

add_sources(
	${SRC_DIR}/public/interface.cpp
	${SRC_DIR}/public/interface.h
	${SRC_DIR}/public/UnicodeFileHelpers.cpp
	${SRC_DIR}/public/UnicodeFileHelpers.h
	Bitmap.cpp
	Bitmap.h
	Border.cpp
	Border.h
	input.cpp
	input.h
	LocalizedStringTable.cpp
	LocalizedStringTable.h
	MemoryBitmap.cpp
	MemoryBitmap.h
	Scheme.cpp
	Scheme.h
	vgui.cpp
	vgui.h
	vgui_key_translation.cpp
	vgui_key_translation.h
	VGuiKeyValues.cpp
	VGuiKeyValues.h
	VPanel.cpp
	VPanel.h
	VPanelWrapper.cpp
	VPanelWrapper.h
)

if( WIN32 )
	add_sources(
		system_win32.cpp
		system_win32.h
	)
else()
endif()

add_subdirectory( ${SRC_DIR}/common common )
add_subdirectory( ${SRC_DIR}/public public )

preprocess_sources()

add_library( ${PROJECT_NAME} SHARED ${PREP_SRCS} )

target_include_directories( ${PROJECT_NAME} PRIVATE
	${SRC_DIR}/common
	${SRC_DIR}/public
	${SRC_DIR}/public/tier1
	${SRC_DIR}
	${EXTERNAL_DIR}/SDL2/include
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
	${SHARED_DEFS}
	NO_MALLOC_OVERRIDE
)

set( PROJECT_LIBS )

if( WIN32 )
	set( PROJECT_LIBS
		${PROJECT_LIBS}
		Shlwapi.lib
	)
endif()

#Link with dependencies
target_link_libraries( ${PROJECT_NAME} 
	vgui2_controls
	vstdlib
	tier1
	tier0
	${PROJECT_LIBS}
	${SHARED_DEPENDENCIES}
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( ${PROJECT_NAME} PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${ENGINE_BASE_PATH}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ENGINE_BASE_PATH}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ENGINE_BASE_PATH}"
)

set( PROJECT_LINK_FLAGS )

set_target_properties( ${PROJECT_NAME} PROPERTIES
	COMPILE_FLAGS "${LINUX_32BIT_FLAG}"
	LINK_FLAGS "${SHARED_ENGINE_LINKER_FLAGS} ${PROJECT_LINK_FLAGS} ${LINUX_32BIT_FLAG}"
)
	
#No lib prefix
set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
