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

#include "pch.h"
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/align.h>
#include <lightmetrica/texture.h>
#include <lightmetrica/assets.h>

LM_NAMESPACE_BEGIN

/*!
	Diffuse BSDF.
	Implements the diffuse BSDF.
*/
class DiffuseBSDF final : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("diffuse");

public:

	DiffuseBSDF() {}
	~DiffuseBSDF() {}

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) override;
	virtual bool Load(std::map<std::string, boost::any>& params) override;

public:

	virtual bool SampleDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual Math::Vec3 SampleAndEstimateDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual bool SampleAndEstimateDirectionBidir(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result) const override;
	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual Math::PDFEval EvaluateDirectionPDF(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual int BSDFTypes() const override { return GeneralizedBSDFType::DiffuseReflection; }

private:

	const Texture* R;
	std::unique_ptr<Texture> constantColorTexture;

};

bool DiffuseBSDF::Load( const ConfigNode& node, const Assets& assets )
{
	// Find 'diffuse_reflectance' node
	auto diffuseReflectanceNode = node.Child("diffuse_reflectance");
	if (diffuseReflectanceNode.Empty())
	{
		LM_LOG_WARN("Missing 'diffuse_reflectance' element");
		return false;
	}

	// 'color' & 'texture' element
	auto colorNode   = diffuseReflectanceNode.Child("color");
	auto textureNode = diffuseReflectanceNode.Child("texture");
	if (!colorNode.Empty() && !textureNode.Empty())
	{
		LM_LOG_INFO("'color' and 'texture' element cannot be used simultaneously");
		return false;
	}
	else if (!colorNode.Empty())
	{
		constantColorTexture.reset(ComponentFactory::Create<Texture>("constant"));
		constantColorTexture->Load(diffuseReflectanceNode, assets);
		R = constantColorTexture.get();
	}
	else if (!textureNode.Empty())
	{
		R = assets.ResolveReferenceToAsset<Texture>(textureNode);
		if (!R)
		{
			return false;
		}
	}
	else
	{
		LM_LOG_INFO("Missing 'color' or 'texture' element");
		return false;
	}

	return true;
}

bool DiffuseBSDF::Load(std::map<std::string, boost::any>& params)
{
	constantColorTexture.reset(ComponentFactory::Create<Texture>("constant"));
	constantColorTexture->Load(params);
	R = constantColorTexture.get();
	return true;
}

bool DiffuseBSDF::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & BSDFTypes()) == 0 || Math::CosThetaZUp(localWi) <= 0)
	{
		return false;
	}

	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.sampledType = GeneralizedBSDFType::DiffuseReflection;
	result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

	return true;
}

Math::Vec3 DiffuseBSDF::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & BSDFTypes()) == 0 || Math::CosThetaZUp(localWi) <= 0)
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

	// f / p_{\sigma^\bot)
	// R * \pi^-1 / (p_\sigma / \cos(w_o))
	// R * \pi^-1 / (\pi^-1 * \cos(w_o) / \cos(w_o))
	// R
	return R->Evaluate(geom.uv) * sf;
}

bool DiffuseBSDF::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
{
	auto localWi = geom.worldToShading * query.wi;
	if ((query.type & BSDFTypes()) == 0 || Math::CosThetaZUp(localWi) <= 0)
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

	auto diffuseR = R->Evaluate(geom.uv);
	result.weight[query.transportDir] = diffuseR * sf;
	result.weight[1-query.transportDir] = diffuseR * sfInv;

	return true;
}

Math::Vec3 DiffuseBSDF::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & BSDFTypes()) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::Vec3();
	}

	Math::Float sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	return R->Evaluate(geom.uv) * Math::Constants::InvPi() * sf;
}

Math::PDFEval DiffuseBSDF::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & BSDFTypes()) == 0 || Math::CosThetaZUp(localWi) <= 0 || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return Math::CosineSampleHemispherePDFProjSA(localWo);
}

LM_COMPONENT_REGISTER_IMPL(DiffuseBSDF, BSDF);

LM_NAMESPACE_END