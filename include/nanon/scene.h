/*
	L I G H T  M E T R I C A

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

#pragma once
#ifndef __LIB_LIGHTMETRICA_SCENE_H__
#define __LIB_LIGHTMETRICA_SCENE_H__

#include "object.h"
#include "math.types.h"
#include <string>
#include <vector>
#include <functional>
#include <boost/signals2.hpp>

namespace pugi
{
	class xml_node;
};

LM_NAMESPACE_BEGIN

class Assets;
class NanonConfig;
class Camera;
struct Primitive;
struct Ray;
struct Intersection;

/*!
	Scene class.
	A base class of the scene.
*/
class LM_PUBLIC_API Scene : public Object
{
public:

	Scene();
	virtual ~Scene();

private:

	LM_DISABLE_COPY_AND_MOVE(Scene);

public:

	/*!
		Load scene from XML element.
		Parse the element #node and load the scene.
		Any reference to the assets are resolved with #assets.
		The function is not reentrant. If the function fails, the state of #assets may be in the unstable state.
		\param node A XML element which consists of \a scene element.
		\retval true Succeeded to load the scene.
		\retval false Failed to load the scene.
	*/
	bool Load(const pugi::xml_node& node, Assets& assets);

	/*!
		Load the asset from the configuration.
		Get the \a scene element from the configuration and load the assets.
		The function is not reentrant.
		\param config Configuration.
		\retval true Succeeded to load the scene.
		\retval false Failed to load the scene.
	*/
	bool Load(const NanonConfig& config, Assets& assets);

	/*!
		Configure the scene.
		\param node XML node for the configuration.
		\retval true Succeeded to configure the scene.
		\retval false Failed to configure the scene.
	*/
	virtual bool Configure(const pugi::xml_node& node) = 0;

	/*!
		Configure the scene.
		\param config Configuration.
		\retval true Succeeded to configure the scene.
		\retval false Failed to configure the scene.
	*/
	bool Configure(const NanonConfig& config);

	/*!
		Build acceleration structure.
		Some scene may have an acceleration structure for the optimization.
		The function must be called after #Load.
		The function must be called before any intersection queries.
		\retval true Succeeded to build.
		\retval false Failed to build.
	*/
	virtual bool Build() = 0;

	/*!
		Intersection query.
		The function checks if #ray hits with the scene.
		When intersected, information on the hit point is stored in the intersection data.
		\param ray Ray.
		\param isect Intersection data.
		\retval true Intersected with the scene.
		\retval false Not intersected with the scene.
	*/
	virtual bool Intersect(Ray& ray, Intersection& isect) const = 0;

	/*!
		Reset the scene.
		Get the scene back to the initial state.
	*/
	void Reset();

	/*!
		Get the number of primitives.
		\return Number of primitives.
	*/
	int NumPrimitives() const;

	/*!
		Get a primitive by index.
		\param index Index of a primitive.
		\return Primitive.
	*/
	const Primitive* PrimitiveByIndex(int index) const;

	/*!
		Get a primitive by ID.
		Note that ID for a primitive is optional.
		\param id ID of a primitive.
		\return Primitive.
	*/
	const Primitive* PrimitiveByID(const std::string& id) const;

	/*!
		Get a main camera.
		\return Main camera.
	*/
	const Camera* MainCamera() const;

	/*!
		Get the scene type.
		\return Scene type.
	*/
	virtual std::string Type() const = 0;
	
public:

	/*!
		Connect to ReportBuildProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportBuildProgress(const std::function<void (double, bool)>& func) = 0;

public:

	/*!
		Load primitives.
		This function is used internally for testing.
		\param primitives List of primitives.
		\retval true Succeeded to load the scene.
		\retval false Failed to load the scene.
	*/
	bool LoadPrimitives(const std::vector<Primitive*>& primitives);

protected:

	/*!
		Implementation specific reset function.
		Get the scene back to the initial state.
	*/
	virtual void ResetScene() = 0;

	/*!
		Store intersection data using barycentric coordinates.
		Reconstruct intersection information from some information.
		The function is used internally.
		\param primitiveIndex A index of the primitive.
		\param triangleIndex A index of the triangle of the primitive specified by #primitiveIndex.
		\param ray Intersected ray.
		\param b Barycentric coordinates of the intersection point.
		\param isect Intersection structure to store data.
	*/
	void StoreIntersectionFromBarycentricCoords(
		unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray,
		const Math::Vec2& b, Intersection& isect);

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_SCENE_H__