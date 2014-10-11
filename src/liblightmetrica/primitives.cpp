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
#include <lightmetrica/primitives.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/light.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/math.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/emittershape.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>

LM_NAMESPACE_BEGIN

class PrimitivesImpl final : public Primitives
{
public:

	LM_COMPONENT_IMPL_DEF("default");

public:

	PrimitivesImpl();

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) override;
	virtual bool PostConfigure(const Scene& scene) override;
	virtual bool IntersectEmitterShapes(Ray& ray, Intersection& isect) const override;
	virtual AABB GetAABBEmitterShapes() const override;
	virtual void Reset() override;
	virtual int NumPrimitives() const override										{ return static_cast<int>(primitives.size()); }
	virtual const Primitive* PrimitiveByIndex(int index) const override				{ return index < static_cast<int>(primitives.size()) ? primitives[index].get() : nullptr; }
	virtual const Primitive* PrimitiveByID(const std::string& id) const override	{ return idPrimitiveIndexMap.find(id) != idPrimitiveIndexMap.end() ? primitives[idPrimitiveIndexMap.at(id)].get() : nullptr; }
	virtual const Camera* MainCamera() const override								{ return mainCamera; }
	virtual int NumLights() const override											{ return static_cast<int>(lights.size()); }
	virtual const Light* LightByIndex(int index) const override						{ return index < static_cast<int>(lights.size()) ? lights[index] : nullptr; }

private:

	// Traverse the scene and create primitives.
	bool Traverse(const ConfigNode& node, const Assets& assets, const Math::Mat4& parentWorldTransform);

	// Create transformation from the element 'transform'
	Math::Mat4 ParseTransform(const ConfigNode& transformNode);

private:

	bool loaded;
	Camera* mainCamera;													//!< Main camera.
	Light* environmentLight;											//!< Environment light.
	std::vector<Light*> lights;											//!< Lights with other types.
	std::vector<std::unique_ptr<Primitive>> primitives;					//!< Primitives
	boost::unordered_map<std::string, size_t> idPrimitiveIndexMap;		//!< Primitive name and index of primitives
	std::vector<std::unique_ptr<EmitterShape>> emitterShapes;			//!< Emitter shapes.

};

PrimitivesImpl::PrimitivesImpl()
{
	Reset();
}

void PrimitivesImpl::Reset()
{
	loaded = false;
	mainCamera = nullptr;
	environmentLight = nullptr;
	lights.clear();
	primitives.clear();
	idPrimitiveIndexMap.clear();
}

bool PrimitivesImpl::Load( const ConfigNode& node, const Assets& assets )
{
	if (loaded)
	{
		LM_LOG_ERROR("Already loaded");
		return false;
	}

	// ----------------------------------------------------------------------

	// # Traverse 'root' element
	auto rootNode = node.Child("root");
	if (rootNode.Empty())
	{
		LM_LOG_ERROR("Missing 'root' node");
		return false;
	}
	if (!Traverse(rootNode, assets, Math::Mat4::Identity()))
	{
		Reset();
		return false;
	}

	// ----------------------------------------------------------------------

	// # Environment light
	// 'environment_light' element
	auto environmentLightNode = node.Child("environment_light");
	if (!environmentLightNode.Empty())
	{
		// Resolve reference to the environment light
		environmentLight = assets.ResolveReferenceToAsset<Light>(environmentLightNode);
		if (!environmentLight)
		{
			Reset();
			return false;
		}
		if (!environmentLight->EnvironmentLight())
		{
			LM_LOG_ERROR("The light referenced by 'environment_light' is not an environment light");
			return false;
		}

		lights.push_back(environmentLight);
	}

	// ----------------------------------------------------------------------

	// # Register the primitive to light or camera
	// Note that the registration step must be called after creating triangle mesh,
	// for some implementation of lights or cameras could need its reference.
	std::vector<Primitive*> referencedPrimitives;

	// ## Camera
	if (!mainCamera)
	{
		LM_LOG_WARN("Missing 'camera' in the scene");
	}
	else
	{
		for (auto& primitive : primitives)
		{
			if (primitive->camera == mainCamera)
			{
				referencedPrimitives.push_back(primitive.get());
				break;
			}
		}
		mainCamera->RegisterPrimitives(referencedPrimitives);
	}

	// ## Light
	referencedPrimitives.clear();
	for (auto* light : lights)
	{
		for (auto& primitive : primitives)
		{
			if (primitive->light == light)
			{
				referencedPrimitives.push_back(primitive.get());
			}
		}
		light->RegisterPrimitives(referencedPrimitives);
	}
	if (environmentLightNode.Empty() && referencedPrimitives.empty())
	{
		LM_LOG_WARN("Missing lights in the scene");
	}

	// ----------------------------------------------------------------------

	LM_LOG_INFO("Successfully loaded " + std::to_string(primitives.size()) + " primitives");
	loaded = true;

	return true;
}

