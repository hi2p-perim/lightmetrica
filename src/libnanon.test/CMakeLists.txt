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

set(_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/nanon.test)
set(
	_HEADER_FILES
	"${_INCLUDE_DIR}/common.h"
	"${_INCLUDE_DIR}/base.h"
	"${_INCLUDE_DIR}/base.math.h"
	"${_INCLUDE_DIR}/stub.asset.h"
	"${_INCLUDE_DIR}/stub.assetfactory.h"
	"${_INCLUDE_DIR}/stub.assets.h"
	"${_INCLUDE_DIR}/stub.trianglemesh.h"
	"${_INCLUDE_DIR}/stub.bsdf.h"
)
set(
	_SOURCE_FILES
	"base.cpp"
	"stub.assetfactory.cpp"
	
	# gtest
	"${PROJECT_SOURCE_DIR}/external/src/gtest-1.7.0/src/gtest-all.cc"
	"${PROJECT_SOURCE_DIR}/external/src/gtest-1.7.0/src/gtest_main.cc"

	# pugixml
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.cpp"
)

pch_add_library(libnanon.test STATIC PCH_HEADER "pch.h" ${_HEADER_FILES} ${_SOURCE_FILES})
