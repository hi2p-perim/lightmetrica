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