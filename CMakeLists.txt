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

cmake_minimum_required(VERSION 2.8)

project(lightmetrica)

#

# Add module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# For windows environment, set search path for find_package
if (MSVC)
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external")
	if (CMAKE_CL_64)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "i386")
	endif()
endif()

# Set Release as the default build mode
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING
		"Choose build type (Debug, Release, RelWithDebInfo, or MinSizeRel)" FORCED)
endif()

#

# External libraries

# OpenMP
find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# Boost
# Use dynamic libraries
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)		# Specify to use dynamic libraries
#add_definitions(-DBOOST_ALL_NO_LIB)		# Disable auto-linking in VS 
find_package(Boost 1.53 REQUIRED COMPONENTS "program_options" "filesystem" "system" "regex")
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND COMMON_LIBRARY_FILES ${Boost_LIBRARIES})
set(Boost_LIBRARIES_RELEASE
	${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
	${Boost_FILESYSTEM_LIBRARY_RELEASE}
	${Boost_SYSTEM_LIBRARY_RELEASE}
	${Boost_REGEX_LIBRARY_RELEASE})
set(Boost_LIBRARIES_DEBUG
	${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG}
	${Boost_FILESYSTEM_LIBRARY_DEBUG}
	${Boost_SYSTEM_LIBRARY_DEBUG}
	${Boost_REGEX_LIBRARY_DEBUG})

# GLM
#find_package(GLM REQUIRED)
#include_directories(${GLM_INCLUDE_DIRS})

# GLEW
#set(GLEW_USE_STATIC_LIBS ON)
find_package(GLEW)
if (GLEW_FOUND)
	include_directories(${GLEW_INCLUDE_DIRS})
	add_definitions(${GLEW_DEFINITIONS})
	list(APPEND LIBRARY_FILES ${GLEW_LIBRARIES})
endif()

# GLFW
#set(GLFW_USE_STATIC_LIBS ON)
find_package(GLFW)
if (GLFW_FOUND)
	include_directories(${GLFW_INCLUDE_DIRS})
	list(APPEND LIBRARY_FILES ${GLFW_LIBRARIES})
endif()

# Assimp
find_package(Assimp REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS})
list(APPEND LIBRARY_FILES ${ASSIMP_LIBRARIES})

# FreeImage
find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})
list(APPEND LIBRARY_FILES ${FREEIMAGE_LIBRARIES})

# Google test
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/external/src/gtest-1.7.0")
include_directories("${PROJECT_SOURCE_DIR}/external/src/gtest-1.7.0/include")
if(MSVC AND MSVC_VERSION EQUAL 1700)
	# Workaround for VS2012
	add_definitions(-D_VARIADIC_MAX=10)
endif()

# Pugixml
include_directories("${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src")

# Add the binary directory to the include directories for generated headers
include_directories(${PROJECT_BINARY_DIR})

#

# Build options

include(CMakeDependentOption)

option(LM_SINGLE_PRECISION "Use single presicion floating-point number" ON)
if (LM_SINGLE_PRECISION)
	add_definitions(-DLM_SINGLE_PRECISION)
endif()

cmake_dependent_option(
	LM_DOUBLE_PRECISION "Use double precision floating-point number" ON
	"NOT LM_SINGLE_PRECISION" OFF)
if (LM_DOUBLE_PRECISION)
	add_definitions(-DLM_DOUBLE_PRECISION)
endif()

cmake_dependent_option(
	LM_MULTI_PRECISION "Use multi precision floating-point number" ON
	"NOT LM_SINGLE_PRECISION;NOT LM_DOUBLE_PRECISION" OFF)
if (LM_MULTI_PRECISION)
	add_definitions(-DLM_ENABLE_MULTI_PRECISION)
	add_definitions(-DLM_MULTI_PRECISION)
endif()

option(LM_FORCE_NO_SIMD "Specified not to use SIMD operations" OFF)
if (LM_FORCE_NO_SIMD)
	add_definitions(-DLM_FORCE_NO_SIMD)
