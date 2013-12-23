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
#include <nanon/scene.h>
#include <nanon/config.h>
#include <nanon/assets.h>
#include <nanon/asset.h>
#include <nanon/light.h>
#include <nanon/camera.h>
#include <nanon/trianglemesh.h>
#include <nanon/bsdf.h>
#include <nanon/logger.h>
#include <nanon/math.h>
#include <nanon/pugihelper.h>
#include <nanon/primitive.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class Scene::Impl
{
public:

	Impl(Scene* self);
	~Impl();

public:
	
	void Reset();
	bool Load(const pugi::xml_node& node, Assets& assets);
	bool LoadPrimitives(const std::vector<std::shared_ptr<Primitive>>& primitives);
	int NumPrimitives() const;
	const Primitive* PrimitiveByIndex(int index) const;
	const Primitive* PrimitiveByID(const std::string& id) const;
	const Camera* MainCamera() const { return mainCamera; }

private:

	// Traverse the scene and create primitives.
	bool Traverse(const pugi::xml_node& node, Assets& assets, const Math::Mat4& parentWorldTransform);

	// Create transformation from the element 'transform'.
	// The result is stored in the transform.
	bool CreateTransform(const pugi::xml_node& transformNode, Math::Mat4& transform);

	// Resolve reference to an asset using 'ref' attribute.
	// If 'ref' attribute is not found, returns nullptr.
	Asset* ResolveReferenceToAsset(const pugi::xml_node& node, const std::string& name, Assets& assets);

private:

	Scene* self;
	bool loaded;

	// Scene components
	Camera* mainCamera;
	std::vector<Light*> lights;
	std::vector<std::shared_ptr<Primitive>> primitives;
	boost::unordered_map<std::string, size_t> idPrimitiveIndexMap;

};

Scene::Impl::Impl(Scene* self)
	: self(self)
{
	Reset();
}

Scene::Impl::~Impl()
{

}

void Scene::Impl::Reset()
{
	loaded = false;
	mainCamera = nullptr;
	lights.clear();
	primitives.clear();
	idPrimitiveIndexMap.clear();
}

bool Scene::Impl::Load( const pugi::xml_node& node, Assets& assets )
{
	if (loaded)
	{
		NANON_LOG_ERROR("Already loaded");
		return false;
	}

	// Check the element name
	if (std::strcmp(node.name(), "scene") != 0)
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid element name '%s' (expected 'scene')") % node.name()));
		return false;
	}

	// Check the scene type
	if (self->Type() != node.attribute("type").as_string())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid scene type '%s' (expected '%s')") % node.attribute("type").as_string() % self->Type()));
		return false;
	}

	// Traverse 'root' element
	auto rootNode = node.child("root");
	if (!rootNode)
	{
		NANON_LOG_ERROR("Missing 'root' node");
		return false;
	}
	if (!Traverse(rootNode, assets, Math::Mat4::Identity()))
	{
		Reset();
		NANON_LOG_DEBUG_EMPTY();
		return false;
	}

	loaded = true;
	return true;
}

bool Scene::Impl::LoadPrimitives( const std::vector<std::shared_ptr<Primitive>>& primitives )
{
	if (loaded)
	{
		NANON_LOG_ERROR("Already loaded");
		return false;
	}

	this->primitives = primitives;

	loaded = true;
	return true;
}

