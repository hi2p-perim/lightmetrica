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
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/pathutils.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubConfig_PathUtilsTest : public StubConfig
{
public:

	virtual std::string BasePath() const { return "/tmp/aaa/bbb"; }

};

class PathUtilsTest : public TestBase
{
protected:

	StubConfig_PathUtilsTest config;

};

TEST_F(PathUtilsTest, ResolveAssetPath_Absolute)
{
	EXPECT_EQ("/home/hello.png", PathUtils::ResolveAssetPath(config, "/home/hello.png"));
}

TEST_F(PathUtilsTest, ResolveAssetPath_Relative)
{
	EXPECT_EQ("/tmp/aaa/bbb/hello.png", PathUtils::ResolveAssetPath(Config, "hello.png"));
	EXPECT_EQ("/tmp/aaa/bbb/hello.png", PathUtils::ResolveAssetPath(Config, "./hello.png"));
	EXPECT_EQ("/tmp/aaa/hello.png", PathUtils::ResolveAssetPath(Config, "../hello.png"));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
