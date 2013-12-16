#
# Project libnanon
#

include (PCHTargets)

set(_HEADER_FILES_ROOT "Header Files")
SET(_SOURCE_FILES_ROOT "Source Files")
set(_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/nanon)
set(
	_HEADER_FILES
	"${_INCLUDE_DIR}/common.h"
	"${_INCLUDE_DIR}/config.h"
	"${_INCLUDE_DIR}/logger.h"
	"${_INCLUDE_DIR}/version.h"
	"${_INCLUDE_DIR}/pugihelper.h"
	
	# pugixml headers
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
)
set(
	_SOURCE_FILES
	"config.cpp"
	"logger.cpp"
	"version.cpp"
	"pugihelper.cpp"
	
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
)
set(
	_ASSETS_SOURCES
	"asset.cpp"
	"defaultassets.cpp"
	"assetfactory.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\assets" FILES ${_ASSETS_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\assets" FILES ${_ASSETS_SOURCES})
list(APPEND _HEADER_FILES ${_ASSETS_HEADERS})
list(APPEND _SOURCE_FILES ${_ASSETS_SOURCES})

#

set(
	_ASSETS_CAMERAS_HEADERS
	"${_INCLUDE_DIR}/camera.h"
	"${_INCLUDE_DIR}/camerafactory.h"
)
set(
	_ASSETS_CAMERAS_SOURCES
	"camera.cpp"
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
	"${_INCLUDE_DIR}/lightfactory.h"
)
set(
	_ASSETS_LIGHTS_SOURCES
	"light.cpp"
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
	"${_INCLUDE_DIR}/filmfactory.h"
)
set(
	_ASSETS_FILMS_SOURCES
	"film.cpp"
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
)
set(
	_ASSETS_BSDFS_SOURCES
	"bsdf.cpp"
	"bsdffactory.cpp"
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
)
set(
	_ASSETS_TRIANGLE_MESHES_SOURCES
	"trianglemesh.cpp"
	"trianglemeshfactory.cpp"
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
)
source_group("${_HEADER_FILES_ROOT}\\math" FILES ${_MATH_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_HEADERS})

#

set(
	_MATH_FUNCTIONS_HEADERS
	"${_INCLUDE_DIR}/mathfunctions.h"
	"${_INCLUDE_DIR}/mathfunctions.inl"
)
source_group("${_HEADER_FILES_ROOT}\\math\\functions" FILES ${_MATH_FUNCTIONS_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_FUNCTIONS_HEADERS})

#

set(
	_MATH_TYPES_HEADERS
	"${_INCLUDE_DIR}/mathtypes.h"
	"${_INCLUDE_DIR}/vector.h"
	"${_INCLUDE_DIR}/vector.inl"
	"${_INCLUDE_DIR}/matrix.h"
	"${_INCLUDE_DIR}/matrix.inl"
	"${_INCLUDE_DIR}/quat.h"
	"${_INCLUDE_DIR}/quat.inl"
	"${_INCLUDE_DIR}/ssevector.h"
	"${_INCLUDE_DIR}/ssevector.inl"
	"${_INCLUDE_DIR}/ssematrix.h"
	"${_INCLUDE_DIR}/ssematrix.inl"
	"${_INCLUDE_DIR}/ssequat.h"
	"${_INCLUDE_DIR}/ssequat.inl"
	"${_INCLUDE_DIR}/avxvector.h"
	"${_INCLUDE_DIR}/avxvector.inl"
	"${_INCLUDE_DIR}/avxmatrix.h"
	"${_INCLUDE_DIR}/avxmatrix.inl"
	"${_INCLUDE_DIR}/avxquat.h"
	"${_INCLUDE_DIR}/avxquat.inl"
)
source_group("${_HEADER_FILES_ROOT}\\math\\types" FILES ${_MATH_TYPES_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_TYPES_HEADERS})

#

set(
	_MATH_SIMD_HEADERS
	"${_INCLUDE_DIR}/sse.h"
	"${_INCLUDE_DIR}/avx.h"
)
source_group("${_HEADER_FILES_ROOT}\\math\\simd" FILES ${_MATH_SIMD_HEADERS})
list(APPEND _HEADER_FILES ${_MATH_SIMD_HEADERS})

#

set(
	_RENDERER_HEADERS
	"${_INCLUDE_DIR}/renderer.h"
	"${_INCLUDE_DIR}/rendererfactory.h"
	"${_INCLUDE_DIR}/raycast.h"
)
set(
	_RENDERER_SOURCES
	"renderer.cpp"
	"rendererfactory.cpp"
	"raycast.cpp"
)
source_group("${_HEADER_FILES_ROOT}\\renderer" FILES ${_RENDERER_HEADERS})
source_group("${_SOURCE_FILES_ROOT}\\renderer" FILES ${_RENDERER_SOURCES})
list(APPEND _HEADER_FILES ${_RENDERER_HEADERS})
list(APPEND _SOURCE_FILES ${_RENDERER_SOURCES})

#

set(
	_SCENE_HEADERS
	"${_INCLUDE_DIR}/scene.h"
	"${_INCLUDE_DIR}/scenefactory.h"
	"${_INCLUDE_DIR}/naivescene.h"
	"${_INCLUDE_DIR}/primitive.h"
)
set(
	_SCENE_SOURCES
	"scene.cpp"
	"scenefactory.cpp"
	"naivescene.cpp"
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
add_definitions(-DNANON_EXPORTS)

pch_add_library(libnanon SHARED PCH_HEADER "pch.h" ${_HEADER_FILES} ${_SOURCE_FILES})
target_link_libraries(libnanon ${COMMON_LIBRARY_FILES})

# Copy dynamic libaries as post-build events
if(WIN32)
	file(GLOB BINARY_FILES "${CMAKE_SOURCE_DIR}/external/bin/${CMAKE_LIBRARY_ARCHITECTURE}/*.dll")
	foreach(BINARY_FILE_NAME ${BINARY_FILES})
	add_custom_command(
		TARGET libnanon
		POST_BUILD
			# Execute "cmake -E copy_if_different ..."
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${BINARY_FILE_NAME}
				# Same directory as the target executable
				# For details, see 'generator expressions' in the documentation
				$<TARGET_FILE_DIR:libnanon>)
	endforeach()
endif()
