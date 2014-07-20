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
#include <lightmetrica/scene.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/primitives.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/triaccel.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/aabb.h>
#include <lightmetrica/align.h>
#include <thread>

LM_NAMESPACE_BEGIN

struct BVHNode : public SIMDAlignedType
{

	enum class NodeType
	{
		Leaf,
		Internal
	};

	BVHNode(int begin, int end, const AABB& bound)
		: type(NodeType::Leaf)
		, begin(begin)
		, end(end)
		, bound(bound)
	{

	}

	BVHNode(int splitAxis, const std::shared_ptr<BVHNode>& left, const std::shared_ptr<BVHNode>& right)
		: type(NodeType::Internal)
		, splitAxis(splitAxis)
		, left(left)
		, right(right)
	{
		bound = left->bound.Union(right->bound);
	}

	NodeType type;

	// Leaf node data
	// Primitives index in [begin, end)
	int begin, end;
	AABB bound;

	// Internal node data
	int splitAxis;
	std::shared_ptr<BVHNode> left, right;

};

struct BVHBuildData
{
	// Bounds of the triangles
	std::vector<AABB, aligned_allocator<AABB, std::alignment_of<AABB>::value>> triBounds;
	// Centroids of the bounds of the triangles
	std::vector<Math::Vec3, aligned_allocator<Math::Vec3, std::alignment_of<Math::Vec3>::value>> triBoundCentroids;
};

class CompareToBucket
{
public:

	CompareToBucket(int splitAxis, int numBuckets, int minCostIdx, const BVHBuildData& data, const AABB& centroidBound)
		: splitAxis(splitAxis)
		, numBuckets(numBuckets)
		, minCostIdx(minCostIdx)
		, data(data)
		, centroidBound(centroidBound)
	{

	}

	bool operator()(int i) const
	{
		int bucketIdx =
			Math::Cast<int>(Math::Float(
				Math::Float(numBuckets) * ((data.triBoundCentroids[i][splitAxis] - centroidBound.min[splitAxis])
					/ (centroidBound.max[splitAxis] - centroidBound.min[splitAxis]))));

		bucketIdx = Math::Min(bucketIdx, numBuckets - 1);
		return bucketIdx <= minCostIdx;
	}

private:

	int splitAxis;
	int numBuckets;
	int minCostIdx;
	const BVHBuildData& data;
	const AABB& centroidBound;

};

struct BVHTraversalData
{

	BVHTraversalData(Ray& ray)
		: ray(ray)
	{
		invRayDirMinT = Math::Vec3(
			Math::IsZero(ray.d.x) ? Math::Float(0) : Math::Float(Math::Float(1) / ray.d.x),
			Math::IsZero(ray.d.y) ? Math::Float(0) : Math::Float(Math::Float(1) / ray.d.y),
			Math::IsZero(ray.d.z) ? Math::Float(0) : Math::Float(Math::Float(1) / ray.d.z));

		invRayDirMaxT = Math::Vec3(
			Math::IsZero(ray.d.x) ? Math::Constants::Inf() : Math::Float(Math::Float(1) / ray.d.x),
			Math::IsZero(ray.d.y) ? Math::Constants::Inf() : Math::Float(Math::Float(1) / ray.d.y),
			Math::IsZero(ray.d.z) ? Math::Constants::Inf() : Math::Float(Math::Float(1) / ray.d.z));

		rayDirNegative = Math::Vec3i(
			ray.d.x < Math::Float(0),
			ray.d.y < Math::Float(0),
			ray.d.z < Math::Float(0));
	}

	Ray& ray;
	Math::Vec3i rayDirNegative;			// Each component of the rayDir is negative
	Math::Vec3 invRayDirMinT;				// Inverse of the rayDir (for larger t)
	Math::Vec3 invRayDirMaxT;				// Inverse of the rayDir (for smaller t)

