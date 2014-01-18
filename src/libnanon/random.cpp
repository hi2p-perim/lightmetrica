/*
	L I G H T  M E T R I C A

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

#include "pch.h"
#include <lightmetrica/random.h>
#include <random>

LM_NAMESPACE_BEGIN

class Random::Impl
{
public:

	Impl() {}
	Impl(unsigned int seed) { SetSeed(seed); }
	Math::Float Next() { return Math::Float(uniformReal(engine)); }
	void SetSeed(unsigned int seed);

private:

	std::mt19937 engine;
	std::uniform_real_distribution<double> uniformReal;

};

void Random::Impl::SetSeed( unsigned int seed )
{
	engine.seed(seed);
	uniformReal.reset();
}

// --------------------------------------------------------------------------------

Random::Random()
	: p(new Impl)
{

}

Random::Random( unsigned int seed )
	: p(new Impl(seed))
{

}

Random::~Random()
{
	LM_SAFE_DELETE(p);
}

Math::Float Random::Next()
{
	return p->Next();
}

void Random::SetSeed( unsigned int seed )
{
	p->SetSeed(seed);
}

LM_NAMESPACE_END
