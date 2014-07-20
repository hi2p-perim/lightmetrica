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
#include <lightmetrica/config.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <pugixml.hpp>

namespace
{
	const std::string ConfigFileVersion = "1.0.dev";
}

LM_NAMESPACE_BEGIN

/*!
	Default config.
	Default implementation of the config class.
*/
class ConfigImpl : public Config
{
public:

	LM_COMPONENT_IMPL_DEF("default");

public:

	ConfigImpl() : loaded(false) {}

public:

	virtual bool Load(const std::string& path);
	virtual bool Load(const std::string& path, const std::string& basePath);
	virtual bool LoadFromString(const std::string& data, const std::string& basePath);
	virtual const ConfigNode Root() const;
	virtual std::string BasePath() const { return basePath; }

private:

	bool HandleLoadResult(const pugi::xml_parse_result& result);

public:

	bool loaded;
	std::string path;
	std::string basePath;
	pugi::xml_document doc;
	pugi::xml_node assetsNode;
	pugi::xml_node sceneNode;
	pugi::xml_node rendererNode;
	pugi::xml_node rootNode;

};

bool ConfigImpl::Load( const std::string& path )
{
	return Load(path, "");
}

bool ConfigImpl::Load( const std::string& path, const std::string& basePath )
{
	if (loaded)
	{
		LM_LOG_ERROR("Configuration is already loaded");
		return false;
	}

	this->path = path;
	this->basePath = basePath.empty()
		? boost::filesystem::canonical(boost::filesystem::path(path).parent_path()).string()
		: basePath;

	LM_LOG_INFO("Setting asset base path to " + this->basePath);
	LM_LOG_INFO("Loading configuration from " + path);
	auto result = doc.load_file(path.c_str());

	return HandleLoadResult(result);
}

bool ConfigImpl::LoadFromString( const std::string& data, const std::string& basePath )
{
	if (loaded)
	{
		LM_LOG_ERROR("Configuration is already loaded");
		return false;
	}

	this->basePath = basePath;

	LM_LOG_INFO("Loading configuration");
	auto result = doc.load_buffer(static_cast<const void*>(data.c_str()), data.size());

	return HandleLoadResult(result);
}

bool ConfigImpl::HandleLoadResult( const pugi::xml_parse_result& result )
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

const ConfigNode ConfigImpl::Root() const
{
	return ConfigNode(rootNode.internal_object(), this);
}

LM_COMPONENT_REGISTER_IMPL(ConfigImpl, Config);

LM_NAMESPACE_END
