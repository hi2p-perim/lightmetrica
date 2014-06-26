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
#include <lightmetrica/logger.h>
#include <lightmetrica/fp.h>

namespace fs = boost::filesystem;

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

const long long TestBase::OutputProcessTimeout = 500;

void TestBase::SetUp()
{
	// Logger
	Logger::Reset();
	Logger::SetOutputMode(Logger::LogOutputMode::Stdout);
	Logger::SetUpdateMode(Logger::LogUpdateMode::Immediate);
	//Logger::SetOutputFileName("lightmetrica.test.log");

	// Floating-point control
#if LM_STRICT_FP && LM_PLATFORM_WINDOWS
	EXPECT_TRUE(FloatintPointUtils::EnableFPControl());
#endif
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
