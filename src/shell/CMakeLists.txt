cmake_minimum_required (VERSION 2.6)
project (shell)

join_list("${CASPARCG_MODULE_INCLUDE_STATEMENTS}" "\n" CASPARCG_MODULE_INCLUDE_STATEMENTS)
join_list("${CASPARCG_MODULE_INIT_STATEMENTS}" "\n" CASPARCG_MODULE_INIT_STATEMENTS)
join_list("${CASPARCG_MODULE_UNINIT_STATEMENTS}" "\n" CASPARCG_MODULE_UNINIT_STATEMENTS)
join_list("${CASPARCG_MODULE_COMMAND_LINE_ARG_INTERCEPTORS_STATEMENTS}" "\n" CASPARCG_MODULE_COMMAND_LINE_ARG_INTERCEPTORS_STATEMENTS)
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/included_modules.tmpl" "${CMAKE_BINARY_DIR}/generated/included_modules.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/packages.config" "${CMAKE_CURRENT_BINARY_DIR}/packages.config")

if (MSVC)
	set(OS_SPECIFIC_SOURCES
			resource.h
			shell.rc
			windows_specific.cpp
	)
elseif (APPLE)
	set(OS_SPECIFIC_SOURCES
			macos_specific.cpp
			macos_event_loop.mm
	)
else ()
	set(OS_SPECIFIC_SOURCES
			linux_specific.cpp
	)
endif ()

set(SOURCES
		casparcg.config
		main.cpp
		server.cpp
)
set(HEADERS
		platform_specific.h
		server.h
)

if (APPLE)
	set(XCODE_ATTRIBUTE_PRODUCT_NAME "CasparCG Server Helper")
	set(MACOSX_BUNDLE_PRODUCT_NAME "CasparCG Server Helper")
	set(MACOSX_BUNDLE_BUNDLE_NAME "CasparCG Server Helper")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.casparcg.server-helper")
	set(MACOSX_BUNDLE_ICON_FILE "icon.icns")
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.1")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.1")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "2.1")
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
	set(CASPARCG_HELPER_NAME "casparcg Helper")
	include_directories(${CEF_ROOT_DIR})
	add_executable(${CASPARCG_HELPER_NAME} MACOSX_BUNDLE macos_helper.cpp)
	set_target_properties(${CASPARCG_HELPER_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks")
	target_link_libraries(${CASPARCG_HELPER_NAME}
		ffmpeg
		html
	)

	set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
	set(XCODE_ATTRIBUTE_PRODUCT_NAME "CasparCG Server")
	set(MACOSX_BUNDLE_PRODUCT_NAME "CasparCG Server")
	set(MACOSX_BUNDLE_BUNDLE_NAME "CasparCG Server")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.casparcg.server")
	set(MACOSX_BUNDLE_ICON_FILE "icon.icns")
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.1")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.1")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "2.1")
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
	add_executable(casparcg MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${OS_SPECIFIC_SOURCES})
	add_dependencies(casparcg "casparcg Helper")
else ()
	add_executable(casparcg ${SOURCES} ${HEADERS} ${OS_SPECIFIC_SOURCES})
endif ()

include_directories(..)
include_directories(${BOOST_INCLUDE_PATH})
include_directories(${TBB_INCLUDE_PATH})