bool Scene::Impl::Traverse( const pugi::xml_node& node, Assets& assets, const Math::Mat4& parentWorldTransform )
{
	// Process transform

	// Local transform
	Math::Mat4 localTransform;
	auto transformNode = node.child("transform");
	if (transformNode)
	{
		// Create transform from the node
		if (!CreateTransform(transformNode, localTransform))
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}
	}
	else
	{
		// If 'transform' node does not exists, use identity as local transform
		localTransform = Math::Mat4::Identity();
	}

	// World transform
	Math::Mat4 transform;
	auto globalTransformNode = node.child("global_transform");
	if (globalTransformNode)
	{
		// If the 'global_transform' node exists, use it as the transform of the node.
		// Local transform is ignored.
		if (!CreateTransform(globalTransformNode, transform))
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}
	}
	else
	{
		// Apply local transform
		transform = parentWorldTransform * localTransform;
	}

	// --------------------------------------------------------------------------------

	Light* light = nullptr;
	Camera* camera = nullptr;

	auto cameraNode = node.child("camera");
	auto lightNode = node.child("light");

	// Camera and light element cannot be used in the same time
	if (cameraNode && lightNode)
	{
		NANON_LOG_ERROR("'camera' and 'light' elements cannot be used in the same time");
		NANON_LOG_ERROR(PugiHelper::ElementInString(node));
		return false;
	}

	// Process light
	if (lightNode)
	{
		// Resolve the reference to the light
		light = dynamic_cast<Light*>(ResolveReferenceToAsset(lightNode, "light", assets));
		if (!light)
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}

		// Register the light to the scene
		lights.push_back(light);
	}	

	// Process camera
	// If the camera is already found, ignore the seconds one
	if (cameraNode && !mainCamera)
	{
		// Resolve the reference to the camera
		camera = dynamic_cast<Camera*>(ResolveReferenceToAsset(cameraNode, "camera", assets));
		if (!camera)
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}

		// Register the camera to the scene
		mainCamera = camera;
	}

	// --------------------------------------------------------------------------------
	
	// Process triangle mesh
	auto triangleMeshNode = node.child("triangle_mesh");
	if (triangleMeshNode)
	{
		// Triangle mesh must be associated with a bsdf
		auto bsdfNode = node.child("bsdf");
		if (!bsdfNode)
		{
			NANON_LOG_ERROR("Missing 'bsdf' element");
			NANON_LOG_ERROR(PugiHelper::ElementInString(node));
			return false;
		}

		// Resolve the reference to the triangle mesh
		auto* triangleMesh = dynamic_cast<TriangleMesh*>(ResolveReferenceToAsset(triangleMeshNode, "triangle_mesh", assets));
		if (!triangleMesh)
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}

		// Resolve the reference to the bsdf
		auto* bsdf = dynamic_cast<BSDF*>(ResolveReferenceToAsset(bsdfNode, "bsdf", assets));
		if (!bsdf)
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}

		// Create primitive
		std::shared_ptr<Primitive> primitive;
		if (camera)
		{
			// Register the primitive to the camera
			primitive = std::make_shared<Primitive>(transform, triangleMesh, bsdf, camera);
			if (!camera->RegisterPrimitive(primitive.get()))
			{
				NANON_LOG_DEBUG_EMPTY();
				return false;
			}
		}
		else if (light)
		{
			// Register the primitive to the light
			primitive = std::make_shared<Primitive>(transform, triangleMesh, bsdf, light);
			if (!light->RegisterPrimitive(primitive.get()))
			{
				NANON_LOG_DEBUG_EMPTY();
				return false;
			}
		}
		else
		{
			primitive = std::make_shared<Primitive>(transform, triangleMesh, bsdf);
		}
		
		// Register the primitive to the scene
		primitives.push_back(primitive);

		// Optionally, register ID for the primitive, if 'id' attribute exists
		auto idAttr = node.attribute("id");
		if (idAttr)
		{
			// Check if already exists
			std::string id = idAttr.as_string();
			if (idPrimitiveIndexMap.find(id) != idPrimitiveIndexMap.end())
			{
				NANON_LOG_ERROR(boost::str(boost::format("ID '%s' for the node is already used") % id));
				NANON_LOG_ERROR(PugiHelper::StartElementInString(node));
				return false;
			}

			idPrimitiveIndexMap[id] = primitives.size() - 1;
		}
	}

	// --------------------------------------------------------------------------------

	// Leaf node cannot have children
	bool isLeaf = lightNode || cameraNode || triangleMeshNode;
	if (node.child("node") && isLeaf)
	{
		NANON_LOG_ERROR("Leaf node cannot have children");
		NANON_LOG_ERROR(PugiHelper::ElementInString(node));
		return false;
	}

	// Process children
	for (auto child : node.children("node"))
	{
		if (!Traverse(child, assets, transform))
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}
	}

	return true;
}

