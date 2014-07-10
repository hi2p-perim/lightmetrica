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
#ifndef LIB_LIGHTMETRICA_PM_KERNEL_H
#define LIB_LIGHTMETRICA_PM_KERNEL_H

#include "common.h"
#include "component.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

struct Photon;

/*!
	Photon density estimation (PDE) kernel.
	An interface for PDE kernels.
*/
class PhotonDensityEstimationKernel : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("pm.kernel");

public:

	PhotonDensityEstimationKernel() {}
	virtual ~PhotonDensityEstimationKernel() {}

private:

	LM_DISABLE_COPY_AND_MOVE(PhotonDensityEstimationKernel);

public:

	/*!
		Evaluate the kernel.
		\param p Query position.
		\param photon Target photon
		\param maxDist2 Square of max distance obtained in k-NN query
		\return Evaluated kernel value.
	*/
	virtual Math::Float Evaluate(const Math::Vec3& p, const Photon& photon, const Math::Float& maxDist2) const = 0;
	
};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PM_KERNEL_H