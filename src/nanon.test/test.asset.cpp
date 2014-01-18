/*
	L I G H T  M E T R I C A

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
#include <lightmetrica.test/stub.asset.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica/defaultassets.h>

namespace
{

	const std::string Asset_Success = LM_TEST_MULTILINE_LITERAL(
		<asset id="test_1" type="success" />
	);

	const std::string Asset_FailOnCreate = LM_TEST_MULTILINE_LITERAL(
		<Asset id="test_2" type="fail_on_create" />	
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class AssetTest : public TestBase
{
protected:

	StubAssets assets;

};

TEST_F(AssetTest, Load)
{
	StubAsset_Success asset("");
	EXPECT_TRUE(asset.Load(LoadXMLBuffer(Asset_Success), assets));
}

TEST_F(AssetTest, Create_Failed)
{
	StubAsset_FailOnCreate asset("");
	EXPECT_FALSE(asset.Load(LoadXMLBuffer(Asset_FailOnCreate), assets));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
