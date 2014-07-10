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

set(_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/lightmetrica.test")
set(_GTEST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/external/src/gtest-1.7.0")
set(_PUGIXML_SOURCE_DIR "${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2")
set(
	_HEADER_FILES
	"${_INCLUDE_DIR}/common.h"
	"${_INCLUDE_DIR}/base.h"
	"${_INCLUDE_DIR}/base.math.h"
	"${_INCLUDE_DIR}/stub.asset.h"
	"${_INCLUDE_DIR}/stub.assets.h"
	"${_INCLUDE_DIR}/stub.trianglemesh.h"
	"${_INCLUDE_DIR}/stub.bsdf.h"
	"${_INCLUDE_DIR}/stub.config.h"
	"${_INCLUDE_DIR}/stub.film.h"
)
set(
	_SOURCE_FILES
	"base.cpp"
	
	# gtest
	"${_GTEST_SOURCE_DIR}/src/gtest-all.cc"
	#"${_GTEST_SOURCE_DIR}/src/gtest_main.cc"

	# pugixml
	"${_PUGIXML_SOURCE_DIR}/src/pugixml.hpp"
	"${_PUGIXML_SOURCE_DIR}/src/pugiconfig.hpp"
	"${_PUGIXML_SOURCE_DIR}/src/pugixml.cpp"
)

pch_add_library(liblightmetrica.test STATIC PCH_HEADER "pch.h" ${_HEADER_FILES} ${_SOURCE_FILES})

# Solution directory
set_target_properties(liblightmetrica.test PROPERTIES FOLDER "core")
