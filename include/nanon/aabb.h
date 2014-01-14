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
#ifndef __LIB_NANON_AABB_H__
#define __LIB_NANON_AABB_H__

#include "math.types.h"

NANON_NAMESPACE_BEGIN

/*!
*/
struct AABB
{

	Math::Vec3 min, max;

	NANON_FORCE_INLINE AABB();
	NANON_FORCE_INLINE AABB(const Math::Vec3& p);
	NANON_FORCE_INLINE AABB(const Math::Vec3& p1, const Math::Vec3& p2);

	NANON_FORCE_INLINE bool Intersect(const AABB& b) const;
	NANON_FORCE_INLINE bool Contain(const Math::Vec3& p) const;
	NANON_FORCE_INLINE double SurfaceArea() const;
	NANON_FORCE_INLINE double Volume() const;
	NANON_FORCE_INLINE int LongestAxis() const;
	NANON_FORCE_INLINE AABB Union(const AABB& b) const;
	NANON_FORCE_INLINE AABB Union(const Math::Vec3& p) const;

	//bool operator==(const AABB& b) const { return min == b.min && max == b.max; }
	//bool operator!=(const AABB& b) const { return min != b.min || max != b.max; }
	NANON_FORCE_INLINE const Math::Vec3& operator[](int i) const { return (&min)[i]; }
	NANON_FORCE_INLINE Math::Vec3& operator[](int i) { return (&min)[i]; }

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_AABB_H__