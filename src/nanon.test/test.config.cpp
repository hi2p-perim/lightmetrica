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
#include <nanon/config.h>

namespace fs = boost::filesystem;

namespace
{
	const std::string ConfigData_Success = NANON_TEST_MULTILINE_LITERAL(
		<?xml version="1.0" ?>
		<nanon version="1.0.dev">
			<assets />
			<scene />
			<renderer />
		</nanon>
	);

	const std::string ConfigData_Fail_MissingElement = NANON_TEST_MULTILINE_LITERAL(
		<?xml version="1.0" ?>
		<nanon version="1.0.dev">
		</nanon>
	);

	const std::string ConfigData_Fail_DifferentVersion = NANON_TEST_MULTILINE_LITERAL(
		<?xml version="1.0" ?>
		<nanon version="some.version">
			<assets />
			<scene />
			<renderer />
		</nanon>
	);
}

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class NanonConfigTest : public TestBase
{
protected:

	NanonConfig config;

};

TEST_F(NanonConfigTest, Load)
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

TEST_F(NanonConfigTest, Load_Failed_MissingFile)
{
	const std::string filename = (fs::temp_directory_path() / "test.nanon").string();
	if (fs::exists(filename))
	{
		EXPECT_TRUE(fs::remove(filename));
	}

	EXPECT_FALSE(config.Load(filename));
}

TEST_F(NanonConfigTest, LoadString)
{
	EXPECT_TRUE(config.LoadFromString(ConfigData_Success));
}

TEST_F(NanonConfigTest, LoadString_Failed)
{
	EXPECT_FALSE(config.LoadFromString(ConfigData_Fail_MissingElement));
	EXPECT_FALSE(config.LoadFromString(ConfigData_Fail_DifferentVersion));
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END