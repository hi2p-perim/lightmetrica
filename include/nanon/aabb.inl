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

#include "aabb.h"

NANON_NAMESPACE_BEGIN

NANON_FORCE_INLINE AABB::AABB()
	: min(Math::Constants::Inf)
	, max(-Math::Constants::Inf)
{

}

NANON_FORCE_INLINE AABB::AABB(const Math::Vec3& p)
	: min(p)
	, max(p)
{

}

NANON_FORCE_INLINE AABB::AABB(const Math::Vec3& p1, const Math::Vec3& p2)
	: min(Math::Min(p1, p2))
	, max(Math::Max(p1, p2))
{

}

NANON_FORCE_INLINE bool AABB::Intersect(const AABB& b) const
{
	bool x = (max.x >= b.min.x) && (min.x <= b.max.x);
	bool y = (max.y >= b.min.y) && (min.y <= b.max.y);
	bool z = (max.z >= b.min.z) && (min.z <= b.max.z);
	return x && y && z;
}

NANON_FORCE_INLINE bool AABB::Contain(const Math::Vec3& p) const
{
	return
		p.x >= min.x && p.x <= max.x &&
		p.y >= min.y && p.y <= max.y &&
		p.z >= min.z && p.z <= max.z;
}

NANON_FORCE_INLINE double AABB::SurfaceArea() const
{
	auto d = max - min;
	return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
}

NANON_FORCE_INLINE double AABB::Volume() const
{
	Math::Vec3 d = max - min;
	return d.x * d.y * d.z;
}

NANON_FORCE_INLINE int AABB::LongestAxis() const
{
	auto d = max - min;
	return d.x > d.y && d.x > d.z ? 0 : d.y > d.z ? 1 : 2;
}

NANON_FORCE_INLINE AABB AABB::Union(const AABB& b) const
{
	AABB r;
	r.min = Math::Min(min, b.min);
	r.max = Math::Max(max, b.max);
	return r;
}

NANON_FORCE_INLINE AABB AABB::Union(const Math::Vec3& p) const
{
	AABB r;
	r.min = Math::Min(min, p);
	r.max = Math::Max(max, p);
	return r;
}

NANON_FORCE_INLINE const Math::Vec3& AABB::operator[](int i) const
{
	return (&min)[i];
}

NANON_FORCE_INLINE Math::Vec3& AABB::operator[](int i)
{
	return (&min)[i];
}

NANON_NAMESPACE_END