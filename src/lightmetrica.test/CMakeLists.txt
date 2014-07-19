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
	"main.cpp"
	"test.logger.cpp"
	"test.component.cpp"
	"test.confignode.cpp"
	"test.defaultconfig.cpp"
	"test.defaultassets.cpp"
	"test.defaultexpts.cpp"
	"test.asset.cpp"
	"test.scene.cpp"
	"test.scene.intersection.cpp"
	"test.qbvhscene.cpp"
	"test.objmesh.cpp"
	"test.rawmesh.cpp"
	"test.hdrfilm.cpp"
	"test.bitmap.cpp"
	"test.bitmaptexture.cpp"
	"test.perspectivecamera.cpp"
	"test.thinlenscamera.cpp"
	"test.pssmlt.sampler.cpp"
	"test.math.vector.cpp"
	"test.math.matrix.cpp"
	"test.math.basic.cpp"
	"test.math.transform.cpp"
	"test.math.stats.cpp"
	"test.bpt.mis.cpp"
	"test.bpt.mis.power.cpp"
	"test.bpt.fullpath.cpp"
	"test.bpt.fullpath2.cpp"
	"test.pm.photonmap.cpp"
	"test.generalizedbsdf.cpp"
	"test.plugin.cpp"
)

if(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
			endif()
endif()

pch_add_executable(lightmetrica.test PCH_HEADER "pch.h" ${_SOURCE_FILES})
target_link_libraries(lightmetrica.test liblightmetrica liblightmetrica.test ${COMMON_LIBRARY_FILES} ${FREEIMAGE_LIBRARIES})
add_dependencies(lightmetrica.test liblightmetrica liblightmetrica.test)

# Solution directory
set_target_properties(lightmetrica.test PROPERTIES FOLDER "test")

# Add a target for coverage calculation
#if(CMAKE_COMPILER_IS_GNUCXX)
#	if(CMAKE_BUILD_TYPE STREQUAL Debug)
#		include(CodeCoverage)
#		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
#		setup_target_for_coverage(test.coverage lightmetrica.test test.coverage)
#	endif()
#endif()

