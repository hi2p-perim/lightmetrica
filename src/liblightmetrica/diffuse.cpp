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
#include <lightmetrica/math.stats.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/align.h>

LM_NAMESPACE_BEGIN

/*!
	Diffuse BSDF.
	Implements the diffuse BSDF.
*/
class DiffuseBSDF : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("diffuse");

public:

	DiffuseBSDF() {}
	~DiffuseBSDF() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual bool SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual bool Degenerated() const { return false; }
	virtual int BSDFTypes() const { return GeneralizedBSDFType::DiffuseReflection; }

private:

	Math::Vec3 diffuseReflectance;

};

bool DiffuseBSDF::Load( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("diffuse_reflectance", Math::Vec3(Math::Float(1)), diffuseReflectance);
	return true;
}

bool DiffuseBSDF::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return false;
	}

	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::DiffuseReflection;
	result.pdf = Math::CosineSampleHemispherePDF(localWo);
	if (Math::IsZero(result.pdf.v))
	{
		return false;
	}

	// Convert to projected solid angle measure
	result.pdf.v /= Math::CosThetaZUp(localWo);
	result.pdf.measure = Math::ProbabilityMeasure::ProjectedSolidAngle;

	return true;
}

Math::Vec3 DiffuseBSDF::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return Math::Vec3();
	}

	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::DiffuseReflection;
	result.pdf = Math::CosineSampleHemispherePDF(localWo);
	if (Math::IsZero(result.pdf.v))
	{
		return Math::Vec3();
	}

	result.pdf.v /= Math::CosThetaZUp(localWo);
	result.pdf.measure = Math::ProbabilityMeasure::ProjectedSolidAngle;

	Math::Float sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	// f / p_{\sigma^\bot)
	// R * \pi^-1 / (p_\sigma / \cos(w_o))
	// R * \pi^-1 / (\pi^-1 * \cos(w_o) / \cos(w_o))
	// R
	return diffuseReflectance * sf;
}

bool DiffuseBSDF::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return false;
	}

	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::DiffuseReflection;
	result.pdf[query.transportDir] = Math::CosineSampleHemispherePDFProjSA(localWo);
	result.pdf[1-query.transportDir] = Math::CosineSampleHemispherePDFProjSA(localWi);

	auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
	if (Math::IsZero(sf))
	{
		return false;
	}

	auto sfInv = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
	if (Math::IsZero(sfInv))
	{
		return false;
	}

	result.weight[query.transportDir] = diffuseReflectance * sf;
	result.weight[1-query.transportDir] = diffuseReflectance * sfInv;

	return true;
}

Math::Vec3 DiffuseBSDF::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::Vec3();
	}

	Math::Float sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	return diffuseReflectance * Math::Constants::InvPi() * sf;
}

Math::PDFEval DiffuseBSDF::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	auto pdfD = Math::CosineSampleHemispherePDF(localWo);
	pdfD.v /= Math::CosThetaZUp(localWo);
	pdfD.measure = Math::ProbabilityMeasure::ProjectedSolidAngle;

	return pdfD;
}

LM_COMPONENT_REGISTER_IMPL(DiffuseBSDF, BSDF);

LM_NAMESPACE_END