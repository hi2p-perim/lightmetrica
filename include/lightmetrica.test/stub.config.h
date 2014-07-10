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
	virtual bool Load( const std::string& path, const std::string& basePath ) { return false; }
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