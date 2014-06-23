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

	virtual void Build(const std::vector<Photon>& photons) { this->photons = photons; }
	virtual void CollectPhotons(int n, const Math::Vec3& p, std::vector<const Photon*>& collectedPhotons, Math::Float& maxDist2) const;
	virtual void GetPhotons(std::vector<const Photon*>& photons) const;

private:

	std::vector<Photon> photons;

};

void NaivePhotonMap::CollectPhotons( int n, const Math::Vec3& p, std::vector<const Photon*>& collectedPhotons, Math::Float& maxDist2 ) const
{
	std::vector<size_t> photonIdx(photons.size());
	for (size_t i = 0; i < photons.size(); i++)
	{
		photonIdx[i] = i;
	}

	std::sort(photonIdx.begin(), photonIdx.end(), [&](size_t v1, size_t v2)
	{
		return Math::Length2(photons[v1].p - p) < Math::Length2(photons[v2].p - p);
	});

	int found = 0;
	for (size_t i : photonIdx)
	{
		collectedPhotons.push_back(&photons.at(i));
		if (++found >= n)
		{
			break;
		}
	}

	maxDist2 = Math::Length2(collectedPhotons.back()->p - p);
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