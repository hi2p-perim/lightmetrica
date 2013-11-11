#ifndef __LIB_NANON_COMMON_H__
#define __LIB_NANON_COMMON_H__

// C++ compiler is required
#ifndef __cplusplus
	#error "C++ compiler is required"
#endif

// Debug mode?
#ifndef NDEBUG
	#define NN_DEBUG_MODE
#endif

// ----------------------------------------------------------------------

// Platform
#ifdef _WIN32
	#define NN_PLATFORM_WINDOWS
#elif defined(__linux)
	#define NN_PLATFORM_LINUX
#else
	#error "Unsupportted platform"
#endif

#ifdef NN_PLATFORM_WINDOWS
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
#endif

// Compiler and architecture
#ifdef _MSC_VER
	// Microsoft Visual C++
	#define NN_COMPILER_MSVC
	#ifdef _M_IX86
		#define NN_ARCH_X86
	#elif defined(_M_X64)
		#define NN_ARCH_X64
	#elif defined(_M_ARM)
		#define NN_ARCH_ARM
	#else
		#error "Unsupportted architecture"
	#endif
#elif (defined(__GNUC__) || defined(__MINGW32__))
	// GNU GCC
	#define NN_COMPILER_GCC
	#ifdef __i386__
		#define NN_ARCH_X86
	#elif defined(__x86_64__)
		#define NN_ARCH_X64
	#elif defined(__arm__)
		#define NN_ARCH_ARM
	#else
		#error "Unsupportted architecture"
	#endif
#else
	#error "Unsupportted compiler"
#endif

// ----------------------------------------------------------------------

// Library import or export
#ifdef NN_COMPILER_MSVC
	#ifdef NN_EXPORTS
		#define NN_PUBLIC_API __declspec(dllexport)
	#else
		#define NN_PUBLIC_API __declspec(dllimport)
	#endif
	#define NN_HIDDEN_API
#elif defined(NN_COMPILER_GCC)
	#define NN_PUBLIC_API __attribute__ ((visibility("default")))
	#define NN_HIDDEN_API __attribute__ ((visibility("hidden")))
#else
	#define NN_PUBLIC_API
	#define NN_HIDDEN_API
#endif

// Plugin export
#ifdef NN_COMPILER_MSVC
	#define NN_PLUGIN_API __declspec(dllexport)
#elif defined(NN_COMPILER_GCC)
	#define NN_PLUGIN_API __attribute__ ((visibility("default")))
#else
	#define NN_PLUGIN_API 
#endif

// ----------------------------------------------------------------------

/*!
	\def NN_FORCE_NO_SIMD
	Specifies not to use SIMD.
*/

// SSE support
#ifndef NN_FORCE_NO_SIMD
	#define NN_USE_SIMD
	#ifdef NN_COMPILER_MSVC
		#if defined(NN_ARCH_X86) || defined(NN_ARCH_X64)
			#define NN_USE_SSE
			#include <immintrin.h>
		#elif defined(NN_ARCH_ARM)
			#define NN_USE_NEON
			#include <arm_neon.h>
		#endif
	#elif defined(NN_COMPILER_GCC)
		#if defined(NN_ARCH_X86) || defined(NN_ARCH_X64)
			#define NN_USE_SSE
			#ifdef __AVX2__
				#include <immintrin.h>
			#elif defined(__SSE4__)
				#include <smmintrin.h>
			#elif defined(__SSE3__)
				#include <pmmintrin.h>
			#else
				#include <emmintrin.h>
			#endif
		#elif defined(NN_ARCH_ARM)
			#define NN_USE_NEON
			#include <arm_neon.h>
		#endif
	#endif
#endif

// Force inline
#ifdef NN_COMPILER_MSVC
	#define NN_FORCE_INLINE __forceinline
#elif defined(NN_COMPILER_GCC)
	#define NN_FORCE_INLINE inline __attribute__((always_inline))
#endif

// Alignment
#ifdef NN_COMPILER_MSVC
	#define NN_ALIGN(x) __declspec(align(x))
#elif defined(NN_COMPILER_GCC)
	#define NN_ALIGN(x) __attribute__((aligned(x)))
#endif
#define NN_ALIGN_16 NN_ALIGN(16)
#define NN_ALIGN_32 NN_ALIGN(32)

// ----------------------------------------------------------------------

// Namespace
#define NN_NAMESPACE_BEGIN namespace nanon {
#define NN_NAMESPACE_END }

// ----------------------------------------------------------------------

#define NN_SAFE_DELETE(val) if ((val) != NULL ) { delete (val); (val) = NULL; }
#define NN_SAFE_DELETE_ARRAY(val) if ((val) != NULL ) { delete[] (val); (val) = NULL; }

#endif // __LIB_NANON_COMMON_H__