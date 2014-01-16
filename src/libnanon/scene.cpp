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
#include <nanon/ray.h>
#include <nanon/intersection.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class Scene::Impl : public Object
{
public:

	Impl(Scene* self);
	~Impl();

public:
	
	void Reset();
	bool Load(const pugi::xml_node& node, Assets& assets);
	bool LoadPrimitives(const std::vector<Primitive*>& primitives);
	int NumPrimitives() const;
	const Primitive* PrimitiveByIndex(int index) const;
	const Primitive* PrimitiveByID(const std::string& id) const;
	const Camera* MainCamera() const { return mainCamera; }
	void StoreIntersectionFromBarycentricCoords(unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray, const Math::Vec2& b, Intersection& isect);

private:

	// Traverse the scene and create primitives.
	bool Traverse(const pugi::xml_node& node, Assets& assets, const Math::Mat4& parentWorldTransform);

	// Create transformation from the element 'transform'
	Math::Mat4 ParseTransform(const pugi::xml_node& transformNode);

private:

	Scene* self;
	bool loaded;

	// Scene components
	Camera* mainCamera;
	std::vector<Light*> lights;
	std::vector<std::unique_ptr<Primitive>> primitives;
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
		return false;
	}

	// Register the primitive to light or camera
	// Note that the registration step must be called after creating triangle mesh,
	// for some implementation of lights or cameras could need its reference.

	// Camera
	if (!mainCamera)
	{
		NANON_LOG_WARN("Missing 'camera' in the scene");
	}
	else
	{
		for (auto& primitive : primitives)
		{
			if (primitive->camera == mainCamera)
			{
				mainCamera->RegisterPrimitive(primitive.get());
				break;
			}
		}
	}
	
	// Light
	std::vector<Primitive*> referencedPrimitives;
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
	if (referencedPrimitives.empty())
	{
		NANON_LOG_WARN("Missing 'light' in the scene");
	}

	NANON_LOG_INFO("Successfully loaded " + std::to_string(primitives.size()) + " primitives");
	
	loaded = true;
	return true;
}

bool Scene::Impl::LoadPrimitives( const std::vector<Primitive*>& primitives )
{
	if (loaded)
	{
		NANON_LOG_ERROR("Already loaded");
		return false;
	}

	for (auto* p : primitives)
	{
		this->primitives.push_back(std::unique_ptr<Primitive>(p));
	}

	loaded = true;
	return true;
}

bool Scene::Impl::Traverse( const pugi::xml_node& node, Assets& assets, const Math::Mat4& parentWorldTransform )
{
	//
	// Process transform
	//

	// Local transform
	Math::Mat4 localTransform;
	auto transformNode = node.child("transform");
	if (transformNode)
	{
		// Create transform from the node
		localTransform = ParseTransform(transformNode);
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
		transform = ParseTransform(globalTransformNode);
	}
	else
	{
		// Apply local transform
		transform = parentWorldTransform * localTransform;
	}

	// --------------------------------------------------------------------------------

	//
	// Create primitive
	//

	// Transform must be specified beforehand
	std::unique_ptr<Primitive> primitive(new Primitive(transform));

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
		primitive->light = dynamic_cast<Light*>(assets.ResolveReferenceToAsset(lightNode, "light"));
		if (!primitive->light)
		{
			return false;
		}

		// Register the light to the scene
		lights.push_back(primitive->light);
	}	

	// Process camera
	// If the camera is already found, ignore the seconds one
	if (cameraNode && !mainCamera)
	{
		// Resolve the reference to the camera
		primitive->camera = dynamic_cast<Camera*>(assets.ResolveReferenceToAsset(cameraNode, "camera"));
		if (!primitive->camera)
		{
			return false;
		}

		// Register the camera to the scene
		mainCamera = primitive->camera;
	}

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
		primitive->mesh = dynamic_cast<TriangleMesh*>(assets.ResolveReferenceToAsset(triangleMeshNode, "triangle_mesh"));
		if (!primitive->mesh)
		{
			return false;
		}

		// Resolve the reference to the bsdf
		primitive->bsdf = dynamic_cast<BSDF*>(assets.ResolveReferenceToAsset(bsdfNode, "bsdf"));
		if (!primitive->bsdf)
		{
			return false;
		}
	}

	if (primitive->camera || primitive->light || primitive->mesh)
	{
		auto idAttr = node.attribute("id");
		std::string id = idAttr.as_string();

		NANON_LOG_INFO("Creating primitive (id : '" + id + "')");
		{
			NANON_LOG_INDENTER();

			if (primitive->camera)
				NANON_LOG_INFO("Reference to camera '" + primitive->camera->ID() + "'");
			if (primitive->light)
				NANON_LOG_INFO("Reference to light '" + primitive->light->ID() + "'");
			if (primitive->mesh)
				NANON_LOG_INFO("Reference to triangle mesh '" + primitive->mesh->ID() + "'");
			if (primitive->bsdf)
				NANON_LOG_INFO("Reference to BSDF '" + primitive->bsdf->ID() + "'");

			// Register the primitive to the scene
			primitives.push_back(std::move(primitive));

			// Optionally, register ID for the primitive, if 'id' attribute exists
			if (idAttr)
			{
				// Check if already exists
				if (idPrimitiveIndexMap.find(id) != idPrimitiveIndexMap.end())
				{
					NANON_LOG_ERROR(boost::str(boost::format("ID '%s' for the node is already used") % id));
					NANON_LOG_ERROR(PugiHelper::StartElementInString(node));
					return false;
				}

				idPrimitiveIndexMap[id] = primitives.size() - 1;
			}
		}
	}

	// --------------------------------------------------------------------------------

	//
	// Process children
	//

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
			return false;
		}
	}

	return true;
}

