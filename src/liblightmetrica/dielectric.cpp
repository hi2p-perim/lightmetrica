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
#include <lightmetrica/align.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/assert.h>

LM_NAMESPACE_BEGIN

/*!
	Dielectric BSDF.
	Implements dielectric BSDF.
*/
class DielectricBSDF final : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("dielectric");

public:

	DielectricBSDF() {}
	~DielectricBSDF() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const override;
	virtual Math::Vec3 SampleAndEstimateDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual bool SampleAndEstimateDirectionBidir(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result) const override;
	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual Math::PDFEval EvaluateDirectionPDF(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual int BSDFTypes() const override { return GeneralizedBSDFType::Specular; }

private:

	Math::Float EvalFrDielectic(const Math::Float& etaI, const Math::Float& etaT, const Math::Float& cosThetaI, Math::Float& cosThetaT) const;
	bool CheckRefract(const Math::Float& etaI, const Math::Float& etaT, const Math::Float& cosThetaI, const Math::Float& cosThetaT) const;

private:

	Math::Vec3 R;			// Specular reflectance
	Math::Vec3 T;			// Specular transmittance
	Math::Float n1;			// External IOR
	Math::Float n2;			// Internal IOR

};

bool DielectricBSDF::Load( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("specular_reflectance", Math::Vec3(Math::Float(1)), R);
	node.ChildValueOrDefault("specular_transmittance", Math::Vec3(Math::Float(1)), T);
	node.ChildValueOrDefault("external_ior", Math::Float(1), n1);
	node.ChildValueOrDefault("internal_ior", Math::Float(1), n2);
	return true;
}

