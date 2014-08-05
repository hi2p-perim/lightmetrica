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

#include "pch.test.h"
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/stub.asset.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/assets.h>

namespace
{

	const std::string AssetsNode_Success = LM_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assets>
				<stub_asset id="id1" type="success" />
				<stub_asset id="id2" type="success" />
			</stub_assets>
		</assets>
	);

	const std::string AssetsNode_Fail_InvalidElementName = LM_TEST_MULTILINE_LITERAL(
		<invalid_name>
		</invalid_name>
	);

	const std::string AssetsNode_Fail_SameID = LM_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assets>
				<stub_asset id="wood" type="success" />
				<stub_asset id="wood" type="success" />
			</stub_assets>
		</assets>
	);

	const std::string AssetNode_Fail_FailedToCreate = LM_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assets>
				<stub_asset id="id" type="fail_on_create" />
			</stub_assets>
		</assets>	
	);

	const std::string AssetNode_Dependency_Success = LM_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assets_a>
				<stub_asset_a id="a" type="a" />
			</stub_assets_a>
			<stub_assets_b>
				<stub_asset_b id="b" type="b">
					<stub_asset_a ref="a" />
				</stub_asset_b>
			</stub_assets_b>
			<stub_assets_c>
				<stub_asset_c id="c" type="c">
					<stub_asset_a ref="a" />
					<stub_asset_b ref="b" />
				</stub_asset_c>
			</stub_assets_c>
			<stub_assets_d>
				<stub_asset_d id="d" type="d">
					<stub_asset_a ref="a" />
					<stub_asset_b ref="b" />
					<stub_asset_c ref="c" />
				</stub_asset_d>
			</stub_assets_d>
		</assets>	
	);

	const std::string AssetNode_Dependency_Failed = LM_TEST_MULTILINE_LITERAL(
		<assets>
			<stub_assets_e>
				<stub_asset_e id="e" type="e" />
			</stub_assets_e>
			<stub_assets_f>
				<stub_asset_f id="f" type="f" />
			</stub_assets_f>
		</assets>	
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

LM_COMPONENT_REGISTER_IMPL(StubAsset_Success, StubAsset);
LM_COMPONENT_REGISTER_IMPL(StubAsset_FailOnCreate, StubAsset);

LM_COMPONENT_REGISTER_IMPL(StubAsset_A_Impl, StubAsset_A);
LM_COMPONENT_REGISTER_IMPL(StubAsset_B_Impl, StubAsset_B);
LM_COMPONENT_REGISTER_IMPL(StubAsset_C_Impl, StubAsset_C);
LM_COMPONENT_REGISTER_IMPL(StubAsset_D_Impl, StubAsset_D);

LM_COMPONENT_REGISTER_IMPL(StubAsset_E_Impl, StubAsset_E);
LM_COMPONENT_REGISTER_IMPL(StubAsset_F_Impl, StubAsset_F);

class AssetsTest : public TestBase
{
public:

	AssetsTest()
		: assets(ComponentFactory::Create<Assets>())
	{

	}

protected:

	std::unique_ptr<Assets> assets;
	StubConfig config;

};

TEST_F(AssetsTest, RegisterInterface)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset>());
}

TEST_F(AssetsTest, RegisterInterface_Failed)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset>());
	EXPECT_FALSE(assets->RegisterInterface<StubAsset>());
}

TEST_F(AssetsTest, Load)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset>());
	EXPECT_TRUE(assets->Load(config.LoadFromStringAndGetFirstChild(AssetsNode_Success)));

	auto* id1 = assets->GetAssetByName("id1");
	ASSERT_NE(nullptr, id1);
	EXPECT_EQ("id1", id1->ID());
	EXPECT_EQ("stub_asset", id1->ComponentInterfaceTypeName());
	EXPECT_EQ("success", id1->ComponentImplTypeName());

	auto* id2 = assets->GetAssetByName("id2");
	ASSERT_NE(nullptr, id2);
	EXPECT_EQ("id2", id2->ID());
	EXPECT_EQ("stub_asset", id2->ComponentInterfaceTypeName());
	EXPECT_EQ("success", id2->ComponentImplTypeName());
}

TEST_F(AssetsTest, Load_Failed)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset>());
	EXPECT_FALSE(assets->Load(config.LoadFromStringAndGetFirstChild(AssetsNode_Fail_InvalidElementName)));
	EXPECT_FALSE(assets->Load(config.LoadFromStringAndGetFirstChild(AssetsNode_Fail_SameID)));
}

TEST_F(AssetsTest, Load_Dependency)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset_A>());
	EXPECT_TRUE(assets->RegisterInterface<StubAsset_B>());
	EXPECT_TRUE(assets->RegisterInterface<StubAsset_C>());
	EXPECT_TRUE(assets->RegisterInterface<StubAsset_D>());
	EXPECT_TRUE(assets->Load(config.LoadFromStringAndGetFirstChild(AssetNode_Dependency_Success)));
}

TEST_F(AssetsTest, Load_Dependency_Failed)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset_E>());
	EXPECT_TRUE(assets->RegisterInterface<StubAsset_F>());
	EXPECT_FALSE(assets->Load(config.LoadFromStringAndGetFirstChild(AssetNode_Dependency_Failed)));
}

TEST_F(AssetsTest, GetAssetByName_Failed)
{
	EXPECT_TRUE(assets->RegisterInterface<StubAsset>());
	EXPECT_TRUE(assets->Load(config.LoadFromStringAndGetFirstChild(AssetsNode_Success)));
	EXPECT_FALSE(assets->GetAssetByName("id3"));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
