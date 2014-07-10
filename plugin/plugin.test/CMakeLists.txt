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
#pch_add_library(plugin.test SHARED PCH_HEADER "pch.h" ${_SOURCE_FILES})
add_library(plugin.test SHARED ${_SOURCE_FILES})
target_link_libraries(plugin.test liblightmetrica ${COMMON_LIBRARY_FILES})
add_dependencies(plugin.test liblightmetrica)

# Output directory
#set_target_properties(plugin.test PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/plugin")
#set_target_properties(plugin.test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugin")

# Solution directory
set_target_properties(plugin.test PROPERTIES FOLDER "plugin")

# Remove automatic lib- prefix
set_target_properties(plugin.test PROPERTIES PREFIX "")

# Copy dynamic libaries as post-build events
if(WIN32)
	# Add an custom command to copy an DLL to the same directory as executables
	function(add_custom_command_copy_dll BINARY_FILE_NAME)
		add_custom_command(
			TARGET plugin.test
			POST_BUILD
				# Execute "cmake -E copy_if_different ..."
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${BINARY_FILE_NAME}
					# Same directory as the target executable
					# For details, see 'generator expressions' in the documentation
					$<TARGET_FILE_DIR:plugin.test>)
	endfunction()
endif()
