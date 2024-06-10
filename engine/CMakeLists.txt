#
#Hardware engine (OpenGL)
#

set( PROJECT_NAME hw )

#Add in the directories
add_sources(
	${SRC_DIR}/cl_dll/kbutton.h
	${SRC_DIR}/cl_dll/wrect.h
	${SRC_DIR}/common/BaseUI/IBaseUI.h
	${SRC_DIR}/common/GameUI/CareerDefs.h
	${SRC_DIR}/common/GameUI/ICareerUI.h
	${SRC_DIR}/common/GameUI/IClientVGUI.h
	${SRC_DIR}/common/GameUI/IGameConsole.h
	${SRC_DIR}/common/GameUI/IGameUI.h
	${SRC_DIR}/common/BaseSystemModule.cpp
	${SRC_DIR}/common/BaseSystemModule.h
	${SRC_DIR}/common/beamdef.h
	${SRC_DIR}/common/bspfile.h
	${SRC_DIR}/common/cl_entity.h
	${SRC_DIR}/common/com_model.h
	${SRC_DIR}/common/con_nprint.h
	${SRC_DIR}/common/const.h
	${SRC_DIR}/common/crc.h
	${SRC_DIR}/common/cvardef.h
	${SRC_DIR}/common/demo_api.h
	${SRC_DIR}/common/dlight.h
	${SRC_DIR}/common/dll_state.h
	${SRC_DIR}/common/EngineSurface.h
	${SRC_DIR}/common/entity_state.h
	${SRC_DIR}/common/event_api.h
	${SRC_DIR}/common/event_args.h
	${SRC_DIR}/common/event_flags.h
	${SRC_DIR}/common/FilePaths.cpp
	${SRC_DIR}/common/FilePaths.h
	${SRC_DIR}/common/IBaseSystem.h
	${SRC_DIR}/common/ienginevgui.h
	${SRC_DIR}/common/IEngineWrapper.h
	${SRC_DIR}/common/in_buttons.h
	${SRC_DIR}/common/IObjectContainer.h
	${SRC_DIR}/common/ISystemModule.h
	${SRC_DIR}/common/ivoicetweak.h
	${SRC_DIR}/common/net_api.h
	${SRC_DIR}/common/netadr.h
	${SRC_DIR}/common/ObjectList.cpp
	${SRC_DIR}/common/ObjectList.h
	${SRC_DIR}/common/particledef.h
	${SRC_DIR}/common/pmtrace.h
	${SRC_DIR}/common/r_efx.h
	${SRC_DIR}/common/r_studioint.h
	${SRC_DIR}/common/ref_params.h
	${SRC_DIR}/common/screenfade.h
	${SRC_DIR}/common/Sequence.h
	${SRC_DIR}/common/studio_event.h
	${SRC_DIR}/common/TokenLine.cpp
	${SRC_DIR}/common/TokenLine.h
	${SRC_DIR}/common/triangleapi.h
	${SRC_DIR}/common/usercmd.h
	${SRC_DIR}/common/weaponinfo.h
	${SRC_DIR}/common/winheaders.h
	${SRC_DIR}/common/winsani_in.h
	${SRC_DIR}/common/winsani_out.h
	${SRC_DIR}/pm_shared/pm_debug.h
	${SRC_DIR}/pm_shared/pm_defs.h
	${SRC_DIR}/pm_shared/pm_info.h
	${SRC_DIR}/pm_shared/pm_materials.h
	${SRC_DIR}/pm_shared/pm_movevars.h
	${SRC_DIR}/public/engine_hlds_api.h
	${SRC_DIR}/public/engine_launcher_api.h
	${SRC_DIR}/public/idedicatedexports.h
	${SRC_DIR}/public/igameuifuncs.h
	${SRC_DIR}/public/interface.cpp
	${SRC_DIR}/public/interface.h
	${SRC_DIR}/public/IRegistry.h
	${SRC_DIR}/public/keydefs.h
	${SRC_DIR}/public/registry.cpp
	${SRC_DIR}/public/registry.h
	${SRC_DIR}/vgui2/vgui_surfacelib/FontAmalgam.cpp
	${SRC_DIR}/vgui2/vgui_surfacelib/FontAmalgam.h
	${SRC_DIR}/vgui2/vgui_surfacelib/FontManager.cpp
	${SRC_DIR}/vgui2/vgui_surfacelib/FontManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/BasePanel.h
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/BaseUI_Interface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/BaseUI_Interface.h
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/BaseUISurface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/BaseUISurface.h
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/FontTextureCache.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/FontTextureCache.h
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/IMouseControl.h
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/text_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vgui2/text_draw.h
	anorms.h
	APIProxy.h
	buildnum.cpp
	buildnum.h
	cd.cpp
	cdaudio.h
	cdll_exp.cpp
	cdll_exp.h
	cdll_int.cpp
	cdll_int.h
	chase.cpp
	chase.h
	cl_demo.cpp
	cl_demo.h
	cl_draw.cpp
	cl_draw.h
	cl_ents.cpp
	cl_ents.h
	cl_main.cpp
	cl_main.h
	cl_parse.cpp
	cl_parse.h
	cl_parsefn.cpp
	cl_parsefn.h
	cl_pmove.cpp
	cl_pmove.h
	cl_pred.cpp
	cl_pred.h
	cl_spectator.cpp
	cl_spectator.h
	cl_tent.cpp
	cl_tent.h
	client.h
	cmd.cpp
	cmd.h
	cmodel.cpp
	cmodel.h
	com_custom.cpp
	com_custom.h
	common.cpp
	common.h
	console.cpp
	console.h
	crc.cpp
	custom.h
	customentity.h
	cvar.cpp
	cvar.h
	decals.cpp
	decals.h
	delta.cpp
	delta.h
	DemoPlayerWrapper.cpp
	DemoPlayerWrapper.h
	DetailTexture.cpp
	DetailTexture.h
	edict.h
	eiface.h
	eventapi.cpp
	eventapi.h
	filesystem.cpp
	filesystem.h
	filesystem_internal.cpp
	gl_draw.cpp
	gl_draw.h
	gl_mesh.cpp
	gl_mesh.h
	gl_model.cpp
	gl_model.h
	gl_rmain.cpp
	gl_rmain.h
	gl_rmisc.cpp
	gl_rmisc.h
	gl_rsurf.cpp
	gl_rsurf.h
	gl_screen.cpp
	gl_screen.h
	gl_vidnt.cpp
	gl_vidnt.h
	gl_warp.cpp
	glHud.cpp
	glHud.h
	hashpak.cpp
	hashpak.h
	host.cpp
	host.h
	host_cmd.cpp
	host_cmd.h
	HUD.cpp
	HUD.h
	IEngine.h
	IGame.h
	info.cpp
	info.h
	ipratelimit.cpp
	ipratelimit.h
	ipratelimitWrapper.cpp
	ipratelimitWrapper.h
	keys.cpp
	keys.h
	LoadBlob.cpp
	LoadBlob.h
	mathlib.cpp
	mathlib.h
	mem.cpp
	mem.h
	modelgen.h
	modinfo.h
	module.cpp
	module.h
	net.h
	net_api.cpp
	net_api_int.h
	net_chan.cpp
	net_chan.h
	net_ws.cpp
	pmove.cpp
	pmove.h
	pmovetst.cpp
	pmovetst.h
	pr_cmds.cpp
	pr_cmds.h
	pr_edict.cpp
	pr_edict.h
	progdefs.h
	progs.h
	protocol.h
	qgl.cpp
	qgl.h
	quakedef.h
	r_efx.cpp
	r_efx_int.h
	r_part.cpp
	r_part.h
	r_studio.cpp
	r_studio.h
	r_triangle.cpp
	r_triangle.h
	render.h
	sequence.cpp
	server.h
	shake.h
	sound.h
	snd_dma.cpp
	snd_mix.cpp
	snd_sdl.cpp
	spritegn.h
	studio.h
	sv_log.cpp
	sv_log.h
	sv_main.cpp
	sv_main.h
	sv_phys.cpp
	sv_phys.h
	sv_secure.cpp
	sv_secure.h
	sv_steam3.cpp
	sv_steam3.h
	sv_upld.cpp
	sv_upld.h
	sys.h
	sys_dll.cpp
	sys_dll2.cpp
	sys_engine.cpp
	sys_getmodes.cpp
	sys_getmodes.h
	sys_sdlwind.cpp
	SystemWrapper.cpp
	SystemWrapper.h
	textures.cpp
	textures.h
	tmessage.cpp
	tmessage.h
	traceinit.cpp
	traceinit.h
	vgui_EngineSurface.cpp
	vgui_EngineSurface.h
	vgui_EngineSurfaceHW.cpp
	vgui_EngineSurfaceWin32.cpp
	VGUI_EngineSurfaceWrap.cpp
	VGUI_EngineSurfaceWrap.h
	vgui_int.cpp
	vgui_int.h
	vgui_intwrap.cpp
	vgui_intwrap2.cpp
	vid.h
	view.cpp
	view.h
	voice.cpp
	voice.h
	wad.cpp
	wad.h
	world.cpp
	world.h
	zone.cpp
	zone.h
)

