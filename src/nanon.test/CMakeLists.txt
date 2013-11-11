#
# Project nanon.test
#

add_executable(nanon.test main.cpp)
target_link_libraries(nanon.test libnanon ${GTEST_BOTH_LIBRARIES})
add_dependencies(nanon.test libnanon)
