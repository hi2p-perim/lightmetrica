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

#include "math.constants.h"
#include <limits>
#include <boost/math/constants/constants.hpp>

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Zero()
{
	return T(0);
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Pi()
{
	return boost::math::constants::pi<T>();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::InvPi()
{
	return InvTwoPi() * T(2);
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::InvTwoPi()
{
	return boost::math::constants::one_div_two_pi<T>();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Inf()
{
	return std::numeric_limits<T>::max();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Eps()
{
	return T(1e-7);
}

template <>
LM_FORCE_INLINE float TConstants<float>::Eps()
{
	return 1e-4f;
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::EpsLarge()
{
	return T(1e-5);
}

template <>
LM_FORCE_INLINE float TConstants<float>::EpsLarge()
{
	return 1e-3f;
}

template <typename T>
LM_FORCE_INLINE static T MachineEps()
{
	return std::numeric_limits<T>::epsilon();
}

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END