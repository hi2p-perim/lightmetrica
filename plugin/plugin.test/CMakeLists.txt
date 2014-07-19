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

set(
	_SOURCE_FILES
	"test.cpp"

	# Common files
	"${PROJECT_SOURCE_DIR}/include/lightmetrica/plugin.common.h"
	"${PROJECT_SOURCE_DIR}/src/liblightmetrica/plugin.common.cpp"
)

# Create project
pch_add_library(plugin.test SHARED PCH_HEADER "${PROJECT_SOURCE_DIR}/pch/pch.plugin.h" ${_SOURCE_FILES})
target_link_libraries(plugin.test liblightmetrica ${COMMON_LIBRARY_FILES})
add_dependencies(plugin.test liblightmetrica)

# Output directory
set_target_properties(plugin.test PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(plugin.test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Solution directory
set_target_properties(plugin.test PROPERTIES FOLDER "plugin")

# Remove automatic lib- prefix
set_target_properties(plugin.test PROPERTIES PREFIX "")
