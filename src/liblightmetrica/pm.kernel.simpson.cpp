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
#include <lightmetrica/pm.kernel.h>
#include <lightmetrica/pm.photon.h>

LM_NAMESPACE_BEGIN

/*!
	Simpson's kernel.
	Photon density estimation kernel implementation with Simpson's kernel.
*/
class SimpsonPDEKernel : public PhotonDensityEstimationKernel
{
public:

	LM_COMPONENT_IMPL_DEF("simpson");

public:

	virtual Math::Float Evaluate( const Math::Vec3& p, const Photon& photon, const Math::Float& maxDist2 ) const
	{
		auto s = Math::Float(1) - Math::Length2(photon.p - p) / maxDist2;
		return Math::Float(3) * Math::Constants::InvPi() * s * s;
	}

};

LM_COMPONENT_REGISTER_IMPL(SimpsonPDEKernel, PhotonDensityEstimationKernel);

LM_NAMESPACE_END