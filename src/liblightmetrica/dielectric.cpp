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
#include <lightmetrica/dielectric.h>
#include <lightmetrica/align.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN

class DielectricBSDF::Impl : public SIMDAlignedType
{
public:

	Impl(DielectricBSDF* self);

public:

	bool LoadAsset( const ConfigNode& node, const Assets& assets );

public:

	bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;

private:

	Math::Float EvalFrDielectic(Math::Float etaI, Math::Float etaT, Math::Float cosThetaI, Math::Float& cosThetaT) const;
	Math::Vec3 Reflect(const Math::Vec3& wi) const { return Math::Vec3(-wi.x, -wi.y, wi.z); }
	Math::Vec3 Refract(const Math::Vec3& wi, Math::Float eta, Math::Float cosThetaT) const { return Math::Vec3(-eta * wi.x, -eta * wi.y, cosThetaT); }

private:

	DielectricBSDF* self;
	Math::Vec3 R;			// Specular reflectance
	Math::Vec3 T;			// Specular transmittance
	Math::Float n1;			// External IOR
	Math::Float n2;			// Internal IOR

};

DielectricBSDF::Impl::Impl( DielectricBSDF* self )
	: self(self)
{

}

bool DielectricBSDF::Impl::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("specular_reflectance", Math::Vec3(Math::Float(1)), R);
	node.ChildValueOrDefault("specular_transmittance", Math::Vec3(Math::Float(1)), T);
	node.ChildValueOrDefault("external_ior", Math::Float(1), n1);
	node.ChildValueOrDefault("internal_ior", Math::Float(1), n2);
	return true;
}

bool DielectricBSDF::Impl::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	return false;
}

Math::Vec3 DielectricBSDF::Impl::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
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
		if (!useR || Math::Dot(Reflect(localWi), localWo) < Math::Float(1) - Math::Constants::EpsLarge())
		{
			return Math::Vec3();
		}

		// Correction factor for shading normal
		auto sf = self->ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
		if (sf == 0.0)
		{
			return Math::Vec3();
		}

		// f(wi, wo) * cos(theta)
		// = R * Fr / cos(theta) * cos(theta)
		// = R * Fr
		return R * (Fr * sf);
	}
	else
	{
		// Refraction
		// Refracted wi and wo must be same
		if (!useT || Math::Dot(Refract(localWi, eta, cosThetaT2), localWo) < Math::Float(1) - Math::Constants::EpsLarge())
		{
			return Math::Vec3();
		}

		// Correction factor for shading normal
		auto sf = self->ShadingNormalCorrectionFactor(query.transportDir, geom, localWi, localWo, query.wi, query.wo);
		if (Math::IsZero(sf))
		{
			return Math::Vec3();
		}

		// Correction factor for transmission
		auto tf = query.transportDir == TransportDirection::EL ? eta : Math::Float(1);

		// Evaluation
		// Non-adjoint case
		// f(wi, wo) * cos(theta)
		// = (1/eta)^2 * T * (1-Fr) / cos(theta) * cos(theta)
		// = (1/eta)^2 * T * (1-Fr)
		// Adjoint case
		// f(wi, wo) * cos(theta) * eta^2
		// = T * (1-Fr)
		return T * ((1 - Fr) * tf * tf * sf);
	}

	return Math::Vec3();
}

Math::PDFEval DielectricBSDF::Impl::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return Math::PDFEval();
}

Math::Float DielectricBSDF::Impl::EvalFrDielectic( Math::Float etaI, Math::Float etaT, Math::Float cosThetaI, Math::Float& cosThetaT ) const
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

// --------------------------------------------------------------------------------

DielectricBSDF::DielectricBSDF( const std::string& id )
	: BSDF(id)
	, p(new Impl(this))
{

}

DielectricBSDF::~DielectricBSDF()
{
	LM_SAFE_DELETE(p);
}

bool DielectricBSDF::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

bool DielectricBSDF::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	return p->SampleDirection(query, geom, result);
}

Math::Vec3 DielectricBSDF::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return p->EvaluateDirection(query, geom);
}

Math::PDFEval DielectricBSDF::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return p->EvaluateDirectionPDF(query, geom);
}

LM_NAMESPACE_END