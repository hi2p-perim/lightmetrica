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
#ifndef LIB_LIGHTMETRICA_TRIACCEL_H
#define LIB_LIGHTMETRICA_TRIACCEL_H

#include "math.functions.h"
#include "ray.h"

LM_NAMESPACE_BEGIN

// TriAccel by Ingo Wald
// We used the implementation in mitsuba with a slight modification
struct TriAccel
{

	uint32_t k;
	Math::Float n_u;
	Math::Float n_v;
	Math::Float n_d;

	Math::Float a_u;
	Math::Float a_v;
	Math::Float b_nu;
	Math::Float b_nv;

	Math::Float c_nu;
	Math::Float c_nv;
	uint32_t shapeIndex;
	uint32_t primIndex;

	// Construct from vertex data. Returns '1' if there was a failure
	LM_FORCE_INLINE int Load(const Math::Vec3& A, const Math::Vec3& B, const Math::Vec3& C);

	// Fast ray-triangle intersection test
	LM_FORCE_INLINE bool Intersect(const Ray& ray, Math::Float mint, Math::Float maxt, Math::Float& u, Math::Float& v, Math::Float& t) const;

};

LM_FORCE_INLINE int TriAccel::Load(const Math::Vec3& A, const Math::Vec3& B, const Math::Vec3& C)
{
	static const int waldModulo[4] = { 1, 2, 0, 1 };

	Math::Vec3 b = C - A;
	Math::Vec3 c = B - A;
	Math::Vec3 N = Math::Cross(c, b);

	k = 0;
	// Determine the largest projection axis
	for (int j = 0; j < 3; j++)
	{
		if (Math::Abs(N[j]) > Math::Abs(N[k]))
		{
			k = j;
		}
	}

	uint32_t u = waldModulo[k];
	uint32_t v = waldModulo[k+1];
	const Math::Float n_k = N[k];
	const Math::Float denom = b[u]*c[v] - b[v]*c[u];

	if (denom == 0)
	{
		k = 3;
		return 1;
	}

	// Pre-compute intersection calculation constants
	n_u   =  N[u] / n_k;
	n_v   =  N[v] / n_k;
	n_d   =  Math::Dot(Math::Vec3(A), N) / n_k;
	b_nu  =  b[u] / denom;
	b_nv  = -b[v] / denom;
	a_u   =  A[u];
	a_v   =  A[v];
	c_nu  =  c[v] / denom;
	c_nv  = -c[u] / denom;

	return 0;
}

LM_FORCE_INLINE bool TriAccel::Intersect(const Ray& ray, Math::Float mint, Math::Float maxt, Math::Float& u, Math::Float& v, Math::Float& t) const
{
	Math::Float o_u, o_v, o_k, d_u, d_v, d_k;
	switch (k)
	{
	case 0:
		o_u = ray.o[1];
		o_v = ray.o[2];
		o_k = ray.o[0];
		d_u = ray.d[1];
		d_v = ray.d[2];
		d_k = ray.d[0];
		break;

	case 1:
		o_u = ray.o[2];
		o_v = ray.o[0];
		o_k = ray.o[1];
		d_u = ray.d[2];
		d_v = ray.d[0];
		d_k = ray.d[1];
		break;

	case 2:
		o_u = ray.o[0];
		o_v = ray.o[1];
		o_k = ray.o[2];
		d_u = ray.d[0];
		d_v = ray.d[1];
		d_k = ray.d[2];
		break;

	default:
		return false;
	}


#if LM_DEBUG_MODE
	if (d_u * n_u + d_v * n_v + d_k == 0)
	{
		return false;
	}
#endif

	// Calculate the plane intersection (Typo in the thesis?)
	t = (n_d - o_u*n_u - o_v*n_v - o_k) / (d_u * n_u + d_v * n_v + d_k);
	if (t < mint || t > maxt)
	{
		return false;
	}

	// Calculate the projected plane intersection point
	const Math::Float hu = o_u + t * d_u - a_u;
	const Math::Float hv = o_v + t * d_v - a_v;

	// In barycentric coordinates
	u = hv * b_nu + hu * b_nv;
	v = hu * c_nu + hv * c_nv;

	return u >= 0 && v >= 0 && u+v <= 1;
}

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TRIACCEL_H