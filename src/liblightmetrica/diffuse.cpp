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

	const Texture* texture;
	std::unique_ptr<Texture> constantColorTexture;
	Math::Vec3 diffuseReflectance;

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
		Math::Vec3 color;
		diffuseReflectanceNode.ChildValueOrDefault("color", Math::Vec3(Math::Float(1)), diffuseReflectance);
		constantColorTexture.reset(ComponentFactory::Create<Texture>("constant"));
		texture = constantColorTexture.get();
	}
	else if (!textureNode.Empty())
	{
		texture = assets.ResolveReferenceToAsset<Texture>(textureNode);
		if (!texture)
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
	result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

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

	return Math::CosineSampleHemispherePDFProjSA(localWo);
}

LM_COMPONENT_REGISTER_IMPL(DiffuseBSDF, BSDF);

LM_NAMESPACE_END