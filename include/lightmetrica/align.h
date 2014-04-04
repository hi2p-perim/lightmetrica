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
#ifndef LIB_LIGHTMETRICA_ALIGN_H
#define LIB_LIGHTMETRICA_ALIGN_H

#include "common.h"

LM_NAMESPACE_BEGIN

/*!
	Platform independent aligned malloc.
	\param size Size to be allocated.
	\param align Alignment in bytes.
	\return Allocated pointer.
*/
LM_PUBLIC_API void* aligned_malloc(size_t size, size_t align);

/*!
	Platform independent aligned free.
	\param p Pointer to aligned memory.
*/
LM_PUBLIC_API void aligned_free(void* p);

/*!
	Check alignment.
	\param p Pointer.
	\param align Alignment in bytes.
*/
LM_FORCE_INLINE bool is_aligned(const void* p, size_t align)
{
	return (uintptr_t)p % align == 0;
}

/*!
	Aligned type.
	\tparam Align Alignment.
*/
template <size_t Align>
class Aligned
{
public:
	
	virtual ~Aligned() {}

public:
	
	void* operator new(std::size_t size) throw (std::bad_alloc)
	{
		void* p = aligned_malloc( size, Align);
		if (!p) throw std:: bad_alloc();
		return p;
	} 

	void operator delete( void* p)
	{
		aligned_free( p);
	}

};

/*!
	SIMD aligned type.
	Inherited class automatically supports aligned version of
	operator new and delete for SIMD types.
*/
class LM_PUBLIC_API SIMDAlignedType
{
public:

	virtual ~SIMDAlignedType() {}

public:

	void* operator new(std::size_t size) throw (std::bad_alloc);
	void operator delete(void* p);

};

/*!
	Aligned allocator for STL containers.
	\tparam T Data type.
	\tparam Alignment Alignment.
*/
template <typename T, std::size_t Alignment>
class aligned_allocator
{
public:

	// The following will be the same for virtually all allocators.
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;

	T * address(T& r) const { return &r; }
	const T * address(const T& s) const { return &s; }

	std::size_t max_size() const
	{
		// The following has been carefully written to be independent of
		// the definition of size_t and to avoid signed/unsigned warnings.
		return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
	}

	// The following must be the same for all allocators.
	template <typename U>
	struct rebind
	{
		typedef aligned_allocator<U, Alignment> other;
	};

	bool operator!=(const aligned_allocator& other) const
	{
		return !(*this == other);
	}

	void construct(T * const p, const T& t) const
	{
		void * const pv = static_cast<void *>(p);
		new (pv) T(t);
	}

	void destroy(T * const p) const
	{
		p->~T();
	}

	// Returns true if and only if storage allocated from *this
	// can be deallocated from other, and vice versa.
	// Always returns true for stateless allocators.
	bool operator==(const aligned_allocator& other) const
	{
		return true;
	}

	// Default constructor, copy constructor, rebinding constructor, and destructor.
	// Empty for stateless allocators.
	aligned_allocator() {}
	aligned_allocator(const aligned_allocator&) {}
	template <typename U> aligned_allocator(const aligned_allocator<U, Alignment>&) {}
	~aligned_allocator() {}


	// The following will be different for each allocator.
	T * allocate(const std::size_t n) const
	{
		// The return value of allocate(0) is unspecified.
		// Mallocator returns NULL in order to avoid depending
		// on malloc(0)'s implementation-defined behavior
		// (the implementation can define malloc(0) to return NULL,
		// in which case the bad_alloc check below would fire).
		// All allocators can return NULL in this case.
		if (n == 0) {
			return NULL;
		}

		// All allocators should contain an integer overflow check.
		// The Standardization Committee recommends that std::length_error
		// be thrown in the case of integer overflow.
		if (n > max_size())
		{
			throw std::length_error("aligned_allocator<T>::allocate() - Integer overflow.");
		}

		// Mallocator wraps malloc().
		void * const pv = aligned_malloc(n * sizeof(T), Alignment);

		// Allocators should throw std::bad_alloc in the case of memory allocation failure.
		if (pv == NULL)
		{
			throw std::bad_alloc();
		}

		return static_cast<T *>(pv);
	}

	void deallocate(T * const p, const std::size_t n) const
	{
		aligned_free(p);
	}


	// The following will be the same for all allocators that ignore hints.
	template <typename U>
	T * allocate(const std::size_t n, const U * /* const hint */) const
	{
		return allocate(n);
	}

	// Allocators are not required to be assignable, so
	// all allocators should have a private unimplemented
	// assignment operator. Note that this will trigger the
	// off-by-default (enabled under /Wall) warning C4626
	// "assignment operator could not be generated because a
	// base class assignment operator is inaccessible" within
	// the STL headers, but that warning is useless.
private:

	aligned_allocator& operator=(const aligned_allocator&);

};

/*!
	Aligned allocator for boost::pool.
	\tparam Align Alignment.
*/
template <std::size_t Align>
struct boost_pool_aligned_allocator
{

	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	static char* malloc(const size_type bytes) { return static_cast<char*>(aligned_malloc(bytes, Align)); }
	static void free(const char* block) { aligned_free((void*)block); }

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_ALIGN_H