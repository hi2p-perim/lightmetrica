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
#include <lightmetrica/logger.h>

namespace fs = boost::filesystem;

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

const long long TestBase::OutputProcessTimeout = 500;

void TestBase::SetUp()
{

}

void TestBase::TearDown()
{

}

// --------------------------------------------------------------------------------

TemporaryFile::~TemporaryFile()
{
	if (fs::exists(path))
	{
		EXPECT_TRUE(fs::remove(path));
	}
}

TemporaryTextFile::TemporaryTextFile( const std::string& filename, const std::string& content )
{
	path = (fs::temp_directory_path() / filename).string();
	std::ofstream ofs(path, std::ios::out | std::ios::trunc);
	EXPECT_TRUE(ofs.is_open());
	ofs << content;
}

TemporaryBinaryFile::TemporaryBinaryFile( const std::string& filename, const unsigned char* content, unsigned int length )
{
	path = (fs::temp_directory_path() / filename).string();
	std::ofstream ofs(path, std::ios::out | std::ios::binary);
	EXPECT_TRUE(ofs.is_open());
	ofs.write((const char*)content, length);
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