source_group(sources ./*)

if (APPLE)
	target_link_libraries(casparcg
		"-framework CoreFoundation"
		"-framework AppKit"
	)
endif ()

target_link_libraries(casparcg
		accelerator
		common
		core
		protocol

		"${CASPARCG_MODULE_PROJECTS}"
)

if (MSVC)
	target_link_libraries(casparcg
		Winmm.lib
		Ws2_32.lib
		optimized tbb.lib
		debug tbb_debug.lib
		OpenGL32.lib
		glew32.lib
		openal32.lib
		debug zlibstaticd.lib
		optimized zlibstatic.lib
		debug sfml-graphics-d.lib
		debug sfml-window-d.lib
		debug sfml-system-d.lib
		optimized sfml-graphics.lib
		optimized sfml-window.lib
		optimized sfml-system.lib

		avformat.lib
		avcodec.lib
		avutil.lib
		avfilter.lib
		avdevice.lib
		swscale.lib
		swresample.lib

		d3d9.lib
		d3d11.lib
		dxgi.lib
	)
elseif (APPLE)
	target_link_libraries(${CASPARCG_HELPER_NAME}
		${Boost_LIBRARIES}
		${TBB_LIBRARIES}
		${TBB_MALLOC_LIBRARIES}
		${TBB_MALLOC_PROXY_LIBRARIES}
	)
	target_link_libraries(casparcg
		${Boost_LIBRARIES}
		${TBB_LIBRARIES}
		${TBB_MALLOC_LIBRARIES}
		${TBB_MALLOC_PROXY_LIBRARIES}
		${SFML_LIBRARIES}
		GLEW::glew
		iconv
		${OPENGL_gl_LIBRARY}
		${FFMPEG_LIBRARIES}
		dl
		z
		pthread
	)
else ()
	target_link_libraries(casparcg
		${Boost_LIBRARIES}
		${TBB_LIBRARIES}
		${TBB_MALLOC_LIBRARIES}
		${TBB_MALLOC_PROXY_LIBRARIES}
		${SFML_LIBRARIES}
		${GLEW_LIBRARIES}
		${OPENGL_gl_LIBRARY}
		${X11_LIBRARIES}
		${FFMPEG_LIBRARIES}
		dl
		icui18n
		icuuc
		z
		pthread
	)
endif ()

add_custom_target(casparcg_copy_dependencies ALL)

set(OUTPUT_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
add_custom_command(TARGET casparcg_copy_dependencies POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${OUTPUT_FOLDER}\")



if (MSVC)
	foreach(FILE_TO_COPY ${CASPARCG_RUNTIME_DEPENDENCIES})
		if(IS_DIRECTORY ${FILE_TO_COPY})
			get_filename_component(FOLDER_NAME "${FILE_TO_COPY}" NAME)
			add_custom_command(TARGET casparcg_copy_dependencies POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory \"${FILE_TO_COPY}\" \"${OUTPUT_FOLDER}/${FOLDER_NAME}\")
			add_custom_command(TARGET casparcg_copy_dependencies POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory \"${FILE_TO_COPY}\" \"${CMAKE_CURRENT_BINARY_DIR}/${FOLDER_NAME}\")
		else()
			add_custom_command(TARGET casparcg_copy_dependencies POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${FILE_TO_COPY}\" \"${OUTPUT_FOLDER}/\")
			add_custom_command(TARGET casparcg_copy_dependencies POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${FILE_TO_COPY}\" \"${CMAKE_CURRENT_BINARY_DIR}/\")
		endif()
	endforeach(FILE_TO_COPY)
elseif (APPLE)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CEF_ROOT_DIR}/\${CONFIGURATION}/Chromium Embedded Framework.framework" "\${TARGET_BUILD_DIR}/\${CONTENTS_FOLDER_PATH}/Frameworks/Chromium Embedded Framework.framework")
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "\${BUILT_PRODUCTS_DIR}/casparcg Helper.app" "\${TARGET_BUILD_DIR}/\${CONTENTS_FOLDER_PATH}/Frameworks/casparcg Helper.app")
else ()
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/shell/casparcg ${CMAKE_BINARY_DIR}/staging/bin/casparcg)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/shell/casparcg.config ${CMAKE_BINARY_DIR}/staging/casparcg.config)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/shell/liberation-fonts/LiberationMono-Regular.ttf ${CMAKE_BINARY_DIR}/staging/LiberationMono-Regular.ttf)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/shell/run.sh ${CMAKE_BINARY_DIR}/staging/run.sh)

# Disabled as HTML module is disabled
  if (ENABLE_HTML)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CEF_RESOURCE_PATH}/locales ${CMAKE_BINARY_DIR}/staging/bin/locales)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_RESOURCE_PATH}/cef.pak ${CMAKE_BINARY_DIR}/staging/bin/cef.pak)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_RESOURCE_PATH}/cef_100_percent.pak ${CMAKE_BINARY_DIR}/staging/bin/cef_100_percent.pak)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_RESOURCE_PATH}/cef_200_percent.pak ${CMAKE_BINARY_DIR}/staging/bin/cef_200_percent.pak)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_RESOURCE_PATH}/cef_extensions.pak ${CMAKE_BINARY_DIR}/staging/bin/cef_extensions.pak)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_RESOURCE_PATH}/devtools_resources.pak ${CMAKE_BINARY_DIR}/staging/bin/devtools_resources.pak)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_RESOURCE_PATH}/icudtl.dat ${CMAKE_BINARY_DIR}/staging/bin/icudtl.dat)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CEF_BIN_PATH}/swiftshader ${CMAKE_BINARY_DIR}/staging/bin/swiftshader)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BIN_PATH}/natives_blob.bin ${CMAKE_BINARY_DIR}/staging/bin/natives_blob.bin)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BIN_PATH}/snapshot_blob.bin ${CMAKE_BINARY_DIR}/staging/bin/snapshot_blob.bin)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BIN_PATH}/v8_context_snapshot.bin ${CMAKE_BINARY_DIR}/staging/bin/v8_context_snapshot.bin)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BIN_PATH}/libcef.so ${CMAKE_BINARY_DIR}/staging/lib/libcef.so)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BIN_PATH}/libEGL.so ${CMAKE_BINARY_DIR}/staging/lib/libEGL.so)
	ADD_CUSTOM_COMMAND (TARGET casparcg POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BIN_PATH}/libGLESv2.so ${CMAKE_BINARY_DIR}/staging/lib/libGLESv2.so)
  endif ()
endif ()
