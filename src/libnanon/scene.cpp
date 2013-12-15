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
#include <nanon/logger.h>
#include <nanon/math.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class Scene::Impl
{
public:

	Impl(Scene* self);
	~Impl();

public:

	bool Load(const pugi::xml_node& node, Assets& assets);

private:

	bool Traverse(const pugi::xml_node& node, Assets& assets, const Mat4& parentWorldTransform);
	Mat4 CreateTransform(const pugi::xml_node& transformNode);

	// Resolve reference to an asset using 'ref' attribute.
	// If 'ref' attribute is not found, returns nullptr.
	Asset* ResolveReferenceToAsset(const pugi::xml_node& node, Assets& assets);

private:

	Scene* self;
	bool loaded;

};

Scene::Impl::Impl(Scene* self)
	: self(self)
	, loaded(false)
{

}

Scene::Impl::~Impl()
{

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
	if (!Traverse(rootNode, assets, Mat4::Identity()))
	{
		NANON_LOG_DEBUG("");
		return false;
	}

	loaded = true;
	return true;
}

bool Scene::Impl::Traverse( const pugi::xml_node& node, Assets& assets, const Mat4& parentWorldTransform )
{
	// Process transform

	// Local transform
	Mat4 localTransform;
	auto transformNode = node.child("transform");
	if (transformNode)
	{
		// Create transform from the node
		localTransform = CreateTransform(transformNode);
	}
	else
	{
		// If 'transform' node does not exists, use identity as local transform
		localTransform = Mat4::Identity();
	}

	// Transformation of the node
	Mat4 transform;
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

	// ----------------------------------------------------------------------

	// Process light
	Asset* light = nullptr;
	auto lightNode = node.child("light");
	if (lightNode)
	{
		// 'light' element must have 'ref' attribute
		auto refAttr = lightNode.attribute("ref");
		if (!refAttr)
		{
			NANON_LOG_ERROR("'light' element in 'node' must have 'ref' attribute");
			return false;
		}

		// Find the triangle mesh specified by 'ref'
		auto* triangleAsset = assets.GetAssetByName(refAttr.as_string());
		if (!triangleAsset)
		{
			NANON_LOG_ERROR(boost::str(boost::format("Triangle mesh referenced by '%s' is not found") % refAttr.as_string()));
			return false;
		}

	}

	// ----------------------------------------------------------------------

	// Process camera
	Asset* camera = nullptr;
	auto cameraNode = node.child("camera");
	if (cameraNode)
	{

	}

	// ----------------------------------------------------------------------

	// Process triangle mesh
	auto triangleMeshNode = node.child("triangle_mesh");
	if (triangleMeshNode)
	{
		// 'triangle_mesh' element must have 'ref' attribute
		auto refAttr = triangleMeshNode.attribute("ref");
		if (!refAttr)
		{
			NANON_LOG_ERROR("'triangle_mesh' element in 'node' must have 'ref' attribute");
			return false;
		}

		// Find the triangle mesh specified by 'ref'
		auto* triangleAsset = assets.GetAssetByName(refAttr.as_string());
		if (!triangleAsset)
		{
			NANON_LOG_ERROR(boost::str(boost::format("Triangle mesh referenced by '%s' is not found") % refAttr.as_string()));
			return false;
		}

		
	}

	// ----------------------------------------------------------------------

	// Process children
	for (auto child : node.children("node"))
	{
		if (!Traverse(node, assets, transform))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	return true;
}

Mat4 Scene::Impl::CreateTransform( const pugi::xml_node& transformNode )
{
	// TODO
	return Mat4::Identity();
}

// ----------------------------------------------------------------------

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

NANON_NAMESPACE_END