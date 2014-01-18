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
#include <lightmetrica/asset.h>
#include <lightmetrica/logger.h>
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

class Asset::Impl
{
public:

	Impl(const std::string& id);
	~Impl();

public:

	std::string ID() const { return id; }

private:

	std::string id;

};

Asset::Impl::Impl( const std::string& id )
	: id(id)
{

}

Asset::Impl::~Impl()
{

}

// --------------------------------------------------------------------------------

Asset::Asset(const std::string& id)
	: p(new Impl(id))
{

}

Asset::~Asset()
{
	LM_SAFE_DELETE(p);
}

std::string Asset::ID() const
{
	return p->ID();
}

bool Asset::Load( const pugi::xml_node& node, const Assets& assets )
{
	// Check name and type
	if (node.name() != Name())
	{
		LM_LOG_ERROR(boost::str(boost::format("Invalid node name '%s'") % node.name()));
		return false;
	}

	if (node.attribute("type").as_string() != Type())
	{
		LM_LOG_ERROR(boost::str(boost::format("Invalid camera type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// Call implementation detail load function
	return LoadAsset(node, assets);
}

LM_NAMESPACE_END
