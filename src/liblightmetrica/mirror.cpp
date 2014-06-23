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
#include <lightmetrica/align.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN

/*!
	Perfect mirror BSDF.
	Implements perfect mirror BSDF.
*/
class PerfectMirrorBSDF : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("mirror");

public:

	PerfectMirrorBSDF() {}
	~PerfectMirrorBSDF() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual bool Degenerated() const { return true; }
	virtual int BSDFTypes() const { return GeneralizedBSDFType::SpecularReflection; }

private:

	Math::Vec3 R;	// Specular reflectance

};

bool PerfectMirrorBSDF::Load( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("specular_reflectance", Math::Vec3(Math::Float(1)), R);
	return true;
}

bool PerfectMirrorBSDF::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0)
	{
		return false;
	}

	auto localWi = geom.worldToShading * query.wi;
	result.wo = geom.shadingToWorld * Math::ReflectZUp(localWi);
	result.sampledType = GeneralizedBSDFType::SpecularReflection;
	result.pdf = Math::PDFEval(Math::Float(1) / Math::CosThetaZUp(localWi), Math::ProbabilityMeasure::ProjectedSolidAngle);

	return true;
}

Math::Vec3 PerfectMirrorBSDF::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::Vec3();
	}

	if (Math::LInfinityNorm(Math::ReflectZUp(localWi) - localWo) > Math::Constants::EpsLarge())
	{
		return Math::Vec3();
	}

	auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	// f(wi, wo) = R / cos(theta)
	return R * (sf / Math::CosThetaZUp(localWi));
}

Math::PDFEval PerfectMirrorBSDF::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	if (Math::LInfinityNorm(Math::ReflectZUp(localWi) - localWo) > Math::Constants::EpsLarge())
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return Math::PDFEval(Math::Float(1) / Math::CosThetaZUp(localWi), Math::ProbabilityMeasure::ProjectedSolidAngle);
}

LM_COMPONENT_REGISTER_IMPL(PerfectMirrorBSDF, BSDF);

LM_NAMESPACE_END