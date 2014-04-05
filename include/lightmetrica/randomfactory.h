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
#ifndef LIB_LIGHTMETRICA_RANDOM_FACTORY_H
#define LIB_LIGHTMETRICA_RANDOM_FACTORY_H

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN

class Random;

/*!
	Random number generator factory.
	A factory class for random number generator implementations.
*/
class LM_PUBLIC_API RandomFactory
{
private:

	RandomFactory() {}
	LM_DISABLE_COPY_AND_MOVE(RandomFactory);

public:

	/*!
		Create instance of a random number generator.
		The function creates an instance of the random number generator specified by #type.
		\param type Type of the random number generator.
		\return Instance.
	*/
	static Random* Create(const std::string& type);

	/*!
		Check support.
		Checks if given #type of random number generator is supported.
		\param type Type of the random number generator.
		\retval true #type is supported.
		\retval false #type is not supported.
	*/
	static bool CheckSupport(const std::string& type);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RANDOM_FACTORY_H