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
#ifndef LIB_LIGHTMETRICA_PM_PHOTON_H
#define LIB_LIGHTMETRICA_PM_PHOTON_H

#include "common.h"
#include "math.types.h"
#include "align.h"

LM_NAMESPACE_BEGIN

/*!
	Photon.
	Represents single photon.
*/
struct Photon
{
	Math::Vec3 p;				// Surface point
	Math::Vec3 throughput;		// Current throughput
	Math::Vec3 wi;				// Incident ray direction
};

// Vector type for Photon
typedef std::vector<Photon, aligned_allocator<Photon, std::alignment_of<Photon>::value>> Photons;

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PM_PHOTON_H