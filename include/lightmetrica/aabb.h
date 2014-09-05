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
#ifndef LIB_LIGHTMETRICA_AABB_H
#define LIB_LIGHTMETRICA_AABB_H

#include "math.types.h"

LM_NAMESPACE_BEGIN

struct AABB
{

	Math::Vec3 min, max;

	LM_FORCE_INLINE AABB();
	LM_FORCE_INLINE AABB(const Math::Vec3& p);
	LM_FORCE_INLINE AABB(const Math::Vec3& p1, const Math::Vec3& p2);

	LM_FORCE_INLINE bool Intersect(const AABB& b) const;
	LM_FORCE_INLINE bool Contain(const Math::Vec3& p) const;
	LM_FORCE_INLINE Math::Float SurfaceArea() const;
	LM_FORCE_INLINE Math::Float Volume() const;
	LM_FORCE_INLINE int LongestAxis() const;
	LM_FORCE_INLINE AABB Union(const AABB& b) const;
	LM_FORCE_INLINE AABB Union(const Math::Vec3& p) const;

	//bool operator==(const AABB& b) const { return min == b.min && max == b.max; }
	//bool operator!=(const AABB& b) const { return min != b.min || max != b.max; }
	LM_FORCE_INLINE const Math::Vec3& operator[](int i) const;
	LM_FORCE_INLINE Math::Vec3& operator[](int i);

};

LM_NAMESPACE_END

#include "aabb.inl"

#endif // LIB_LIGHTMETRICA_AABB_H