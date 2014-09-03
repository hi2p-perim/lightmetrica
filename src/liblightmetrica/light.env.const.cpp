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
#include <lightmetrica/light.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/boundingsphere.h>
#include <lightmetrica/scene.h>

LM_NAMESPACE_BEGIN

/*!
	Constant environment light.
	Implements environment light with constant luminance.
*/
class ConstantEnvironmentLight : public Light
{
public:

	LM_COMPONENT_IMPL_DEF("env.const");

public:

	ConstantEnvironmentLight() {}
	~ConstantEnvironmentLight() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual bool SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual bool Degenerated() const { return false; }
	virtual int BSDFTypes() const { return GeneralizedBSDFType::LightDirection; }

public:

	virtual void SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const;
	virtual Math::Vec3 EvaluatePosition( const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluatePositionPDF( const SurfaceGeometry& geom ) const;
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives) {}
	virtual void ConfigureAfterSceneBuild( const Scene& scene );

public:

	virtual bool EnvironmentLight() const { return true; }

private:

	Math::Vec3 Le;				//!< Luminance
	BoundingSphere bsphere;		//!< Bounding sphere containing the entire scene

};

bool ConstantEnvironmentLight::Load( const ConfigNode& node, const Assets& assets )
{
	if (!node.ChildValue<Math::Vec3>("luminance", Le)) return true;
}

void ConstantEnvironmentLight::ConfigureAfterSceneBuild( const Scene& scene )
{
	// Create bounding sphere
	bsphere = scene.GetAABB().ConvertToBoundingSphere();
}

bool ConstantEnvironmentLight::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE))
	{
		return false;
	}

	result.sampledType = GeneralizedBSDFType::LightDirection;
	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

	return true;
}

Math::Vec3 ConstantEnvironmentLight::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE))
	{
		return Math::Vec3();
	}

	result.sampledType = GeneralizedBSDFType::LightDirection;
	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

	return Math::Vec3(Math::Float(1));
}

bool ConstantEnvironmentLight::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
{
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE))
	{
		return false;
	}

	result.sampledType = GeneralizedBSDFType::LightDirection;
	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.pdf[query.transportDir] = Math::CosineSampleHemispherePDFProjSA(localWo);
	result.pdf[1-query.transportDir] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	result.weight[query.transportDir] = Math::Vec3(Math::Float(1));
	result.weight[1-query.transportDir] = Math::Vec3();

	return true;
}

Math::Vec3 ConstantEnvironmentLight::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE) || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::Vec3();
	}

	return Math::Vec3(Math::Constants::InvPi());
}

Math::PDFEval ConstantEnvironmentLight::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE) || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return CosineSampleHemispherePDFProjSA(localWo);
}

void ConstantEnvironmentLight::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{
	
}

Math::Vec3 ConstantEnvironmentLight::EvaluatePosition( const SurfaceGeometry& geom ) const
{

}

Math::PDFEval ConstantEnvironmentLight::EvaluatePositionPDF( const SurfaceGeometry& geom ) const
{

}

LM_COMPONENT_REGISTER_IMPL(ConstantEnvironmentLight, Light);

LM_NAMESPACE_END