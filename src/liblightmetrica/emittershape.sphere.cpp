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

LM_NAMESPACE_BEGIN

/*!
	Sphere for emitter shape.
	Spheres associated with environment light emitter or directional light emitter.
*/
class SphereEmitterShape : public EmitterShape
{
public:

	LM_COMPONENT_IMPL_DEF("sphere");

public:

	virtual bool Configure( std::map<std::string, boost::any>& params );
	virtual bool Intersect( Ray& ray, Math::Float& t ) const;
	virtual void StoreIntersection( const Ray& ray, const Math::Float& minT, Intersection& isect ) const;
	virtual AABB GetAABB() const;

private:

	Math::Vec3 center;
	Math::Float radius;

};

bool SphereEmitterShape::Configure( std::map<std::string, boost::any>& params )
{

}

bool SphereEmitterShape::Intersect( Ray& ray, Math::Float& t ) const
{
	
}

void SphereEmitterShape::StoreIntersection( const Ray& ray, const Math::Float& minT, Intersection& isect ) const
{
	
}

lightmetrica::AABB SphereEmitterShape::GetAABB() const
{

}

LM_COMPONENT_REGISTER_IMPL(SphereEmitterShape, EmitterShape);

LM_NAMESPACE_END