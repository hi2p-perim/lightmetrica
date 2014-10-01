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
#include <lightmetrica/emittershape.h>
#include <lightmetrica/emitter.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/light.h>

LM_NAMESPACE_BEGIN

/*!
	Sphere for emitter shape.
	Spheres associated with environment light emitter or directional light emitter.
*/
class SphereEmitterShape final : public EmitterShape
{
public:

	LM_COMPONENT_IMPL_DEF("sphere");

public:

	virtual bool Configure(std::map<std::string, boost::any>& params) override;
	virtual bool Intersect(Ray& ray, Math::Float& t) const override;
	virtual void StoreIntersection(const Ray& ray, Intersection& isect) const override;
	virtual AABB GetAABB() const override;

public:

	bool CheckParam(const std::string& key, std::map<std::string, boost::any>& params) const;

private:

	Math::Vec3 center;
	Math::Float radius;
	const Emitter* emitter;

};

bool SphereEmitterShape::Configure( std::map<std::string, boost::any>& params )
{
	// Check required parameters
	if (!CheckParam("center", params) ||
		!CheckParam("radius", params) ||
		!CheckParam("emitter", params))
	{
		return false;
	}

	try
	{
		// Read parameters
		center = boost::any_cast<Math::Vec3>(params["center"]);
		radius = boost::any_cast<Math::Float>(params["radius"]);
		emitter = boost::any_cast<const Emitter*>(params["emitter"]);
	}
	catch (const boost::bad_any_cast& e)
	{
		LM_LOG_ERROR("Invalid type : " + std::string(e.what()));
		return false;
	}

	return true;
}

bool SphereEmitterShape::CheckParam( const std::string& key, std::map<std::string, boost::any>& params ) const
{
	if (params.find(key) == params.end())
	{
		LM_LOG_ERROR("Missing parameter : '" + key + "'");
		return false;
	}

	return true;
}

bool SphereEmitterShape::Intersect( Ray& ray, Math::Float& t ) const
{
	// # Temporary variables
	auto o = ray.o - center;
	auto d = ray.d;
	auto a = Math::Length2(d);
	auto b = Math::Float(2) * Math::Dot(o, d);
	auto c = Math::Length2(o) - radius * radius;

	// ----------------------------------------------------------------------

	// # Solve quadratic
	auto det = b * b - Math::Float(4) * a * c;
	if (det < Math::Float(0))
	{
		return false;
	}

	auto e = Math::Sqrt(det);
	auto denom = Math::Float(2) * a;
	auto t0 = (-b - e) / denom;
	auto t1 = (-b + e) / denom;
	if (t0 > ray.maxT || t1 < ray.minT)
	{
		return false;
	}

	// ----------------------------------------------------------------------

	// # Check range
	t = t0;
	if (t < ray.minT)
	{
		t = t1;
		if (t > ray.maxT)
		{
			return false;
		}
	}

	return true;
}

void SphereEmitterShape::StoreIntersection( const Ray& ray, Intersection& isect ) const
{
	// Intersection point
	isect.geom.p = ray.o + ray.d * ray.maxT;

	// Geometry & shading normal
	isect.geom.gn = isect.geom.sn = Math::Normalize(isect.geom.p - center);
	isect.geom.ComputeTangentSpace();

	// Surface is not degenerated
	isect.geom.degenerated = false;

	// Emitters
	isect.camera = nullptr;
	isect.light = dynamic_cast<const Light*>(emitter);
}

lightmetrica::AABB SphereEmitterShape::GetAABB() const
{
	AABB aabb;
	aabb.min = center - Math::Vec3(radius);
	aabb.max = center + Math::Vec3(radius);
	return aabb;
}

LM_COMPONENT_REGISTER_IMPL(SphereEmitterShape, EmitterShape);

LM_NAMESPACE_END