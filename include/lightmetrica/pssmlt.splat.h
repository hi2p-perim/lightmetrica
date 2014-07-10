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
#ifndef LIB_LIGHTMETRICA_PSSMLT_SPLAT_H
#define LIB_LIGHTMETRICA_PSSMLT_SPLAT_H

#include "common.h"
#include "math.types.h"
#include "align.h"
#include <vector>

LM_NAMESPACE_BEGIN

/*!
	Splat for path sampler.
	Used as a sampled result of light path sampler in PSSMLT.
*/
struct PSSMLTSplat
{

	int s;						//!< # of light subpath vertices
	int t;						//!< # of eye subpath vertices
	Math::Vec2 rasterPos;		//!< Raster position
	Math::Vec3 L;				//!< Radiance

	PSSMLTSplat(int s, int t, const Math::Vec2& rasterPos, const Math::Vec3& L)
		: s(s)
		, t(t)
		, rasterPos(rasterPos)
		, L(L)
	{
		
	}

	PSSMLTSplat(const Math::Vec2& rasterPos, const Math::Vec3& L)
		: s(0)
		, t(0)
		, rasterPos(rasterPos)
		, L(L)
	{

	}

};

class Film;

/*!
	List of splats.
	List of evaluated result of the sampled light paths.
*/
struct PSSMLTSplats
{

	std::vector<PSSMLTSplat, aligned_allocator<PSSMLTSplat, std::alignment_of<PSSMLTSplat>::value>> splats;

	LM_PUBLIC_API Math::Float SumI() const;
	LM_PUBLIC_API void AccumulateContributionToFilm(Film& film, const Math::Float& weight) const;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PSSMLT_SPLAT_H