bool DielectricBSDF::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	bool useR = (query.type & GeneralizedBSDFType::SpecularReflection) > 0;
	bool useT = (query.type & GeneralizedBSDFType::SpecularTransmission) > 0;

	if (!useR && !useT)
	{
		return false;
	}

	auto localWi = geom.worldToShading * query.wi;
	auto cosThetaI = Math::CosThetaZUp(localWi);
	bool entering = cosThetaI > Math::Float(0);

	// Index of refraction
	auto etaI = n1;
	auto etaT = n2;
	if (!entering)
	{
		std::swap(etaI, etaT);
	}

	auto eta = etaI / etaT;
	
	// Fresnel term
	Math::Float cosThetaT;
	auto Fr = EvalFrDielectic(etaI, etaT, cosThetaI, cosThetaT);

	if (useR && useT)
	{
		// Choose reflection or transmission using RR
		if (query.uComp <= Fr)
		{
			// Reflection
			auto localWo = Math::ReflectZUp(localWi);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularReflection;
			result.pdf = Math::PDFEval(Fr / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
		else
		{
			// Transmission
			auto localWo = Math::RefractZUp(localWi, eta, cosThetaT);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularTransmission;
			result.pdf = Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
	}
	else if (useR)
	{
		auto localWo = Math::ReflectZUp(localWi);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularReflection;
		result.pdf = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}
	else if (useT)
	{
		// Total internal reflection
		if (Math::IsZero(cosThetaT))
		{
			return false;
		}

		auto localWo = Math::RefractZUp(localWi, eta, cosThetaT);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularTransmission;
		result.pdf = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return true;
}

Math::Vec3 DielectricBSDF::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	bool useR = (query.type & GeneralizedBSDFType::SpecularReflection) > 0;
	bool useT = (query.type & GeneralizedBSDFType::SpecularTransmission) > 0;

	if (!useR && !useT)
	{
		return false;
	}

	auto localWi = geom.worldToShading * query.wi;
	auto cosThetaI = Math::CosThetaZUp(localWi);
	bool entering = cosThetaI > Math::Float(0);

	// Index of refraction
	auto etaI = n1;
	auto etaT = n2;
	if (!entering)
	{
		std::swap(etaI, etaT);
	}

	auto eta = etaI / etaT;

	// Fresnel term
	Math::Float cosThetaT;
	auto Fr = EvalFrDielectic(etaI, etaT, cosThetaI, cosThetaT);

	if (useR && useT)
	{
		// Choose reflection or transmission using RR
		if (query.uComp <= Fr)
		{
			// Reflection
			auto localWo = Math::ReflectZUp(localWi);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularReflection;
			result.pdf = Math::PDFEval(Fr / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);

			// Correction factor for shading normal
			auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
			if (Math::IsZero(sf))
			{
				return Math::Vec3();
			}

			// f / p_{\sigma^\bot}
			// = R * Fr / cos(w_o) / (p_\sigma / cos(w_o))
			// = R * Fr / cos(w_o) / (Fr / cos(w_o))
			// = R
			return R * sf;
		}
		else
		{
			// Transmission
			auto localWo = Math::RefractZUp(localWi, eta, cosThetaT);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularTransmission;
			result.pdf = Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);

			// Correction factor for shading normal
			auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
			if (Math::IsZero(sf))
			{
				return Math::Vec3();
			}

			// Correction factor for transmission
			auto tf = query.transportDir == TransportDirection::EL ? eta : Math::Float(1);

			// Evaluation
			// Non-adjoint case
			// f / p_{\sigma^\bot}
			// = f / (p_\sigma / cos(w_o))
			// = (1/eta)^2 * T * (1 - Fr) / cos(theta) / ((1 - Fr) / cos(w_o))
			// = (1/eta)^2 * T
			// Adjoint case
			// f / p_{\sigma^\bot} * eta^2
			// = (1/eta)^2 * T * eta^2
			// = T
			return T * (tf * tf * sf);
		}
	}
	else if (useR)
	{
		// Reflection
		auto localWo = Math::ReflectZUp(localWi);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularReflection;
		result.pdf = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);

		// Correction factor for shading normal
		auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
		if (Math::IsZero(sf))
		{
			return Math::Vec3();
		}

		return R * sf * Fr;
	}
	else if (useT)
	{
		// Total internal reflection
		if (Math::IsZero(cosThetaT))
		{
			return false;
		}

		// Transmission
		auto localWo = Math::RefractZUp(localWi, eta, cosThetaT);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularTransmission;
		result.pdf = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);

		// Correction factor for shading normal
		auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
		if (Math::IsZero(sf))
		{
			return Math::Vec3();
		}

		// Correction factor for transmission
		auto tf = query.transportDir == TransportDirection::EL ? eta : Math::Float(1);

		return T * (tf * tf * sf) * (Math::Float(1) - Fr);
	}

	LM_UNREACHABLE();
}

