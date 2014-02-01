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
#include <lightmetrica/defaultconfig.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <pugixml.hpp>

namespace
{
	const std::string ConfigFileVersion = "1.0.dev";
}

LM_NAMESPACE_BEGIN

class DefaultConfig::Impl : public Object
{
public:

	Impl(DefaultConfig* self);
	~Impl();

public:

	bool Load(const std::string& path);
	bool LoadFromString(const std::string& data);
	const ConfigNode Root() const;
	std::string BasePath() const;

private:

	bool HandleLoadResult(const pugi::xml_parse_result& result);

public:

	DefaultConfig* self;

	bool loaded;
	std::string path;
	pugi::xml_document doc;
	pugi::xml_node assetsNode;
	pugi::xml_node sceneNode;
	pugi::xml_node rendererNode;
	pugi::xml_node rootNode;

};

DefaultConfig::Impl::Impl(DefaultConfig* self)
	: self(self)
	, loaded(false)
{

}

DefaultConfig::Impl::~Impl()
{

}

bool DefaultConfig::Impl::Load( const std::string& path )
{
	if (loaded)
	{
		LM_LOG_ERROR("Configuration is already loaded");
		return false;
	}

	this->path = path;

	LM_LOG_INFO("Loading configuration from " + path);
	auto result = doc.load_file(path.c_str());
	
	return HandleLoadResult(result);
}

bool DefaultConfig::Impl::LoadFromString( const std::string& data )
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

bool DefaultConfig::Impl::HandleLoadResult( const pugi::xml_parse_result& result )
{
	loaded = false;

	if (!result)
	{
		LM_LOG_ERROR("Failed to load the configuration file")
		LM_LOG_ERROR(boost::str(boost::format("%s (offset : %d)") % result.description() % result.offset));
		return false;
	}

	// Validate root node
	rootNode = doc.child("nanon");
	if (!rootNode)
	{
		LM_LOG_ERROR("Missing <nanon> elemement");
		return false;
	}
	
	// Validate version number
	std::string version = rootNode.attribute("version").as_string();
	if (version != ConfigFileVersion)
	{
		LM_LOG_ERROR("Different version : " + version + " ( expected : " + ConfigFileVersion + " )");
		return false;
	}

	// Check some required elements
	assetsNode = rootNode.child("assets");
	if (!assetsNode)
	{
		LM_LOG_ERROR("Missing 'assets' element");
		return false;
	}

	sceneNode = rootNode.child("scene");
	if (!sceneNode)
	{
		LM_LOG_ERROR("Missing 'scene' element");
		return false;
	}

	rendererNode = rootNode.child("renderer");
	if (!rendererNode)
	{
		LM_LOG_ERROR("Missing 'renderer' element");
		return false;
	}

	loaded = true;
	return true;
}

const ConfigNode DefaultConfig::Impl::Root() const
{
	return ConfigNode(rootNode.internal_object(), self);
}

std::string DefaultConfig::Impl::BasePath() const
{
	namespace fs = boost::filesystem;
	return fs::canonical(fs::path(path).parent_path()).string();
}

// --------------------------------------------------------------------------------

DefaultConfig::DefaultConfig()
	: p(new Impl(this))
{

}

DefaultConfig::~DefaultConfig()
{
	LM_SAFE_DELETE(p);
}

bool DefaultConfig::Load( const std::string& path )
{
	return p->Load(path);
}

bool DefaultConfig::LoadFromString( const std::string& data )
{
	return p->LoadFromString(data);
}

ConfigNode DefaultConfig::Root() const
{
	return p->Root();
}

std::string DefaultConfig::BasePath() const
{
	return p->BasePath();
}

LM_NAMESPACE_END
