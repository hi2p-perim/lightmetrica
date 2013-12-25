#
#	nanon : A research-oriented renderer
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
	"common.h"
	"base.h"
	"base.cpp"
	"base.math.h"
	"stub.assetfactory.h"
	"stub.assetfactory.cpp"
	"stub.asset.h"
	"stub.assets.h"
	"stub.trianglemesh.h"
	"stub.bsdf.h"
	"test.logger.cpp"
	"test.config.cpp"
	"test.defaultassets.cpp"
	"test.asset.cpp"
	"test.scene.cpp"
	"test.scene.intersection.cpp"
	"test.objmesh.cpp"
	"test.hdrfilm.cpp"
	"test.perspectivecamera.cpp"
	
	# math
	"test.math.vector.cpp"
	"test.math.matrix.cpp"
	"test.math.transform.cpp"
	
	# pugixml
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.cpp"
)

pch_add_executable(nanon.test PCH_HEADER "pch.h" ${_SOURCE_FILES})
target_link_libraries(nanon.test libnanon ${GTEST_BOTH_LIBRARIES})
add_dependencies(nanon.test libnanon)
