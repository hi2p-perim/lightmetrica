#
#	Lightmetrica : A research-oriented renderer
#
#	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in
#	all copies or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#	THE SOFTWARE.
#

include (PCHTargets)

set(_HEADER_FILES_ROOT "Header Files")
set(_SOURCE_FILES_ROOT "Source Files")
set(_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/lightmetrica)
set(
	_HEADER_FILES
	"${_INCLUDE_DIR}/common.h"
	"${_INCLUDE_DIR}/object.h"
	"${_INCLUDE_DIR}/align.h"
	"${_INCLUDE_DIR}/pool.h"
	"${_INCLUDE_DIR}/assert.h"
	"${_INCLUDE_DIR}/config.h"
	"${_INCLUDE_DIR}/confignode.h"
	"${_INCLUDE_DIR}/defaultconfig.h"
	"${_INCLUDE_DIR}/logger.h"
	"${_INCLUDE_DIR}/version.h"
	"${_INCLUDE_DIR}/pugihelper.h"
	"${_INCLUDE_DIR}/ray.h"
	"${_INCLUDE_DIR}/intersection.h"
	"${_INCLUDE_DIR}/random.h"
	"${_INCLUDE_DIR}/surfacegeometry.h"
	
	# pugixml headers
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
)
set(
	_SOURCE_FILES
	"object.cpp"
	"align.cpp"
	"confignode.cpp"
	"defaultconfig.cpp"
	"logger.cpp"
	"version.cpp"
	"pugihelper.cpp"
	"random.cpp"
	
	# pugixml sources
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.cpp"
)

#

