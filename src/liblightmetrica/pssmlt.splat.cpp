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

#include <lightmetrica/pssmlt.splat.h>
#include <lightmetrica/film.h>

LM_NAMESPACE_BEGIN

Math::Float PSSMLTSplats::SumI() const
{
	Math::Float sumI(0);
	for (auto& splat : splats)
	{
		sumI += Math::Luminance(splat.L);
	}
	return sumI;
}

void PSSMLTSplats::AccumulateContributionToFilm( Film& film, const Math::Float& weight ) const
{
	for (auto& splat : splats)
	{
		film.AccumulateContribution(splat.rasterPos, splat.L * weight);
	}
}

LM_NAMESPACE_END