	// The following data is filled when intersected
	unsigned int intersectedTriIdx;		// Intersected triangle index
	Math::Vec2 intersectedTriB;			// Intersected triangle's barycentric coordinates

};

// --------------------------------------------------------------------------------

/*!
	BVH scene.
	Naive bounding volume hierarchy implementation.
	Based on pbrt's BVH implementation.
*/
class BVHScene : public Scene
{
public:

	LM_COMPONENT_IMPL_DEF("bvh");

public:

	BVHScene() : maxTriInNode(255) {}

public:

	virtual bool Build();
	virtual bool Intersect(Ray& ray, Intersection& isect) const;
	virtual boost::signals2::connection Connect_ReportBuildProgress(const std::function<void (double, bool)>& func) { return signal_ReportBuildProgress.connect(func); }
	virtual bool Configure( const ConfigNode& node ) { return true; }

private:

	bool Intersect(const std::shared_ptr<BVHNode>& node, BVHTraversalData& data) const;
	bool Intersect(const AABB& bound, BVHTraversalData& data) const;
	std::shared_ptr<BVHNode> Build(const BVHBuildData& data, int begin, int end);
	void LoadPrimitives(const std::string& scenePath);

private:

	// The function is called when a leaf node is created
	// report progress w.r.t. # of triangles fixed as leafs
	void ReportProgress(int begin, int end);
	void ResetProgress();

private:

	const int maxTriInNode;
	std::vector<int> bvhTriIndices;
	std::shared_ptr<BVHNode> root;
	std::vector<TriAccel> triAccels;
	boost::signals2::signal<void (double, bool)> signal_ReportBuildProgress;
	int numProcessedTris;

};

bool BVHScene::Build()
{
	BVHBuildData data;

	{
		LM_LOG_INFO("Creating triaccels");
		LM_LOG_INDENTER();

		for (int i = 0; i < primitives->NumPrimitives(); i++)
		{
			const auto* primitive = primitives->PrimitiveByIndex(i);
			const auto* mesh = primitive->mesh;
			if (mesh)
			{
				// Enumerate all triangles and create triaccels
				const auto* positions = mesh->Positions();
				const auto* faces = mesh->Faces();
				for (int j = 0; j < mesh->NumFaces() / 3; j++)
				{
					int triIdx = static_cast<int>(triAccels.size());

					// Create triaccel
					triAccels.push_back(TriAccel());
					triAccels.back().shapeIndex = j;
					triAccels.back().primIndex = i;
					unsigned int i1 = faces[3*j  ];
					unsigned int i2 = faces[3*j+1];
					unsigned int i3 = faces[3*j+2];
					Math::Vec3 p1(primitive->transform * Math::Vec4(positions[3*i1], positions[3*i1+1], positions[3*i1+2], Math::Float(1)));
					Math::Vec3 p2(primitive->transform * Math::Vec4(positions[3*i2], positions[3*i2+1], positions[3*i2+2], Math::Float(1)));
					Math::Vec3 p3(primitive->transform * Math::Vec4(positions[3*i3], positions[3*i3+1], positions[3*i3+2], Math::Float(1)));
					triAccels.back().Load(p1, p2, p3);

					// Initial index
					bvhTriIndices.push_back(triIdx);

					// Create primitive bound from points
					AABB triBound(p1, p2);
					triBound = triBound.Union(p3);

					data.triBounds.push_back(triBound);
					data.triBoundCentroids.push_back((triBound.min + triBound.max) * Math::Float(0.5));
				}
			}
		}

		LM_LOG_INFO("Successfully created " + std::to_string(triAccels.size()) + " triaccels");
	}

	// Build BVH
	{
		LM_LOG_INFO("Building BVH");
		LM_LOG_INDENTER();

		ResetProgress();

		auto start = std::chrono::high_resolution_clock::now();
		root = Build(data, 0, static_cast<int>(triAccels.size()));
		auto end = std::chrono::high_resolution_clock::now();

		double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.0;
		LM_LOG_INFO("Completed in " + std::to_string(elapsed) + " seconds");
	}

	return true;
}

