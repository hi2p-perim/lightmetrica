#
# Project libnanon
#

include (PCHTargets)

set(_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/nanon)
set(
	_HEADER_FILES
	"${_INCLUDE_DIR}/common.h"
	"${_INCLUDE_DIR}/config.h"
	"${_INCLUDE_DIR}/logger.h"
	"${_INCLUDE_DIR}/asset.h"
	"${_INCLUDE_DIR}/assets.h"
	"${_INCLUDE_DIR}/assetfactory.h"
	
	# pugixml headers
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
)

set(
	_SOURCE_FILES
	"config.cpp"
	"logger.cpp"
	"asset.cpp"
	"assets.cpp"
	"assetfactory.cpp"
	
	# pugixml sources
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.cpp"
)

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
