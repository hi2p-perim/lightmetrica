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
#ifndef LIB_LIGHTMETRICA_REWINDABLE_SAMPLER_H
#define LIB_LIGHTMETRICA_REWINDABLE_SAMPLER_H

#include "sampler.h"
#include "random.h"

LM_NAMESPACE_BEGIN

/*!
	Rewindable sampler.
	A sampler implementation for restoring sample sequence afterwards.
	This sampler is introduced to avoid to save unnecessary light paths
	in the initial sampling process of MLT or PSSMLT.
*/
class RewindableSampler : public Sampler
{
public:

	LM_COMPONENT_INTERFACE_DEF("rewindablesampler");

public:

	RewindableSampler() {};
	virtual ~RewindableSampler() {}

public:

	/*!
		Configure the sampler.
		Initializes underlying random number generator with the given value.
		This function must be called before use.
		Ownership of #rng is delegated to this class.
		Seed is not explicitly refreshed, so do not forget to call #SetSeed
		after calling this function before use.
		\param rng Random number generator.
	*/
	virtual void Configure(Random* rng) = 0;

	/*!
		Rewind the sampler to the given index.
		Moves the index of the sampler to #index and gets the sampler ready
		to regenerate random numbers after #index.
		The index values can be obtained by #SampleIndex function.
		In the condition of the same seed, this function guarantees
		to generate same sequence of samples after #index.
		\param index Sample index.
		\sa SampleIndex
	*/
	virtual void Rewind(int index) = 0;

	/*!
		Get the current sampler index.
		The value is used for rewinding the sampler.
		\return Current sample index.
		\sa Rewind
	*/
	virtual int SampleIndex() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_REWINDABLE_SAMPLER_H