#
# Project nanon
#

set(
	_SOURCE_FILES
	main.cpp
)

add_executable(nanon ${_SOURCE_FILES})
target_link_libraries(nanon libnanon ${COMMON_LIBRARY_FILES})
add_dependencies(nanon libnanon)
