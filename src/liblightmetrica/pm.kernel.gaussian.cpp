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
	Gaussian filter.
	Photon density estimation kernel implementation with Gaussian filter.
*/
class GaussianFilterPDEKernel : public PhotonDensityEstimationKernel
{
public:

	LM_COMPONENT_IMPL_DEF("gaussian");

public:

	virtual Math::Float Evaluate( const Math::Vec3& p, const Photon& photon, const Math::Float& maxDist2 ) const
	{
		const auto Alpha = Math::Float(1.818);
		const auto Beta = Math::Float(1.953);
		const auto BetaExp = Math::Exp(-Beta);
		auto dist2 = Math::Length2(p - photon.p);
		auto t = Math::Float(1) - Math::Exp(-Beta * dist2 / (Math::Float(2) * maxDist2));
		return Alpha * (Math::Float(1) - t / (Math::Float(1) - BetaExp)) * Math::Constants::InvPi();
	}

};

LM_COMPONENT_REGISTER_IMPL(GaussianFilterPDEKernel, PhotonDensityEstimationKernel);

LM_NAMESPACE_END