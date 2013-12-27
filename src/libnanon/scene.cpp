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

	// Create transformation from the element 'transform'
	Math::Mat4 CreateTransform(const pugi::xml_node& transformNode);
	Math::Vec3 ParseVec3(const pugi::xml_node& node);
	Math::Mat4 ParseMat4(const pugi::xml_node& node);

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
		localTransform = CreateTransform(transformNode);
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
		transform = CreateTransform(globalTransformNode);
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
		light = dynamic_cast<Light*>(assets.ResolveReferenceToAsset(lightNode, "light"));
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
		camera = dynamic_cast<Camera*>(assets.ResolveReferenceToAsset(cameraNode, "camera"));
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
		auto* triangleMesh = dynamic_cast<TriangleMesh*>(assets.ResolveReferenceToAsset(triangleMeshNode, "triangle_mesh"));
		if (!triangleMesh)
		{
			NANON_LOG_DEBUG_EMPTY();
			return false;
		}

		// Resolve the reference to the bsdf
		auto* bsdf = dynamic_cast<BSDF*>(assets.ResolveReferenceToAsset(bsdfNode, "bsdf"));
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
			camera->RegisterPrimitive(primitive.get());
		}
		else if (light)
		{
			// Register the primitive to the light
			primitive = std::make_shared<Primitive>(transform, triangleMesh, bsdf, light);
			light->RegisterPrimitive(primitive.get());
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

Math::Mat4 Scene::Impl::CreateTransform( const pugi::xml_node& transformNode )
{
	// Default transform
	auto transform = Math::Mat4::Identity();

	// The element 'matrix' specifies column major, 4x4 matrix
	auto matrixNode = transformNode.child("matrix");
	if (matrixNode)
	{
		transform = ParseMat4(matrixNode);
	}
	else
	{
		// The element 'lookat' offers useful transform especially for the camera node
		auto lookAtNode = transformNode.child("lookat");
		if (lookAtNode)
		{
			// Position, center, up
			auto position = ParseVec3(lookAtNode.child("position"));
			auto center = ParseVec3(lookAtNode.child("center"));
			auto up = ParseVec3(lookAtNode.child("up"));

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
			auto translateNode = transformNode.child("translate");
			if (translateNode)
			{
				translateMat = Math::Translate(ParseVec3(translateNode));
			}

			// 'rotate' node
			auto rotateNode = transformNode.child("rotate");
			if (rotateNode)
			{
				// 'matrix' node
				auto matrixNode = rotateNode.child("matrix");
				if (matrixNode)
				{
					rotateMat = ParseMat4(matrixNode);
				}
				else
				{
					// 'quat' node
					auto quatNode = rotateNode.child("quat");
					if (quatNode)
					{
						throw std::exception("not implemented");
						//translateMat = ;
					}
					else
					{
						// 'angle' and 'axis'
						auto angle = Math::Float(std::stod(rotateNode.child("angle").child_value()));
						auto axis = ParseVec3(rotateNode.child("axis"));
						rotateMat = Math::Rotate(angle, axis);
					}
				}
			}

			// 'scale' node
			auto scaleNode = transformNode.child("scale");
			if (scaleNode)
			{
				scaleMat = Math::Scale(ParseVec3(scaleNode));
			}

			// Create combined transform
			transform = translateMat * rotateMat * scaleMat;
		}
	}

	return transform;
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

Math::Vec3 Scene::Impl::ParseVec3( const pugi::xml_node& node )
{
	// Parse vector elements (in double)
	std::vector<double> v;
	std::stringstream ss(node.child_value());

	double t;
	while (ss >> t) v.push_back(t);
	if (v.size() != 3)
	{
		NANON_LOG_WARN("Invalid number of elements in '" + std::string(node.name()) + "'");
		return Math::Vec3();
	}

	// Convert type and return
	return Math::Vec3(Math::Float(v[0]), Math::Float(v[1]), Math::Float(v[2]));
}

Math::Mat4 Scene::Impl::ParseMat4( const pugi::xml_node& node )
{
	// Parse matrix elements (in double)
	std::vector<double> m;
	std::stringstream ss(node.child_value());

	double t;
	while (ss >> t) m.push_back(t);
	if (m.size() != 16)
	{
		NANON_LOG_WARN("Invalid number of elements in '" + std::string(node.name()) + "'");
		return Math::Mat4::Identity();
	}

	// Convert to Float and create matrix
	std::vector<Math::Float> m2(16);
	std::transform(m.begin(), m.end(), m2.begin(), [](double v){ return Math::Float(v); });
	return Math::Mat4(&m2[0]);
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