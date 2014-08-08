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

#pragma once
#ifndef LIB_LIGHTMETRICA_MATH_DISTRIBUTION_H
#define LIB_LIGHTMETRICA_MATH_DISTRIBUTION_H

#include "math.types.h"
#include "logger.h"
#include <vector>

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

/*!
	Discrete 1D distribution.
	Offers interface for creating and sampling from 1D discrete PDF.
*/
class DiscreteDistribution1D
{
public:

	DiscreteDistribution1D()
	{
		Clear();
	}

private:

	LM_DISABLE_COPY_AND_MOVE(DiscreteDistribution1D);

public:

	void Add(const Math::Float& v)
	{
		cdf.push_back(cdf.back() + v);
	}

	void Normalize()
	{
		auto sum = cdf.back();
		if (sum > Math::Float(0))
		{
			auto invSum = Math::Float(1) / sum;
			for (auto& v : cdf)
			{
				v *= invSum;
			}
		}
		else
		{
			LM_LOG_WARN("Unable to normalize. Sum is zero.")
		}
	}

	size_t Sample(const Math::Float& u) const
	{
		return Math::Clamp<size_t>(
			static_cast<size_t>(std::upper_bound(cdf.begin(), cdf.end(), u) - cdf.begin()) - 1,
			0, cdf.size() - 2);
	}
	
	Math::Float EvaluatePDF(size_t i) const
	{
		return cdf[i+1] - cdf[i];
	}

	void Clear()
	{
		cdf.clear();
		cdf.push_back(Math::Float(0));
	}

private:

	std::vector<Math::Float> cdf;

};

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_MATH_DISTRIBUTION_H