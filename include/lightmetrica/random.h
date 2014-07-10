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