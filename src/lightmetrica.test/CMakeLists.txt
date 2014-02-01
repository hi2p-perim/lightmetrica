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
	"test.logger.cpp"
	"test.confignode.cpp"
	"test.defaultconfig.cpp"
	"test.defaultassets.cpp"
	"test.asset.cpp"
	"test.scene.cpp"
	"test.scene.intersection.cpp"
	"test.qbvhscene.cpp"
	"test.objmesh.cpp"
	"test.rawmesh.cpp"
	"test.hdrfilm.cpp"
	"test.perspectivecamera.cpp"
	"test.math.vector.cpp"
	"test.math.matrix.cpp"
	"test.math.basic.cpp"
	"test.math.transform.cpp"
	"test.math.stats.cpp"
)

if(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
			endif()
endif()

pch_add_executable(lightmetrica.test PCH_HEADER "pch.h" ${_SOURCE_FILES})
target_link_libraries(lightmetrica.test liblightmetrica liblightmetrica.test ${COMMON_LIBRARY_FILES})
add_dependencies(lightmetrica.test liblightmetrica liblightmetrica.test)

# Add a target for coverage calculation
#if(CMAKE_COMPILER_IS_GNUCXX)
#	if(CMAKE_BUILD_TYPE STREQUAL Debug)
#		include(CodeCoverage)
#		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
#		setup_target_for_coverage(test.coverage lightmetrica.test test.coverage)
#	endif()
#endif()

