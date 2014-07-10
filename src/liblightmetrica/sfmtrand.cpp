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
#include <lightmetrica/random.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/align.h>
#include <lightmetrica/assert.h>
#include <SFMT.h>

LM_NAMESPACE_BEGIN

/*!
	SFMT random number generator.
	An random number generator using SIMD-oriented Fast Mersenne Twister (SFMT)  
	using an implementation by Mutsuo Saito and Makoto Matsumoto:
	http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/
*/
class SFMTRandom : public Random
{
public:

	LM_COMPONENT_IMPL_DEF("sfmt");

public:

	virtual unsigned int NextUInt() { return sfmt_genrand_uint32(&sfmt); }
	virtual void SetSeed( unsigned int seed ) { sfmt_init_gen_rand(&sfmt, seed); }
	virtual Random* Clone() const { return new SFMTRandom; }

private:

	sfmt_t sfmt;

};

LM_COMPONENT_REGISTER_IMPL(SFMTRandom, Random);

LM_NAMESPACE_END
