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
#include <lightmetrica/light.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/math.linalgebra.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/align.h>

LM_NAMESPACE_BEGIN

/*!
	Area light.
	Implements an area light source.
*/
class AreaLight : public Light
{
public:

	LM_COMPONENT_IMPL_DEF("area");

public:

	AreaLight() {}
	~AreaLight() {}

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
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives);

private:

	Math::Vec3 Le;
	typedef std::tuple<Math::Vec3, Math::Vec3, Math::Vec3> TrianglePosition;
	std::vector<TrianglePosition, aligned_allocator<TrianglePosition, std::alignment_of<TrianglePosition>::value>> triangles;
	std::vector<Math::Float> triangleAreaCdf;
	Math::Float area;
	Math::Vec3 power;

};

bool AreaLight::Load( const ConfigNode& node, const Assets& /*assets*/ )
{
	if (!node.ChildValue<Math::Vec3>("luminance", Le)) return true;

	// For testing configuration
	// TODO : This smells
	auto testingNode = node.Child("testing");
	if (!testingNode.Empty())
	{
		LM_LOG_WARN("Testing configuration is enabled");
		if (!testingNode.ChildValue<Math::Float>("area", area))
		{
			return false;
		}
	}

	return true;
}

void AreaLight::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{
	// Create CDF
	triangleAreaCdf.clear();
	triangleAreaCdf.push_back(Math::Float(0));
	for (size_t i = 0; i < primitives.size(); i++)
	{
		auto& primitive = primitives[i];
		const auto* ps = primitive->mesh->Positions();
		const auto* fs = primitive->mesh->Faces();
		for (int f = 0; f < primitive->mesh->NumFaces() / 3; f++)
		{
			unsigned int v1 = fs[3*f  ];
			unsigned int v2 = fs[3*f+1];
			unsigned int v3 = fs[3*f+2];
			Math::Vec3 p1(primitive->transform * Math::Vec4(ps[3*v1], ps[3*v1+1], ps[3*v1+2], Math::Float(1)));
			Math::Vec3 p2(primitive->transform * Math::Vec4(ps[3*v2], ps[3*v2+1], ps[3*v2+2], Math::Float(1)));
			Math::Vec3 p3(primitive->transform * Math::Vec4(ps[3*v3], ps[3*v3+1], ps[3*v3+2], Math::Float(1)));
			triangles.push_back(std::make_tuple(p1, p2, p3));

			// Area of the triangle
			auto area = Math::Length(Math::Cross(p2 - p1, p3 - p1)) / Math::Float(2);
			triangleAreaCdf.push_back(triangleAreaCdf.back() + area);
		}
	}

	// Normalize
	area = triangleAreaCdf.back();
	for (auto& v : triangleAreaCdf)
	{
		v /= area;
	}

	power = Le * Math::Constants::Pi() * area;
}

void AreaLight::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{
	Math::Vec2 ps(sample);

	// Choose a primitive according to the area
	int index =
		Math::Clamp(
		static_cast<int>(std::upper_bound(triangleAreaCdf.begin(), triangleAreaCdf.end(), ps.y) - triangleAreaCdf.begin() - 1),
		0, static_cast<int>(triangleAreaCdf.size()) - 2);

	// Reuse sample
	ps.y = (ps.y - triangleAreaCdf[index]) / (triangleAreaCdf[index+1] - triangleAreaCdf[index]);

	// Triangle vertex positions
	const auto& p1 = std::get<0>(triangles[index]);
	const auto& p2 = std::get<1>(triangles[index]);
	const auto& p3 = std::get<2>(triangles[index]);

	// Sample position
	auto b = Math::UniformSampleTriangle(ps);
	geom.p = p1 * (Math::Float(1) - b.x - b.y) + p2 * b.x + p3 * b.y;
	
	// Geometry normal at #p
	// Shading normal is set to #gn for convenience
	geom.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));
	geom.sn = geom.gn;
	geom.ComputeTangentSpace();

	// Not degenerated
	geom.degenerated = false;

	// Evaluation of PDF
	pdf = Math::PDFEval(Math::Float(1) / area, Math::ProbabilityMeasure::Area);
}

bool AreaLight::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
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

Math::Vec3 AreaLight::SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE))
	{
		return Math::Vec3();
	}

	result.sampledType = GeneralizedBSDFType::LightDirection;
	auto localWo = Math::CosineSampleHemisphere(query.sample);
	result.wo = geom.shadingToWorld * localWo;
	result.pdf = Math::CosineSampleHemispherePDFProjSA(localWo);

	// Le_D / p_{\sigma^\bot}
	// = \pi^-1 / (p_\sigma / cos(w_o))
	// = \pi^-1 / (\pi^-1 * cos(w_o) / cos(w_o))
	// = 1
	return Math::Vec3(Math::Float(1));
}

bool AreaLight::SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const
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

Math::Vec3 AreaLight::EvaluatePosition( const SurfaceGeometry& /*geom*/ ) const
{
	return Le * Math::Constants::Pi();
}

Math::PDFEval AreaLight::EvaluatePositionPDF( const SurfaceGeometry& /*geom*/ ) const
{
	return Math::PDFEval(Math::Float(1) / area, Math::ProbabilityMeasure::Area);
}

Math::Vec3 AreaLight::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE) || Math::Dot(query.wo, geom.gn) <= 0)
	{
		return Math::Vec3();
	}
	else
	{
		return Math::Vec3(Math::Constants::InvPi());
	}
}

Math::PDFEval AreaLight::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	auto localWo = geom.worldToShading * query.wo;
	if ((query.type & GeneralizedBSDFType::LightDirection) == 0 || (query.transportDir != TransportDirection::LE) || Math::CosThetaZUp(localWo) <= 0)
	{
		return Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
	}

	return CosineSampleHemispherePDFProjSA(localWo);
}

LM_COMPONENT_REGISTER_IMPL(AreaLight, Light);

LM_NAMESPACE_END
