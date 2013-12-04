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
#include <nanon/assets.h>
#include <nanon/logger.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class Assets::Impl
{
public:

	bool Load(const pugi::xml_node& node);

private:

	bool LoadTexture(const pugi::xml_node& textureNode);
	bool LoadMaterial(const pugi::xml_node& materialNode);
	bool LoadTriangleMesh(const pugi::xml_node& triangleMeshNode);
	bool LoadFilm(const pugi::xml_node& filmNode);
	bool LoadCamera(const pugi::xml_node& cameraNode);
	bool LoadLight(const pugi::xml_node& lightNode);

};

bool Assets::Impl::Load( const pugi::xml_node& node )
{
	// Element name must be 'assets'
	if (std::strcmp(node.name(), "assets") != 0)
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid element name : %s (expected : assets)") % node.name()));
		return false;
	}

	// Load textures
	for (auto textureNode : node.child("textures").children())
	{
		if (!LoadTexture(textureNode))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	// Load materials
	// Materials can depend on the textures
	for (auto materialNode : node.child("materials").children())
	{
		if (!LoadMaterial(materialNode))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	// Load triangle meshes
	for (auto triangleMeshNode : node.child("triangle_meshes").children())
	{
		if (!LoadTriangleMesh(triangleMeshNode))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	// Load films
	for (auto filmNode : node.child("films").children())
	{
		if (!LoadFilm(filmNode))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	// Load cameras
	for (auto cameraNode : node.child("cameras").children())
	{
		if (!LoadCamera(cameraNode))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	// Load lights
	for (auto lightNode : node.child("lights").children())
	{
		if (!LoadLight(lightNode))
		{
			NANON_LOG_DEBUG("");
			return false;
		}
	}

	return true;
}

bool Assets::Impl::LoadTexture( const pugi::xml_node& textureNode )
{

}

// ----------------------------------------------------------------------

Assets::Assets()
	: p(new Impl)
{

}

Assets::~Assets()
{
	NANON_SAFE_DELETE(p);
}

bool Assets::Load( const pugi::xml_node& node )
{
	return p->Load(node);
}

NANON_NAMESPACE_END