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
#include <lightmetrica/bitmap.h>
#include <lightmetrica/logger.h>

LM_NAMESPACE_BEGIN

LM_PUBLIC_API Math::Float BitmapImage::EvaluateRMSE( const BitmapImage& bitmap ) const
{
	return EvaluateRMSE(bitmap, Math::Float(1));
}

LM_PUBLIC_API Math::Float BitmapImage::EvaluateRMSE(const BitmapImage& bitmap, const Math::Float& weight) const
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
		auto t = data1[i] - data2[i] * weight;
		sum += t * t;
	}

	return Math::Sqrt(Math::Float(sum / Math::Float(data1.size())));
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