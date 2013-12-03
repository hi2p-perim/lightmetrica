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

NANON_NAMESPACE_BEGIN

class NanonConfig::Impl
{
public:

	Impl();
	~Impl();

public:

	bool Load(const std::string& path);
	bool LoadFromString(const std::string& data);

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
	auto result = doc.load_file(path.c_str());

	NANON_LOG_INFO(result.description());
	NANON_LOG_INFO(boost::str(boost::format("Offset : %d") % result.offset));

	return result;
}

bool NanonConfig::Impl::LoadFromString( const std::string& data )
{
	auto result = doc.load_buffer(static_cast<const void*>(data.c_str()), data.size());

	NANON_LOG_INFO(result.description());
	NANON_LOG_INFO(boost::str(boost::format("Offset : %d") % result.offset));

	return result;
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