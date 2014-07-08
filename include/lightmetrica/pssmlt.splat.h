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

	Math::Float SumI() const;
	void AccumulateContributionToFilm(Film& film, const Math::Float& weight) const;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PSSMLT_SPLAT_H