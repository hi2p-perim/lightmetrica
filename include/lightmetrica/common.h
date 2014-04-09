/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#pragma once
#ifndef LIB_LIGHTMETRICA_COMMON_H
#define LIB_LIGHTMETRICA_COMMON_H

// C++ compiler is required
#ifndef __cplusplus
	#error "C++ compiler is required"
#endif

// Debug mode?
#ifndef NDEBUG
	#define LM_DEBUG_MODE 1
#else
	#define LM_DEBUG_MODE 0
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
	#pragma warning(disable:4996)	// _SCL_SECURE_NO_WARNINGS
	#pragma warning(disable:4819)	// Character that cannot be represented
	#pragma warning(disable:4290)	// Exception specification ignored
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

// In the debug mode, the hidden API is exposed
#if LM_DEBUG_MODE
	#undef RF_HIDDEN_API
	#define RF_HIDDEN_API RF_PUBLIC_API
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

// Namespace
#define LM_NAMESPACE_BEGIN namespace lightmetrica {
#define LM_NAMESPACE_END }

// --------------------------------------------------------------------------------

#define LM_SAFE_DELETE(val) if ((val) != NULL ) { delete (val); (val) = NULL; }
#define LM_SAFE_DELETE_ARRAY(val) if ((val) != NULL ) { delete[] (val); (val) = NULL; }

#define LM_DISABLE_COPY_AND_MOVE(TypeName) \
	TypeName(const TypeName &); \
	TypeName(TypeName&&); \
	void operator=(const TypeName&); \
	void operator=(TypeName&&)

#endif // LIB_LIGHTMETRICA_COMMON_H
