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
#ifndef LIB_LIGHTMETRICA_RANDOM_H
#define LIB_LIGHTMETRICA_RANDOM_H

#include "component.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

/*!
	Random number generator.
	An interface for uniform random number generators.
*/
class Random : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("random");

public:

	Random() {}
	virtual ~Random() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Random);

public:

	/*!
		Generate pseudorandom number as unsigned integer type.
		\return Generated number.
	*/
	virtual unsigned int NextUInt() = 0;

	/*!
		Set seed and initialize internal state.
		\param seed Seed.
	*/
	virtual void SetSeed(unsigned int seed) = 0;

	/*!
		Clone.
		\return Duplicated instance.
	*/
	virtual Random* Clone() const = 0;

public:

	/*!
		Generate pseudorandom number as floating point type.
		\return Generated number.
	*/
	LM_FORCE_INLINE Math::Float Next();

	/*!
		Generate pseudorandom number as Vec2 type.
		\return Generated number.
	*/
	LM_FORCE_INLINE Math::Vec2 NextVec2();

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#include "random.inl"

#endif // LIB_LIGHTMETRICA_RANDOM_H