std::shared_ptr<BVHNode> BVHScene::Build( const BVHBuildData& data, int begin, int end )
{
	std::shared_ptr<BVHNode> node;

	// Bound of the primitive [begin, end)
	AABB bound;
	for (int i = begin; i < end; i++)
	{
		bound = bound.Union(data.triBounds[bvhTriIndices[i]]);
	}

	// Number of primitives in the node
	int numPrimitives = end - begin;
	if (numPrimitives == 1)
	{
		// Leaf node
		node = std::shared_ptr<BVHNode>(new BVHNode(begin, end, bound));
		ReportProgress(begin, end);
	}
	else
	{
		// Internal node

		// Choose the axis to split
		AABB centroidBound;
		for (int i = begin; i < end; i++)
		{
			centroidBound = centroidBound.Union(data.triBoundCentroids[bvhTriIndices[i]]);
		}

		// Select split axis
		int splitAxis = centroidBound.LongestAxis();

		// If the centroid bound according to the split axis is degenerated, take the node as a leaf.
		if (centroidBound.min[splitAxis] == centroidBound.max[splitAxis])
		{
			node = std::shared_ptr<BVHNode>(new BVHNode(begin, end, bound));
			ReportProgress(begin, end);
		}
		else
		{
			// Split primitives using SAH, surface area heuristic.
			// Considering all possible partitions is rather heavy in the computation cost,
			// so in the application the primitives is separated to some buckets according to the split axis
			// and reduce the combination of the partitions.

			// Create buckets
			const int numBuckets = 12;
			AABB bucketTriBound[numBuckets];
			int bucketTriCount[numBuckets] = {0};
			for (int i = begin; i < end; i++)
			{
				int bucketIdx = Math::Cast<int>(Math::Float(
					Math::Float(numBuckets) * ((data.triBoundCentroids[bvhTriIndices[i]][splitAxis] - centroidBound.min[splitAxis])
						/ (centroidBound.max[splitAxis] - centroidBound.min[splitAxis]))));

				bucketIdx = Math::Min(bucketIdx, numBuckets - 1);
				bucketTriCount[bucketIdx]++;
				bucketTriBound[bucketIdx] = bucketTriBound[bucketIdx].Union(data.triBounds[bvhTriIndices[i]]);
			}

			// For each partition compute the cost
			// Note that the number of possible partitions is numBuckets - 1
			Math::Float costs[numBuckets - 1];
			for (int i = 0; i < numBuckets - 1; i++)
			{
				AABB b1;
				AABB b2;
				int count1 = 0;
				int count2 = 0;

				// [0, i]
				for (int j = 0; j <= i; j++)
				{
					b1 = b1.Union(bucketTriBound[j]);
					count1 += bucketTriCount[j];
				}

				// (i, numBuckets - 1]
				for (int j = i + 1; j < numBuckets; j++)
				{
					b2 = b2.Union(bucketTriBound[j]);
					count2 += bucketTriCount[j];
				}

				// Assume the intersection cost is 1 and traversal cost is 1/8.
				costs[i] = Math::Float(0.125) + (Math::Float(count1) * b1.SurfaceArea() + Math::Float(count2) * b2.SurfaceArea()) / bound.SurfaceArea();
			}

			// Find minimum partition
			int minCostIdx = 0;
			double minCost = costs[0];
			for (int i = 1; i < numBuckets - 1; i++)
			{
				if (minCost > costs[i])
				{
					minCost = costs[i];
					minCostIdx = i;
				}
			}

			// Partition if the minimum cost is lower than the leaf cost (numPrimitives)
			// or the current number of primitives is higher than the limit.
			if (minCost < Math::Float(numPrimitives) || numPrimitives > maxTriInNode)
			{
				int mid = static_cast<int>(std::partition(&bvhTriIndices[begin], &bvhTriIndices[end - 1] + 1, CompareToBucket(splitAxis, numBuckets, minCostIdx, data, centroidBound)) - &bvhTriIndices[0]);
				node = std::shared_ptr<BVHNode>(new BVHNode(splitAxis, Build(data, begin, mid), Build(data, mid, end)));
			}
			else
			{
				// Otherwise make leaf node
				node = std::shared_ptr<BVHNode>(new BVHNode(begin, end, bound));
				ReportProgress(begin, end);
			}
		}
	}

	return node;
}

