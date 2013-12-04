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
#include <nanon/config.h>
#include <nanon/logger.h>
#include <pugixml.hpp>

namespace
{
	const std::string ConfigFileVersion = "1.0.dev";
}

NANON_NAMESPACE_BEGIN

class NanonConfig::Impl
{
public:

	Impl();
	~Impl();

public:

	bool Load(const std::string& path);
	bool LoadFromString(const std::string& data);
	
private:

	bool HandleLoadResult(const pugi::xml_parse_result& result);

public:

	pugi::xml_document doc;

};

NanonConfig::Impl::Impl()
{

}

NanonConfig::Impl::~Impl()
{

}

bool NanonConfig::Impl::Load( const std::string& path )
{
	NANON_LOG_INFO("Loading configuration from " + path);
	auto result = doc.load_file(path.c_str());
	return HandleLoadResult(result);
}

bool NanonConfig::Impl::LoadFromString( const std::string& data )
{
	NANON_LOG_INFO("Loading configuration");
	auto result = doc.load_buffer(static_cast<const void*>(data.c_str()), data.size());
	return HandleLoadResult(result);
}

bool NanonConfig::Impl::HandleLoadResult( const pugi::xml_parse_result& result )
{
	if (!result)
	{
		NANON_LOG_ERROR("Failed to load the configuration file")
		NANON_LOG_ERROR(boost::str(boost::format("%s (offset : %d)") % result.description() % result.offset));
		return false;
	}

	// Validate root node
	auto nanonNode = doc.child("nanon");
	if (!nanonNode)
	{
		NANON_LOG_ERROR("Missing <nanon> elemement");
		return false;
	}
	
	// Validate version number
	std::string version = nanonNode.attribute("version").as_string();
	if (version != ConfigFileVersion)
	{
		NANON_LOG_ERROR("Different version : " + version + " ( expected : " + ConfigFileVersion + " )");
		return false;
	}

	// Check if some required elements
	if (!nanonNode.child("assets") || !nanonNode.child("scene"))
	{
		NANON_LOG_ERROR("Missing <assets> or <scene> element");
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

NanonConfig::NanonConfig()
	: p(new Impl)
{

}

NanonConfig::~NanonConfig()
{
	NANON_SAFE_DELETE(p);
}

bool NanonConfig::Load( const std::string& path )
{
	return p->Load(path);
}

bool NanonConfig::LoadFromString( const std::string& data )
{
	return p->LoadFromString(data);
}

NANON_NAMESPACE_END