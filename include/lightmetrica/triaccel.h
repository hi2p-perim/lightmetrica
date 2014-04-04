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


#ifdef LM_DEBUG_MODE
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