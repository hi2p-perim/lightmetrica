/*
	Lightmetrica : A research-oriented renderer

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
#include <lightmetrica/scene.h>
#include <lightmetrica/config.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/asset.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/math.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

class Scene::Impl : public Object
{
public:

	Impl(Scene* self);
	~Impl();

public:
	
	void Reset();
	bool Load(const ConfigNode& node, Assets& assets);
	bool LoadPrimitives(const std::vector<Primitive*>& primitives);
	int NumPrimitives() const { return static_cast<int>(primitives.size()); }
	const Primitive* PrimitiveByIndex(int index) const;
	const Primitive* PrimitiveByID(const std::string& id) const;
	const Camera* MainCamera() const { return mainCamera; }
	void StoreIntersectionFromBarycentricCoords(unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray, const Math::Vec2& b, Intersection& isect);
	int NumLights() const { return lights.size(); }
	const Light* LightByIndex(int index) const;

private:

	// Traverse the scene and create primitives.
	bool Traverse(const ConfigNode& node, Assets& assets, const Math::Mat4& parentWorldTransform);

	// Create transformation from the element 'transform'
	Math::Mat4 ParseTransform(const ConfigNode& transformNode);

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

bool Scene::Impl::Load( const ConfigNode& node, Assets& assets )
{
	if (loaded)
	{
		LM_LOG_ERROR("Already loaded");
		return false;
	}

	// Check the element name
	if (node.Name() != "scene")
	{
		LM_LOG_ERROR("Invalid element name '" + node.Name() + "' (expected 'scene')");
		return false;
	}

	// Check the scene type
	if (node.AttributeValue("type") != self->Type())
	{
		LM_LOG_ERROR("Invalid scene type '" + node.AttributeValue("type") + "' (expected '" + self->Type() + "')");
		return false;
	}

	// Traverse 'root' element
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

	// Register the primitive to light or camera
	// Note that the registration step must be called after creating triangle mesh,
	// for some implementation of lights or cameras could need its reference.

	// Camera
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
		LM_LOG_WARN("Missing 'light' in the scene");
	}

	LM_LOG_INFO("Successfully loaded " + std::to_string(primitives.size()) + " primitives");
	
	loaded = true;
	return true;
}

bool Scene::Impl::LoadPrimitives( const std::vector<Primitive*>& primitives )
{
	if (loaded)
	{
		LM_LOG_ERROR("Already loaded");
		return false;
	}

	for (auto* p : primitives)
	{
		this->primitives.push_back(std::unique_ptr<Primitive>(p));
	}

	loaded = true;
	return true;
}

bool Scene::Impl::Traverse( const ConfigNode& node, Assets& assets, const Math::Mat4& parentWorldTransform )
{
	//
	// Process transform
	//

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

	//
	// Create primitive
	//

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

	// Process light
	if (!lightNode.Empty())
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
	if (!cameraNode.Empty() && !mainCamera)
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

	//
	// Process children
	//

	// Leaf node cannot have children
	bool isLeaf = !lightNode.Empty() || !cameraNode.Empty() || !triangleMeshNode.Empty();
	if (!node.Child("node").Empty() && isLeaf)
	{
		LM_LOG_ERROR("Leaf node cannot have children");
		//LM_LOG_ERROR(PugiHelper::ElementInString(node));
		return false;
	}

	// Process children
	for (auto child = node.Child("node"); !child.Empty(); child = child.NextChild("node"))
	{
		if (!Traverse(child, assets, transform))
		{
			return false;
		}
	}

	return true;
}

Math::Mat4 Scene::Impl::ParseTransform( const ConfigNode& transformNode )
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

const Light* Scene::Impl::LightByIndex( int index ) const
{
	return index < lights.size() ? lights[index] : nullptr;
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
	isect.geom.p = ray.o + ray.d * ray.maxT;

	// Geometry normal
	int v1 = faces[3*triangleIndex  ];
	int v2 = faces[3*triangleIndex+1];
	int v3 = faces[3*triangleIndex+2];
	Math::Vec3 p1(isect.primitive->transform * Math::Vec4(positions[3*v1], positions[3*v1+1], positions[3*v1+2], Math::Float(1)));
	Math::Vec3 p2(isect.primitive->transform * Math::Vec4(positions[3*v2], positions[3*v2+1], positions[3*v2+2], Math::Float(1)));
	Math::Vec3 p3(isect.primitive->transform * Math::Vec4(positions[3*v3], positions[3*v3+1], positions[3*v3+2], Math::Float(1)));
	isect.geom.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));

	// Shading normal
	Math::Mat3 normalTransform(Math::Transpose(Math::Inverse(isect.primitive->transform)));
	Math::Vec3 n1 = normalTransform * Math::Vec3(normals[3*v1], normals[3*v1+1], normals[3*v1+2]);
	Math::Vec3 n2 = normalTransform * Math::Vec3(normals[3*v2], normals[3*v2+1], normals[3*v2+2]);
	Math::Vec3 n3 = normalTransform * Math::Vec3(normals[3*v3], normals[3*v3+1], normals[3*v3+2]);
	isect.geom.sn = Math::Normalize(n1 * (Math::Float(1) - b[0] - b[1]) + n2 * b[0] + n3 * b[1]);

	// Texture coordinates
	if (texcoords)
	{
		Math::Vec2 uv1(texcoords[2*v1], texcoords[2*v1+1]);
		Math::Vec2 uv2(texcoords[2*v2], texcoords[2*v2+1]);
		Math::Vec2 uv3(texcoords[2*v3], texcoords[2*v3+1]);
		isect.geom.uv = uv1 * Math::Float(Math::Float(1) - b[0] - b[1]) + uv2 * b[0] + uv3 * b[1];
	}

	// Scene surface is not degenerated
	isect.geom.degenerated = false;

	// Compute tangent space
	isect.geom.ComputeTangentSpace();
}

// --------------------------------------------------------------------------------

Scene::Scene()
	: p(new Impl(this))
{

}

Scene::~Scene()
{
	LM_SAFE_DELETE(p);
}

bool Scene::Load( const ConfigNode& node, Assets& assets )
{
	return p->Load(node, assets);
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

int Scene::NumLights() const
{
	return p->NumLights();
}

const Light* Scene::LightByIndex( int index ) const
{
	return p->LightByIndex(index);
}

const Light* Scene::SampleLightSelection( Math::Vec2& lightSampleP, Math::PDFEval& selectionPdf ) const
{
	int nl = NumLights();
	int li = Math::Min(static_cast<int>(lightSampleP.x * nl), nl - 1);
	lightSampleP.x = lightSampleP.x * nl - Math::Float(li);
	selectionPdf = Math::PDFEval(Math::Float(1.0 / nl), Math::ProbabilityMeasure::Discrete);
	return LightByIndex(li);
}

LM_NAMESPACE_END
