#
#	Lightmetrica : A research-oriented renderer
#
#	Copyright (c) 2014 Hisanari Otsu
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

include (PCHTargets)

# Embree
find_package(Embree)
if (EMBREE_FOUND)
	include_directories(${EMBREE_INCLUDE_DIRS})

	set(
		_SOURCE_FILES
		"scene.embree.cpp"

		# Common files
		"${PROJECT_SOURCE_DIR}/include/lightmetrica/plugin.common.h"
		"${PROJECT_SOURCE_DIR}/src/liblightmetrica/plugin.common.cpp"
	)

	# Create project
	pch_add_library(plugin.embree SHARED PCH_HEADER "${PROJECT_SOURCE_DIR}/pch/pch.plugin.h" ${_SOURCE_FILES})
	target_link_libraries(plugin.embree liblightmetrica ${COMMON_LIBRARY_FILES} ${EMBREE_LIBRARIES})
	add_dependencies(plugin.embree liblightmetrica)

	# Output directory
	set_target_properties(plugin.embree PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	set_target_properties(plugin.embree PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

	# Solution directory
	set_target_properties(plugin.embree PROPERTIES FOLDER "plugin")

	# Remove automatic lib- prefix
	set_target_properties(plugin.embree PROPERTIES PREFIX "")
endif()