Math::Mat4 Scene::Impl::ParseTransform( const pugi::xml_node& transformNode )
{
	// Default transform
	auto transform = Math::Mat4::Identity();

	// The element 'matrix' specifies column major, 4x4 matrix
	auto matrixNode = transformNode.child("matrix");
	if (matrixNode)
	{
		transform = PugiHelper::ParseMat4(matrixNode);
	}
	else
	{
		// The element 'lookat' offers useful transform especially for the camera node
		auto lookAtNode = transformNode.child("lookat");
		if (lookAtNode)
		{
			// Position, center, up
			auto position = PugiHelper::ParseVec3(lookAtNode.child("position"));
			auto center = PugiHelper::ParseVec3(lookAtNode.child("center"));
			auto up = PugiHelper::ParseVec3(lookAtNode.child("up"));

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
				translateMat = Math::Translate(PugiHelper::ParseVec3(translateNode));
			}

			// 'rotate' node
			auto rotateNode = transformNode.child("rotate");
			if (rotateNode)
			{
				// 'matrix' node
				auto matrixNode = rotateNode.child("matrix");
				if (matrixNode)
				{
					rotateMat = PugiHelper::ParseMat4(matrixNode);
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
						auto axis = PugiHelper::ParseVec3(rotateNode.child("axis"));
						rotateMat = Math::Rotate(angle, axis);
					}
				}
			}

			// 'scale' node
			auto scaleNode = transformNode.child("scale");
			if (scaleNode)
			{
				scaleMat = Math::Scale(PugiHelper::ParseVec3(scaleNode));
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

void Scene::Impl::StoreIntersectionFromBarycentricCoords( unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray, const Math::Vec2& b, Intersection& isect )
{
	// Primitive
	isect.primitiveIndex = primitiveIndex;
	isect.triangleIndex = triangleIndex;
	isect.primitive = self->PrimitiveByIndex(primitiveIndex);

	const auto* mesh = isect.primitive->mesh;
	const auto* positions = mesh->Positions();
	const auto* normals = mesh->Normals();
	const auto* texcoords = mesh->TexCoords();
	const auto* faces = mesh->Faces();

	// Intersection point
	isect.p = ray.o + ray.d * ray.maxT;

	// Geometry normal
	int v1 = faces[3*triangleIndex  ];
	int v2 = faces[3*triangleIndex+1];
	int v3 = faces[3*triangleIndex+2];
	Math::Vec3 p1(isect.primitive->transform * Math::Vec4(positions[3*v1], positions[3*v1+1], positions[3*v1+2], Math::Float(1)));
	Math::Vec3 p2(isect.primitive->transform * Math::Vec4(positions[3*v2], positions[3*v2+1], positions[3*v2+2], Math::Float(1)));
	Math::Vec3 p3(isect.primitive->transform * Math::Vec4(positions[3*v3], positions[3*v3+1], positions[3*v3+2], Math::Float(1)));
	isect.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));

	// Shading normal
	Math::Mat3 normalTransform(Math::Transpose(Math::Inverse(isect.primitive->transform)));
	Math::Vec3 n1 = normalTransform * Math::Vec3(normals[3*v1], normals[3*v1+1], normals[3*v1+2]);
	Math::Vec3 n2 = normalTransform * Math::Vec3(normals[3*v2], normals[3*v2+1], normals[3*v2+2]);
	Math::Vec3 n3 = normalTransform * Math::Vec3(normals[3*v3], normals[3*v3+1], normals[3*v3+2]);
	isect.sn = Math::Normalize(n1 * Math::Float(Math::Float(1) - b[0] - b[1]) + n2 * b[0] + n3 * b[1]);

	// Texture coordinates
	if (texcoords)
	{
		auto& uv1 = Math::Vec2(texcoords[2*v1], texcoords[2*v1+1]);
		auto& uv2 = Math::Vec2(texcoords[2*v2], texcoords[2*v2+1]);
		auto& uv3 = Math::Vec2(texcoords[2*v3], texcoords[2*v3+1]);
		isect.uv = uv1 * Math::Float(Math::Float(1) - b[0] - b[1]) + uv2 * b[0] + uv3 * b[1];
	}

	// Tangent vectors
	Math::OrthonormalBasis(isect.sn, isect.ss, isect.st);

	// Shading coordinates conversion
	isect.worldToShading = Math::Transpose(Math::Mat3(isect.ss, isect.st, isect.sn));
	isect.shadingToWorld = Math::Inverse(isect.worldToShading);
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
	// Note that virtual function call in ctor is prohibited
	// and ResetScene function is called here instead of in Reset.
	p->Reset();
	ResetScene();
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

bool Scene::LoadPrimitives( const std::vector<Primitive*>& primitives )
{
	return p->LoadPrimitives(primitives);
}

const Camera* Scene::MainCamera() const
{
	return p->MainCamera();
}

void Scene::StoreIntersectionFromBarycentricCoords( unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray, const Math::Vec2& b, Intersection& isect )
{
	p->StoreIntersectionFromBarycentricCoords(primitiveIndex, triangleIndex, ray, b, isect);
}

bool Scene::Configure( const NanonConfig& config )
{
	return Configure(config.SceneElement());
}

NANON_NAMESPACE_END