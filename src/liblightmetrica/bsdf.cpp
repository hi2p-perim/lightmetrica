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

#include "pch.h"
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN

BSDF::BSDF()
{

}

BSDF::BSDF(const std::string& id)
	: GeneralizedBSDF(id)
{

}

BSDF::~BSDF()
{

}

Math::Float BSDF::ShadingNormalCorrectionFactor( const TransportDirection& transportDir, const SurfaceGeometry& geom, const Math::Vec3& localWi, const Math::Vec3& localWo, const Math::Vec3& worldWi, const Math::Vec3& worldWo ) const
{
	// Prevent light leak
	// In some cases wi and wo are same side according to the shading normal
	// but opposite side according to the geometry normal.
	Math::Float wiDotNg = Math::Dot(worldWi, geom.gn);
	Math::Float woDotNg = Math::Dot(worldWo, geom.gn);
	Math::Float wiDotNs = Math::CosThetaZUp(localWi);
	Math::Float woDotNs = Math::CosThetaZUp(localWo);
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

LM_NAMESPACE_END