set(
	_ASSETS_HEADERS
	"${_INCLUDE_DIR}/asset.h"
	"${_INCLUDE_DIR}/assets.h"
	"${_INCLUDE_DIR}/defaultassets.h"
	"${_INCLUDE_DIR}/assetfactory.h"
	"${_INCLUDE_DIR}/emitter.h"
	"${_INCLUDE_DIR}/generalizedbsdf.h"
)
set(
	_ASSETS_SOURCES
	"asset.cpp"
	"assets.cpp"
	"defaultassets.cpp"
	"assetfactory.cpp"
	"emitter.cpp"
	"generalizedbsdf.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets" FILES ${_ASSETS_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets" FILES ${_ASSETS_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_SOURCES})

#

set(
	_ASSETS_CAMERAS_HEADERS
	"${_INCLUDE_DIR}/camera.h"
	"${_INCLUDE_DIR}/perspectivecamera.h"
	"${_INCLUDE_DIR}/thinlenscamera.h"
	"${_INCLUDE_DIR}/camerafactory.h"
)
set(
	_ASSETS_CAMERAS_SOURCES
	"camera.cpp"
	"perspectivecamera.cpp"
	"thinlenscamera.cpp"
	"camerafactory.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets\\cameras" FILES ${_ASSETS_CAMERAS_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets\\cameras" FILES ${_ASSETS_CAMERAS_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_CAMERAS_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_CAMERAS_SOURCES})

#

set(
	_ASSETS_LIGHTS_HEADERS
	"${_INCLUDE_DIR}/light.h"
	"${_INCLUDE_DIR}/arealight.h"
	"${_INCLUDE_DIR}/lightfactory.h"
)
set(
	_ASSETS_LIGHTS_SOURCES
	"light.cpp"
	"arealight.cpp"
	"lightfactory.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets\\lights" FILES ${_ASSETS_LIGHTS_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets\\lights" FILES ${_ASSETS_LIGHTS_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_LIGHTS_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_LIGHTS_SOURCES})

#

set(
	_ASSETS_FILMS_HEADERS
	"${_INCLUDE_DIR}/film.h"
	"${_INCLUDE_DIR}/hdrfilm.h"
	"${_INCLUDE_DIR}/filmfactory.h"
)
set(
	_ASSETS_FILMS_SOURCES
	"film.cpp"
	"hdrfilm.cpp"
	"filmfactory.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets\\films" FILES ${_ASSETS_FILMS_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets\\films" FILES ${_ASSETS_FILMS_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_FILMS_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_FILMS_SOURCES})

#

set(
	_ASSETS_BSDFS_HEADERS
	"${_INCLUDE_DIR}/bsdf.h"
	"${_INCLUDE_DIR}/bsdffactory.h"
	"${_INCLUDE_DIR}/diffuse.h"
)
set(
	_ASSETS_BSDFS_SOURCES
	"bsdf.cpp"
	"bsdffactory.cpp"
	"diffuse.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets\\bsdfs" FILES ${_ASSETS_BSDFS_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets\\bsdfs" FILES ${_ASSETS_BSDFS_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_BSDFS_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_BSDFS_SOURCES})

#

set(
	_ASSETS_TRIANGLE_MESHES_HEADERS
	"${_INCLUDE_DIR}/trianglemesh.h"
	"${_INCLUDE_DIR}/trianglemeshfactory.h"
	"${_INCLUDE_DIR}/objmesh.h"
	"${_INCLUDE_DIR}/rawmesh.h"
)
set(
	_ASSETS_TRIANGLE_MESHES_SOURCES
	"trianglemesh.cpp"
	"trianglemeshfactory.cpp"
	"objmesh.cpp"
	"rawmesh.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets\\trianglemeshes" FILES ${_ASSETS_TRIANGLE_MESHES_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets\\trianglemeshes" FILES ${_ASSETS_TRIANGLE_MESHES_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_TRIANGLE_MESHES_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_TRIANGLE_MESHES_SOURCES})

#

set(
	_ASSETS_TEXTURES_HEADERS
	"${_INCLUDE_DIR}/texture.h"
	"${_INCLUDE_DIR}/texturefactory.h"
)
set(
	_ASSETS_TEXTURES_SOURCES
	"texture.cpp"
	"texturefactory.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets\\textures" FILES ${_ASSETS_TEXTURES_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets\\textures" FILES ${_ASSETS_TEXTURES_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_TEXTURES_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_TEXTURES_SOURCES})

#

set(
	_MATH_HEADERS
	"${_INCLUDE_DIR}/math.h"
	"${_INCLUDE_DIR}/math.common.h"
	"${_INCLUDE_DIR}/math.simd.h"
)
source_group("${_HEADER_FILES_ROOT}\\math" FILES ${_MATH_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_HEADERS})

#

set(
	_MATH_FUNCTIONS_HEADERS
	"${_INCLUDE_DIR}/math.functions.h"
	"${_INCLUDE_DIR}/math.basic.h"
	"${_INCLUDE_DIR}/math.basic.inl"
	"${_INCLUDE_DIR}/math.transform.h"
	"${_INCLUDE_DIR}/math.transform.inl"
	"${_INCLUDE_DIR}/math.linalgebra.h"
	"${_INCLUDE_DIR}/math.linalgebra.inl"
	"${_INCLUDE_DIR}/math.stats.h"
	"${_INCLUDE_DIR}/math.stats.inl"
)
source_group("${_HEADER_FILES_ROOT}\\math\\functions" FILES ${_MATH_FUNCTIONS_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_FUNCTIONS_HEADERS})

#

set(
	_MATH_TYPES_HEADERS
	"${_INCLUDE_DIR}/math.types.h"
	"${_INCLUDE_DIR}/math.vector.h"
	"${_INCLUDE_DIR}/math.vector.inl"
	"${_INCLUDE_DIR}/math.matrix.h"
	"${_INCLUDE_DIR}/math.matrix.inl"
	"${_INCLUDE_DIR}/math.quat.h"
	"${_INCLUDE_DIR}/math.quat.inl"
	"${_INCLUDE_DIR}/math.constants.h"
	"${_INCLUDE_DIR}/math.constants.inl"
	"${_INCLUDE_DIR}/math.colors.h"
	"${_INCLUDE_DIR}/math.cast.h"
	"${_INCLUDE_DIR}/math.pdf.h"
)
source_group("${_HEADER_FILES_ROOT}\\math\\types" FILES ${_MATH_TYPES_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_TYPES_HEADERS})

#

set(
	_RENDERER_HEADERS
	"${_INCLUDE_DIR}/renderer.h"
	"${_INCLUDE_DIR}/rendererfactory.h"
	"${_INCLUDE_DIR}/renderutils.h"
	"${_INCLUDE_DIR}/raycast.h"
	"${_INCLUDE_DIR}/pathtrace.h"
	"${_INCLUDE_DIR}/lighttrace.h"
	"${_INCLUDE_DIR}/simplebpt.h"
	"${_INCLUDE_DIR}/explicitpathtrace.h"
	"${_INCLUDE_DIR}/bpt.h"
)
set(
	_RENDERER_SOURCES
	"rendererfactory.cpp"
	"renderutils.cpp"
	"raycast.cpp"
	"pathtrace.cpp"
	"lighttrace.cpp"
	"simplebpt.cpp"
	"explicitpathtrace.cpp"
	"bpt.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\renderer" FILES ${_RENDERER_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\renderer" FILES ${_RENDERER_SOURCES})
list(APPEND _HEADER_FILES ${_RENDERER_HEADERS})
list(APPEND _SOURCE_FILES ${_RENDERER_SOURCES})

#

set(
	_RENDERER_DAGPT_HEADERS
	"${_INCLUDE_DIR}/dagpt.h"
	"${_INCLUDE_DIR}/dagpt.common.h"
	"${_INCLUDE_DIR}/dagpt.graph.h"
	"${_INCLUDE_DIR}/dagpt.eval.h"
	"${_INCLUDE_DIR}/dagpt.pool.h"
)
set(
	_RENDERER_DAGPT_SOURCES
	"dagpt.cpp"
	"dagpt.graph.cpp"
	"dagpt.pool.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\renderer\\dagpt" FILES ${_RENDERER_DAGPT_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\renderer\\dagpt" FILES ${_RENDERER_DAGPT_SOURCES})
list(APPEND _HEADER_FILES ${_RENDERER_DAGPT_HEADERS})
list(APPEND _SOURCE_FILES ${_RENDERER_DAGPT_SOURCES})

#

set(
	_RENDERER_DAGPT_SAMPLER_HEADERS
	"${_INCLUDE_DIR}/dagpt.sampler.h"
	"${_INCLUDE_DIR}/dagpt.samplerfactory.h"
	"${_INCLUDE_DIR}/dagpt.pathtrace.h"
)
set(
	_RENDERER_DAGPT_SAMPLER_SOURCES
	"dagpt.samplerfactory.cpp"
	"dagpt.pathtrace.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\renderer\\dagpt\\sampler" FILES ${_RENDERER_DAGPT_SAMPLER_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\renderer\\dagpt\\sampler" FILES ${_RENDERER_DAGPT_SAMPLER_SOURCES})
list(APPEND _HEADER_FILES ${_RENDERER_DAGPT_SAMPLER_HEADERS})
list(APPEND _SOURCE_FILES ${_RENDERER_DAGPT_SAMPLER_SOURCES})

#

set(
	_SCENE_HEADERS
	"${_INCLUDE_DIR}/scene.h"
	"${_INCLUDE_DIR}/scenefactory.h"
	"${_INCLUDE_DIR}/naivescene.h"
	"${_INCLUDE_DIR}/bvhscene.h"
	"${_INCLUDE_DIR}/qbvhscene.h"
	"${_INCLUDE_DIR}/primitive.h"
	"${_INCLUDE_DIR}/triaccel.h"
	"${_INCLUDE_DIR}/triangleref.h"
	"${_INCLUDE_DIR}/aabb.h"
	"${_INCLUDE_DIR}/aabb.inl"
)
set(
	_SCENE_SOURCES
	"scene.cpp"
	"scenefactory.cpp"
	"naivescene.cpp"
	"bvhscene.cpp"
	"qbvhscene.cpp"
	"primitive.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\scene" FILES ${_SCENE_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\scene" FILES ${_SCENE_SOURCES})
list(APPEND _HEADER_FILES ${_SCENE_HEADERS})
list(APPEND _SOURCE_FILES ${_SCENE_SOURCES})

#

# Generate header for the version number
set(_GENERATED_FILES_ROOT "Generated Files")
set(_GENERATED_VERSION_DEF "${PROJECT_BINARY_DIR}/versiondef.h")
configure_file(
	"versiondef.h.in"
	${_GENERATED_VERSION_DEF}
)
source_group("${_GENERATED_FILES_ROOT}" FILES ${_GENERATED_VERSION_DEF})
list(APPEND _SOURCE_FILES ${_GENERATED_VERSION_DEF})

#

# Generate header for SIMD support
set(_GENERATED_SIMD_SUPPORT "${PROJECT_BINARY_DIR}/simdsupport.h")
configure_file(
	"simdsupport.h.in"
	${_GENERATED_SIMD_SUPPORT}
)
source_group("${_GENERATED_FILES_ROOT}" FILES ${_GENERATED_SIMD_SUPPORT})
list(APPEND _SOURCE_FILES ${_GENERATED_SIMD_SUPPORT})

#

# Proprocessor definition for exporting symbols
add_definitions(-DLM_EXPORTS)

# Create project
pch_add_library(liblightmetrica SHARED PCH_HEADER "pch.h" ${_HEADER_FILES} ${_SOURCE_FILES})
target_link_libraries(liblightmetrica ${COMMON_LIBRARY_FILES} ${ASSIMP_LIBRARIES} ${FREEIMAGE_LIBRARIES})

#

# Copy dynamic libaries as post-build events
if(WIN32)
	# Add an custom command to copy an DLL to the same directory as executables
	function(add_custom_command_copy_dll BINARY_FILE_NAME)
		add_custom_command(
			TARGET liblightmetrica
			POST_BUILD
				# Execute "cmake -E copy_if_different ..."
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${BINARY_FILE_NAME}
					# Same directory as the target executable
					# For details, see 'generator expressions' in the documentation
					$<TARGET_FILE_DIR:liblightmetrica>)
	endfunction()

	# Libraries in external directory
	file(GLOB BINARY_FILES "${CMAKE_SOURCE_DIR}/external/bin/${CMAKE_LIBRARY_ARCHITECTURE}/*.dll")
	foreach(BINARY_FILE_NAME ${BINARY_FILES})
		add_custom_command_copy_dll(${BINARY_FILE_NAME})
	endforeach()
	
	# Boost libraries
	# Release and RelWithDebInfo modes
	foreach(LIBRARY_NAME ${Boost_LIBRARIES_RELEASE})
		get_filename_component(LIBRARY_NAME_WE ${LIBRARY_NAME} NAME_WE)
		get_filename_component(LIBRARY_DIRECTORY ${LIBRARY_NAME} DIRECTORY)
		set(BINARY_FILE_NAME "${LIBRARY_DIRECTORY}/${LIBRARY_NAME_WE}.dll")
		add_custom_command_copy_dll(${BINARY_FILE_NAME})
	endforeach()
	
	# Debug mode
	foreach(LIBRARY_NAME ${Boost_LIBRARIES_DEBUG})
		get_filename_component(LIBRARY_NAME_WE ${LIBRARY_NAME} NAME_WE)
		get_filename_component(LIBRARY_DIRECTORY ${LIBRARY_NAME} DIRECTORY)
		set(BINARY_FILE_NAME "${LIBRARY_DIRECTORY}/${LIBRARY_NAME_WE}.dll")
		add_custom_command_copy_dll(${BINARY_FILE_NAME})
	endforeach()
	
endif()
