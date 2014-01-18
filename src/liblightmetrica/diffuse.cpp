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
#include <lightmetrica/diffuse.h>
#include <lightmetrica/pdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <lightmetrica/math.stats.h>
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

DiffuseBSDF::DiffuseBSDF( const std::string& id )
	: BSDF(id)
{

}

DiffuseBSDF::~DiffuseBSDF()
{

}

bool DiffuseBSDF::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	// 'diffuse_reflectance'
	auto diffuseReflectanceNode = node.child("diffuse_reflectance");
	if (!diffuseReflectanceNode)
	{
		diffuseReflectance = Math::Vec3(Math::Float(1));
		LM_LOG_WARN("Using default value for 'diffuse_reflectance'");
	}
	else
	{
		// 'rgb'
		auto rgbNode = diffuseReflectanceNode.child("rgb");
		if (!rgbNode)
		{
			LM_LOG_ERROR("Missing 'rgb'");
			return false;
		}

		diffuseReflectance = PugiHelper::ParseVec3(rgbNode);
	}

	return true;
}

Math::Vec3 DiffuseBSDF::Evaluate( const BSDFEvaluateQuery& query, const Intersection& isect ) const
{
	if ((query.type & BSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(query.wi) <= 0 || Math::CosThetaZUp(query.wo) <= 0)
	{
		return Math::Vec3();
	}

	Math::Float sf = ShadingNormalCorrectionFactor(query, isect);
	if (sf == 0.0)
	{
		return Math::Vec3();
	}

	return diffuseReflectance * Math::Constants::InvPi() * Math::CosThetaZUp(query.wo) * sf;
}

PDF DiffuseBSDF::Pdf( const BSDFEvaluateQuery& query ) const
{
	if ((query.type & BSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(query.wi) <= 0 || Math::CosThetaZUp(query.wo) <= 0)
	{
		return PDF();
	}

	return PDF(
		Math::CosThetaZUp(query.wo) * Math::Constants::InvPi(),
		ProbabilityMeasure::SolidAngle);
}

bool DiffuseBSDF::SampleWo( const BSDFSampleQuery& query, BSDFSampledData& sampled ) const
{
	if ((query.type & BSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(query.wi) <= 0)
	{
		return false;
	}

	sampled.wo = Math::CosineSampleHemisphere(query.u);
	sampled.sampledType = BSDFType::DiffuseReflection;
	sampled.pdf = Pdf(BSDFEvaluateQuery(query, sampled));
	if (sampled.pdf.v == Math::Float(0))
	{
		return false;
	}

	return true;
}

LM_NAMESPACE_END