bool PrimitivesImpl::PostConfigure( const Scene& scene )
{
	if (environmentLight != nullptr)
	{
		// Post configure environment light
		environmentLight->PostConfigure(scene);

		// If emitter shape is associated with emitter
		// record it to the special shape list.
		std::unique_ptr<EmitterShape> shape(environmentLight->CreateEmitterShape());
		if (shape != nullptr)
		{
			// Register to list
			emitterShapes.push_back(std::move(shape));
		}
	}

	return true;
}

bool PrimitivesImpl::IntersectEmitterShapes( Ray& ray, Intersection& isect ) const
{
	bool intersected = false;
	size_t minIndex = 0;

	for (size_t i = 0; i < emitterShapes.size(); i++)
	{
		// Intersection query
		Math::Float t;
		if (emitterShapes[i]->Intersect(ray, t))
		{
			ray.maxT = t;
			minIndex = i;
			intersected = true;
		}
	}

	if (intersected)
	{
		// Store additional information into #isect if intersected
		emitterShapes[minIndex]->StoreIntersection(ray, isect);
	}

	return intersected;
}

AABB PrimitivesImpl::GetAABBEmitterShapes() const
{
	AABB aabb;
	for (auto& shape : emitterShapes)
	{
		aabb = aabb.Union(shape->GetAABB());
	}

	return aabb;
}

bool PrimitivesImpl::Traverse( const ConfigNode& node, const Assets& assets, const Math::Mat4& parentWorldTransform )
{
	// # Process transform

	// Local transform
	Math::Mat4 localTransform;
	auto transformNode = node.Child("transform");
	if (transformNode.Empty())
	{
		// If 'transform' node does not exists, use identity as local transform
		localTransform = Math::Mat4::Identity();
	}
	else
	{
		// Create transform from the node
		localTransform = ParseTransform(transformNode);
	}

	// World transform
	Math::Mat4 transform;
	auto globalTransformNode = node.Child("global_transform");
	if (globalTransformNode.Empty())
	{
		// Apply local transform
		transform = parentWorldTransform * localTransform;
	}
	else
	{
		// If the 'global_transform' node exists, use it as the transform of the node.
		// Local transform is ignored.
		transform = ParseTransform(globalTransformNode);
	}

	// --------------------------------------------------------------------------------

	// # Create primitive

	// Transform must be specified beforehand
	std::unique_ptr<Primitive> primitive(new Primitive(transform));

	auto cameraNode = node.Child("camera");
	auto lightNode = node.Child("light");

	// Camera and light element cannot be used in the same time
	if (!cameraNode.Empty() && !lightNode.Empty())
	{
		LM_LOG_ERROR("'camera' and 'light' elements cannot be used in the same time");
		//LM_LOG_ERROR(PugiHelper::ElementInString(node));
		return false;
	}

	// ## Process light
	if (!lightNode.Empty())
	{
		// Resolve the reference to the light
		primitive->light = assets.ResolveReferenceToAsset<Light>(lightNode);
		if (!primitive->light)
		{
			return false;
		}

		// Register the light to the scene
		lights.push_back(primitive->light);
	}

	// ## Process camera
	// If the camera is already found, ignore the seconds one
	if (!cameraNode.Empty() && !mainCamera)
	{
		// Resolve the reference to the camera
		primitive->camera = assets.ResolveReferenceToAsset<Camera>(cameraNode);
		if (!primitive->camera)
		{
			return false;
		}

		// Register the camera to the scene
		mainCamera = primitive->camera;
	}

	// ## Process triangle mesh
	auto triangleMeshNode = node.Child("triangle_mesh");
	if (!triangleMeshNode.Empty())
	{
		// Triangle mesh must be associated with a bsdf
		auto bsdfNode = node.Child("bsdf");
		if (bsdfNode.Empty())
		{
			LM_LOG_ERROR("Missing 'bsdf' element");
			//LM_LOG_ERROR(PugiHelper::ElementInString(node));
			return false;
		}

		// Resolve the reference to the triangle mesh
		primitive->mesh = assets.ResolveReferenceToAsset<TriangleMesh>(triangleMeshNode);
		if (!primitive->mesh)
		{
			return false;
		}

		// Resolve the reference to the bsdf
		primitive->bsdf = assets.ResolveReferenceToAsset<BSDF>(bsdfNode);
		if (!primitive->bsdf)
		{
			return false;
		}
	}

	if (primitive->camera || primitive->light || primitive->mesh)
	{
		auto id = node.AttributeValue("id");

		LM_LOG_INFO("Creating primitive (id : '" + id + "')");
		{
			LM_LOG_INDENTER();

			if (primitive->camera)
				LM_LOG_INFO("Reference to camera '" + primitive->camera->ID() + "'");
			if (primitive->light)
				LM_LOG_INFO("Reference to light '" + primitive->light->ID() + "'");
			if (primitive->mesh)
				LM_LOG_INFO("Reference to triangle mesh '" + primitive->mesh->ID() + "'");
			if (primitive->bsdf)
				LM_LOG_INFO("Reference to BSDF '" + primitive->bsdf->ID() + "'");

			// Register the primitive to the scene
			primitives.push_back(std::move(primitive));

			// Optionally, register ID for the primitive, if 'id' attribute exists
			if (!id.empty())
			{
				// Check if already exists
				if (idPrimitiveIndexMap.find(id) != idPrimitiveIndexMap.end())
				{
					LM_LOG_ERROR(boost::str(boost::format("ID '%s' for the node is already used") % id));
					//LM_LOG_ERROR(PugiHelper::StartElementInString(node));
					return false;
				}

				idPrimitiveIndexMap[id] = primitives.size() - 1;
			}
		}
	}

	// --------------------------------------------------------------------------------

	// # Process children

	// ## Leaf node cannot have children
	bool isLeaf = !lightNode.Empty() || !cameraNode.Empty() || !triangleMeshNode.Empty();
	if (!node.Child("node").Empty() && isLeaf)
	{
		LM_LOG_ERROR("Leaf node cannot have children");
		//LM_LOG_ERROR(PugiHelper::ElementInString(node));
		return false;
	}

	// ## Process children
	for (auto child = node.Child("node"); !child.Empty(); child = child.NextChild("node"))
	{
		if (!Traverse(child, assets, transform))
		{
			return false;
		}
	}

	return true;
}

