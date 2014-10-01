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
#include <lightmetrica/emittershape.h>

LM_NAMESPACE_BEGIN

/*!
	Constant environment light.
	Implements environment light with constant luminance.
*/
class ConstantEnvironmentLight final : public Light
{
public:

	LM_COMPONENT_IMPL_DEF("env.const");

public:

	ConstantEnvironmentLight() {}
	~ConstantEnvironmentLight() {}

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) override;

public:

	virtual bool SampleDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual Math::Vec3 SampleAndEstimateDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const override;
	virtual bool SampleAndEstimateDirectionBidir(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result) const override;
	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual Math::PDFEval EvaluateDirectionPDF(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const override;
	virtual int BSDFTypes() const override { return GeneralizedBSDFType::LightDirection; }

public:

	virtual void SamplePosition(const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf) const override;
	virtual Math::Vec3 EvaluatePosition(const SurfaceGeometry& geom) const override;
	virtual Math::PDFEval EvaluatePositionPDF(const SurfaceGeometry& geom) const override;
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives) override {}
	virtual void PostConfigure(const Scene& scene) override;
	virtual EmitterShape* CreateEmitterShape() const override;
	virtual AABB GetAABB() const override;

public:

	virtual bool EnvironmentLight() const override { return true; }

private:

	Math::Vec3 Le;				//!< Luminance.
	BoundingSphere bsphere;		//!< Bounding sphere containing the entire scene.
	Math::Float area;			//!< Area of the bounding sphere.
	Math::Float invArea;		//!< Inverse of #area.

};

bool ConstantEnvironmentLight::Load( const ConfigNode& node, const Assets& assets )
{
	if (!node.ChildValue<Math::Vec3>("luminance", Le)) return false;
	return true;
}

void ConstantEnvironmentLight::PostConfigure( const Scene& scene )
{
	// Create bounding sphere
	auto aabb = scene.GetAABB();
	bsphere.center = (aabb.max + aabb.min) / Math::Float(2);
	bsphere.radius = Math::Length(bsphere.center - aabb.max);

	// Compute area
	area = Math::Float(4) * Math::Constants::Pi() * bsphere.radius * bsphere.radius;
	invArea = Math::Float(1) / area;
}

EmitterShape* ConstantEnvironmentLight::CreateEmitterShape() const
{
	// Create sphere
	std::unique_ptr<EmitterShape> shape(ComponentFactory::Create<EmitterShape>("sphere"));
	
	// Configure parameters
	std::map<std::string, boost::any> params;
	params["center"] = bsphere.center;
	params["radius"] = bsphere.radius;
	params["emitter"] = this;
	if (!shape->Configure(params))
	{
		return nullptr;
	}

	return shape.release();
}

AABB ConstantEnvironmentLight::GetAABB() const
{
	AABB aabb;
	aabb.min = bsphere.center - Math::Vec3(bsphere.radius);
	aabb.max = bsphere.center + Math::Vec3(bsphere.radius);
	return aabb;
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
	auto d = Math::UniformSampleSphere(sample);
	geom.degenerated = false;
	geom.p = bsphere.center + d * bsphere.radius;
	geom.gn = geom.sn = -d;
	geom.ComputeTangentSpace();
	pdf = Math::PDFEval(invArea, Math::ProbabilityMeasure::Area);
}

Math::Vec3 ConstantEnvironmentLight::EvaluatePosition( const SurfaceGeometry& geom ) const
{
	return Le * Math::Constants::Pi();
}

Math::PDFEval ConstantEnvironmentLight::EvaluatePositionPDF( const SurfaceGeometry& geom ) const
{
	return Math::PDFEval(invArea, Math::ProbabilityMeasure::Area);
}

LM_COMPONENT_REGISTER_IMPL(ConstantEnvironmentLight, Light);

LM_NAMESPACE_END