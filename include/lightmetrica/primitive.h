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

#pragma once
#ifndef LIB_LIGHTMETRICA_PRIMITIVE_H
#define LIB_LIGHTMETRICA_PRIMITIVE_H

#include "object.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

class TriangleMesh;
class BSDF;
class Camera;
class Light;

/*!
	Primitive.
	Primitive is an element of the scene used for managing transformable objects.
	A primitive corresponds to a node in the scene.
*/
struct Primitive : public Object
{

	Primitive(const Math::Mat4& transform)
		: transform(transform)
		, normalTransform(Math::Transpose(Math::Inverse(transform)))
		, mesh(nullptr)
		, bsdf(nullptr)
		, camera(nullptr)
		, light(nullptr)
	{

	}

	Math::Mat4 transform;
	Math::Mat3 normalTransform;
	TriangleMesh* mesh;
	BSDF* bsdf;
	Camera* camera;
	Light* light;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PRIMITIVE_H