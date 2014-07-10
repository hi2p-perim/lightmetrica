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

#include "pch.h"
#include <lightmetrica.test/base.h>
#include <lightmetrica/defaultconfig.h>

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

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class DefaultConfigTest : public TestBase
{
protected:

	DefaultConfig config;

};

TEST_F(DefaultConfigTest, Load)
{
	TemporaryTextFile file("test.lm.xml", ConfigData_Success);
	EXPECT_TRUE(config.Load(file.Path()));
}

TEST_F(DefaultConfigTest, Load_Failed_MissingFile)
{
	const std::string filename = (fs::temp_directory_path() / "test.nanon").string();
	if (fs::exists(filename))
	{
		EXPECT_TRUE(fs::remove(filename));
	}

	EXPECT_FALSE(config.Load(filename));
}

TEST_F(DefaultConfigTest, LoadString)
{
	EXPECT_TRUE(config.LoadFromString(ConfigData_Success, ""));
}

TEST_F(DefaultConfigTest, LoadString_Failed)
{
	EXPECT_FALSE(config.LoadFromString(ConfigData_Fail_MissingElement, ""));
	EXPECT_FALSE(config.LoadFromString(ConfigData_Fail_DifferentVersion, ""));
}
	
TEST_F(DefaultConfigTest, BasePath)
{
	TemporaryTextFile file("test.lm.xml", ConfigData_Success);
	EXPECT_TRUE(config.Load(file.Path()));
	EXPECT_EQ(fs::canonical(fs::path(file.Path()).parent_path()), config.BasePath());
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
