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
#include <lightmetrica.test/base.math.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/bitmap.h>
#include <FreeImage.h>

namespace
{

	const std::string FilmNode_1 = LM_TEST_MULTILINE_LITERAL(
		<film id="test" type="hdr">
			<width>40</width>
			<height>30</height>
			<path>test.hdr</path>
		</film>
	);

	const std::string FilmNode_Blank = LM_TEST_MULTILINE_LITERAL(
		<film id="test" type="hdr">
			<width>40</width>
			<height>30</height>
			<path>%s</path>
		</film>
	);

	const std::string FilmNode_Fail_MissingElement = LM_TEST_MULTILINE_LITERAL(
		<film id="test" type="hdr">
			<height>30</height>
			<path>test.hdr</path>
		</film>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class HDRBitmapFilmTest : public TestBase
{
public:

	HDRBitmapFilmTest()
		: film(ComponentFactory::Create<BitmapFilm>("hdr"))
	{
		
	}

protected:

	std::unique_ptr<BitmapFilm> film;
	StubAssets assets;
	StubConfig config;

};

TEST_F(HDRBitmapFilmTest, Load)
{
	EXPECT_TRUE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_1), assets));
	EXPECT_EQ(40, film->Width());
	EXPECT_EQ(30, film->Height());
	EXPECT_EQ(BitmapImageType::RadianceHDR, film->ImageType());
}

TEST_F(HDRBitmapFilmTest, Load_Fail)
{
	EXPECT_FALSE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_Fail_MissingElement), assets));
}

TEST_F(HDRBitmapFilmTest, RecordContribution)
{
	EXPECT_TRUE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_1), assets));

	for (int y = 0; y < film->Height(); y++)
	{
		for (int x = 0; x < film->Width(); x++)
		{
			Math::Vec2 rasterPos(
				(Math::Float(x) + Math::Float(0.5)) / Math::Float(film->Width()),
				(Math::Float(y) + Math::Float(0.5)) / Math::Float(film->Height()));
			film->RecordContribution(rasterPos, (x + y) % 2 == 0 ? Math::Colors::Green() : Math::Colors::Red());
		}
	}

	// Check data
	const auto& data = film->Bitmap().InternalData();
	for (size_t i = 0; i < data.size() / 3; i+=3)
	{
		size_t x = i % film->Width();
		size_t y = i / film->Width();
		if ((x + y) % 2 == 0)
		{
			EXPECT_TRUE(ExpectNear(Math::Colors::Green()[0], data[3*i  ]));
			EXPECT_TRUE(ExpectNear(Math::Colors::Green()[1], data[3*i+1]));
			EXPECT_TRUE(ExpectNear(Math::Colors::Green()[2], data[3*i+2]));
		}
		else
		{
			EXPECT_TRUE(ExpectNear(Math::Colors::Red()[0], data[3*i  ]));
			EXPECT_TRUE(ExpectNear(Math::Colors::Red()[1], data[3*i+1]));
			EXPECT_TRUE(ExpectNear(Math::Colors::Red()[2], data[3*i+2]));
		}
	}
}

TEST_F(HDRBitmapFilmTest, AccumulateContribution)
{
	EXPECT_TRUE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_1), assets));

	// Accumulate #Count times to (0, 0) and (1, 1)
	const int Count = 10;
	for (int i = 0; i < Count; i++)
	{
		film->AccumulateContribution(Math::Vec2(), Math::Vec3(Math::Float(1)));
		film->AccumulateContribution(Math::Vec2(Math::Float(1)), Math::Vec3(Math::Float(2)));
	}
	
	// Check data
	const auto& data = film->Bitmap().InternalData();

	EXPECT_TRUE(ExpectNear(Math::Float(Count), data[0]));
	EXPECT_TRUE(ExpectNear(Math::Float(Count), data[1]));
	EXPECT_TRUE(ExpectNear(Math::Float(Count), data[2]));

	size_t i = data.size() - 3;
	EXPECT_TRUE(ExpectNear(Math::Float(Count * 2), data[i  ]));
	EXPECT_TRUE(ExpectNear(Math::Float(Count * 2), data[i+1]));
	EXPECT_TRUE(ExpectNear(Math::Float(Count * 2), data[i+2]));
}

TEST_F(HDRBitmapFilmTest, AccumulateContribution_2)
{
	// Create a film with constant value
	std::unique_ptr<BitmapFilm> film2(ComponentFactory::Create<BitmapFilm>("hdr"));
	film2->Allocate(40, 30);
	for (int y = 0; y < film2->Height(); y++)
	{
		for (int x = 0; x < film2->Width(); x++)
		{
			Math::Vec2 rasterPos(
				(Math::Float(x) + Math::Float(0.5)) / Math::Float(film2->Width()),
				(Math::Float(y) + Math::Float(0.5)) / Math::Float(film2->Height()));
			film2->RecordContribution(rasterPos, Math::Vec3(Math::Float(1)));
		}
	}

	// Accumulate to #film
	const int Count = 10;
	EXPECT_TRUE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_1), assets));
	for (int i = 0; i < Count; i++)
	{
		film->AccumulateContribution(*film2);
	}
	
	// Check data
	const auto& data = film->Bitmap().InternalData();
	for (size_t i = 0; i < data.size(); i++)
	{
		EXPECT_TRUE(ExpectNear(Math::Float(Count), data[i]));
	}
}

