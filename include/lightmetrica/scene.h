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

#include "component.h"
#include "math.types.h"
#include <string>
#include <functional>
#include <memory>
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

class Assets;
class ConfigNode;
class Camera;
class Light;
class Primitives;
struct Primitive;
struct Ray;
struct Intersection;

/*!
	Scene class.
	A base class of the scene.
*/
class Scene : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("scene");

public:

	LM_PUBLIC_API Scene();
	LM_PUBLIC_API virtual ~Scene();

private:

	LM_DISABLE_COPY_AND_MOVE(Scene);

public:

	/*!
		Load primitives.
		Ownership of #primitives is delegated to this class.
		\param primitives Primitives.
	*/
	LM_PUBLIC_API void Load(Primitives* primitives);

	/*!
		Get a main camera.
		\return Main camera.
	*/
	LM_PUBLIC_API const Camera* MainCamera() const;

	/*!
		Choose a light included in the scene (reusable version).
		Note that only the x component of #lightSampleP is used
		and reusable in the following procedure, e.g. positional sampling on the light.
		\param lightSampleP Light sample.
		\param selectionPdf PDF evaluation of the selection (discrete measure).
		\return Selected light.
	*/
	LM_PUBLIC_API const Light* SampleLightSelection(Math::Vec2& lightSampleP, Math::PDFEval& selectionPdf) const;

	/*!
		Choose a light included in the scene.
		\param lightSample Light sample.
		\param selectionPdf PDF evaluation of the selection (discrete measure).
		\return Selected light.
	*/
	LM_PUBLIC_API const Light* SampleLightSelection(const Math::Float& lightSample, Math::PDFEval& selectionPdf) const;

	/*!
		PDF evaluation for light selection sampling.
		\return Evaluated PDF.
	*/
	LM_PUBLIC_API Math::PDFEval LightSelectionPdf() const;

public:

	/*!
		Configure the scene.
		\param node XML node for the configuration.
		\retval true Succeeded to configure the scene.
		\retval false Failed to configure the scene.
	*/
	virtual bool Configure( const ConfigNode& node) = 0;

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

public:

	/*!
		Connect to ReportBuildProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportBuildProgress(const std::function<void (double, bool)>& func) = 0;

protected:

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
	void StoreIntersectionFromBarycentricCoords(unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray, const Math::Vec2& b, Intersection& isect) const;

protected:

	std::unique_ptr<Primitives> primitives;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_SCENE_H