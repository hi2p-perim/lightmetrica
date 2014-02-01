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
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/confignode.h>

namespace
{

	const std::string ConfigNodeData_1 = LM_TEST_MULTILINE_LITERAL(
		<test id="hello">
			<a>10</a>
			<b>1.5</b>
			<c>world</c>
		</test>
	);

	const std::string ConfigNodeData_2 = LM_TEST_MULTILINE_LITERAL(
		<test>
			<v>1 2 3</v>
			<m>
				1 2 3 4
				5 6 7 8
				9 10 11 12
				13 14 15 16
			</m>
		</test>
	);

	// v1 : Vec3 missing elements
	// v2 : Vec3 excessive elements
	// m1 : Mat4 missing elements
	// m2 : Vec3 excessive elements
	const std::string ConfigNodeData_2_Failed = LM_TEST_MULTILINE_LITERAL(
		<test>
			<v1>1 2</v1>
			<v2>1 2 3 4</v2>
			<m1>
				1 2 3 4
				5 6 7 8
				9 10 11 12
				13 14
			</m1>
			<m2>
				1 2 3 4
				5 6 7 8
				9 10 11 12
				13 14 15 16 17
			</m2>
		</test>
	);

	const std::string ConfigNodeData_3 = LM_TEST_MULTILINE_LITERAL(
		<test>
			<v>A</v>
			<w>0</w>
			<v>B</v>
			<w>1</w>
			<w>2</w>
			<v>C</v>
		</test>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class ConfigNodeTest : public TestBase
{
public:

	StubConfig config;

};

TEST_F(ConfigNodeTest, GetConfig)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	EXPECT_EQ(&config, node.GetConfig());
}

TEST_F(ConfigNodeTest, Empty)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	EXPECT_FALSE(node.Empty());
	EXPECT_TRUE(ConfigNode().Empty());
}

TEST_F(ConfigNodeTest, Child)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	auto a = node.Child("a");
	auto d = node.Child("d");
	EXPECT_FALSE(a.Empty());
	EXPECT_TRUE(d.Empty());
}

TEST_F(ConfigNodeTest, Name)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	EXPECT_EQ("test", node.Name());
	EXPECT_EQ("a", node.Child("a").Name());
}

TEST_F(ConfigNodeTest, Value)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	EXPECT_EQ(10, node.Child("a").Value<int>());
	EXPECT_TRUE(ExpectNear(Math::Float(1.5), node.Child("b").Value<Math::Float>()));
	EXPECT_EQ("world", node.Child("c").Value());
	EXPECT_EQ("world", node.Child("c").Value<std::string>());
}

TEST_F(ConfigNodeTest, Value_Failed)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);

	// When the type conversion fails,
	// ConfigNode::Value returns 0 for int or Math::Float types.
	EXPECT_EQ(0, node.Child("c").Value<int>());
	EXPECT_EQ(Math::Float(0), node.Child("c").Value<Math::Float>());

	// Calling Value function for empty node returns empty string
	EXPECT_TRUE(ConfigNode().Value().empty());
}

TEST_F(ConfigNodeTest, Value_2)
{
	typedef Math::Float F;
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_2);

	Math::Vec3 expect_v(F(1), F(2), F(3));
	EXPECT_TRUE(ExpectVec3Near(expect_v, node.Child("v").Value<Math::Vec3>()));

	Math::Mat4 expect_m(
		F(1), F(2), F(3), F(4),
		F(5), F(6), F(7), F(8),
		F(9), F(10), F(11), F(12),
		F(13), F(14), F(15), F(16));
	EXPECT_TRUE(ExpectMat4Near(expect_m, node.Child("m").Value<Math::Mat4>()));
}

TEST_F(ConfigNodeTest, Value_2_Failed)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_2_Failed);

	// For Vec3 or Mat4 types,
	// 0 is returned if type conversion is failed.
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(), node.Child("v1").Value<Math::Vec3>()));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(), node.Child("v2").Value<Math::Vec3>()));
	EXPECT_TRUE(ExpectMat4Near(Math::Mat4(), node.Child("m1").Value<Math::Mat4>()));
	EXPECT_TRUE(ExpectMat4Near(Math::Mat4(), node.Child("m2").Value<Math::Mat4>()));
}

TEST_F(ConfigNodeTest, AttributeValue)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	EXPECT_EQ("hello", node.AttributeValue("id"));
}

TEST_F(ConfigNodeTest, ChildValue)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);

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
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);

	int v1;
	ASSERT_TRUE(node.ChildValueOrDefault("a", 42, v1));
	EXPECT_EQ(10, v1);
	ASSERT_FALSE(node.ChildValueOrDefault("d", 42, v1));
	EXPECT_EQ(42, v1);
}

TEST_F(ConfigNodeTest, Children)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_1);
	const std::string expect[3] = { "a", "b", "c" };

	int idx = 0;
	for (auto child = node.FirstChild(); !child.Empty(); child = child.NextChild())
	{
		EXPECT_EQ(expect[idx++], child.Name());
	}
}

TEST_F(ConfigNodeTest, Children_2)
{
	const auto node = config.LoadFromStringAndGetFirstChild(ConfigNodeData_3);

	int idx = 0;
	for (auto child = node.Child("w"); !child.Empty(); child = child.NextChild("w"))
	{
		EXPECT_EQ(idx++, child.Value<int>());
	}

	EXPECT_EQ(3, idx);
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