TEST_F(HDRBitmapFilmTest, Save)
{
	// Create a film
	EXPECT_TRUE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_1), assets));
	for (int y = 0; y < film->Height(); y++)
	{
		for (int x = 0; x < film->Width(); x++)
		{
			Math::Vec2 rasterPos(
				(Math::Float(x) + Math::Float(0.5)) / Math::Float(film->Width()),
				(Math::Float(y) + Math::Float(0.5)) / Math::Float(film->Height()));
			film->RecordContribution(rasterPos, Math::Vec3(Math::Float(x), Math::Float(y), Math::Float(1)));
		}
	}

	// Output image to temporary directory
	namespace fs = boost::filesystem;
	const std::string path = (fs::temp_directory_path() / "lightmetrica.test.hdr").string();
	if (fs::exists(path))
	{
		EXPECT_TRUE(fs::remove(path));
	}

	for (int i = 0; i < 2; i++)
	{
		Math::Float weight =
			i == 0
				? Math::Float(1)	// With weighting
				: Math::Float(2);	// Without weighting

		// Save
		if (i == 0)
		{
			EXPECT_TRUE(film->Save(path));
		}
		else
		{
			EXPECT_TRUE(film->RescaleAndSave(path, weight));
		}

		// Image data of #film
		const auto& data = film->Bitmap().InternalData();

		// Load image and check data
		auto* bitmap = FreeImage_Load(FIF_HDR, path.c_str(), 0);
		EXPECT_NE(nullptr, bitmap);
		int width = FreeImage_GetWidth(bitmap);
		int height = FreeImage_GetHeight(bitmap);
		EXPECT_EQ(film->Width(), width);
		EXPECT_EQ(film->Height(), height);
		for (int y = 0; y < height; y++)
		{
			FIRGBF* bits = reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bitmap, y));
			for (int x = 0; x < width; x++)
			{
				int i = y * width + x;
				EXPECT_TRUE(ExpectNear(data[3*i  ] * weight, Math::Float(bits[x].red)));
				EXPECT_TRUE(ExpectNear(data[3*i+1] * weight, Math::Float(bits[x].green)));
				EXPECT_TRUE(ExpectNear(data[3*i+2] * weight, Math::Float(bits[x].blue)));
			}
		}
		FreeImage_Unload(bitmap);
	}

	// Clean up
	if (fs::exists(path))
	{
		EXPECT_TRUE(fs::remove(path));
	}
}

TEST_F(HDRBitmapFilmTest, Clone)
{
	// Create a film
	EXPECT_TRUE(film->Load(config.LoadFromStringAndGetFirstChild(FilmNode_1), assets));
	for (int y = 0; y < film->Height(); y++)
	{
		for (int x = 0; x < film->Width(); x++)
		{
			Math::Vec2 rasterPos(
				(Math::Float(x) + Math::Float(0.5)) / Math::Float(film->Width()),
				(Math::Float(y) + Math::Float(0.5)) / Math::Float(film->Height()));
			film->RecordContribution(rasterPos, Math::Vec3(Math::Float(1)));
		}
	}

	// Clone to #film2
	BitmapFilm* film2;
	ASSERT_NO_THROW(film2 = dynamic_cast<BitmapFilm*>(film->Clone()));
	EXPECT_NE(nullptr, film2);

	// Check data
	const auto& data = film->Bitmap().InternalData();
	for (size_t i = 0; i < data.size(); i++)
	{
		EXPECT_TRUE(ExpectNear(Math::Float(1), data[i]));
	}

	LM_SAFE_DELETE(film2);
}

TEST_F(HDRBitmapFilmTest, Allocate)
{
	film->SetImageType(BitmapImageType::RadianceHDR);
	film->Allocate(40, 30);
	EXPECT_EQ(40, film->Width());
	EXPECT_EQ(30, film->Height());
	EXPECT_EQ(BitmapImageType::RadianceHDR, film->ImageType());

	const auto& data = film->Bitmap().InternalData();
	EXPECT_EQ(40 * 30 * 3, data.size());
}

TEST_F(HDRBitmapFilmTest, Rescale)
{
	// Initialize film
	film->Allocate(40, 30);
	for (int y = 0; y < film->Height(); y++)
	{
		for (int x = 0; x < film->Width(); x++)
		{
			Math::Vec2 rasterPos(
				(Math::Float(x) + Math::Float(0.5)) / Math::Float(film->Width()),
				(Math::Float(y) + Math::Float(0.5)) / Math::Float(film->Height()));
			film->RecordContribution(rasterPos, Math::Vec3(Math::Float(1)));
		}
	}

	// Rescale and check original data
	film->Rescale(Math::Float(2));
	const auto& data = film->Bitmap().InternalData();
	for (auto& v : data)
	{
		EXPECT_TRUE(ExpectNear(Math::Float(2), v));
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
