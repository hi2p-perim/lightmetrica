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
	Dielectric BSDF.
	Implements dielectric BSDF.
*/
class DielectricBSDF : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("dielectric");

public:

	DielectricBSDF() {}
	~DielectricBSDF() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;

private:

	Math::Float EvalFrDielectic(Math::Float etaI, Math::Float etaT, Math::Float cosThetaI, Math::Float& cosThetaT) const;
	Math::Vec3 Reflect(const Math::Vec3& wi) const { return Math::Vec3(-wi.x, -wi.y, wi.z); }
	Math::Vec3 Refract(const Math::Vec3& wi, Math::Float eta, Math::Float cosThetaT) const { return Math::Vec3(-eta * wi.x, -eta * wi.y, cosThetaT); }

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
	if ((query.type & GeneralizedBSDFType::SpecularReflection) == 0 ||
		(query.type & GeneralizedBSDFType::SpecularTransmission) == 0)
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

	// Choose reflection or transmission using RR
	if (query.uComp < Fr)
	{
		// Reflection
		auto localWo = Reflect(localWi);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularReflection;
		result.pdf = Math::PDFEval(Fr / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
		//result.pdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::ProjectedSolidAngle);
		//result.pdf = Math::PDFEval(Fr, Math::ProbabilityMeasure::ProjectedSolidAngle);
	}
	else
	{
		// Transmission
		auto localWo = Refract(localWi, eta, cosThetaT);
		result.wo = geom.shadingToWorld * localWo;
		result.sampledType = GeneralizedBSDFType::SpecularTransmission;
		result.pdf = Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaT), Math::ProbabilityMeasure::ProjectedSolidAngle);
		//result.pdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::ProjectedSolidAngle);
		//result.pdf = Math::PDFEval(Math::Float(1) - Fr, Math::ProbabilityMeasure::ProjectedSolidAngle);
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
		if (!useR || Math::Dot(Reflect(localWi), localWo) < Math::Float(1) - Math::Constants::Eps())
		{
			return Math::Vec3();
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
		//return R * sf;
		//return R * (Fr * sf);
	}

	// Refraction
	// Refracted wi and wo must be same
	if (!useT || Math::Dot(Refract(localWi, eta, cosThetaT2), localWo) < Math::Float(1) - Math::Constants::Eps())
	{
		return Math::Vec3();
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
	//return T * (tf * tf * sf);
	//return T * ((1 - Fr) * tf * tf * sf);
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

	auto eta = etaI / etaT;

	// Fresnel term
	Math::Float cosThetaT2;
	auto Fr = EvalFrDielectic(etaI, etaT, cosThetaI, cosThetaT2);

	if (cosThetaI * cosThetaT >= Math::Float(0))
	{
		// Reflection
		if (!useR || Math::Dot(Reflect(localWi), localWo) < Math::Float(1) - Math::Constants::Eps())
		{
			return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}

		return Math::PDFEval(Fr / Math::Abs(cosThetaI), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	// Refraction
	if (!useT || Math::Dot(Refract(localWi, eta, cosThetaT2), localWo) < Math::Float(1) - Math::Constants::Eps())
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return Math::PDFEval((Math::Float(1) - Fr) / Math::Abs(cosThetaT2), Math::ProbabilityMeasure::ProjectedSolidAngle);
}

Math::Float DielectricBSDF::EvalFrDielectic( Math::Float etaI, Math::Float etaT, Math::Float cosThetaI, Math::Float& cosThetaT ) const
{
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
		cosThetaI = Math::Abs(cosThetaI);
		cosThetaT = Math::Sqrt(Math::Float(1) - sinThetaTSq);

		if (etaI == etaT)
		{
			Fr = Math::Float(0);
		}
		else
		{
			Math::Float Rs = (etaI * cosThetaI - etaT * cosThetaT) / (etaI * cosThetaI + etaT * cosThetaT);
			Math::Float Rp = (etaT * cosThetaI - etaI * cosThetaT) / (etaT * cosThetaI + etaI * cosThetaT);

			Fr = (Rs * Rs + Rp * Rp) / Math::Float(2);

			// Flip theta_t if incoming ray comes from negative z
			if (entering)
			{
				cosThetaT = -cosThetaT;
			}
		}
	}

	return Fr;
}

LM_COMPONENT_REGISTER_IMPL(DielectricBSDF, BSDF);

LM_NAMESPACE_END