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

#pragma once
#ifndef LIB_LIGHTMETRICA_TEST_STUB_CONFIG_H
#define LIB_LIGHTMETRICA_TEST_STUB_CONFIG_H

#include "common.h"
#include <lightmetrica/config.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <pugixml.hpp>
#include <gtest/gtest.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubConfig : public Config
{
public:

	virtual bool Load( const std::string& path ) { return false; }
	virtual ConfigNode Root() const { return ConfigNode(doc.root().internal_object(), this); }
	virtual std::string BasePath() const { return ""; }

	virtual bool LoadFromString( const std::string& data, const std::string& basePath )
	{
		auto result = doc.load_buffer(static_cast<const void*>(data.c_str()), data.size());
		if (!result) LM_LOG_ERROR(result.description());
		return result;
	}

public:

	ConfigNode LoadFromStringAndGetFirstChild(const std::string& data)
	{
		EXPECT_TRUE(LoadFromString(data, ""));
		return ConfigNode(doc.first_child().internal_object(), this);
	}

private:

	pugi::xml_document doc;

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_STUB_CONFIG_H