bool DielectricBSDF::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
{
	bool useR = (query.type & GeneralizedBSDFType::SpecularReflection) > 0;
	bool useT = (query.type & GeneralizedBSDFType::SpecularTransmission) > 0;

	if (!useR && !useT)
	{
		return false;
	}

	auto localWi = geom.worldToShading * query.wi;
	auto cosThetaI = Math::CosThetaZUp(localWi);
	bool entering = cosThetaI > Math::Float(0);

	// Index of refraction
	auto etaI = n1;
	auto etaT = n2;
	if (!entering)
	{
		std::swap(etaI, etaT);
	}

	auto eta = etaI / etaT;

	// Fresnel term
	Math::Float cosThetaT;
	auto Fr = EvalFrDielectic(etaI, etaT, cosThetaI, cosThetaT);

	if (useR && useT)
	{
		// Choose reflection or transmission using RR
		if (query.uComp <= Fr)
		{
			// Reflection
			auto localWo = Math::ReflectZUp(localWi);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularReflection;
			result.pdf[query.transportDir] = Math::PDFEval(Fr / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
			result.pdf[1 - query.transportDir] = result.pdf[query.transportDir];

			// Correction factor for shading normal
			auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
			if (Math::IsZero(sf))
			{
				return false;
			}

			auto sfInv = ShadingNormalCorrectionFactor(TransportDirection(1 - query.transportDir), geom, localWo, localWi, result.wo, query.wi);
			if (Math::IsZero(sfInv))
			{
				return false;
			}

			result.weight[query.transportDir] = R * sf;
			result.weight[1 - query.transportDir] = R * sfInv;
		}
		else
		{
			// Transmission
			auto localWo = Math::RefractZUp(localWi, eta, cosThetaT);
			result.wo = geom.shadingToWorld * localWo;
			result.sampledType = GeneralizedBSDFType::SpecularTransmission;
			result.pdf[query.transportDir] = Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);
			result.pdf[1 - query.transportDir] = Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);

			// Correction factor for shading normal
			auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
			if (Math::IsZero(sf))
			{
				return false;
			}

			auto sfInv = ShadingNormalCorrectionFactor(TransportDirection(1 - query.transportDir), geom, localWo, localWi, result.wo, query.wi);
			if (Math::IsZero(sfInv))
			{
				return false;
			}

			// Correction factor for transmission
			auto tf = query.transportDir == TransportDirection::EL ? eta : Math::Float(1);
			auto tfInv = query.transportDir == TransportDirection::LE ? Math::Float(1) / eta : Math::Float(1);

			result.weight[query.transportDir] = T * (tf * tf * sf);
			result.weight[1 - query.transportDir] = T * (tfInv * tfInv * sfInv);
		}
	}
	else if (useR)
	{
		// Reflection
		auto localWo = Math::ReflectZUp(localWi);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularReflection;
		result.pdf[query.transportDir] = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
		result.pdf[1 - query.transportDir] = result.pdf[query.transportDir];

		// Correction factor for shading normal
		auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
		if (Math::IsZero(sf))
		{
			return false;
		}

		auto sfInv = ShadingNormalCorrectionFactor(TransportDirection(1 - query.transportDir), geom, localWo, localWi, result.wo, query.wi);
		if (Math::IsZero(sfInv))
		{
			return false;
		}

		result.weight[query.transportDir] = R * sf * Fr;
		result.weight[1 - query.transportDir] = R * sfInv * Fr;
	}
	else if (useT)
	{
		// Total internal reflection
		if (Math::IsZero(cosThetaT))
		{
			return false;
		}

		// Transmission
		auto localWo = Math::RefractZUp(localWi, eta, cosThetaT);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularTransmission;
		result.pdf[query.transportDir] = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);
		result.pdf[1 - query.transportDir] = Math::PDFEval(Math::Float(1) / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);

		// Correction factor for shading normal
		auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, result.wo);
		if (Math::IsZero(sf))
		{
			return false;
		}

		auto sfInv = ShadingNormalCorrectionFactor(TransportDirection(1 - query.transportDir), geom, localWo, localWi, result.wo, query.wi);
		if (Math::IsZero(sfInv))
		{
			return false;
		}

		// Correction factor for transmission
		auto tf = query.transportDir == TransportDirection::EL ? eta : Math::Float(1);
		auto tfInv = query.transportDir == TransportDirection::LE ? Math::Float(1) / eta : Math::Float(1);

		result.weight[query.transportDir] = T * (tf * tf * sf) * (Math::Float(1) - Fr);
		result.weight[1 - query.transportDir] = T * (tfInv * tfInv * sfInv) * (Math::Float(1) - Fr);
	}

	return true;
}

