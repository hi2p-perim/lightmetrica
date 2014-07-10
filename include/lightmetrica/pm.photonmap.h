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
#ifndef LIB_LIGHTMETRICA_PM_PHOTONMAP_H
#define LIB_LIGHTMETRICA_PM_PHOTONMAP_H

#include "common.h"
#include "component.h"
#include "pm.photon.h"
#include <functional>

LM_NAMESPACE_BEGIN

/*!
	Photon map.
	Interface for photon map.
*/
class PhotonMap : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("pm.photonmap");

public:

	PhotonMap() {}
	virtual ~PhotonMap() {}

private:

	LM_DISABLE_COPY_AND_MOVE(PhotonMap);

public:

	//! Function called when a photon is collected in CollectPhotons
	typedef std::function<void (const Math::Vec3&, const Photon&, Math::Float&)> PhotonCollectFunc;

public:

	virtual void Build(const Photons& photons) = 0;
	virtual void CollectPhotons(const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc) const = 0;
	virtual void GetPhotons(std::vector<const Photon*>& photons) const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PM_PHOTONMAP_H