Math::Mat4 PrimitivesImpl::ParseTransform( const ConfigNode& transformNode )
{
	// Default transform
	auto transform = Math::Mat4::Identity();

	// The element 'matrix' specifies column major, 4x4 matrix
	auto matrixNode = transformNode.Child("matrix");
	if (!matrixNode.Empty())
	{
		transform = matrixNode.Value<Math::Mat4>();
	}
	else
	{
		// The element 'lookat' offers useful transform especially for the camera node
		auto lookAtNode = transformNode.Child("lookat");
		if (!lookAtNode.Empty())
		{
			// Position, center, up
			auto position = lookAtNode.Child("position").Value<Math::Vec3>();
			auto center = lookAtNode.Child("center").Value<Math::Vec3>();
			auto up = lookAtNode.Child("up").Value<Math::Vec3>();

			// Create transform
			transform = Math::LookAt(position, center, up);
		}
		else
		{
			// If the 'matrix' element is not specified, use 'translate', 'rotate', 'scale' elements
			auto translateMat = Math::Mat4::Identity();
			auto rotateMat = Math::Mat4::Identity();
			auto scaleMat = Math::Mat4::Identity();

			// 'translate' node
			auto translateNode = transformNode.Child("translate");
			if (!translateNode.Empty())
			{
				translateMat = Math::Translate(translateNode.Value<Math::Vec3>());
			}

			// 'rotate' node
			auto rotateNode = transformNode.Child("rotate");
			if (!rotateNode.Empty())
			{
				// 'matrix' node
				auto matrixNode = rotateNode.Child("matrix");
				if (!matrixNode.Empty())
				{
					rotateMat = matrixNode.Value<Math::Mat4>();
				}
				else
				{
					// 'quat' node
					auto quatNode = rotateNode.Child("quat");
					if (!quatNode.Empty())
					{
						throw std::runtime_error("not implemented");
						//translateMat = ;
					}
					else
					{
						// 'angle' and 'axis'
						auto angle = rotateNode.Child("angle").Value<Math::Float>();
						auto axis = rotateNode.Child("axis").Value<Math::Vec3>();
						rotateMat = Math::Rotate(angle, axis);
					}
				}
			}

			// 'scale' node
			auto scaleNode = transformNode.Child("scale");
			if (!scaleNode.Empty())
			{
				scaleMat = Math::Scale(scaleNode.Value<Math::Vec3>());
			}

			// Create combined transform
			transform = translateMat * rotateMat * scaleMat;
		}
	}

	return transform;
}

LM_COMPONENT_REGISTER_IMPL(PrimitivesImpl, Primitives);

LM_NAMESPACE_END