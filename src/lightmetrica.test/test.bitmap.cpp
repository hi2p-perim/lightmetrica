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
#include <lightmetrica/bitmap.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class BitmapImageTest : public TestBase
{
public:

	BitmapImageTest()
	{
		auto& data1 = image1.InternalData();
		data1.push_back(Math::Float(1));
		data1.push_back(Math::Float(2));
		data1.push_back(Math::Float(3));

		auto& data2 = image2.InternalData();
		data2.push_back(Math::Float(3));
		data2.push_back(Math::Float(2));
		data2.push_back(Math::Float(1));
	}

protected:

	BitmapImage image1;
	BitmapImage image2;

};

TEST_F(BitmapImageTest, InternalData)
{
	auto& data = image1.InternalData();
	EXPECT_TRUE(ExpectNear(Math::Float(1), data[0]));
	EXPECT_TRUE(ExpectNear(Math::Float(2), data[1]));
	EXPECT_TRUE(ExpectNear(Math::Float(3), data[2]));
}

TEST_F(BitmapImageTest, EvaluateRMSE)
{
	// image1 vs. image1
	EXPECT_TRUE(ExpectNear(Math::Float(0), image1.EvaluateRMSE(image1)));

	// image1 vs. image2
	auto t = image1.EvaluateRMSE(image2);
	EXPECT_TRUE(ExpectNear(Math::Float(8), t*t));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
