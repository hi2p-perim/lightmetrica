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
#include <lightmetrica/pm.photonmap.h>

LM_NAMESPACE_BEGIN

/*!
	Naive photon map.
	Implements photon map in a naive way for debugging.
	This is VERY SLOW.
*/
class NaivePhotonMap : public PhotonMap
{
public:

	LM_COMPONENT_IMPL_DEF("naive");

public:

	virtual void Build(const Photons& photons) { this->photons = photons; }
	virtual void CollectPhotons(const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc) const;
	virtual void GetPhotons(std::vector<const Photon*>& photons) const;

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