bool Scene::Impl::CreateTransform( const pugi::xml_node& transformNode, Math::Mat4& transform )
{
	// Default transform
	transform = Math::Mat4::Identity();

	// The element 'matrix' specifies column major, 4x4 matrix
	auto matrixNode = transformNode.child("matrix");
	if (matrixNode)
	{
		// Parse matrix elements (in double)
		std::vector<double> m;
		std::stringstream ss(matrixNode.child_value());

		double v;
		while (ss >> v) m.push_back(v);

		// Check number of elements
		if (m.size() != 16)
		{
			NANON_LOG_ERROR("Invalid number of elements in 'matrix'");
			return false;
		}

		// Convert to Float and create matrix
		std::vector<Math::Float> m2(16);
		std::transform(m.begin(), m.end(), m2.begin(), [](double v){ return Math::Float(v); });
		transform = Math::Mat4(&m2[0]);
	}
	else
	{
		// If the 'matrix' element is not specified, use 'translate', 'rotate', 'scale' elements
		auto translateNode = transformNode.child("translate");
		auto rotate = transformNode.child("rotate");
		auto scale = transformNode.child("scale");
		
		// 'translate' node 
		if (translateNode)
		{
			throw std::exception("TODO");
		}

		// 'rotate' node
		if (rotate)
		{
			throw std::exception("TODO");
		}

		// 'scale' node
		if (scale)
		{
			throw std::exception("TODO");
		}
	}

	return true;
}

Asset* Scene::Impl::ResolveReferenceToAsset( const pugi::xml_node& node, const std::string& name, Assets& assets )
{
	// The element must have 'ref' attribute
	auto refAttr = node.attribute("ref");
	if (!refAttr)
	{
		NANON_LOG_ERROR(boost::str(boost::format("'%s' element in 'node' must have 'ref' attribute") % name));
		NANON_LOG_ERROR(PugiHelper::StartElementInString(node));
		return nullptr;
	}

	// Find the light specified by 'ref'
	auto* asset = assets.GetAssetByName(refAttr.as_string());
	if (!asset)
	{
		NANON_LOG_ERROR(boost::str(boost::format("The asset referenced by '%s' is not found") % refAttr.as_string()));
		NANON_LOG_ERROR(PugiHelper::StartElementInString(node));
		return nullptr;
	}
	else if (asset->Name() != name)
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid asset name '%s' (expected '%s')") % asset->Name() % name));
		NANON_LOG_ERROR(PugiHelper::StartElementInString(node));
		return nullptr;
	}

	return asset;
}

int Scene::Impl::NumPrimitives() const
{
	return static_cast<int>(primitives.size());
}

const Primitive* Scene::Impl::PrimitiveByIndex( int index ) const
{
	return index < primitives.size() ? primitives[index].get() : nullptr;
}

const Primitive* Scene::Impl::PrimitiveByID( const std::string& id ) const
{
	return idPrimitiveIndexMap.find(id) != idPrimitiveIndexMap.end()
		? primitives[idPrimitiveIndexMap.at(id)].get()
		: nullptr;
}

// --------------------------------------------------------------------------------

Scene::Scene()
	: p(new Impl(this))
{

}

Scene::~Scene()
{
	NANON_SAFE_DELETE(p);
}

bool Scene::Load( const pugi::xml_node& node, Assets& assets )
{
	return p->Load(node, assets);
}

bool Scene::Load( const NanonConfig& config, Assets& assets )
{
	return p->Load(config.SceneElement(), assets);
}

void Scene::Reset()
{
	p->Reset();
}

int Scene::NumPrimitives() const
{
	return p->NumPrimitives();
}

const Primitive* Scene::PrimitiveByIndex( int index ) const
{
	return p->PrimitiveByIndex(index);
}

const Primitive* Scene::PrimitiveByID( const std::string& id ) const
{
	return p->PrimitiveByID(id);
}

bool Scene::LoadPrimitives( const std::vector<std::shared_ptr<Primitive>>& primitives )
{
	return p->LoadPrimitives(primitives);
}

const Camera* Scene::MainCamera() const
{
	return p->MainCamera();
}

NANON_NAMESPACE_END