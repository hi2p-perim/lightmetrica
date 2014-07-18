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
#ifndef LIB_LIGHTMETRICA_SCENE_H
#define LIB_LIGHTMETRICA_SCENE_H

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
class ConfigNode;
class Camera;
class Light;
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
	bool Load(const ConfigNode& node, const Assets& assets);

	/*!
		Configure the scene.
		\param node XML node for the configuration.
		\retval true Succeeded to configure the scene.
		\retval false Failed to configure the scene.
	*/
	virtual bool Configure(const ConfigNode& node) = 0;

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

	/*!
		Get the number of lights.
		\return Number of lights.
	*/
	int NumLights() const;

	/*!
		Get a light by index.
		\param index Index of a light.
		\return Light.
	*/
	const Light* LightByIndex(int index) const;

	/*!
		Choose a light included in the scene.
		Note that only the x component of #lightSampleP is used
		and reusable in the following procedure, e.g. positional sampling on the light.
		\param lightSampleP Light sample.
		\param selectionPdf PDF evaluation of the selection (discrete measure).
		\return Selected light.
	*/
	const Light* SampleLightSelection(Math::Vec2& lightSampleP, Math::PDFEval& selectionPdf) const;
	const Light* SampleLightSelection(const Math::Float& lightSample, Math::PDFEval& selectionPdf) const;

	/*!
		PDF evaluation for light selection sampling.
		\return Evaluated PDF.
	*/
	Math::PDFEval LightSelectionPdf() const;

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

#endif // LIB_LIGHTMETRICA_SCENE_H