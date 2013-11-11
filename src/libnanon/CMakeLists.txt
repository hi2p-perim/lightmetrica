#
# Project libnanon
#

set(_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/nanon)
set(
	_HEADER_FILES
	${_INCLUDE_DIR}/common.h
	${_INCLUDE_DIR}/test.h
)

set(
	_SOURCE_FILES
	test.cpp
)

# Proprocessor definition for exporting symbols
add_definitions(-DNN_EXPORTS)

add_library(libnanon SHARED ${_HEADER_FILES} ${_SOURCE_FILES})
target_link_libraries(libnanon ${COMMON_LIBRARY_FILES})
