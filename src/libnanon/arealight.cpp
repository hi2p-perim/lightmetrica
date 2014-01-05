/*
	nanon : A research-oriented renderer

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
#include <nanon/arealight.h>
#include <nanon/intersection.h>
#include <nanon/primitive.h>
#include <nanon/trianglemesh.h>
#include <nanon/logger.h>
#include <nanon/pugihelper.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class AreaLight::Impl : public Object
{
public:

	bool LoadAsset( const pugi::xml_node& node, const Assets& assets );

public:

	Math::Vec3 EvaluateLe( const Math::Vec3& d, const Intersection& isect ) const;
	void RegisterPrimitives(const std::vector<Primitive*>& primitives);
	
private:

	Math::Vec3 Le;
	std::vector<Math::Float> primitiveAreaCdf;
	Math::Float area;
	Math::Vec3 power;

};

bool AreaLight::Impl::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	// 'luminance'
	auto luminanceNode = node.child("luminance");
	if (!luminanceNode)
	{
		NANON_LOG_DEBUG("Missing 'luminance' element");
		return false;
	}
	Le = PugiHelper::ParseVec3(luminanceNode);

	return true;
}

Math::Vec3 AreaLight::Impl::EvaluateLe( const Math::Vec3& d, const Intersection& isect ) const
{
	return Math::Dot(d, isect.gn) < Math::Float(0)
		? Math::Vec3()
		: Le;
}

void AreaLight::Impl::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{
	// Create CDF
	primitiveAreaCdf.clear();
	primitiveAreaCdf.push_back(Math::Float(0));
	for (auto& primitive : primitives)
	{
		// Area of the triangle mesh
		Math::Float area(0);
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
			area += Math::Length(Math::Cross(p2 - p1, p3 - p1)) / Math::Float(2);
		}
		primitiveAreaCdf.push_back(primitiveAreaCdf.back() + area);
	}

	// Normalize
	area = primitiveAreaCdf.back();
	for (auto& v : primitiveAreaCdf)
	{
		v /= area;
	}

	power = Le * Math::Constants::Pi * area;
}

// --------------------------------------------------------------------------------

AreaLight::AreaLight(const std::string& id)
	: Light(id)
	, p(new Impl)
{

}

AreaLight::~AreaLight()
{
	NANON_SAFE_DELETE(p);
}

Math::Vec3 AreaLight::EvaluateLe( const Math::Vec3& d, const Intersection& isect ) const
{
	return p->EvaluateLe(d, isect);
}

void AreaLight::RegisterPrimitives(const std::vector<Primitive*>& primitives)
{
	p->RegisterPrimitives(primitives);
}

bool AreaLight::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

NANON_NAMESPACE_END