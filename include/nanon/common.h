/*
	nanon : A research-oriented renderer

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
#ifndef __LIB_NANON_COMMON_H__
#define __LIB_NANON_COMMON_H__

// C++ compiler is required
#ifndef __cplusplus
	#error "C++ compiler is required"
#endif

// Debug mode?
#ifndef NDEBUG
	#define NANON_DEBUG_MODE
#endif

// --------------------------------------------------------------------------------

// Platform
#ifdef _WIN32
	#define NANON_PLATFORM_WINDOWS
#elif defined(__linux)
	#define NANON_PLATFORM_LINUX
#else
	#error "Unsupportted platform"
#endif

#ifdef NANON_PLATFORM_WINDOWS
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#pragma warning(disable:4996)	// _SCL_SECURE_NO_WARNINGS
	#pragma warning(disable:4819)	// Character that cannot be represented
	#pragma warning(disable:4290)	// Exception specification ignored
#endif

// Compiler and architecture
#ifdef _MSC_VER
	// Microsoft Visual C++
	#define NANON_COMPILER_MSVC
	#ifdef _M_IX86
		#define NANON_ARCH_X86
	#elif defined(_M_X64)
		#define NANON_ARCH_X64
	#else
		#error "Unsupportted architecture"
	#endif
#elif (defined(__GNUC__) || defined(__MINGW32__))
	// GNU GCC
	#define NANON_COMPILER_GCC
	#ifdef __i386__
		#define NANON_ARCH_X86
	#elif defined(__x86_64__)
		#define NANON_ARCH_X64
	#else
		#error "Unsupportted architecture"
	#endif
#else
	#error "Unsupportted compiler"
#endif

// --------------------------------------------------------------------------------

// Library import or export
#ifdef NANON_COMPILER_MSVC
	#ifdef NANON_EXPORTS
		#define NANON_PUBLIC_API __declspec(dllexport)
	#else
		#define NANON_PUBLIC_API __declspec(dllimport)
	#endif
	#define NANON_HIDDEN_API
#elif defined(NANON_COMPILER_GCC)
	#define NANON_PUBLIC_API __attribute__ ((visibility("default")))
	#define NANON_HIDDEN_API __attribute__ ((visibility("hidden")))
#else
	#define NANON_PUBLIC_API
	#define NANON_HIDDEN_API
#endif

// In the debug mode, the hidden API is exposed
#ifdef RF_DEBUG_MODE
	#undef RF_HIDDEN_API
	#define RF_HIDDEN_API RF_PUBLIC_API
#endif

// Plugin export
#ifdef NANON_COMPILER_MSVC
	#define NANON_PLUGIN_API __declspec(dllexport)
#elif defined(NANON_COMPILER_GCC)
	#define NANON_PLUGIN_API __attribute__ ((visibility("default")))
#else
	#define NANON_PLUGIN_API 
#endif

// --------------------------------------------------------------------------------

// Force inline
#ifdef NANON_COMPILER_MSVC
	#define NANON_FORCE_INLINE __forceinline
#elif defined(NANON_COMPILER_GCC)
	#define NANON_FORCE_INLINE inline __attribute__((always_inline))
#endif

// Alignment
#ifdef NANON_COMPILER_MSVC
	#define NANON_ALIGN(x) __declspec(align(x))
#elif defined(NANON_COMPILER_GCC)
	#define NANON_ALIGN(x) __attribute__((aligned(x)))
#endif
#define NANON_ALIGN_16 NANON_ALIGN(16)
#define NANON_ALIGN_32 NANON_ALIGN(32)

// --------------------------------------------------------------------------------

// Namespace
#define NANON_NAMESPACE_BEGIN namespace nanon {
#define NANON_NAMESPACE_END }

// --------------------------------------------------------------------------------

#define NANON_SAFE_DELETE(val) if ((val) != NULL ) { delete (val); (val) = NULL; }
#define NANON_SAFE_DELETE_ARRAY(val) if ((val) != NULL ) { delete[] (val); (val) = NULL; }

#define NANON_DISABLE_COPY_AND_MOVE(TypeName) \
	TypeName(const TypeName &); \
	TypeName(TypeName&&); \
	void operator=(const TypeName&); \
	void operator=(TypeName&&)

#endif // __LIB_NANON_COMMON_H__