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
#include "base.h"
#include "stub.asset.h"
#include "stub.assetfactory.h"
#include <nanon/defaultassets.h>

using namespace nanon;

namespace
{

	const std::string AssetsNode_Success = NANON_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assetfactory>
				<asset id="id1" type="success" />
				<asset id="id2" type="success" />
			</stub_assetfactory>
		</assets>
	);

	const std::string AssetsNode_Fail_InvalidElementName = NANON_TEST_MULTILINE_LITERAL(
		<invalid_name>
		</invalid_name>
	);

	const std::string AssetsNode_Fail_SameID = NANON_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assetfactory>
				<asset id="wood" type="success" />
				<asset id="wood" type="success" />
			</stub_assetfactory>
		</assets>
	);

	const std::string AssetNode_Fail_FailedToCreate = NANON_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assetfactory>
				<asset id="id" type="fail_on_create" />
			</stub_assetfactory>
		</assets>	
	);

}

NANON_TEST_NAMESPACE_BEGIN

class AssetsTest : public TestBase
{
protected:

	virtual void SetUp();

protected:

	DefaultAssets assets;

};

void AssetsTest::SetUp()
{
	TestBase::SetUp();
	EXPECT_TRUE(assets.RegisterAssetFactory(AssetFactoryEntry("stub_assetfactory", "asset", 0, std::make_shared<StubAssetFactory>())));
}

// ----------------------------------------------------------------------

TEST_F(AssetsTest, RegisterAssetFactory)
{
	EXPECT_TRUE(assets.RegisterAssetFactory(AssetFactoryEntry("test", "asset", 0, std::make_shared<StubAssetFactory>())));
}

TEST_F(AssetsTest, RegisterAssetFactory_Failed)
{
	EXPECT_TRUE(assets.RegisterAssetFactory(AssetFactoryEntry("test", "asset", 0, std::make_shared<StubAssetFactory>())));
	EXPECT_FALSE(assets.RegisterAssetFactory(AssetFactoryEntry("test", "asset", 0, std::make_shared<StubAssetFactory>())));
}

TEST_F(AssetsTest, Load)
{
	EXPECT_TRUE(assets.Load(LoadXMLBuffer(AssetsNode_Success)));

	auto* id1 = assets.GetAssetByName("id1");
	ASSERT_TRUE(id1);
	EXPECT_EQ("id1", id1->ID());
	EXPECT_EQ("asset", id1->Name());
	EXPECT_EQ("success", id1->Type());

	auto* id2 = assets.GetAssetByName("id2");
	ASSERT_TRUE(id2);
	EXPECT_EQ("id2", id2->ID());
	EXPECT_EQ("asset", id2->Name());
	EXPECT_EQ("success", id2->Type());
}

TEST_F(AssetsTest, Load_Failed)
{
	EXPECT_FALSE(assets.Load(LoadXMLBuffer(AssetsNode_Fail_InvalidElementName)));
	EXPECT_FALSE(assets.Load(LoadXMLBuffer(AssetsNode_Fail_SameID)));
}

TEST_F(AssetsTest, GetAssetByName_Failed)
{
	EXPECT_TRUE(assets.Load(LoadXMLBuffer(AssetsNode_Success)));
	EXPECT_FALSE(assets.GetAssetByName("id3"));
}

NANON_TEST_NAMESPACE_END