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

// ----------------------------------------------------------------------

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
#endif

// Compiler and architecture
#ifdef _MSC_VER
	// Microsoft Visual C++
	#define NANON_COMPILER_MSVC
	#ifdef _M_IX86
		#define NANON_ARCH_X86
	#elif defined(_M_X64)
		#define NANON_ARCH_X64
	#elif defined(_M_ARM)
		#define NANON_ARCH_ARM
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
	#elif defined(__arm__)
		#define NANON_ARCH_ARM
	#else
		#error "Unsupportted architecture"
	#endif
#else
	#error "Unsupportted compiler"
#endif

// ----------------------------------------------------------------------

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

// Plugin export
#ifdef NANON_COMPILER_MSVC
	#define NANON_PLUGIN_API __declspec(dllexport)
#elif defined(NANON_COMPILER_GCC)
	#define NANON_PLUGIN_API __attribute__ ((visibility("default")))
#else
	#define NANON_PLUGIN_API 
#endif

// ----------------------------------------------------------------------

/*!
	\def NANON_FORCE_NO_SIMD
	Specifies not to use SIMD.
*/

// SSE support
#ifndef NANON_FORCE_NO_SIMD
	#define NANON_USE_SIMD
	#ifdef NANON_COMPILER_MSVC
		#if defined(NANON_ARCH_X86) || defined(NANON_ARCH_X64)
			#define NANON_USE_SSE
			#include <immintrin.h>
		#elif defined(NANON_ARCH_ARM)
			#define NANON_USE_NEON
			#include <arm_neon.h>
		#endif
	#elif defined(NANON_COMPILER_GCC)
		#if defined(NANON_ARCH_X86) || defined(NANON_ARCH_X64)
			#define NANON_USE_SSE
			#ifdef __AVX2__
				#include <immintrin.h>
			#elif defined(__SSE4__)
				#include <smmintrin.h>
			#elif defined(__SSE3__)
				#include <pmmintrin.h>
			#else
				#include <emmintrin.h>
			#endif
		#elif defined(NANON_ARCH_ARM)
			#define NANON_USE_NEON
			#include <arm_neon.h>
		#endif
	#endif
#endif

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

// ----------------------------------------------------------------------

// Namespace
#define NANON_NAMESPACE_BEGIN namespace nanon {
#define NANON_NAMESPACE_END }

// ----------------------------------------------------------------------

#define NANON_SAFE_DELETE(val) if ((val) != NULL ) { delete (val); (val) = NULL; }
#define NANON_SAFE_DELETE_ARRAY(val) if ((val) != NULL ) { delete[] (val); (val) = NULL; }

#define NANON_DISABLE_COPY_AND_MOVE(TypeName) \
	TypeName(const TypeName &); \
	TypeName(TypeName&&); \
	void operator=(const TypeName&); \
	void operator=(TypeName&&)

#endif // __LIB_NANON_COMMON_H__