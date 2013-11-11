#
# Project nanon.gui
#

add_executable(nanon.gui main.cpp)
target_link_libraries(nanon.gui libnanon ${COMMON_LIBRARY_FILES} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
add_dependencies(nanon.gui libnanon)
