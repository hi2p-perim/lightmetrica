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
#include <lightmetrica/config.h>
#include <lightmetrica/logger.h>
#include <pugixml.hpp>

namespace
{
	const std::string ConfigFileVersion = "1.0.dev";
}

LM_NAMESPACE_BEGIN

class NanonConfig::Impl : public Object
{
public:

	Impl();
	~Impl();

public:

	bool Load(const std::string& path);
	bool LoadFromString(const std::string& data);
	const pugi::xml_node AssetsElement() const;
	const pugi::xml_node SceneElement() const;
	const pugi::xml_node RendererElement() const;
	std::string SceneType() const;
	std::string RendererType() const;
	
private:

	bool HandleLoadResult(const pugi::xml_parse_result& result);

public:

	bool loaded;
	pugi::xml_document doc;
	pugi::xml_node assetsNode;
	pugi::xml_node sceneNode;
	pugi::xml_node rendererNode;

};

NanonConfig::Impl::Impl()
	: loaded(false)
{

}

NanonConfig::Impl::~Impl()
{

}

bool NanonConfig::Impl::Load( const std::string& path )
{
	if (loaded)
	{
		LM_LOG_ERROR("Configuration is already loaded");
		return false;
	}

	LM_LOG_INFO("Loading configuration from " + path);
	auto result = doc.load_file(path.c_str());
	
	return HandleLoadResult(result);
}

bool NanonConfig::Impl::LoadFromString( const std::string& data )
{
	if (loaded)
	{
		LM_LOG_ERROR("Configuration is already loaded");
		return false;
	}

	LM_LOG_INFO("Loading configuration");
	auto result = doc.load_buffer(static_cast<const void*>(data.c_str()), data.size());

	return HandleLoadResult(result);
}

bool NanonConfig::Impl::HandleLoadResult( const pugi::xml_parse_result& result )
{
	loaded = false;

	if (!result)
	{
		LM_LOG_ERROR("Failed to load the configuration file")
		LM_LOG_ERROR(boost::str(boost::format("%s (offset : %d)") % result.description() % result.offset));
		return false;
	}

	// Validate root node
	auto nanonNode = doc.child("nanon");
	if (!nanonNode)
	{
		LM_LOG_ERROR("Missing <nanon> elemement");
		return false;
	}
	
	// Validate version number
	std::string version = nanonNode.attribute("version").as_string();
	if (version != ConfigFileVersion)
	{
		LM_LOG_ERROR("Different version : " + version + " ( expected : " + ConfigFileVersion + " )");
		return false;
	}

	// Check some required elements
	assetsNode = nanonNode.child("assets");
	if (!assetsNode)
	{
		LM_LOG_ERROR("Missing 'assets' element");
		return false;
	}

	sceneNode = nanonNode.child("scene");
	if (!sceneNode)
	{
		LM_LOG_ERROR("Missing 'scene' element");
		return false;
	}

	rendererNode = nanonNode.child("renderer");
	if (!rendererNode)
	{
		LM_LOG_ERROR("Missing 'renderer' element");
		return false;
	}

	loaded = true;
	return true;
}

const pugi::xml_node NanonConfig::Impl::AssetsElement() const
{
	return loaded ? assetsNode : pugi::xml_node();
}

const pugi::xml_node NanonConfig::Impl::SceneElement() const
{
	return loaded ? sceneNode : pugi::xml_node();
}

const pugi::xml_node NanonConfig::Impl::RendererElement() const
{
	return loaded ? rendererNode : pugi::xml_node();
}

std::string NanonConfig::Impl::SceneType() const
{
	return sceneNode.attribute("type").as_string();
}

std::string NanonConfig::Impl::RendererType() const
{
	return rendererNode.attribute("type").as_string();
}

// --------------------------------------------------------------------------------

NanonConfig::NanonConfig()
	: p(new Impl)
{

}

NanonConfig::~NanonConfig()
{
	LM_SAFE_DELETE(p);
}

bool NanonConfig::Load( const std::string& path )
{
	return p->Load(path);
}

bool NanonConfig::LoadFromString( const std::string& data )
{
	return p->LoadFromString(data);
}

const pugi::xml_node NanonConfig::AssetsElement() const
{
	return p->AssetsElement();
}

const pugi::xml_node NanonConfig::SceneElement() const
{
	return p->SceneElement();
}

const pugi::xml_node NanonConfig::RendererElement() const
{
	return p->RendererElement();
}

std::string NanonConfig::SceneType() const
{
	return p->SceneType();
}

std::string NanonConfig::RendererType() const
{
	return p->RendererType();
}

LM_NAMESPACE_END
