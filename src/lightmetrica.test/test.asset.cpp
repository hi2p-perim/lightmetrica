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
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica.test/stub.config.h>

namespace
{

	const std::string Asset_Success = LM_TEST_MULTILINE_LITERAL(
		<asset id="test_1" type="success" />
	);

	const std::string Asset_FailOnCreate = LM_TEST_MULTILINE_LITERAL(
		<asset id="test_2" type="fail_on_create" />	
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class AssetTest : public TestBase
{
protected:

	StubAssets assets;
	StubConfig config;

};

TEST_F(AssetTest, Load)
{
	StubAsset_Success asset;
	EXPECT_TRUE(asset.Load(config.LoadFromStringAndGetFirstChild(Asset_Success), assets));
}

TEST_F(AssetTest, Create_Failed)
{
	StubAsset_FailOnCreate asset;
	EXPECT_FALSE(asset.Load(config.LoadFromStringAndGetFirstChild(Asset_FailOnCreate), assets));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
