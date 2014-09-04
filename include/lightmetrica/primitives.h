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
#ifndef LIB_LIGHTMETRICA_PRIMITIVES_H
#define LIB_LIGHTMETRICA_PRIMITIVES_H

#include "component.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

class Assets;
class ConfigNode;
struct Primitive;
class Camera;
class Light;
class Scene;

/*!
	Primitives.
	A set of primitives for scene description.
*/
class Primitives : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("primitives");

public:

	Primitives() {}
	virtual ~Primitives() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Primitives);

public:

	/*!
		Load primitives from XML element.
		Parse the element #node and load the scene.
		Any reference to the assets are resolved with #assets.
		The function is not reentrant. If the function fails, the state of #assets may be in the unstable state.
		\param node A XML element which consists of \a scene element.
		\retval true Succeeded to load the scene.
		\retval false Failed to load the scene.
	*/
	virtual bool Load(const ConfigNode& node, const Assets& assets) = 0;

	/*!
		Post configuration of the primitive.
		\param scene Scene.
		\retval true Succeeded to configure the scene.
		\retval false Failed to configure the scene.
	*/
	virtual bool PostConfigure(const Scene& scene) = 0;

	/*!
		Intersection query with emitter shapes.
		When intersected, information on the hit point is stored in the intersection data.
		\param ray Ray.
		\param isect Intersection data.
		\retval true Intersected with the scene.
		\retval false Not intersected with the scene.
	*/
	virtual bool IntersectEmitterShapes(Ray& ray, Intersection& isect) const = 0;

	/*!
		Reset the scene.
		Get the scene back to the initial state.
	*/
	virtual void Reset() = 0;

	/*!
		Get the number of primitives.
		\return Number of primitives.
	*/
	virtual int NumPrimitives() const = 0;

	/*!
		Get a primitive by index.
		\param index Index of a primitive.
		\return Primitive.
	*/
	virtual const Primitive* PrimitiveByIndex(int index) const = 0;

	/*!
		Get a primitive by ID.
		Note that ID for a primitive is optional.
		\param id ID of a primitive.
		\return Primitive.
	*/
	virtual const Primitive* PrimitiveByID(const std::string& id) const = 0;

	/*!
		Get a main camera.
		\return Main camera.
	*/
	virtual const Camera* MainCamera() const = 0;

	/*!
		Get the number of lights.
		\return Number of lights.
	*/
	virtual int NumLights() const = 0;

	/*!
		Get a light by index.
		\param index Index of a light.
		\return Light.
	*/
	virtual const Light* LightByIndex(int index) const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PRIMITIVES_H