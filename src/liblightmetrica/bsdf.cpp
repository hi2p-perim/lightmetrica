/*
	L I G H T  M E T R I C A

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
#include <lightmetrica/intersection.h>
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

BSDF::BSDF(const std::string& id)
	: Asset(id)
{

}

BSDF::~BSDF()
{

}

Math::Float BSDF::ShadingNormalCorrectionFactor( const BSDFEvaluateQuery& query, const Intersection& isect ) const
{
	// Prevent light leak
	// In some cases wi and wo are same side according to the shading normal
	// but opposite side according to the geometry normal.
	auto worldWi = isect.shadingToWorld * query.wi;
	auto worldWo = isect.shadingToWorld * query.wo;
	Math::Float wiDotNg = Math::Dot(worldWi, isect.gn);
	Math::Float woDotNg = Math::Dot(worldWo, isect.gn);
	if (wiDotNg * Math::CosThetaZUp(query.wi) <= 0 || woDotNg * Math::CosThetaZUp(query.wo) <= 0)
	{
		return Math::Float(0);
	}

	// Special handling for adjoint case
	// Be careful of the difference of the notation between Veach's thesis;
	// in the framework, wo is always the propagating direction.
	if (query.transportDir == TransportDirection::LightToCamera)
	{
		// |w_i, N_s| * |w_o, N_g| / |w_i, N_g| / |w_o, N_s| 
		return Math::CosThetaZUp(query.wi) * woDotNg / (Math::CosThetaZUp(query.wo) * wiDotNg);
	}

	return Math::Float(1);
}

LM_NAMESPACE_END
