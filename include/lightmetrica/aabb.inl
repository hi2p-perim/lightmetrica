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

#include "aabb.h"

LM_NAMESPACE_BEGIN

LM_FORCE_INLINE AABB::AABB()
	: min(Math::Constants::Inf())
	, max(-Math::Constants::Inf())
{

}

LM_FORCE_INLINE AABB::AABB(const Math::Vec3& p)
	: min(p)
	, max(p)
{

}

LM_FORCE_INLINE AABB::AABB(const Math::Vec3& p1, const Math::Vec3& p2)
	: min(Math::Min(p1, p2))
	, max(Math::Max(p1, p2))
{

}

LM_FORCE_INLINE bool AABB::Intersect(const AABB& b) const
{
	bool x = (max.x >= b.min.x) && (min.x <= b.max.x);
	bool y = (max.y >= b.min.y) && (min.y <= b.max.y);
	bool z = (max.z >= b.min.z) && (min.z <= b.max.z);
	return x && y && z;
}

LM_FORCE_INLINE bool AABB::Contain(const Math::Vec3& p) const
{
	return
		p.x >= min.x && p.x <= max.x &&
		p.y >= min.y && p.y <= max.y &&
		p.z >= min.z && p.z <= max.z;
}

LM_FORCE_INLINE Math::Float AABB::SurfaceArea() const
{
	auto d = max - min;
	return Math::Float(2) * (d.x * d.y + d.y * d.z + d.z * d.x);
}

LM_FORCE_INLINE Math::Float AABB::Volume() const
{
	Math::Vec3 d = max - min;
	return d.x * d.y * d.z;
}

LM_FORCE_INLINE int AABB::LongestAxis() const
{
	auto d = max - min;
	return d.x > d.y && d.x > d.z ? 0 : d.y > d.z ? 1 : 2;
}

LM_FORCE_INLINE AABB AABB::Union(const AABB& b) const
{
	AABB r;
	r.min = Math::Min(min, b.min);
	r.max = Math::Max(max, b.max);
	return r;
}

LM_FORCE_INLINE AABB AABB::Union(const Math::Vec3& p) const
{
	AABB r;
	r.min = Math::Min(min, p);
	r.max = Math::Max(max, p);
	return r;
}

LM_FORCE_INLINE const Math::Vec3& AABB::operator[](int i) const
{
	return (&min)[i];
}

LM_FORCE_INLINE Math::Vec3& AABB::operator[](int i)
{
	return (&min)[i];
}

LM_NAMESPACE_END
