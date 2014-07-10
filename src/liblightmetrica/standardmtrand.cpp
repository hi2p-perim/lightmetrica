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
#include <random>

LM_NAMESPACE_BEGIN

/*!
	Standard Mersenne Twister random number generator.
	An implementation of random number generator using std::mt19937.
*/
class StandardMTRandom : public Random
{
public:

	LM_COMPONENT_IMPL_DEF("standardmt");

public:

	virtual unsigned int NextUInt() { return uniformInt(engine); }
	virtual void SetSeed( unsigned int seed ) { engine.seed(seed); uniformInt.reset(); }
	virtual Random* Clone() const { return new StandardMTRandom; }

private:

	std::mt19937 engine;
	std::uniform_int_distribution<unsigned int> uniformInt;

};

LM_COMPONENT_REGISTER_IMPL(StandardMTRandom, Random);

LM_NAMESPACE_END
