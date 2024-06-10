###################################################
#                                                 #
#                                                 #
#   Tier1 CMake build file                        #
#                                                 #
#                                                 #
###################################################

set( PROJECT_NAME tier1 )

add_sources(
	bitbuf.cpp
	byteswap.cpp
	CharacterSet.cpp
	checksum_crc.cpp
	checksum_md5.cpp
	ConVar.cpp
	DataManager.cpp
	Diff.cpp
	generichash.cpp
	#interface.cpp
	JobThread.cpp
	KeyValues.cpp
	MemPool.cpp
	MemStack.cpp
	netadr.cpp
	processor_detect.cpp
	RangeCheckedVar.cpp
	StringPool.cpp
	strtools.cpp
	tier1.cpp
	TokenReader.cpp
	unicode_strtools.cpp
	UtlBuffer.cpp
	UtlString.cpp
	UtlSymbol.cpp
	xboxstubs.cpp
)

#Included by processor_detect.cpp
#if( UNIX )
#	add_sources(
#		processor_detect_linux.cpp
#	)
#endif()

preprocess_sources()

add_library( ${PROJECT_NAME} STATIC ${PREP_SRCS} )

target_include_directories( ${PROJECT_NAME} PRIVATE
	${SRC_DIR}/public
	${SRC_DIR}/public/tier1
	${SRC_DIR}/common
	${SRC_DIR}/public
	${SRC_DIR}
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
	${SHARED_DEFS}
	TIER1_STATIC_LIB
)

set( PROJECT_LINK_FLAGS )

if( NOT MSVC AND NOT APPLE )
	#Generate a map file that provides information about the linking stage.
	set( PROJECT_LINK_FLAGS
		${PROJECT_LINK_FLAGS} "-Wl,-Map,${PROJECT_NAME}_map.txt "
	)
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES
	COMPILE_FLAGS "${LINUX_32BIT_FLAG}"
	LINK_FLAGS "${SHARED_ENGINE_LINKER_FLAGS} ${PROJECT_LINK_FLAGS} ${LINUX_32BIT_FLAG}"
)

#No lib prefix
set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