endif()

#

if (MSVC)
	# Ignore linker warnings in VS
	# Do not use list(APPEND ..)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
	# _SCL_SECURE_NO_WARNINGS 
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	
	# Increase stack size to 10MB
	#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	# Enabling C++11 for gcc
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	add_definitions(-std=c++11)

	# If the build type is Debug, define macro
	if (CMAKE_BUILD_TYPE STREQUAL Debug)
		add_definitions(-D_DEBUG)
	endif()

	# Enable -fPIC
	# TODO : Find why with the static version of boost the linker generates an error
	#add_definitions(-fPIC)
endif()

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")

#

# Version number and codename
set(LM_VERSION_MAJOR "0")
set(LM_VERSION_MINOR "0")
set(LM_VERSION_PATCH "1")
set(LM_VERSION_CODENAME "nanon")

# Execute git command in the project root and check revision number
find_package(Git REQUIRED)
execute_process(
	COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD"
	OUTPUT_VARIABLE LM_VERSION_REVISION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#

# Current date
if (WIN32)
	execute_process(COMMAND "cmd" "/C date /T" OUTPUT_VARIABLE LM_CURRENT_DATE)
else()
	execute_process(COMMAND "date" "+%Y/%m/%d" OUTPUT_VARIABLE LM_CURRENT_DATE)
endif()
string(REGEX REPLACE "([0-9]+)/([0-9]+)/([0-9]+) *.*(\n|\r)$" "\\1.\\2.\\3" LM_CURRENT_DATE ${LM_CURRENT_DATE})

#

if (NOT LM_FORCE_NO_SIMD)

	# SIMD compatibility check
	try_run(
		_SSE_CHECK_RUN_RESULT
		_SSE_CHECK_COMPILE_RESULT
		"${CMAKE_BINARY_DIR}/CMakeFiles/ssecheck"
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/ssecheck.cpp"
		COMPILE_OUTPUT_VARIABLE _SSE_CHECK_COMPILE_OUTPUT
		RUN_OUTPUT_VARIABLE _SSE_CHECK_RUN_OUTPUT
	)
	if (NOT _SSE_CHECK_RUN_OUTPUT)
		message(FATAL_ERROR "Failed to check SIMD support")
	endif()
	
	string(REGEX REPLACE ".*SSE:([0-1]).*" "\\1" LM_USE_SSE ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSE2:([0-1]).*" "\\1" LM_USE_SSE2 ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSE3:([0-1]).*" "\\1" LM_USE_SSE3 ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSSE3:([0-1]).*" "\\1" LM_USE_SSSE3 ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSE4\\.1:([0-1]).*" "\\1" LM_USE_SSE4_1 ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSE4\\.2:([0-1]).*" "\\1" LM_USE_SSE4_2 ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSE4a:([0-1]).*" "\\1" LM_USE_SSE4A ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*SSE5:([0-1]).*" "\\1" LM_USE_SSE5 ${_SSE_CHECK_RUN_OUTPUT})
	string(REGEX REPLACE ".*AVX:([0-1]).*" "\\1" LM_USE_AVX ${_SSE_CHECK_RUN_OUTPUT})

	# Add compiler options
	if (MSVC)
		if (LM_USE_AVX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
			#add_definitions("/arch:AVX")
		elseif (LM_USE_SSE2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
			#add_definitions("/arch:SSE2")
		elseif (LM_USE_SSE)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE")
			#add_definitions("/arch:SSE")
		endif()
	elseif (CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
		#add_definitions("-march=native")
	endif()
endif()

#

# Include directory for libraries
include_directories("include")

# Add projects
add_subdirectory("src/liblightmetrica")
add_subdirectory("src/liblightmetrica.test")
add_subdirectory("src/lightmetrica.test")
add_subdirectory("src/lightmetrica.perf")
add_subdirectory("src/lightmetrica")
if (GLEW_FOUND AND GLFW_FOUND)
	add_subdirectory("src/lightmetrica.gui")
endif()