if( WIN32 )
	add_sources(
		${SRC_DIR}/vgui2/vgui_surfacelib/Win32Font.cpp
		${SRC_DIR}/vgui2/vgui_surfacelib/Win32Font.h
	)
elseif( UNIX )
	add_sources(
		${SRC_DIR}/vgui2/vgui_surfacelib/linuxfont.cpp
		${SRC_DIR}/vgui2/vgui_surfacelib/linuxfont.h
	)
endif()

preprocess_sources()

add_library( ${PROJECT_NAME} SHARED ${PREP_SRCS} )

target_include_directories( ${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${SRC_DIR}
	${SRC_DIR}/cl_dll
	${SRC_DIR}/common
	${SRC_DIR}/pm_shared/
	${SRC_DIR}/public
	${SRC_DIR}/public/tier1
	${SRC_DIR}/utils/vgui/include
	${SHARED_INCLUDE_PATHS}
	${EXTERNAL_DIR}/SDL2/include
	${EXTERNAL_DIR}/GLEW/include
	${OPENGL_INCLUDE_DIR}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
	${SHARED_DEFS}
	USE_BREAKPAD_HANDLER
	GLEW_STATIC
)

#Find dependencies
set( PROJECT_LIBS )
if( WIN32 )
	set( PROJECT_LIBS ${PROJECT_LIBS} wsock32 ws2_32 )
endif()

if( UNIX )
	if ( OSX )
		set( VGUI1_NAME "vgui.dylib" )
	else()
		set( VGUI1_NAME "vgui.so" )
	endif()
else()
	set( VGUI1_NAME "vgui.lib" )
endif()

#VGUI1 has issues linking due to its non-standard filename. To work around this, we can instead import it and manually specify the location.
#That sets the correct linker path and filename.
add_library( vgui SHARED IMPORTED )
set_property( TARGET vgui PROPERTY IMPORTED_LOCATION ${ENGINE_BASE_PATH}/${VGUI1_NAME} )
set_property( TARGET vgui PROPERTY IMPORTED_IMPLIB ${SRC_DIR}/utils/vgui/lib/win32_vc6/vgui.lib )

#Link with dependencies
target_link_libraries( ${PROJECT_NAME} 
	${SDL2}
	${GLEW2}
	${OPENGL_LIBRARIES}
	vgui2_controls
	vgui
	vstdlib
	tier1
	tier0
	${STEAM_API}
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

#
#End library
#