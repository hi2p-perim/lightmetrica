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
#ifndef LIB_LIGHTMETRICA_BSDF_H
#define LIB_LIGHTMETRICA_BSDF_H

#include "generalizedbsdf.h"
#include "surfacegeometry.h"

LM_NAMESPACE_BEGIN

/*!
	BSDF.
	A base class for BSDF implementations.
*/
class BSDF : public GeneralizedBSDF
{
public:

	LM_ASSET_INTERFACE_DEF("bsdf", "bsdfs");
	LM_ASSET_DEPENDENCIES("texture");

public:

	BSDF() {}
	virtual ~BSDF() {}

protected:

	/*!
		Compute correction factor for shading normal.
		See [Veach 1997] for details.
		\param transportDir Transport direction.
		\param geom Surface geometry.
		\param localWi #wi in local shading coordinates.
		\param localWo #wo in local shading coordinates.
		\param worldWi #wi in world coordinates.
		\param worldWo #wo in world coordinates.
	*/
	LM_FORCE_INLINE static Math::Float ShadingNormalCorrectionFactor(
		const TransportDirection& transportDir, const SurfaceGeometry& geom,
		const Math::Vec3& localWi, const Math::Vec3& localWo, const Math::Vec3& worldWi, const Math::Vec3& worldWo)
	{
		// Prevent light leak
		// In some cases wi and wo are same side according to the shading normal
		// but opposite side according to the geometry normal.
		auto wiDotNg = Math::Dot(worldWi, geom.gn);
		auto woDotNg = Math::Dot(worldWo, geom.gn);
		auto wiDotNs = Math::CosThetaZUp(localWi);
		auto woDotNs = Math::CosThetaZUp(localWo);
		if (wiDotNg * wiDotNs <= 0 || woDotNg * woDotNs <= 0)
		{
			return Math::Float(0);
		}

		// Special handling for adjoint case
		// Be careful of the difference of the notation between Veach's thesis;
		// in the framework, wo is always the propagating direction.
		if (transportDir == TransportDirection::LE)
		{
			// |w_i, N_s| * |w_o, N_g| / |w_i, N_g| / |w_o, N_s| 
			return wiDotNs * woDotNg / (woDotNs * wiDotNg);
		}

		return Math::Float(1);
	}

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BSDF_H