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
#ifndef LIB_LIGHTMETRICA_REWINDABLE_SAMPLER_H
#define LIB_LIGHTMETRICA_REWINDABLE_SAMPLER_H

#include "sampler.h"
#include "random.h"

LM_NAMESPACE_BEGIN

class Random;

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