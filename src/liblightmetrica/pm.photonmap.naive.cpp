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
#include <lightmetrica/pm.photonmap.h>

LM_NAMESPACE_BEGIN

/*!
	Naive photon map.
	Implements photon map in a naive way for debugging.
	This is VERY SLOW.
*/
class NaivePhotonMap final : public PhotonMap
{
public:

	LM_COMPONENT_IMPL_DEF("naive");

public:

	virtual void Build(const Photons& photons) override { this->photons = photons; }
	virtual void CollectPhotons(const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc) const override;
	virtual void GetPhotons(std::vector<const Photon*>& photons) const override;

private:

	Photons photons;

};

void NaivePhotonMap::CollectPhotons( const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc ) const
{
	for (const auto& photon : photons)
	{
		auto dist2 = Math::Length2(photon.p - p);
		if (dist2 < maxDist2)
		{
			collectFunc(p, photon, maxDist2);
		}
	}
}

void NaivePhotonMap::GetPhotons( std::vector<const Photon*>& photons ) const
{
	photons.clear();
	for (const auto& photon : this->photons)
	{
		photons.push_back(&photon);
	}
}

LM_COMPONENT_REGISTER_IMPL(NaivePhotonMap, PhotonMap);

LM_NAMESPACE_END