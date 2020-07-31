cmake_minimum_required (VERSION 2.6)
project (accelerator)

if (APPLE)
	set(METAL_SOURCES
		metal/image/fragment.metal
		metal/image/vertex.metal
	)
	set_source_files_properties(${METAL_SOURCES} PROPERTIES LANGUAGE METAL)

	set(SOURCES
		metal/image/image_mixer.mm
		metal/image/image_shader.mm

		metal/util/buffer.mm
		metal/util/device.mm
		metal/util/shader.mm
		metal/util/texture.mm

		accelerator.cpp
		StdAfx.cpp
	)
	set(HEADERS
		metal/image/image_mixer.h
		metal/image/image_shader.h

		metal/util/buffer.h
		metal/util/device.h
		metal/util/shader.h
		metal/util/texture.h

		accelerator.h
		StdAfx.h
	)

	source_group(sources\\metal\\image metal/image/.*)
	source_group(sources\\metal\\util metal/util/.*)
else ()
	set(SOURCES
		ogl/image/image_kernel.cpp
		ogl/image/image_mixer.cpp
		ogl/image/image_shader.cpp

		ogl/util/buffer.cpp
		ogl/util/device.cpp
		ogl/util/shader.cpp
		ogl/util/texture.cpp

		accelerator.cpp
		StdAfx.cpp
	)
	if (MSVC)
		set(WIN32_SPECIFIC_SOURCES
			d3d/d3d_device.cpp
			d3d/d3d_device_context.cpp
			d3d/d3d_texture2d.cpp
		)
	endif ()
	set(HEADERS
		ogl/image/image_kernel.h
		ogl/image/image_mixer.h
		ogl/image/image_shader.h

		ogl/util/buffer.h
		ogl/util/device.h
		ogl/util/shader.h
		ogl/util/texture.h

		ogl_image_vertex.h
		ogl_image_fragment.h

		accelerator.h
		StdAfx.h
	)
	if (MSVC)
		set(WIN32_SPECIFIC_HEADERS
			d3d/d3d_device.h
			d3d/d3d_device_context.h
			d3d/d3d_texture2d.h
		)
	endif ()

	bin2c("ogl/image/shader.vert" "ogl_image_vertex.h" "caspar::accelerator::ogl" "vertex_shader")
	bin2c("ogl/image/shader.frag" "ogl_image_fragment.h" "caspar::accelerator::ogl" "fragment_shader")

	source_group(sources\\ogl\\image ogl/image/.*)
	source_group(sources\\ogl\\util ogl/util/.*)
	source_group(sources\\d3d d3d/.*)

	include_directories(${GLEW_INCLUDE_PATH})
	include_directories(${SFML_INCLUDE_PATH})
endif ()

add_library(accelerator ${SOURCES} ${HEADERS} ${WIN32_SPECIFIC_SOURCES} ${WIN32_SPECIFIC_HEADERS} ${METAL_SOURCES})
add_precompiled_header(accelerator StdAfx.h FORCEINCLUDE)

include_directories(..)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BOOST_INCLUDE_PATH})
include_directories(${TBB_INCLUDE_PATH})
include_directories(${FFMPEG_INCLUDE_PATH})

source_group(sources ./.*)
source_group(sources\\cpu\\image cpu/image/.*)
source_group(sources\\cpu\\util cpu/util/.*)


target_link_libraries(accelerator common core)
