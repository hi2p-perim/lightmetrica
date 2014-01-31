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

class ConfigNode::Impl : public Object
{
public:

	Impl() {}
	Impl(void* node, const Config* config);

public:

	pugi::xml_node node;
	const Config* config;

};

ConfigNode::Impl::Impl( void* node, const Config* config )
	: node(static_cast<pugi::xml_node_struct*>(node))
	, config(config)
{

}

// --------------------------------------------------------------------------------

ConfigNode::ConfigNode()
	: p(new Impl)
{

}

ConfigNode::ConfigNode( void* node, const Config* config )
	: p(new Impl(node, config))
{

}

ConfigNode::ConfigNode( const ConfigNode& config )
	: p(new Impl(config.p->node, config.p->config))
{
	
}

void ConfigNode::operator=( const ConfigNode& config )
{
	*p = *config.p;
}

ConfigNode::ConfigNode( ConfigNode&& config )
{
	this->p = config.p;
	config.p = nullptr;
}

void ConfigNode::operator=( ConfigNode&& config )
{
	this->p = config.p;
	config.p = nullptr;
}

ConfigNode::~ConfigNode()
{
	LM_SAFE_DELETE(p);
}

bool ConfigNode::Empty() const
{
	return p->node.empty();
}

const ConfigNode ConfigNode::Child( const std::string& name ) const
{
	return ConfigNode(p->node.child(name.c_str()).internal_object(), p->config);
}

std::string ConfigNode::Value() const
{
	return p->node.child_value();
}

std::string ConfigNode::AttributeValue( const std::string& name ) const
{
	return p->node.attribute(name.c_str()).value();
}

template <>
std::string ConfigNode::Value<std::string>() const
{
	return Value();
}

template <>
int ConfigNode::Value<int>() const
{
	return std::stoi(Value());
}

template <>
Math::Float ConfigNode::Value<Math::Float>() const
{
	return Math::Float(std::stod(Value()));
}

// --------------------------------------------------------------------------------

class Config::Impl : public Object
{
public:

	Impl(Config* self);
	~Impl();

public:

	bool Load(const std::string& path);
	bool LoadFromString(const std::string& data);
	const pugi::xml_node AssetsElement() const;
	const pugi::xml_node SceneElement() const;
	const pugi::xml_node RendererElement() const;
	std::string SceneType() const;
	std::string RendererType() const;
	const ConfigNode Root() const;

private:

	bool HandleLoadResult(const pugi::xml_parse_result& result);

public:

	Config* self;

	bool loaded;
	pugi::xml_document doc;
	pugi::xml_node assetsNode;
	pugi::xml_node sceneNode;
	pugi::xml_node rendererNode;

};

Config::Impl::Impl(Config* self)
	: self(self)
	, loaded(false)
{

}

Config::Impl::~Impl()
{

}

bool Config::Impl::Load( const std::string& path )
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

bool Config::Impl::LoadFromString( const std::string& data )
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

bool Config::Impl::HandleLoadResult( const pugi::xml_parse_result& result )
{
	loaded = false;

	if (!result)
	{
		LM_LOG_ERROR("Failed to load the configuration file")
		LM_LOG_ERROR(boost::str(boost::format("%s (offset : %d)") % result.description() % result.offset));
		return false;
	}

	// Validate root node
	auto rootNode = doc.child("nanon");
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

const pugi::xml_node Config::Impl::AssetsElement() const
{
	return loaded ? assetsNode : pugi::xml_node();
}

const pugi::xml_node Config::Impl::SceneElement() const
{
	return loaded ? sceneNode : pugi::xml_node();
}

const pugi::xml_node Config::Impl::RendererElement() const
{
	return loaded ? rendererNode : pugi::xml_node();
}

std::string Config::Impl::SceneType() const
{
	return sceneNode.attribute("type").as_string();
}

std::string Config::Impl::RendererType() const
{
	return rendererNode.attribute("type").as_string();
}

const ConfigNode Config::Impl::Root() const
{
	return ConfigNode(doc.root().internal_object(), self);
}

// --------------------------------------------------------------------------------

Config::Config()
	: p(new Impl(this))
{

}

Config::~Config()
{
	LM_SAFE_DELETE(p);
}

bool Config::Load( const std::string& path )
{
	return p->Load(path);
}

bool Config::LoadFromString( const std::string& data )
{
	return p->LoadFromString(data);
}

const pugi::xml_node Config::AssetsElement() const
{
	return p->AssetsElement();
}

const pugi::xml_node Config::SceneElement() const
{
	return p->SceneElement();
}

const pugi::xml_node Config::RendererElement() const
{
	return p->RendererElement();
}

std::string Config::SceneType() const
{
	return p->SceneType();
}

std::string Config::RendererType() const
{
	return p->RendererType();
}

const ConfigNode Config::Root() const
{
	return p->Root();
}

LM_NAMESPACE_END
