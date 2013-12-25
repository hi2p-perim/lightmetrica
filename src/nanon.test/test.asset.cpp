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
#include "stub.assets.h"
#include <nanon/defaultassets.h>

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class AssetTest : public TestBase
{
protected:

	StubAssets assets;

};

TEST_F(AssetTest, Load)
{
	StubAsset_Success asset("");
	EXPECT_TRUE(asset.Load(pugi::xml_node(), assets));
}

TEST_F(AssetTest, Create_Failed)
{
	StubAsset_FailOnCreate asset("");
	EXPECT_FALSE(asset.Load(pugi::xml_node(), assets));
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END