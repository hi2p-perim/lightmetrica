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
#include <lightmetrica/bitmap.h>
#include <lightmetrica/logger.h>

LM_NAMESPACE_BEGIN

LM_PUBLIC_API Math::Float BitmapImage::EvaluateRMSE( const BitmapImage& bitmap ) const
{
	const auto& data1 = InternalData();
	const auto& data2 = bitmap.InternalData();

	// Check size
	if (data1.size() != data2.size())
	{
		LM_LOG_WARN("Invalid image size : " + std::to_string(data1.size()) + " != " + std::to_string(data2.size()));
		return Math::Float(0);
	}

	// Calculate RMSE
	Math::Float sum(0);
	for (size_t i = 0; i < data1.size(); i++)
	{
		auto t = data1[i] - data2[i];
		sum += t * t;
	}

	return Math::Sqrt(sum / Math::Float(data1.size()));
}

LM_PUBLIC_API std::vector<Math::Float>& BitmapImage::InternalData()
{
	return data;
}

LM_PUBLIC_API const std::vector<Math::Float>& BitmapImage::InternalData() const
{
	return data;
}

LM_PUBLIC_API void BitmapImage::Clear()
{
	data.clear();
}

LM_NAMESPACE_END