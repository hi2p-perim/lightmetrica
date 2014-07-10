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

#include <lightmetrica/plugin.common.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/math.stats.h>

LM_NAMESPACE_BEGIN

/*!
	Test plugin.
	This plugin implements almost-do-nothing BSDF.
*/
class TestBSDF : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("plugin.testbsdf");

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets ) 
	{
		R = Math::Vec3(Math::Float(1), Math::Float(0), Math::Float(0));
		return true;
	}

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
	{
		auto localWi = geom.worldToShading * query.wi;
		if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
		{
			return false;
		}

		auto localWo = Math::CosineSampleHemisphere(query.sample);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::DiffuseReflection;
		result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

		return true;
	}

	virtual Math::Vec3 SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
	{
		auto localWi = geom.worldToShading * query.wi;
		if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0)
		{
			return Math::Vec3();
		}

		auto localWo = Math::CosineSampleHemisphere(query.sample);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::DiffuseReflection;
		result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

		Math::Float sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
		if (Math::IsZero(sf))
		{
			return Math::Vec3();
		}

		return sf * TexFunc(geom.uv);
	}

	virtual bool SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
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

		result.weight[query.transportDir] = sf * TexFunc(geom.uv);
		result.weight[1-query.transportDir] = sfInv * TexFunc(geom.uv);

		return true;
	}

	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
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

		return Math::Constants::InvPi() * sf * TexFunc(geom.uv);
	}

	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
	{
		auto localWi = geom.worldToShading * query.wi;
		auto localWo = geom.worldToShading * query.wo;
		if ((query.type & GeneralizedBSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
		{
			return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}

		return Math::CosineSampleHemispherePDFProjSA(localWo);
	}

	virtual bool Degenerated() const
	{
		return false;
	}

	virtual int BSDFTypes() const
	{
		return GeneralizedBSDFType::DiffuseReflection;
	}

private:

	Math::Vec3 TexFunc(const Math::Vec2& uv) const
	{
		const Math::Float Scale(10);
		auto u = uv.x * Scale;
		auto v = uv.y * Scale;
		if ((static_cast<int>(u) + static_cast<int>(v)) % 2 == 0)
		{
			return R;
		}
		
		return Math::Vec3(Math::Float(1));
	}

private:

	Math::Vec3 R;

};

LM_COMPONENT_REGISTER_PLUGIN_IMPL(TestBSDF, BSDF);

LM_NAMESPACE_END