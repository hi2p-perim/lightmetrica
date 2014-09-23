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
#ifndef LIB_LIGHTMETRICA_RANDOM_SAMPLER_H
#define LIB_LIGHTMETRICA_RANDOM_SAMPLER_H

#include "component.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

class Assets;
class ConfigNode;
class Random;

/*!
	Sampler.
	An interface for samplers.
*/
class Sampler : public Component
{
public:

	Sampler() {}
	virtual ~Sampler() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Sampler);

public:

	/*!
		Clone the sampler.
		\return Duplicated sampler.
	*/
	virtual Sampler* Clone() const = 0;

	/*!
		Set seed and initialize internal state.
		This function is valid only for 
		\param seed Seed.
	*/
	virtual void SetSeed(unsigned int seed) = 0;

	/*!
		Sample a floating-point value.
		\return Sampled value.
	*/
	virtual Math::Float Next() = 0;

	/*!
		Sample a unsigned integer value.
		\return Sampled value.
	*/
	virtual unsigned int NextUInt() = 0;

	/*!
		Sample a floating-point 2d vector value.
		\return Sampled value.
	*/
	virtual Math::Vec2 NextVec2() = 0;

	/*!
		Ger underlying random number generator if available.
		\return Random number generator.
	*/
	virtual Random* Rng() = 0;

public:

	/*!
		Set seed with current time.
		Sets the sampler seed with current time.
	*/
	LM_PUBLIC_API void SetSeedWithCurrentTime();

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RANDOM_SAMPLER_H