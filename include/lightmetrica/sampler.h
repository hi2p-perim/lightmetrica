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
	virtual Sampler* Clone() = 0;

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