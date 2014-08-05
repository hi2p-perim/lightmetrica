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

#include "pch.test.h"
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
		data1.push_back(Math::Float(2));

		auto& data2 = image2.InternalData();
		data2.push_back(Math::Float(3));
		data2.push_back(Math::Float(2));
		data2.push_back(Math::Float(1));
		data2.push_back(Math::Float(2));
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
	EXPECT_TRUE(ExpectNear(Math::Float(2), data[3]));
}

TEST_F(BitmapImageTest, EvaluateRMSE)
{
	// image1 vs. image1
	EXPECT_TRUE(ExpectNear(Math::Float(0), image1.EvaluateRMSE(image1)));

	// image1 vs. image2
	auto t = image1.EvaluateRMSE(image2);
	EXPECT_TRUE(ExpectNear(Math::Float(2), t*t));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
