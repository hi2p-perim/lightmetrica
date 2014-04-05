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
#ifndef __LIB_LIGHTMETICA_SURFACE_GEOMETRY_H__
#define __LIB_LIGHTMETICA_SURFACE_GEOMETRY_H__

#include "common.h"
#include "math.types.h"
#include "math.linalgebra.h"

LM_NAMESPACE_BEGIN

/*!
	Surface geometry.
	Contains geometry information of a point on the scene surfaces.
	Normals and tangents are defined only if the surface geometry at #p is not degenerated.
*/
struct SurfaceGeometry
{

	bool degenerated;				//!< The surface geometry is positionally degenerated if true

	Math::Vec3 p;					//!< Intersection point
	Math::Vec3 gn;					//!< Geometry normal
	Math::Vec3 sn;					//!< Shading normal
	Math::Vec3 ss, st;				//!< Tangent vectors w.r.t. shading normal
	Math::Vec2 uv;					//!< Texture coordinates

	Math::Mat3 worldToShading;		//!< Convarsion to local shading coordinates from world coordinates
	Math::Mat3 shadingToWorld;		//!< Convarsion to world coordinates from local shading coordinates

	/*!
		Compute tangent space w.r.t. shading normal.
		Computes #sn, #sn, #worldToShading and #shadingToWorld from #sn.
	*/
	LM_FORCE_INLINE void ComputeTangentSpace()
	{
		// Tangent vectors
		Math::OrthonormalBasis(sn, ss, st);

		// Shading coordinates conversion
		shadingToWorld = Math::Mat3(ss, st, sn);
		worldToShading = Math::Transpose(shadingToWorld);
	}

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETICA_SURFACE_GEOMETRY_H__