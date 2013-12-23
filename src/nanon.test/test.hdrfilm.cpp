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
#include "base.math.h"
#include <nanon/hdrfilm.h>

namespace
{

	const std::string FilmNode_Success = NANON_TEST_MULTILINE_LITERAL(
		<film id="test" type="hdr">
			<width>40</width>
			<height>30</height>
			<path>test.hdr</path>
		<\film>
	);

	const std::string FilmNode_Blank = NANON_TEST_MULTILINE_LITERAL(
		<film id="test" type="hdr">
			<width>40</width>
			<height>30</height>
			<path>%s</path>
		<\film>
	);

	const std::string FilmNode_Fail_MissingElement = NANON_TEST_MULTILINE_LITERAL(
		<film id="test" type="hdr">
			<height>30</height>
			<path>test.hdr</path>
		<\film>
	);

}

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class HDRBitmapFilmTest : public TestBase
{
public:

	HDRBitmapFilmTest()
		: film("test")
	{
		
	}

protected:

	HDRBitmapFilm film;

};

TEST_F(HDRBitmapFilmTest, Load)
{
	EXPECT_TRUE(film.Load(LoadXMLBuffer(FilmNode_Success)));
	EXPECT_EQ(40, film.Width());
	EXPECT_EQ(30, film.Height());
}

TEST_F(HDRBitmapFilmTest, Load_Fail)
{
	EXPECT_FALSE(film.Load(LoadXMLBuffer(FilmNode_Fail_MissingElement)));
}

TEST_F(HDRBitmapFilmTest, RecordContribution)
{
	EXPECT_TRUE(film.Load(LoadXMLBuffer(FilmNode_Success)));

	for (int y = 0; y < film.Height(); y++)
	{
		for (int x = 0; x < film.Width(); x++)
		{
			Math::Vec2 rasterPos(
				Math::Float(x) / Math::Float(film.Width()),
				Math::Float(y) / Math::Float(film.Height()));
			film.RecordContribution(rasterPos, (x + y) % 2 == 0 ? Math::Colors::Green : Math::Colors::Red);
		}
	}

	// Check data
	std::vector<Math::Vec3> data;
	film.InternalData(data);
	for (size_t i = 0; i < data.size(); i++)
	{
		size_t x = i % film.Width();
		size_t y = i / film.Width();
		if ((x + y) % 2 == 0)
		{
			EXPECT_TRUE(ExpectVec3Near(Math::Colors::Green, data[i]));
		}
		else
		{
			EXPECT_TRUE(ExpectVec3Near(Math::Colors::Red, data[i]));
		}
	}
}

//TEST_F(HDRBitmapFilmTest, Save)
//{
//	// Dest on temporary directory
//	const std::string filename = (fs::temp_directory_path() / "test.hdr").string();
//	if (fs::exists(filename))
//	{
//		EXPECT_TRUE(fs::remove(filename));
//	}
//
//	// TODO : Should we specify the output directory by Save?
//
//	// Save
//	film.Save();
//	
//}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END