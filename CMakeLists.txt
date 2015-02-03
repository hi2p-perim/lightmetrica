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

cmake_minimum_required(VERSION 2.8)

project(lightmetrica)

################################################################################

# Add module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include modules
#include(LightmetricaUtils)

# For windows environment, set search path for find_package
if (MSVC)
	# Set prefix path according to generator
	# Currently only VC11 and VC12 are supported
	if ((CMAKE_GENERATOR STREQUAL "Visual Studio 11 Win64") OR (CMAKE_GENERATOR STREQUAL "Visual Studio 11 2012 Win64"))
		set(LM_GENERATOR_PREFIX "vc11")
	elseif ((CMAKE_GENERATOR STREQUAL "Visual Studio 12 Win64") OR (CMAKE_GENERATOR STREQUAL "Visual Studio 12 2013 Win64"))
		set(LM_GENERATOR_PREFIX "vc12")
	else()
		message(FATAL_ERROR "Invalid generator")
	endif()
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external")
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/${LM_GENERATOR_PREFIX}")

	# Set architecture
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

################################################################################

# External libraries

# OpenMP
find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

find_package(MPI)
if (MPI_FOUND)
	set(CMAKE_CXX_COMPILER_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
	set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
	include_directories(${MPI_INCLUDE_PATH})
	add_definitions(-DLM_USE_MPI)
endif()

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
if (MSVC AND MSVC_VERSION EQUAL 1700)
	# Workaround for VS2012
	add_definitions(-D_VARIADIC_MAX=10)
endif()

# Pugixml
include_directories("${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src")

# SFMT
include_directories("${PROJECT_SOURCE_DIR}/external/src/SFMT-src-1.4.1")
add_definitions(-DSFMT_MEXP=19937)

# Add the binary directory to the include directories for generated headers
include_directories(${PROJECT_BINARY_DIR})

################################################################################

# Build options

include(CMakeDependentOption)

option(LM_USE_SINGLE_PRECISION "Use single presicion floating-point number" ON)
if (LM_USE_SINGLE_PRECISION)
	add_definitions(-DLM_USE_SINGLE_PRECISION)
endif()

cmake_dependent_option(
	LM_USE_DOUBLE_PRECISION "Use double precision floating-point number" ON
	"NOT LM_USE_SINGLE_PRECISION" OFF)
if (LM_USE_DOUBLE_PRECISION)
	add_definitions(-DLM_USE_DOUBLE_PRECISION)
endif()

cmake_dependent_option(
	LM_USE_MULTI_PRECISION "Use multi precision floating-point number" ON
	"NOT LM_USE_SINGLE_PRECISION;NOT LM_USE_DOUBLE_PRECISION" OFF)
if (LM_USE_MULTI_PRECISION)
	add_definitions(-DLM_ENABLE_MULTI_PRECISION)
	add_definitions(-DLM_USE_MULTI_PRECISION)
endif()

option(LM_ENABLE_FORCE_NO_SIMD "Specified not to use SIMD operations" OFF)
if (LM_ENABLE_FORCE_NO_SIMD)
	add_definitions(-DLM_ENABLE_FORCE_NO_SIMD)
endif()

# Always disabled now
option(LM_ENABLE_EXPERIMENTAL_MODE "Enable experimental features" OFF)
if (LM_ENABLE_EXPERIMENTAL_MODE)
	add_definitions(-DLM_ENABLE_EXPERIMENTAL_MODE)
endif()

option(LM_ENABLE_STRICT_FP "Enable strict floating-point handling" ON)
if (LM_ENABLE_STRICT_FP)
	add_definitions(-DLM_ENABLE_STRICT_FP)
endif()

option(LM_ENABLE_BUILD_DOC "Enable to build documentation by Doxygen" OFF)

################################################################################

if (MSVC)
	# Ignore linker warnings in VS
	# Do not use list(APPEND ..)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
	# _SCL_SECURE_NO_WARNINGS 
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	
	# Increase stack size to 10MB
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
	
	# Warning level 4, treat warning as errors
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")

	# Add floating-point flag if enabled
	if (LM_ENABLE_STRICT_FP)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:strict")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
	endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	# Enabling C++11 for gcc
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

	# If the build type is Debug, define macro
	if (CMAKE_BUILD_TYPE STREQUAL Debug)
		add_definitions(-D_DEBUG)
	endif()

	# Enable all warnings, treat warning as errors
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

	# Ignore unknown pragma
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

	# Enable -fPIC
	# TODO : Find why with the static version of boost the linker generates an error
	#add_definitions(-fPIC)
endif()

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")

################################################################################

# Version number and codename
set(LM_VERSION_MAJOR "0")
set(LM_VERSION_MINOR "0")
set(LM_VERSION_PATCH "1")
set(LM_VERSION_CODENAME "beta")

# Execute git command in the project root and check revision number
find_package(Git REQUIRED)
execute_process(
	COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD"
	OUTPUT_VARIABLE LM_VERSION_REVISION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Current date
if (WIN32)
	execute_process(COMMAND "cmd" "/C date /T" OUTPUT_VARIABLE LM_CURRENT_DATE)
else()
	execute_process(COMMAND "date" "+%Y/%m/%d" OUTPUT_VARIABLE LM_CURRENT_DATE)
endif()
string(REGEX REPLACE "([0-9]+)/([0-9]+)/([0-9]+) *.*(\n|\r)$" "\\1.\\2.\\3" LM_CURRENT_DATE ${LM_CURRENT_DATE})

################################################################################

# SIMD configuration
if (NOT LM_ENABLE_FORCE_NO_SIMD)
	if (MSVC)
		# Automatic SIMD compatibility check
		try_run(
			_SSE_CHECK_RUN_RESULT
			_SSE_CHECK_COMPILE_RESULT
			"${CMAKE_BINARY_DIR}/CMakeFiles/ssecheck"
			"${CMAKE_CURRENT_SOURCE_DIR}/cmake/ssecheck.cpp"
			COMPILE_OUTPUT_VARIABLE _SSE_CHECK_COMPILE_OUTPUT
			RUN_OUTPUT_VARIABLE _SSE_CHECK_RUN_OUTPUT)
		
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

		if (LM_USE_AVX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
		elseif (LM_USE_SSE2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
		elseif (LM_USE_SSE)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE")
		endif()
	elseif (CMAKE_COMPILER_IS_GNUCXX)
		set(LM_USE_SSE 1)
		set(LM_USE_SSE2 1)
		set(LM_USE_SSE3 1)
		set(LM_USE_SSSE3 1)
		set(LM_USE_SSE4_1 1)
		set(LM_USE_SSE4_2 1)
		set(LM_USE_SSE4A 0)
		set(LM_USE_SSE5 0)
		set(LM_USE_AVX 0)
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcx16 -msahf -mno-movbe -mno-aes -mpclmul -mpopcnt -mno-abm -mno-lwp -mno-fma -mno-fma4 -mno-xop -mno-bmi -mno-bmi2 -mno-tbm --mavx -msse4.2 -msse4.1 -mno-lzcnt -mno-f16c -mno-fsgsbase --param l1-cache-size=32 --param l1-cache-line-size=64 --param l2-cache-size=3072 -mtune=generic")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcx16 -msahf -mno-movbe -mno-aes -mno-pclmul -mpopcnt -mno-abm -mno-lwp -mno-fma -mno-fma4 -mno-xop -mno-bmi -mno-bmi2 -mno-tbm  -msse4.2 -msse4.1 -mno-lzcnt -mno-rdrnd -mno-f16c -mno-fsgsbase --param l1-cache-size=32 --param l1-cache-line-size=64 -mtune=generic")
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	endif()
endif()

################################################################################

# Documentation
if (LM_ENABLE_BUILD_DOC)
	# Find doxygen
	find_package(Doxygen REQUIRED)
	
	# Main page
	set(LM_DOXYGEN_MAIN_PAGE "@CMAKE_CURRENT_SOURCE_DIR@/README.md")
	
	# Input directories
	set(LM_DOXYGEN_INPUT_DIR "${LM_DOXYGEN_INPUT_DIR} ${LM_DOXYGEN_MAIN_PAGE}")
	set(LM_DOXYGEN_INPUT_DIR "${LM_DOXYGEN_INPUT_DIR} @CMAKE_CURRENT_SOURCE_DIR@/src")
	set(LM_DOXYGEN_INPUT_DIR "${LM_DOXYGEN_INPUT_DIR} @CMAKE_CURRENT_SOURCE_DIR@/include")
	set(LM_DOXYGEN_INPUT_DIR "${LM_DOXYGEN_INPUT_DIR} @CMAKE_CURRENT_SOURCE_DIR@/plugin")
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/experimental")
		set(LM_DOXYGEN_INPUT_DIR "${LM_DOXYGEN_INPUT_DIR} @CMAKE_CURRENT_SOURCE_DIR@/experimental/plugin")
	endif()
	
	# Generate Doxyfile
	set(_DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
	set(_DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	configure_file(${_DOXYFILE_IN} ${_DOXYFILE} @ONLY)

	# Add custom build target 'doc'
	add_custom_target(
		doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${_DOXYFILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generates API documentation with Doxygen"
		VERBATIM)
endif()

################################################################################

# Include directory for libraries and PCH
include_directories("include")
include_directories("pch")

# Specify to use solution directories
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add projects
add_subdirectory("src/liblightmetrica")
add_subdirectory("src/liblightmetrica.test")
add_subdirectory("src/lightmetrica.test")
add_subdirectory("src/lightmetrica.perf")
add_subdirectory("src/lightmetrica")
if (GLEW_FOUND AND GLFW_FOUND)
	add_subdirectory("src/lightmetrica.gui")
endif()

# Plugins
add_subdirectory("plugin")
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/experimental")
	add_subdirectory("experimental/plugin")
endif()
