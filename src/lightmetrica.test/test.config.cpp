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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/base.math.h>
#include <lightmetrica/config.h>

namespace fs = boost::filesystem;

namespace
{

	const std::string ConfigData_Success = LM_TEST_MULTILINE_LITERAL(
		<?xml version="1.0" ?>
		<nanon version="1.0.dev">
			<assets />
			<scene />
			<renderer />
		</nanon>
	);

	const std::string ConfigData_Fail_MissingElement = LM_TEST_MULTILINE_LITERAL(
		<?xml version="1.0" ?>
		<nanon version="1.0.dev">
		</nanon>
	);

	const std::string ConfigData_Fail_DifferentVersion = LM_TEST_MULTILINE_LITERAL(
		<?xml version="1.0" ?>
		<nanon version="some.version">
			<assets />
			<scene />
			<renderer />
		</nanon>
	);

	const std::string ConfigNodeData_1 = LM_TEST_MULTILINE_LITERAL(
		<test id="hello">
			<a>10</a>
			<b>1.5</b>
			<c>world</c>
		</test>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class ConfigTest : public TestBase
{
protected:

	Config config;

};

TEST_F(ConfigTest, Load)
{
	// Use temporary path for output directory
	const std::string filename = (fs::temp_directory_path() / "test.nanon").string();
	if (fs::exists(filename))
	{
		EXPECT_TRUE(fs::remove(filename));
	}

	// Write to the file
	std::ofstream ofs(filename);
	EXPECT_TRUE(ofs.is_open());
	ofs << ConfigData_Success;
	ofs.close();

	// Open file
	EXPECT_TRUE(config.Load(filename));
}

TEST_F(ConfigTest, Load_Failed_MissingFile)
{
	const std::string filename = (fs::temp_directory_path() / "test.nanon").string();
	if (fs::exists(filename))
	{
		EXPECT_TRUE(fs::remove(filename));
	}

	EXPECT_FALSE(config.Load(filename));
}

TEST_F(ConfigTest, LoadString)
{
	EXPECT_TRUE(config.LoadFromString(ConfigData_Success));
}

TEST_F(ConfigTest, LoadString_Failed)
{
	EXPECT_FALSE(config.LoadFromString(ConfigData_Fail_MissingElement));
	EXPECT_FALSE(config.LoadFromString(ConfigData_Fail_DifferentVersion));
}

// --------------------------------------------------------------------------------

class ConfigNodeTest : public TestBase
{
protected:

	const ConfigNode LoadConfigNode(const std::string& data)
	{
		return ConfigNode(LoadXMLBuffer(data).internal_object(), nullptr);
	}

};

TEST_F(ConfigNodeTest, Empty)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);
	EXPECT_FALSE(node.Empty());
	EXPECT_TRUE(ConfigNode().Empty());
}

TEST_F(ConfigNodeTest, Child)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);
	auto a = node.Child("a");
	auto d = node.Child("d");
	EXPECT_FALSE(a.Empty());
	EXPECT_TRUE(d.Empty());
}

TEST_F(ConfigNodeTest, Value)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);
	EXPECT_EQ(10, node.Child("a").Value<int>());
	EXPECT_TRUE(ExpectNear(Math::Float(1.5), node.Child("b").Value<Math::Float>()));
	EXPECT_EQ("world", node.Child("c").Value());
	EXPECT_EQ("world", node.Child("c").Value<std::string>());
}

TEST_F(ConfigNodeTest, Value_Failed)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);
	EXPECT_ANY_THROW(node.Child("c").Value<int>());
}

TEST_F(ConfigNodeTest, AttributeValue)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);
	EXPECT_EQ("hello", node.AttributeValue("id"));
}

TEST_F(ConfigNodeTest, ChildValue)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);

	int v1;
	ASSERT_TRUE(node.ChildValue("a", v1));
	EXPECT_EQ(10, v1);

	Math::Float v2;
	ASSERT_TRUE(node.ChildValue("b", v2));
	EXPECT_TRUE(ExpectNear(Math::Float(1.5), v2));

	std::string v3;
	ASSERT_TRUE(node.ChildValue("c", v3));
	EXPECT_EQ("world", v3);
}

TEST_F(ConfigNodeTest, ChildValueOrDefault)
{
	const auto node = LoadConfigNode(ConfigNodeData_1);

	int v1;
	ASSERT_TRUE(node.ChildValueOrDefault("a", 42, v1));
	EXPECT_EQ(10, v1);
	ASSERT_FALSE(node.ChildValueOrDefault("d", 42, v1));
	EXPECT_EQ(42, v1);
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
