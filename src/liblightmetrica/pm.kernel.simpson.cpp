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
	Simpson's kernel.
	Photon density estimation kernel implementation with Simpson's kernel.
*/
class SimpsonPDEKernel final : public PhotonDensityEstimationKernel
{
public:

	LM_COMPONENT_IMPL_DEF("simpson");

public:

	virtual Math::Float Evaluate(const Math::Vec3& p, const Photon& photon, const Math::Float& maxDist2) const override
	{
		auto s = Math::Float(1) - Math::Length2(photon.p - p) / maxDist2;
		return Math::Float(3) * Math::Constants::InvPi() * s * s;
	}

};

LM_COMPONENT_REGISTER_IMPL(SimpsonPDEKernel, PhotonDensityEstimationKernel);

LM_NAMESPACE_END