bool BVHScene::Intersect( Ray& ray, Intersection& isect ) const
{
	BVHTraversalData data(ray);

	if (Intersect(root, data))
	{
		// Store required data for the intersection structure
		auto& triAccel = triAccels[data.intersectedTriIdx];
		StoreIntersectionFromBarycentricCoords(triAccel.primIndex, triAccel.shapeIndex, ray, data.intersectedTriB, isect);
		return true;
	}

	return false;
}

bool BVHScene::Intersect( const std::shared_ptr<BVHNode>& node, BVHTraversalData& data ) const
{
	bool intersected = false;

	// Check intersection to the node bound
	if (Intersect(node->bound, data))
	{
		if (node->type == BVHNode::NodeType::Leaf)
		{
			// Leaf node
			// Intersection with the primitives hold in the node
			for (int i = node->begin; i < node->end; i++)
			{
				Math::Float t;
				Math::Vec2 b;
				if (triAccels[bvhTriIndices[i]].Intersect(data.ray, data.ray.minT, data.ray.maxT, b[0], b[1], t))
				{
					data.ray.maxT = t;
					data.intersectedTriIdx = bvhTriIndices[i];
					data.intersectedTriB = b;
					intersected = true;
				}
			}
		}
		else
		{
			// Internal node
			// Traverse the right node first if the ray direction according to the
			// split axis is negative.
			if (data.rayDirNegative[node->splitAxis])
			{
				intersected |= Intersect(node->right, data);
				intersected |= Intersect(node->left, data);
			}
			else
			{
				intersected |= Intersect(node->left, data);
				intersected |= Intersect(node->right, data);
			}
		}
	}

	return intersected;
}

bool BVHScene::Intersect( const AABB& bound, BVHTraversalData& data ) const
{
	auto& rayDirNegative = data.rayDirNegative;
	auto& invRayDirMinT = data.invRayDirMinT;
	auto& invRayDirMaxT = data.invRayDirMaxT;
	auto& ray = data.ray;

	Math::Float tmin  = (bound[    rayDirNegative[0]].x - ray.o.x) * invRayDirMinT.x;
	Math::Float tmax  = (bound[1 - rayDirNegative[0]].x - ray.o.x) * invRayDirMaxT.x;

	Math::Float tymin = (bound[    rayDirNegative[1]].y - ray.o.y) * invRayDirMinT.y;
	Math::Float tymax = (bound[1 - rayDirNegative[1]].y - ray.o.y) * invRayDirMaxT.y;
	if ((tmin > tymax) || (tymin > tmax)) return false;
	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	Math::Float tzmin = (bound[    rayDirNegative[2]].z - ray.o.z) * invRayDirMinT.z;
	Math::Float tzmax = (bound[1 - rayDirNegative[2]].z - ray.o.z) * invRayDirMaxT.z;
	if ((tmin > tzmax) || (tzmin > tmax)) return false;
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;

	return (tmin < ray.maxT) && (tmax > ray.minT);
}

void BVHScene::ReportProgress( int begin, int end )
{
	numProcessedTris += end - begin;
	signal_ReportBuildProgress(static_cast<double>(numProcessedTris) / triAccels.size(), numProcessedTris == static_cast<int>(triAccels.size()));
}

void BVHScene::ResetProgress()
{
	numProcessedTris = 0;
	signal_ReportBuildProgress(0, false);
}

LM_COMPONENT_REGISTER_IMPL(BVHScene, Scene);

LM_NAMESPACE_END
