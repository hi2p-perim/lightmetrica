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
#include <lightmetrica.test/base.math.h>
#include <lightmetrica/math.distribution.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class DiscreteDistribution1DTest : public TestBase {};

TEST_F(DiscreteDistribution1DTest, Add)
{
	Math::DiscreteDistribution1D dist;
	dist.Add(Math::Float(1));
	dist.Add(Math::Float(2));
	EXPECT_TRUE(ExpectNear(Math::Float(1), dist.EvaluatePDF(0)));
	EXPECT_TRUE(ExpectNear(Math::Float(2), dist.EvaluatePDF(1)));
}

TEST_F(DiscreteDistribution1DTest, Normalize)
{
	Math::DiscreteDistribution1D dist;
	dist.Add(Math::Float(1));
	dist.Add(Math::Float(1));
	dist.Normalize();
	EXPECT_TRUE(ExpectNear(Math::Float(0.5), dist.EvaluatePDF(0)));
	EXPECT_TRUE(ExpectNear(Math::Float(0.5), dist.EvaluatePDF(1)));
}

TEST_F(DiscreteDistribution1DTest, Sample)
{
	Math::DiscreteDistribution1D dist;
	dist.Add(Math::Float(1));
	dist.Add(Math::Float(1));
	dist.Add(Math::Float(1));
	dist.Add(Math::Float(1));
	dist.Normalize();
	EXPECT_EQ(0, dist.Sample(Math::Float(0.1)));
	EXPECT_EQ(1, dist.Sample(Math::Float(0.26)));
	EXPECT_EQ(2, dist.Sample(Math::Float(0.51)));
	EXPECT_EQ(3, dist.Sample(Math::Float(0.76)));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END