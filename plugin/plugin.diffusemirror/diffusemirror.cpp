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

#include "pch.plugin.h"
#include <lightmetrica/plugin.common.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

class DiffuseMirrorMixBSDF final : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("plugin.diffusemirror");

public:

	DiffuseMirrorMixBSDF()
		: ComponentProb(0.5)
	{
		
	}

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) override
	{
		node.ChildValueOrDefault("diffuse_reflectance", Math::Vec3(Math::Float(1)), R);
		return true;
	}

public:

	virtual bool SampleDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override
	{
		auto localWi = geom.worldToShading * query.wi;
		auto cosThetaI = Math::CosThetaZUp(localWi);
		if ((query.type & BSDFTypes()) == 0 || cosThetaI <= 0)
		{
			return false;
		}

		if (query.uComp < ComponentProb)
		{
			// Diffuse reflection
			auto localWo = Math::CosineSampleHemisphere(query.sample);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::DiffuseReflection;
			result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo) * ComponentProb;
		}
		else
		{
			// Specular reflection
			auto localWo = Math::ReflectZUp(localWi);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularReflection;
			result.pdf = Math::PDFEval(ComponentProb / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}

		return true;
	}

	virtual Math::Vec3 SampleAndEstimateDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override
	{
		auto localWi = geom.worldToShading * query.wi;
		auto cosThetaI = Math::CosThetaZUp(localWi);
		if ((query.type & BSDFTypes()) == 0 || cosThetaI <= 0)
		{
			return Math::Vec3();
		}

		if (query.uComp < ComponentProb)
		{
			// Diffuse reflection
			auto localWo = Math::CosineSampleHemisphere(query.sample);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::DiffuseReflection;
			result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo) * ComponentProb;

			auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
			if (Math::IsZero(sf))
			{
				return Math::Vec3();
			}

			return sf * R / ComponentProb;
		}
		else
		{
			// Specular reflection
			auto localWo = Math::ReflectZUp(localWi);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularReflection;
			result.pdf = Math::PDFEval(ComponentProb / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);

			auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
			if (Math::IsZero(sf))
			{
				return Math::Vec3();
			}

			return sf * R / ComponentProb;
		}
	}

	virtual bool SampleAndEstimateDirectionBidir(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result) const override
	{
		auto localWi = geom.worldToShading * query.wi;
		auto cosThetaI = Math::CosThetaZUp(localWi);
		if ((query.type & BSDFTypes()) == 0 || cosThetaI <= 0)
		{
			return false;
		}

		if (query.uComp < ComponentProb)
		{
			// Diffuse reflection
			auto localWo = Math::CosineSampleHemisphere(query.sample);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::DiffuseReflection;
			result.pdf[query.transportDir] = Math::CosineSampleHemispherePDFProjSA(localWo) * ComponentProb;
			result.pdf[1 - query.transportDir] = Math::CosineSampleHemispherePDFProjSA(localWi) * ComponentProb;

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

			result.weight[query.transportDir] = sf * R / ComponentProb;
			result.weight[1 - query.transportDir] = sfInv * R / ComponentProb;
		}
		else
		{
			// Specular reflection
			auto localWo = Math::ReflectZUp(localWi);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularReflection;
			result.pdf[query.transportDir] = Math::PDFEval(ComponentProb / Math::CosThetaZUp(localWo), Math::ProbabilityMeasure::ProjectedSolidAngle);
			result.pdf[1 - query.transportDir] = result.pdf[query.transportDir];

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

			result.weight[query.transportDir] = R * sf / ComponentProb;
			result.weight[1 - query.transportDir] = R * sfInv / ComponentProb;
		}

		return true;
	}

	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override
	{
		throw std::runtime_error("Not implemented");
	}

	virtual Math::PDFEval EvaluateDirectionPDF(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override
	{
		throw std::runtime_error("Not implemented");
	}

	virtual int BSDFTypes() const override
	{
		return GeneralizedBSDFType::DiffuseReflection | GeneralizedBSDFType::SpecularReflection;
	}

private:

	Math::Vec3 R;
	const Math::Float ComponentProb;

};

LM_COMPONENT_REGISTER_PLUGIN_IMPL(DiffuseMirrorMixBSDF, BSDF);

LM_NAMESPACE_END