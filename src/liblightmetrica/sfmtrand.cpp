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
class LM_PUBLIC_API SFMTRandom : public Random
{
public:

	LM_COMPONENT_IMPL_DEF("sfmt");

public:

	virtual unsigned int NextUInt() { return sfmt_genrand_uint32(&sfmt); }
	virtual void SetSeed( unsigned int seed ) { sfmt_init_gen_rand(&sfmt, seed); }

private:

	sfmt_t sfmt;

};

LM_COMPONENT_REGISTER_IMPL(SFMTRandom);

LM_NAMESPACE_END

// --------------------------------------------------------------------------------

/*
class SFMTRandom::Impl : public SIMDAlignedType
{
public:

	static const int BlockSize = 100000;

public:

	Impl();

public:

	unsigned int NextUInt();
	void SetSeed( unsigned int seed );

//private:
//
//	void GenerateSamples();

private:

	sfmt_t sfmt;
	//std::unique_ptr<w128_t, std::function<void (w128_t*)>> buffer;
	//int bufferSampleIndex;

};

SFMTRandom::Impl::Impl()
	//: buffer(
	//	(w128_t*)aligned_malloc(sizeof(w128_t) * BlockSize / 4, std::alignment_of<w128_t>::value),
	//	[](w128_t* p){ aligned_free(p); })
	//, bufferSampleIndex(0)
{
	//LM_ASSERT(sfmt_get_min_array_size32(&sfmt) <= BlockSize);
}

unsigned int SFMTRandom::Impl::NextUInt()
{
	return sfmt_genrand_uint32(&sfmt);

	//if (bufferSampleIndex >= BlockSize)
	//{
	//	GenerateSamples();
	//}
	//
	//auto v = buffer.get()[bufferSampleIndex / 4].u[bufferSampleIndex % 4];
	//bufferSampleIndex++;
	//return v;
}

void SFMTRandom::Impl::SetSeed( unsigned int seed )
{
	sfmt_init_gen_rand(&sfmt, seed);
	//GenerateSamples();
}

//void SFMTRandom::Impl::GenerateSamples()
//{
//	sfmt_fill_array32(&sfmt, (uint32_t*)buffer.get(), BlockSize);
//	bufferSampleIndex = 0;
//}

// --------------------------------------------------------------------------------

SFMTRandom::SFMTRandom()
	: p(new Impl)
{

}

SFMTRandom::~SFMTRandom()
{
	LM_SAFE_DELETE(p);
}

unsigned int SFMTRandom::NextUInt()
{
	return p->NextUInt();
}

void SFMTRandom::SetSeed( unsigned int seed )
{
	p->SetSeed(seed);
}
*/