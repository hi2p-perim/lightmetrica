#
#	Lightmetrica : A research-oriented renderer
#
#	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in
#	all copies or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#	THE SOFTWARE.
#

include (PCHTargets)

set(
	_SOURCE_FILES
	"test.cpp"
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
