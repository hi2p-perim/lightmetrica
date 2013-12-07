cmake_minimum_required(VERSION 2.8)

project(nanonrender)

#

# Build options

include(CMakeDependentOption)

option(NANON_SINGLE_PRECISION "Use single presicion floating-point number" ON)
if (NANON_SINGLE_PRECISION)
	add_definitions(-DNANON_SINGLE_PRECISION)
endif()

cmake_dependent_option(
	NANON_DOUBLE_PRECISION "Use double precision floating-point number" ON
	"NOT NANON_SINGLE_PRECISION" OFF)
if (NANON_DOUBLE_PRECISION)
	add_definitions(-DNANON_DOUBLE_PRECISION)
endif()

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

# Boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.49 REQUIRED COMPONENTS "program_options" "filesystem" "system" "regex")
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND COMMON_LIBRARY_FILES ${Boost_LIBRARIES})

# GLM
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# GLEW
#set(GLEW_USE_STATIC_LIBS ON)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
add_definitions(${GLEW_DEFINITIONS})
list(APPEND LIBRARY_FILES ${GLEW_LIBRARIES})

# GLFW
#set(GLFW_USE_STATIC_LIBS ON)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
list(APPEND LIBRARY_FILES ${GLFW_LIBRARIES})

# Google test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
if(MSVC AND MSVC_VERSION EQUAL 1700)
	# Workaround for VS2012
	add_definitions(-D_VARIADIC_MAX=10)
endif()

# Pugixml
include_directories("${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src")

#

# Enabling C++11 for gcc
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
	# If the build type is Debug, define macro
	if (CMAKE_BUILD_TYPE STREQUAL Debug)
		add_definitions(-D_DEBUG)
	endif()
endif()

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")

#

# Version number
set(NANON_VERSION_MAJOR "0")
set(NANON_VERSION_MINOR "0")
set(NANON_VERSION_PATCH "1")

# Execute git command in the project root and check revision number
find_package(Git REQUIRED)
execute_process(
	COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD"
	OUTPUT_VARIABLE NANON_VERSION_REVISION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#

# Current date
if (WIN32)
	execute_process(COMMAND "cmd" "/C date /T" OUTPUT_VARIABLE NANON_CURRENT_DATE)
elseif()
	execute_process(COMMAND "date" "+%Y/%m/%d" OUTPUT_VARIABLE NANON_CURRENT_DATE)
endif()
string(REGEX REPLACE "([0-9]+)/([0-9]+)/([0-9]+)( *)(\n|\r)$" "\\1.\\2.\\3" NANON_CURRENT_DATE ${NANON_CURRENT_DATE})

#

# Include directory for libnanon
include_directories("include")

# Add projects
add_subdirectory("src/libnanon")
add_subdirectory("src/nanon.test")
add_subdirectory("src/nanon")
add_subdirectory("src/nanon.gui")