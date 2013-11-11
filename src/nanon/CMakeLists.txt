#
# Project nanon
#

add_executable(nanon main.cpp)
target_link_libraries(nanon libnanon ${COMMON_LIBRARY_FILES})
add_dependencies(nanon libnanon)
