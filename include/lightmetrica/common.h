/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef LIB_LIGHTMETRICA_COMMON_H
#define LIB_LIGHTMETRICA_COMMON_H

// C++ compiler is required
#ifndef __cplusplus
	#error "C++ compiler is required"
#endif

// Debug mode flag
#ifndef NDEBUG
	#define LM_DEBUG_MODE 1
#else
	#define LM_DEBUG_MODE 0
#endif

// Experimental mode flag
#ifdef LM_ENABLE_EXPERIMENTAL_MODE
	#define LM_EXPERIMENTAL_MODE 1
#else
	#define LM_EXPERIMENTAL_MODE 0
#endif

// Strict floating-point mode flag
#ifdef LM_ENABLE_STRICT_FP
	#define LM_STRICT_FP 1
#else
	#define LM_STRICT_FP 0
#endif

// MPI flag
#ifdef LM_USE_MPI
	#define LM_MPI 1
#else
	#define LM_MPI 0
#endif

// --------------------------------------------------------------------------------

// Platform

// Windows
#ifdef _WIN32
	#define LM_PLATFORM_WINDOWS 1
#else
	#define LM_PLATFORM_WINDOWS 0
#endif

// Linux
#ifdef __linux
	#define LM_PLATFORM_LINUX 1
#else
	#define LM_PLATFORM_LINUX 0
#endif

#if LM_PLATFORM_WINDOWS == LM_PLATFORM_LINUX
	#error "Unsupportted platform"
#endif

#if LM_PLATFORM_WINDOWS
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#pragma warning(disable:4819)	// Level 1. Character that cannot be represented
	#pragma warning(disable:4996)	// Level 3. _SCL_SECURE_NO_WARNINGS
	#pragma warning(disable:4290)	// Level 3. Exception specification ignored
	#pragma warning(disable:4201)	// Level 4. Nonstandard extension used : nameless struct/union
	#pragma warning(disable:4512)	// Level 4. Cannot generate an assignment operator for the given class
	#pragma warning(disable:4127)	// Level 4. Conditional expression is constant
	#pragma warning(disable:4510)	// Level 4. Default constructor could not be generated
	#pragma warning(disable:4610)	// Level 4. User-defined constructor required
	#pragma warning(disable:4100)	// Level 4. Unreferenced formal parameter
	#pragma warning(disable:4505)	// Level 4. Unreferenced local function has been removed
	#pragma warning(disable:4324)	// Level 4. Structure was padded due to __declspec(align())
#endif

// --------------------------------------------------------------------------------

// Compiler

// Microsoft Visual C++
#ifdef _MSC_VER
	#define LM_COMPILER_MSVC 1
#else
	#define LM_COMPILER_MSVC 0
#endif

// GNU GCC
#if defined(__GNUC__) || defined(__MINGW32__)
	#define LM_COMPILER_GCC 1
#else
	#define LM_COMPILER_GCC 0
#endif

#if LM_COMPILER_MSVC == LM_COMPILER_GCC
	#error "Unsupportted compiler"
#endif

// --------------------------------------------------------------------------------

// Architecture
#if LM_COMPILER_MSVC
	#ifdef _M_IX86
		#define LM_ARCH_X86 1
	#else
		#define LM_ARCH_X86 0
	#endif
	#ifdef _M_X64
		#define LM_ARCH_X64 1
	#else
		#define LM_ARCH_X64 0
	#endif
#elif LM_COMPILER_GCC
	#ifdef __i386__
		#define LM_ARCH_X86 1
	#else
		#define LM_ARCH_X86 0
	#endif
	#ifdef __x86_64__
		#define LM_ARCH_X64 1
	#else
		#define LM_ARCH_X64 0
	#endif
#endif

#if LM_ARCH_X86 == LM_ARCH_X64
	#error "Unsupportted architecture"
#endif

// --------------------------------------------------------------------------------

// Library import or export
#if LM_COMPILER_MSVC
	#ifdef LM_EXPORTS
		#define LM_PUBLIC_API __declspec(dllexport)
	#else
		#define LM_PUBLIC_API __declspec(dllimport)
	#endif
	#define LM_HIDDEN_API
#elif LM_COMPILER_GCC
	#ifdef LM_EXPORTS
		#define LM_PUBLIC_API __attribute__ ((visibility("default")))
		#define LM_HIDDEN_API __attribute__ ((visibility("hidden")))
	#else
		#define LM_PUBLIC_API
		#define LM_HIDDEN_API
	#endif
#else
	#define LM_PUBLIC_API
	#define LM_HIDDEN_API
#endif

#if LM_COMPILER_MSVC
	#ifdef LM_PLUGIN_EXPORTS
		#define LM_PLUGIN_PUBLIC_API __declspec(dllexport)
	#else
		#define LM_PLUGIN_PUBLIC_API __declspec(dllimport)
	#endif
	#define LM_PLUGIN_HIDDEN_API
#elif LM_COMPILER_GCC
	#ifdef LM_PLUGIN_EXPORTS
		#define LM_PLUGIN_PUBLIC_API __attribute__ ((visibility("default")))
		#define LM_PLUGIN_HIDDEN_API __attribute__ ((visibility("hidden")))
	#else
		#define LM_PLUGIN_PUBLIC_API
		#define LM_PLUGIN_HIDDEN_API
	#endif
#else
	#define LM_PLUGIN_PUBLIC_API
	#define LM_PLUGIN_HIDDEN_API
#endif

// In the debug mode, the hidden API is exposed
#if LM_DEBUG_MODE
	#undef LM_HIDDEN_API
	#define LM_HIDDEN_API LM_PUBLIC_API
#endif

// Plugin export
#if LM_COMPILER_MSVC
	#define LM_PLUGIN_API __declspec(dllexport)
#elif LM_COMPILER_GCC
	#define LM_PLUGIN_API __attribute__ ((visibility("default")))
#else
	#define LM_PLUGIN_API 
#endif

// --------------------------------------------------------------------------------

// Force inline
#if LM_COMPILER_MSVC
	#define LM_FORCE_INLINE __forceinline
#elif LM_COMPILER_GCC
	#define LM_FORCE_INLINE inline __attribute__((always_inline))
#endif

// Alignment
#if LM_COMPILER_MSVC
	#define LM_ALIGN(x) __declspec(align(x))
#elif LM_COMPILER_GCC
	#define LM_ALIGN(x) __attribute__((aligned(x)))
#endif
#define LM_ALIGN_16 LM_ALIGN(16)
#define LM_ALIGN_32 LM_ALIGN(32)

// --------------------------------------------------------------------------------

// Namespaces
#define LM_NAMESPACE_BEGIN namespace lightmetrica {
#define LM_NAMESPACE_END }

//#define LM_DETAIL_NAMESPACE_BEGIN namespace detail {
//#define LM_DETAIL_NAMESPACE_END }

// --------------------------------------------------------------------------------

#define LM_SAFE_DELETE(val) if ((val) != NULL ) { delete (val); (val) = NULL; }
#define LM_SAFE_DELETE_ARRAY(val) if ((val) != NULL ) { delete[] (val); (val) = NULL; }

#define LM_DISABLE_COPY_AND_MOVE(TypeName) \
	TypeName(const TypeName &); \
	TypeName(TypeName&&); \
	void operator=(const TypeName&); \
	void operator=(TypeName&&)

#endif // LIB_LIGHTMETRICA_COMMON_H