Math::Vec3 DielectricBSDF::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	bool useR = (query.type & GeneralizedBSDFType::SpecularReflection) != 0;
	bool useT = (query.type & GeneralizedBSDFType::SpecularTransmission) != 0;
	if (!useR && !useT)
	{
		return Math::Vec3();
	}

	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	auto cosThetaI = Math::CosThetaZUp(localWi);
	auto cosThetaT = Math::CosThetaZUp(localWo);
	bool entering = cosThetaI > Math::Float(0);

	// Index of refraction
	auto etaI = n1;
	auto etaT = n2;
	if (!entering)
	{
		std::swap(etaI, etaT);
	}

	auto eta = etaI / etaT;

	// Fresnel term
	Math::Float cosThetaT2;
	auto Fr = EvalFrDielectic(etaI, etaT, cosThetaI, cosThetaT2);

	if (cosThetaI * cosThetaT >= Math::Float(0))
	{
		// Reflection
		// Reflected wi and wo must be same
		if (!query.forced)
		{
			auto localWoTemp = Math::ReflectZUp(localWi);
			auto localWiTemp = Math::ReflectZUp(localWo);
			auto woTemp = geom.shadingToWorld * localWoTemp;
			auto wiTemp = geom.shadingToWorld * localWiTemp;
			if (woTemp != query.wo && wiTemp != query.wi)
			{
				return Math::Vec3();
			}
		}

		// Correction factor for shading normal
		auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
		if (Math::IsZero(sf))
		{
			return Math::Vec3();
		}

		// f(wi, wo)
		// = R * Fr / cos(theta)
		return R * (Fr * sf) / Math::Abs(cosThetaI);
	}

	// Refraction
	// Refracted wi and wo must be same
	if (!query.forced)
	{
		Math::Float cosThetaT2Rev;
		EvalFrDielectic(etaT, etaI, cosThetaT, cosThetaT2Rev);
		auto localWoTemp = Math::RefractZUp(localWi, etaI / etaT, cosThetaT2);
		auto localWiTemp = Math::RefractZUp(localWo, etaT / etaI, cosThetaT2Rev);
		auto woTemp = geom.shadingToWorld * localWoTemp;
		auto wiTemp = geom.shadingToWorld * localWiTemp;
		if (!useT || (woTemp != query.wo && wiTemp != query.wi))
		{
			return Math::Vec3();
		}
	}

	// Correction factor for shading normal
	auto sf = ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
	if (Math::IsZero(sf))
	{
		return Math::Vec3();
	}

	// Correction factor for transmission
	auto tf = query.transportDir == TransportDirection::EL ? eta : Math::Float(1);

	// Evaluation
	// Non-adjoint case
	// f(wi, wo)
	// = (1/eta)^2 * T * (1-Fr) / cos(theta)
	// Adjoint case
	// f(wi, wo) * eta^2
	// = T * (1-Fr) / cos(theta)
	return T * ((1 - Fr) * tf * tf * sf) / Math::Abs(cosThetaT2);
}

Math::PDFEval DielectricBSDF::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	bool useR = (query.type & GeneralizedBSDFType::SpecularReflection) != 0;
	bool useT = (query.type & GeneralizedBSDFType::SpecularTransmission) != 0;
	if (!useR && !useT)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	auto localWi = geom.worldToShading * query.wi;
	auto localWo = geom.worldToShading * query.wo;
	auto cosThetaI = Math::CosThetaZUp(localWi);
	auto cosThetaT = Math::CosThetaZUp(localWo);
	bool entering = cosThetaI > Math::Float(0);

	// Index of refraction
	auto etaI = n1;
	auto etaT = n2;
	if (!entering)
	{
		std::swap(etaI, etaT);
	}

	// Fresnel term
	Math::Float cosThetaT2;
	auto Fr = EvalFrDielectic(etaI, etaT, cosThetaI, cosThetaT2);

	if (cosThetaI * cosThetaT >= Math::Float(0))
	{
		// Reflection
		// Reflected wi and wo must be same
		if (!query.forced)
		{
			auto localWoTemp = Math::ReflectZUp(localWi);
			auto localWiTemp = Math::ReflectZUp(localWo);
			auto woTemp = geom.shadingToWorld * localWoTemp;
			auto wiTemp = geom.shadingToWorld * localWiTemp;
			if (woTemp != query.wo && wiTemp != query.wi)
			{
				return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
			}
		}

		return Math::PDFEval(Fr / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	// Refraction
	// Refracted wi and wo must be same
	if (!query.forced)
	{
		Math::Float cosThetaT2Rev;
		EvalFrDielectic(etaT, etaI, cosThetaT, cosThetaT2Rev);
		auto localWoTemp = Math::RefractZUp(localWi, etaI / etaT, cosThetaT2);
		auto localWiTemp = Math::RefractZUp(localWo, etaT / etaI, cosThetaT2Rev);
		auto woTemp = geom.shadingToWorld * localWoTemp;
		auto wiTemp = geom.shadingToWorld * localWiTemp;
		if (!useT || (woTemp != query.wo && wiTemp != query.wi))
		{
			return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
	}

	return Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaT2), Math::ProbabilityMeasure::ProjectedSolidAngle);
}

