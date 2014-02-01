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
#include <lightmetrica/arealight.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <lightmetrica/math.stats.h>
#include <lightmetrica/math.linalgebra.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

class AreaLight::Impl : public Object
{
public:

	bool LoadAsset( const ConfigNode& node, const Assets& assets );

public:

	Math::Vec3 EvaluateLe(const Math::Vec3& d, const Math::Vec3& gn) const;
	void RegisterPrimitives(const std::vector<Primitive*>& primitives);
	void Sample(const LightSampleQuery& query, LightSampleResult& result) const;
	
private:

	Math::Vec3 Le;
	std::vector<std::tuple<Math::Vec3, Math::Vec3, Math::Vec3>> triangles;
	std::vector<Math::Float> triangleAreaCdf;
	Math::Float area;
	Math::Vec3 power;

};

bool AreaLight::Impl::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	if (!node.ChildValue<Math::Vec3>("luminance", Le)) return false;
	return true;
}

Math::Vec3 AreaLight::Impl::EvaluateLe( const Math::Vec3& d, const Math::Vec3& gn ) const
{
	return Math::Dot(d, gn) < Math::Float(0)
		? Math::Vec3()
		: Le;
}

void AreaLight::Impl::RegisterPrimitives( const std::vector<Primitive*>& primitives )
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

void AreaLight::Impl::Sample( const LightSampleQuery& query, LightSampleResult& result ) const
{
	Math::Vec2 ps(query.sampleP);

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
	result.p = p1 * (Math::Float(1) - b.x - b.y) + p2 * b.x + p3 * b.y;
	result.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));
	result.pdfP = Math::PDFEval(Math::Float(1) / area, Math::ProbabilityMeasure::Discrete);

	// Sample direction
	Math::Vec3 s, t;
	Math::OrthonormalBasis(result.gn, s, t);
	auto localToWorld = Math::Mat3(s, t, result.gn);
	auto localDir = Math::CosineSampleHemisphere(query.sampleD);
	result.d = localToWorld * localDir;

	// Returns direction PDF evaluation in projected solid angle measure,
	// in order to handle degenerated cases without branching
	result.pdfD = Math::PDFEval(
		Math::CosineSampleHemispherePDF(localDir).v / Math::CosThetaZUp(localDir),
		Math::ProbabilityMeasure::ProjectedSolidAngle);
}

// --------------------------------------------------------------------------------

AreaLight::AreaLight(const std::string& id)
	: Light(id)
	, p(new Impl)
{

}

AreaLight::~AreaLight()
{
	LM_SAFE_DELETE(p);
}

Math::Vec3 AreaLight::EvaluateLe( const Math::Vec3& d, const Math::Vec3& gn ) const
{
	return p->EvaluateLe(d, gn);
}

void AreaLight::RegisterPrimitives(const std::vector<Primitive*>& primitives)
{
	p->RegisterPrimitives(primitives);
}

bool AreaLight::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

void AreaLight::Sample( const LightSampleQuery& query, LightSampleResult& result ) const
{
	return p->Sample(query, result);
}

LM_NAMESPACE_END
