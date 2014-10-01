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

#include "pch.h"
#include <lightmetrica/pm.kernel.h>
#include <lightmetrica/pm.photon.h>

LM_NAMESPACE_BEGIN

/*!
	Gaussian filter.
	Photon density estimation kernel implementation with Gaussian filter.
*/
class GaussianFilterPDEKernel final : public PhotonDensityEstimationKernel
{
public:

	LM_COMPONENT_IMPL_DEF("gaussian");

public:

	virtual Math::Float Evaluate(const Math::Vec3& p, const Photon& photon, const Math::Float& maxDist2) const override
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