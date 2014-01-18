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
#ifndef __LIB_LIGHTMETRICA_INTERSECTION_H__
#define __LIB_LIGHTMETRICA_INTERSECTION_H__

#include "common.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

struct Primitive;

/*!
	Intersection.
	Intermediate structure for managing intersection data.
	When intersection query is succeeded, information on the intersected point
	is returned with the structure.
*/
struct Intersection
{

	const Primitive* primitive;
	unsigned int primitiveIndex;
	unsigned int triangleIndex;

	Math::Vec3 p;			//!< Intersection point
	Math::Vec3 gn;			//!< Geometry normal
	Math::Vec3 sn;			//!< Shading normal
	Math::Vec3 ss, st;		//!< Tangent vectors w.r.t. shading normal
	Math::Vec2 uv;			//!< Texture coordinates

	Math::Mat3 worldToShading;
	Math::Mat3 shadingToWorld;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_INTERSECTION_H__