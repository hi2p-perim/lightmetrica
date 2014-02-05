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
#include <lightmetrica/logger.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

class DiffuseBSDF::Impl : public Object
{
public:

	Impl(DiffuseBSDF* self);
	bool LoadAsset( const ConfigNode& node, const Assets& assets );
	bool Sample( const BSDFSampleQuery& query, BSDFSampleResult& result ) const;
	Math::Vec3 Evaluate( const BSDFEvaluateQuery& query, const Intersection& isect ) const;
	Math::PDFEval Pdf( const BSDFEvaluateQuery& query ) const;

private:

	DiffuseBSDF* self;
	Math::Vec3 diffuseReflectance;

};

DiffuseBSDF::Impl::Impl( DiffuseBSDF* self )
	: self(self)
{

}

bool DiffuseBSDF::Impl::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("diffuse_reflectance", Math::Vec3(Math::Float(1)), diffuseReflectance);
	return true;
}

bool DiffuseBSDF::Impl::Sample( const BSDFSampleQuery& query, BSDFSampleResult& result ) const
{
	if ((query.type & BSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(query.wi) <= 0)
	{
		return false;
	}

	result.wo = Math::CosineSampleHemisphere(query.sample);
	result.sampledType = BSDFType::DiffuseReflection;
	result.pdf = Math::CosineSampleHemispherePDF(result.wo);
	if (result.pdf.v == Math::Float(0))
	{
		return false;
	}

	return true;
}

Math::Vec3 DiffuseBSDF::Impl::Evaluate( const BSDFEvaluateQuery& query, const Intersection& isect ) const
{
	if ((query.type & BSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(query.wi) <= 0 || Math::CosThetaZUp(query.wo) <= 0)
	{
		return Math::Vec3();
	}

	Math::Float sf = self->ShadingNormalCorrectionFactor(query, isect);
	if (sf == 0.0)
	{
		return Math::Vec3();
	}

	return diffuseReflectance * Math::Constants::InvPi() * sf;
}

Math::PDFEval DiffuseBSDF::Impl::Pdf( const BSDFEvaluateQuery& query ) const
{
	if ((query.type & BSDFType::DiffuseReflection) == 0 || Math::CosThetaZUp(query.wi) <= 0 || Math::CosThetaZUp(query.wo) <= 0)
	{
		return Math::PDFEval();
	}

	return Math::CosineSampleHemispherePDF(query.wo);
}

// --------------------------------------------------------------------------------

DiffuseBSDF::DiffuseBSDF( const std::string& id )
	: BSDF(id)
	, p(new Impl(this))
{

}

DiffuseBSDF::~DiffuseBSDF()
{
	LM_SAFE_DELETE(p);
}

bool DiffuseBSDF::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

Math::Vec3 DiffuseBSDF::Evaluate( const BSDFEvaluateQuery& query, const Intersection& isect ) const
{
	return p->Evaluate(query, isect);
}

Math::PDFEval DiffuseBSDF::Pdf( const BSDFEvaluateQuery& query ) const
{
	return p->Pdf(query);
}

bool DiffuseBSDF::Sample( const BSDFSampleQuery& query, BSDFSampleResult& result ) const
{
	return p->Sample(query, result);
}

LM_NAMESPACE_END