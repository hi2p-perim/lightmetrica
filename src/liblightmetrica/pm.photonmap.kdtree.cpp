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
#include <lightmetrica/aabb.h>

LM_NAMESPACE_BEGIN

/*!
	Kd-tree node.
	Compressed Kd-tree node for photon map.
*/
struct PhotonKdTreeNode
{

	// Initialize as internal node
	void InitializeIntermediateNode(const Math::Float& p, unsigned int axis)
	{
		splitPos = p;
		splitAxis = axis;
		hasLeftChild = 0;
		rightChild = (1<<29)-1;
	}

	// Initialize as leaf node
	void InitializeLeaf()
	{
		splitAxis = 3;
		rightChild = (1<<29)-1;
		hasLeftChild = 0;
	}

	bool Leaf() const { return splitAxis == 3; }

	Math::Float splitPos;
	
	// 0 - split axis is X
	// 1 - split axis is Y
	// 2 - split axis is Z
	// 3 - leaf node
	unsigned int splitAxis		: 2;
	unsigned int hasLeftChild	: 1;
	unsigned int rightChild		: 29;

};

/*!
	Kd-tree photon map.
	Implements photon map with Kd-tree.
*/
class KdTreePhotonMap : public PhotonMap
{
public:

	LM_COMPONENT_IMPL_DEF("kdtree");

public:

	virtual void Build(const Photons& photons);
	virtual void CollectPhotons(const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc) const;
	virtual void GetPhotons(std::vector<const Photon*>& photons) const;

private:

	void RecursiveBuild(size_t nodeIndex, int start, int end, const Photons& photons, std::vector<size_t>& photonIndices, unsigned int& nextNodeIndex);
	void RecursiveCollectPhotons(size_t nodeIndex, const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc) const;

public:

	std::vector<PhotonKdTreeNode> nodes;
	Photons data;

};

void KdTreePhotonMap::Build( const Photons& photons )
{
	// Reverse node and data store
	nodes.assign(photons.size(), PhotonKdTreeNode());
	data.assign(photons.size(), Photon());

	// Temporary array
	std::vector<size_t> photonIndices(photons.size());
	for (size_t i = 0; i < photons.size(); i++)
	{
		photonIndices[i] = i;
	}

	// Build recursively
	unsigned int nextNodeIndex = 1;
	RecursiveBuild(0, 0, static_cast<int>(photons.size()), photons, photonIndices, nextNodeIndex);
}

void KdTreePhotonMap::RecursiveBuild( size_t nodeIndex, int start, int end, const Photons& photons, std::vector<size_t>& photonIndices, unsigned int& nextNodeIndex )
{
	// Leaf node
	if (start + 1 == end)
	{
		nodes[nodeIndex].InitializeLeaf();
		data[nodeIndex] = photons[photonIndices[start]];
		return;
	}

	// Split axis and position
	AABB bound;
	for (int i = start; i < end; i++)
	{
		bound = bound.Union(photons[photonIndices[i]].p);
	}
	int splitAxis = bound.LongestAxis();
	int splitPos = (start + end) / 2;
	std::nth_element(&photonIndices[start], &photonIndices[splitPos], &photonIndices[end-1]+1, [&](size_t i1, size_t i2)
	{
		const auto& p1 = photons[i1];
		const auto& p2 = photons[i2];
		return p1.p[splitAxis] == p2.p[splitAxis] ? i1 < i2 : p1.p[splitAxis] < p2.p[splitAxis];
	});
	
	// Create intermediate node
	const auto& splitPhoton = photons[photonIndices[splitPos]];
	nodes[nodeIndex].InitializeIntermediateNode(splitPhoton.p[splitAxis], splitAxis);
	data[nodeIndex] = splitPhoton;

	// Continue recursively
	if (start < splitPos)
	{
		nodes[nodeIndex].hasLeftChild = 1;
		RecursiveBuild(nextNodeIndex++, start, splitPos, photons, photonIndices, nextNodeIndex);
	}
	if (splitPos + 1 < end)
	{
		nodes[nodeIndex].rightChild = nextNodeIndex++;
		RecursiveBuild(nodes[nodeIndex].rightChild, splitPos+1, end, photons, photonIndices, nextNodeIndex);
	}
}

void KdTreePhotonMap::CollectPhotons( const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc ) const
{
	RecursiveCollectPhotons(0, p, maxDist2, collectFunc);
}

void KdTreePhotonMap::RecursiveCollectPhotons( size_t nodeIndex, const Math::Vec3& p, Math::Float& maxDist2, const PhotonCollectFunc& collectFunc ) const
{
	const auto& node = nodes[nodeIndex];
	if (!node.Leaf())
	{
		// Process children
		int axis = node.splitAxis;
		auto dist2 = (p[axis] - node.splitPos) * (p[axis] - node.splitPos);
		if (p[axis] <= node.splitPos)
		{
			// Query point is located on left half -> left points are nearer
			if (node.hasLeftChild)
			{
				// Left children
				RecursiveCollectPhotons(nodeIndex + 1, p, maxDist2, collectFunc);
			}
			if (dist2 < maxDist2 && node.rightChild < static_cast<unsigned int>(nodes.size()))
			{
				// Right children
				// Distances to the all photons in right half is no less than #dist2
				RecursiveCollectPhotons(node.rightChild, p, maxDist2, collectFunc);
			}
		}
		else
		{
			// Query point is located on right half -> right points are nearer
			if (node.rightChild < static_cast<unsigned int>(nodes.size()))
			{
				// Right children
				RecursiveCollectPhotons(node.rightChild, p, maxDist2, collectFunc);
			}
			if (dist2 < maxDist2 && node.hasLeftChild)
			{
				// Left children
				// Distances to the all photons in left half is no less than #dist2
				RecursiveCollectPhotons(nodeIndex + 1, p, maxDist2, collectFunc);
			}
		}
	}

	// Dispatch photon collect function
	auto dist2 = Math::Length2(data[nodeIndex].p - p);
	if (dist2 < maxDist2)
	{
		collectFunc(p, data[nodeIndex], maxDist2);
	}
}

void KdTreePhotonMap::GetPhotons( std::vector<const Photon*>& photons ) const
{
	photons.clear();
	for (const auto& photon : data)
	{
		photons.push_back(&photon);
	}
}

LM_COMPONENT_REGISTER_IMPL(KdTreePhotonMap, PhotonMap);

LM_NAMESPACE_END