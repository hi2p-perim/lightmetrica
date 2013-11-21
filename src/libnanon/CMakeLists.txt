#
# Project libnanon
#

include (PCHTargets)

set(_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/nanon)
set(
	_HEADER_FILES
	"${_INCLUDE_DIR}/common.h"
	"${_INCLUDE_DIR}/config.h"
	
	# pugixml headers
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
)

set(
	_SOURCE_FILES
	"config.cpp"
	
	# pugixml sources
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.cpp"
)

# Proprocessor definition for exporting symbols
add_definitions(-DNN_EXPORTS)

pch_add_library(
	libnanon SHARED
	PCH_HEADER "pch.h"
	${_HEADER_FILES} ${_SOURCE_FILES})
target_link_libraries(libnanon ${COMMON_LIBRARY_FILES})