Math::Float DielectricBSDF::EvalFrDielectic( const Math::Float& /*etaI*/, const Math::Float& /*etaT*/, const Math::Float& cosThetaI, Math::Float& cosThetaT ) const
{
#if 0
	return Math::Float(1);
#elif 1

	const bool entering = cosThetaI > Math::Float(0);
	const auto eta = n2 / n1;
	if (eta == Math::Float(1))
	{
		cosThetaT = -cosThetaI;
		return Math::Float(0);
	}

	const auto scale = entering ? Math::Float(1) / eta : eta;
	const auto cosThetaTSq = Math::Float(1) - (Math::Float(1) - cosThetaI * cosThetaI) * (scale * scale);

	if (cosThetaTSq <= Math::Float(0))
	{
		cosThetaT = Math::Float(0);
		return Math::Float(1);
	}

	auto cosThetaI_Temp = Math::Abs(cosThetaI);
	auto cosThetaT_Temp = Math::Sqrt(cosThetaTSq);

	auto Rs = (cosThetaI_Temp - eta * cosThetaT_Temp) / (cosThetaI_Temp + eta * cosThetaT_Temp);
	auto Rp = (eta * cosThetaI_Temp - cosThetaT_Temp) / (eta * cosThetaI_Temp + cosThetaT_Temp);

	cosThetaT = entering ? -cosThetaT_Temp : cosThetaT_Temp;
	return Math::Float(0.5) * (Rs * Rs + Rp * Rp);

#else
	Math::Float Fr;
	bool entering = cosThetaI > Math::Float(0);

	// Using Snell's law, calculate sin^2(thetaT)
	Math::Float eta = etaI / etaT;
	Math::Float sinThetaTSq = eta * eta * (Math::Float(1) - cosThetaI * cosThetaI);

	if (sinThetaTSq >= Math::Float(1))
	{
		// Total internal reflection
		Fr = Math::Float(1); 
		cosThetaT = Math::Float(0);
	}
	else
	{
		auto cosThetaI2 = Math::Abs(cosThetaI);
		cosThetaT = Math::Sqrt(Math::Float(1) - sinThetaTSq);

		if (etaI == etaT)
		{
			Fr = Math::Float(0);
		}
		else
		{
			Math::Float Rs = (etaI * cosThetaI2 - etaT * cosThetaT) / (etaI * cosThetaI2 + etaT * cosThetaT);
			Math::Float Rp = (etaT * cosThetaI2 - etaI * cosThetaT) / (etaT * cosThetaI2 + etaI * cosThetaT);

			Fr = (Rs * Rs + Rp * Rp) / Math::Float(2);

			// Flip theta_t if incoming ray comes from negative z
			if (entering)
			{
				cosThetaT = -cosThetaT;
			}
		}
	}

	return Fr;
#endif
}

bool DielectricBSDF::CheckRefract( const Math::Float& etaI, const Math::Float& etaT, const Math::Float& cosThetaI, const Math::Float& cosThetaT ) const
{
	// Due to numerical problem directly compute cosThetaT by EvalFrDielectic
	// might result in poor accuracy, so we use this function instead.

	Math::Float eta = etaI / etaT;
	Math::Float sinThetaTSq = eta * eta * (Math::Float(1) - cosThetaI * cosThetaI);

	if (sinThetaTSq >= Math::Float(1))
	{
		// Total internal reflection
		// Should not be reached here
		//LM_ASSERT(false);
		return false;
	}
	
	auto v1 = cosThetaT * cosThetaT;
	auto v2 = (Math::Float(1) - sinThetaTSq);
	auto abs = Math::Abs(v1 - v2);
	if (abs > Math::Constants::EpsLarge())
	{
		return false;
	}

	return true;
}

LM_COMPONENT_REGISTER_IMPL(DielectricBSDF, BSDF);

LM_NAMESPACE_END