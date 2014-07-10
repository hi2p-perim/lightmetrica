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

#include "math.linalgebra.h"
#include "math.basic.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T>
LM_FORCE_INLINE void OrthonormalBasis(const TVec3<T>& a, TVec3<T>& b, TVec3<T>& c)
{
	c = Math::Abs(a.x) > Math::Abs(a.y)
		? Math::Normalize(Math::Vec3(a.z, T(0), -a.x))
		: Math::Normalize(Math::Vec3(T(0), a.z, -a.y));
	b = Math::Normalize(Math::Cross(c, a));
}

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END
