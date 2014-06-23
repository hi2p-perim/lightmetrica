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
	Kd-tree photon map.
	Implements photon map with Kd-tree.
*/
class KdTreePhotonMap : public PhotonMap
{
public:

	LM_COMPONENT_IMPL_DEF("kdtree");

public:

	virtual void Build(const std::vector<Photon>& photons);
	virtual void CollectPhotons(int n, const Math::Vec3& p, std::vector<const Photon*>& collectedPhotons, Math::Float& maxDist2) const;
	virtual void GetPhotons(std::vector<const Photon*>& photons) const;

};

void KdTreePhotonMap::Build( const std::vector<Photon>& photons )
{

}

void KdTreePhotonMap::CollectPhotons( int n, const Math::Vec3& p, std::vector<const Photon*>& collectedPhotons, Math::Float& maxDist2 ) const
{

}

void KdTreePhotonMap::GetPhotons( std::vector<const Photon*>& photons ) const
{

}

LM_COMPONENT_REGISTER_IMPL(KdTreePhotonMap, PhotonMap);

LM_NAMESPACE_END