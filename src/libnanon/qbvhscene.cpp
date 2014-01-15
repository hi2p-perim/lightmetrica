/*
	nanon : A research-oriented renderer

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
#include "simdsupport.h"
#include <nanon/qbvhscene.h>
#include <nanon/logger.h>
#include <nanon/triaccel.h>
#include <nanon/primitive.h>
#include <nanon/trianglemesh.h>
#include <nanon/aabb.h>
#include <nanon/align.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

// Quad ray structure in SOA format
struct NANON_ALIGN_16 Ray4
{

	__m128 ox, oy, oz;
	__m128 dx, dy, dz;
	__m128 minT, maxT;

	NANON_FORCE_INLINE Ray4(const Ray& ray)
	{
		ox = _mm_set1_ps(ray.o.x);
		oy = _mm_set1_ps(ray.o.y);
		oz = _mm_set1_ps(ray.o.z);
		dx = _mm_set1_ps(ray.d.x);
		dy = _mm_set1_ps(ray.d.y);
		dz = _mm_set1_ps(ray.d.z);
		minT = _mm_set1_ps(ray.minT);
		maxT = _mm_set1_ps(ray.maxT);
	}

};

// Quad triangle structure for SSE optimized triangle intersection
struct QuadTriangle : public Object
{

	__m128 origx, origy, origz;
	__m128 edge1x, edge1y, edge1z;
	__m128 edge2x, edge2y, edge2z;

	int elementIndex;
	int primitiveIndex[4];

	/*
		Load triangles.
		\param positions 3*4 = 12 elements of triangle positions.
	*/
	NANON_FORCE_INLINE void Load(const Math::Vec3* positions)
	{
		for (size_t i = 0; i < 4; i++)
		{
			const auto& p1 = positions[i*3  ];
			const auto& p2 = positions[i*3+1];
			const auto& p3 = positions[i*3*2];
			reinterpret_cast<float*>(&origx)[i] = p1.x;
			reinterpret_cast<float*>(&origy)[i] = p1.y;
			reinterpret_cast<float*>(&origz)[i] = p1.z;
			reinterpret_cast<float*>(&edge1x)[i] = p2.x - p1.x;
			reinterpret_cast<float*>(&edge1y)[i] = p2.y - p1.y;
			reinterpret_cast<float*>(&edge1z)[i] = p2.z - p1.z;
			reinterpret_cast<float*>(&edge2x)[i] = p3.x - p1.x;
			reinterpret_cast<float*>(&edge2y)[i] = p3.y - p1.y;
			reinterpret_cast<float*>(&edge2z)[i] = p3.z - p1.z;
		}
	}

	/*
		Intersection query.
		\param ray4 Quad ray structure.
		\param ray Ray structure.
	*/
	NANON_FORCE_INLINE bool Intersect(Ray4& ray4, Ray& ray, Math::Vec2& resultB)
	{
		// Check 4 intersections simultaneously
		const __m128 zero = _mm_set1_ps(0.f);
		const __m128 s1x = _mm_sub_ps(_mm_mul_ps(ray4.dy, edge2z), _mm_mul_ps(ray4.dz, edge2y));
		const __m128 s1y = _mm_sub_ps(_mm_mul_ps(ray4.dz, edge2x), _mm_mul_ps(ray4.dx, edge2z));
		const __m128 s1z = _mm_sub_ps(_mm_mul_ps(ray4.dx, edge2y), _mm_mul_ps(ray4.dy, edge2x));
		const __m128 divisor = _mm_add_ps(_mm_mul_ps(s1x, edge1x), _mm_add_ps(_mm_mul_ps(s1y, edge1y), _mm_mul_ps(s1z, edge1z)));
		__m128 intersected = _mm_cmpneq_ps(divisor, zero);
		const __m128 dx = _mm_sub_ps(ray4.ox, origx);
		const __m128 dy = _mm_sub_ps(ray4.oy, origy);
		const __m128 dz = _mm_sub_ps(ray4.oz, origz);
		const __m128 b1 = _mm_div_ps(_mm_add_ps(_mm_mul_ps(dx, s1x), _mm_add_ps(_mm_mul_ps(dy, s1y), _mm_mul_ps(dz, s1z))), divisor);
		intersected = _mm_and_ps(intersected, _mm_cmpge_ps(b1, zero));
		const __m128 s2x = _mm_sub_ps(_mm_mul_ps(dy, edge1z), _mm_mul_ps(dz, edge1y));
		const __m128 s2y = _mm_sub_ps(_mm_mul_ps(dz, edge1x), _mm_mul_ps(dx, edge1z));
		const __m128 s2z = _mm_sub_ps(_mm_mul_ps(dx, edge1y), _mm_mul_ps(dy, edge1x));
		const __m128 b2 = _mm_div_ps(_mm_add_ps(_mm_mul_ps(ray4.dx, s2x), _mm_add_ps(_mm_mul_ps(ray4.dy, s2y), _mm_mul_ps(ray4.dz, s2z))), divisor);
		const __m128 b0 = _mm_sub_ps(_mm_set1_ps(1.f), _mm_add_ps(b1, b2));
		intersected = _mm_and_ps(intersected, _mm_and_ps(_mm_cmpge_ps(b2, zero), _mm_cmpge_ps(b0, zero)));
		const __m128 t = _mm_div_ps(_mm_add_ps(_mm_mul_ps(edge2x, s2x), _mm_add_ps(_mm_mul_ps(edge2y, s2y), _mm_mul_ps(edge2z, s2z))), divisor);
		intersected = _mm_and_ps(intersected, _mm_and_ps(_mm_cmpgt_ps(t, ray4.minT), _mm_cmplt_ps(t, ray4.maxT)));

		// Find nearest one among at most 4 intersected triangles
		unsigned int hit = 4;
		for (unsigned int i = 0; i < 4; ++i)
		{
			if (reinterpret_cast<int*>(&intersected)[i] && reinterpret_cast<const float*>(&t)[i] < ray.maxT)
			{
				hit = i;
				ray.maxT = reinterpret_cast<const float *>(&t)[i];
			}
		}
		if (hit == 4)
		{
			// No intersection
			return false;
		}

		// Update maximum distance
		ray4.maxT = _mm_set1_ps(ray.maxT);

		// Store information needed to fill an intersection structure
		resultB = Math::Vec2(
			reinterpret_cast<const float*>(&b1)[hit],
			reinterpret_cast<const float *>(&b2)[hit]);

		return true;
	}

};

// QBVH node (128 bytes)
struct QBVHNode
{
	
	// Constant which indicates a empty leaf node
	static const int EmptyLeafNode = 0xffffffff;

	/*
		Bounds for 4 nodes in SOA format.
			bounds[0][0] : b[0].min for 4 children
			bounds[1][0] : b[0].max for 4 children
			...
			bounds[0][2] : b[2].min for 4 children
			bounds[1][2] : b[2].max for 4 children
	*/
	__m128 bounds[2][3];

	/*
		Child nodes
		If the node is a leaf, the reference to the primitive is encoded to
			[31:31] : 1
			[30:27] : # of triangles in the leaf
			[26: 0] : An index of the first quad triangles
		If the node is a intermediate node, 
			[31:31] : 0
			[30: 0] : An index of the child node
	*/
	int children[4];

	NANON_FORCE_INLINE QBVHNode()
	{
		for (int i = 0; i < 3; i++)
		{
			bounds[0][i] = _mm_set1_ps(Math::Constants::Inf);
			bounds[1][i] = _mm_set1_ps(-Math::Constants::Inf);
		}
		for (int i = 0; i < 4; i++)
		{
			children[i] = EmptyLeafNode;
		}
	}

	/*
		Set a bound to the node.
		\param childIndex Child index.
		\param bound A bound to be set
	*/
	NANON_FORCE_INLINE void SetBound(int childIndex, const AABB& bound)
	{
		for (int axis = 0; axis < 3; axis++)
		{
			reinterpret_cast<float*>(&(bounds[0][axis]))[childIndex] = bound.min[axis];
			reinterpret_cast<float*>(&(bounds[1][axis]))[childIndex] = bound.max[axis];
		}
	}

	/*
		Initialize a child as a leaf.
		\param childIndex Child index.
		\param size Number of triangles
		\param offset Offset in the triangle list
	*/
	NANON_FORCE_INLINE void InitializeLeaf(int childIndex, unsigned int size, unsigned int offset)
	{
		if (size == 0)
		{
			// Empty node
			children[childIndex] = EmptyLeafNode;
		}
		else
		{
			// Encode
			children[childIndex]  = 0x80000000;
			children[childIndex] |= ((static_cast<int>(size) - 1) & 0xf) << 27;
			children[childIndex] |= static_cast<int>(offset) & 0x07ffffff;
		}
	}

	/*
		Initialize a child as an intermediate node.
		\param childIndex Child index.
		\param index Index of the node.
	*/
	NANON_FORCE_INLINE void InitializeIntermediateNode(int childIndex, unsigned int index)
	{
		children[childIndex] = static_cast<int>(index);
	}

	/*
		Extract encoded child data.
		\param data Input leaf data.
		\param size Extracted size value.
		\param offset Extracted offset value.
	*/
	NANON_FORCE_INLINE static void ExtractLeafData(int data, unsigned int& size, unsigned int& offset)
	{
		size = static_cast<unsigned int>(((data >> 27) & 0xf) + 1);
		offset = data & 0x07ffffff;
	}

	/*
		SSE optimized intersection query.
		#invRayDir and #rayDirNegative must be precomputed beforehand.
		\param ray4 Quad ray.
		\param invRayDir Inverse of ray direction in SOA format.
		\param rayDirSign Specifies the component of the ray direction is negative.
		\return Intersection mask.
	*/
	NANON_FORCE_INLINE int Intersect(const Ray4& ray4, const __m128 invRayDir[3], const int rayDirSign[3])
	{
		__m128 minT = ray4.minT;
		__m128 maxT = ray4.maxT;

		// X coordinate
		minT = _mm_max_ps(minT, _mm_mul_ps(_mm_sub_ps(bounds[rayDirSign[0]][0], ray4.ox), invRayDir[0]));
		maxT = _mm_min_ps(maxT, _mm_mul_ps(_mm_sub_ps(bounds[1 - rayDirSign[0]][0], ray4.ox), invRayDir[0]));

		// Y coordinate
		minT = _mm_max_ps(minT, _mm_mul_ps(_mm_sub_ps(bounds[rayDirSign[1]][1], ray4.oy), invRayDir[1]));
		maxT = _mm_min_ps(maxT, _mm_mul_ps(_mm_sub_ps(bounds[1 - rayDirSign[1]][1], ray4.oy), invRayDir[1]));

		// Z coordinate
		minT = _mm_max_ps(minT, _mm_mul_ps(_mm_sub_ps(bounds[rayDirSign[2]][2], ray4.oz), invRayDir[2]));
		maxT = _mm_min_ps(maxT, _mm_mul_ps(_mm_sub_ps(bounds[1 - rayDirSign[2]][2], ray4.oz), invRayDir[2]));

		return _mm_movemask_ps(_mm_cmpge_ps(maxT, minT));
	}

};

// The structure is used on QBVHScene::Build
struct QBVHBuildData
{
	std::vector<AABB> triBounds;				// Bounds of the triangles
	std::vector<Math::Vec3> triBoundCentroids;	// Centroids of the bounds of the triangles
};

enum class IntersectionMode
{
	SSE,			// Use SSE optimized quad triangles for ray-triangle intersection query
	Triaccel		// Use Triaccels quad triangles for ray-triangle intersection query
};

// --------------------------------------------------------------------------------

class QBVHScene::Impl : public Object
{
public:

	Impl(QBVHScene* self);
	~Impl();

public:

	bool Build();
	bool Intersect( Ray& ray, Intersection& isect ) const;
	boost::signals2::connection Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func) { return signal_ReportBuildProgress.connect(func); }
	bool LoadImpl( const pugi::xml_node& node, const Assets& assets );

private:

	// Build a part of QBVH.
	// [begin, end) is the range of primitive indices.
	// #parent indicates the index of the parent node (specify -1 for building root node)
	// and #child indicates the index of the child node relative to the node specified by #parent.
	void Build(const QBVHBuildData& data, unsigned int begin, unsigned int end, int parent, int child, int depth);

	// Determine the split axis and the position
	// Returns false if the split is failed because the primitive bound is degenerated
	bool SplitAxisAndPosition(const QBVHBuildData& data, unsigned int begin, unsigned int end, int& axis, Math::Float& splitPosition);

	// Rearrange primitives by partition according to the split axis and position
	// splitTriIndex is the boundary primitive index
	void PartitionPrimitives(const QBVHBuildData& data, unsigned int begin, unsigned int end, int axis, Math::Float splitPosition, unsigned int& splitTriIndex);

	void CreateLeafNode(unsigned int begin, unsigned int end, int parent, int child, const AABB& bound);
	void CreateIntermediateNode(int parent, int child, const AABB& bound, unsigned int& createdNodeIndex);

private:

	QBVHScene* self;
	boost::signals2::signal<void (double, bool)> signal_ReportBuildProgress;
	int numProcessedTris;

	IntersectionMode mode;					// Triangle intersection mode
	unsigned int maxElementsInLeaf;			// Maximum # of triangle in a node (4 bit : 64 triangles)

	std::vector<TriAccel> triAccels;		// List of triaccels
	std::vector<QuadTriangle> quadTris;		// List of quad triangles
	std::vector<unsigned int> triIndices;	// List of triangle indices. The list is rearranged through build process.
	std::vector<QBVHNode*> nodes;			// List of QBVH nodes

};

QBVHScene::Impl::Impl( QBVHScene* self )
	: self(self)
	, mode(IntersectionMode::SSE)
	, maxElementsInLeaf(64)
{

}

QBVHScene::Impl::~Impl()
{
	for (auto* node : nodes)
	{
		NANON_SAFE_DELETE(node);
	}
}

bool QBVHScene::Impl::LoadImpl( const pugi::xml_node& node, const Assets& assets )
{
	auto intersectionModeNode = node.child("intersection_mode");
	if (!intersectionModeNode)
	{
		mode = IntersectionMode::SSE;
		NANON_LOG_WARN("Using default value 'intersection_mode' = 'triaccel'");
	}
	else
	{
		if (std::strcmp("sse", intersectionModeNode.child_value()) == 0)
		{
			mode = IntersectionMode::SSE;
		}
		else if (std::strcmp("triaccel", intersectionModeNode.child_value()) == 0)
		{
			mode = IntersectionMode::Triaccel;
		}
		else
		{
			NANON_LOG_ERROR(boost::str(boost::format("Invalid intersection mode '%s'") % intersectionModeNode.child_value()));
			return false;
		}
	}
	if (mode == IntersectionMode::SSE)
	{
		// 2^4 = 16
		maxElementsInLeaf = 64;
	}
	else
	{
		// 2^4 * 4 = 64
		maxElementsInLeaf = 16;
	}

	return true;
}

bool QBVHScene::Impl::Build()
{
	QBVHBuildData data;

	{
		// TODO : replace triaccel with SSE optimized quad triangle intersection
		NANON_LOG_INFO("Creating triangle elements (mode : '" + (mode == IntersectionMode::SSE : "sse" ? "triaccel") + "'");
		NANON_LOG_INDENTER();

		// TODO
		//// Process 4 triangles in a batch
		//Math::Vec3 triPositions[12];
		//int triPrimitiveIndex[4];

		for (int i = 0; i < self->NumPrimitives(); i++)
		{
			const auto* primitive = self->PrimitiveByIndex(i);
			const auto* mesh = primitive->mesh;
			if (mesh)
			{
				// Enumerate all triangles and create triaccels
				const auto* positions = mesh->Positions();
				const auto* faces = mesh->Faces();
				for (int j = 0; j < mesh->NumFaces() / 3; j++)
				{
					unsigned int triIdx = static_cast<unsigned int>(triAccels.size());
				
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
					triIndices.push_back(triIdx);

					// Create primitive bound from points
					AABB triBound(p1, p2);
					triBound = triBound.Union(p3);

					data.triBounds.push_back(triBound);
					data.triBoundCentroids.push_back((triBound.min + triBound.max) * Math::Float(0.5));
				}
			}
		}
	}

	// Build QBVH
	{
		NANON_LOG_INFO("Building QBVH");
		NANON_LOG_INDENTER();

		//ResetProgress();

		auto start = std::chrono::high_resolution_clock::now();
		Build(data, 0, static_cast<int>(triAccels.size()), -1, 0, 0);
		auto end = std::chrono::high_resolution_clock::now();

		double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.0;
		NANON_LOG_INFO("Completed in " + std::to_string(elapsed) + " seconds");
	}

	return true;
}

void QBVHScene::Impl::Build( const QBVHBuildData& data, unsigned int begin, unsigned int end, int parent, int child, int depth )
{
	// Bound of the primitives [begin, end)
	AABB bound;
	for (unsigned int i = begin; i < end; i++)
	{
		bound = bound.Union(data.triBounds[triIndices[i]]);
	}
	
	// Leaf node
	if (end - begin <= maxElementsInLeaf)
	{
		CreateLeafNode(begin, end, parent, child, bound);
		return;
	}

	// Determine the split axis and position
	int axis;
	Math::Float splitPosition;
	if (!SplitAxisAndPosition(data, begin, end, axis, splitPosition))
	{
		// The primitive bound is degenerated -> create a leaf node
		CreateLeafNode(begin, end, parent, child, bound);
		return;
	}

	// Partition primitives in [begin, end) according to split axis and position
	unsigned int splitTriIndex;
	PartitionPrimitives(data, begin, end, axis, splitPosition, splitTriIndex);

	// Index of the current and child nodes
	// The value is changed according to the depth of the recursion
	unsigned int current;
	int left, right;
	if (depth % 2 == 1)
	{
		// The process in the depth is the intermediate process creating the node indexed by #parent.
		// In the stage the function is responsible to create two sibling nodes.
		// For example if child = 2,
		// + parent			<- A node indexed by #current, which is created in the parent process
		//   + child 0
		//   + child 1
		//   + child 2		<- A node indexed by #left
		//   + child 3		<- A node indexed by #right
		// The process focuses on separating the primitives in [begin, end) to child 2 and 3.
		current = parent;
		left = child;
		right = child + 1;
	}
	else
	{
		// The process in the depth focus on creating intermediate node (i.e. a parent of at most four child nodes)
		// And left and right node index specifies the starting point of the further split of the sibling two nodes.
		// For example
		// + parent			<- Newly created node indexed by currentNodeIndex
		//   + child 0		<- A node indexed by #left, which means in the child call the child 0 and 1 are processed
		//   + child 1
		//   + child 2      <- A node indexed by #right, which means in the child call the child 2 and 3 are processed
		//   + child 3
		// The process focuses on separating the primitives in [begin, end) to child {0, 1} and child {2, 3}.
		CreateIntermediateNode(parent, child, bound, current);
		left = 0;
		right = 2;
	}

	// Process recursively
	Build(data, begin, splitTriIndex, current, left, depth + 1);
	Build(data, splitTriIndex, end, current, right, depth + 1);
}

bool QBVHScene::Impl::SplitAxisAndPosition( const QBVHBuildData& data, unsigned int begin, unsigned int end, int& axis, Math::Float& splitPosition )
{
	// Choose the axis to split
	AABB centroidBound;
	for (unsigned int i = begin; i < end; i++)
	{
		centroidBound = centroidBound.Union(data.triBoundCentroids[triIndices[i]]);
	}
	axis = centroidBound.LongestAxis();

	// Check if the bound is degenerated
	if (centroidBound.min[axis] == centroidBound.max[axis])
	{
		// Degenerated
		return false;
	}

	// Determine split position by SAH heuristics
	// SAH cost is computed with split bins for efficiency

	// Number of bins
	const int numBins = 12;

	// Some precomputed values
	const float k0 = centroidBound.min[axis];
	const float k1 = static_cast<float>(numBins) / (centroidBound.max[axis] - k0);

	// Compute bounds and count # of triangles for each bin
	AABB binTriBound[numBins];
	int binTris[numBins] = {0};
	for (unsigned int i = begin; i < end; i++)
	{
		const unsigned int index = triIndices[i];
		const int binId = std::max(0, std::min(numBins - 1, static_cast<int>(k1 * (data.triBoundCentroids[index][axis] - k0))));
		binTris[binId]++;
		binTriBound[binId] = binTriBound[binId].Union(data.triBounds[index]);
	}

	// Compute costs for each candidate for partition
	float costs[numBins - 1];
	for (int i = 0; i < numBins - 1; i++)
	{
		AABB b1, b2;
		int count1 = 0, count2 = 0;

		// [0, i]
		for (int j = 0; j <= i; j++)
		{
			b1 = b1.Union(binTriBound[j]);
			count1 += binTris[j];
		}

		// (i, numBins - 1]
		for (int j = i + 1; j < numBins; j++)
		{
			b2 = b2.Union(binTriBound[j]);
			count2 += binTris[j];
		}

		// Compute cost
		costs[i] = static_cast<float>(count1) * b1.SurfaceArea() + static_cast<float>(count2) * b2.SurfaceArea();
	}

	// Find minimum partition
	int minCostIdx = 0;
	double minCost = costs[0];
	for (int i = 1; i < numBins - 1; i++)
	{
		if (minCost > costs[i])
		{
			minCost = costs[i];
			minCostIdx = i;
		}
	}

	splitPosition = centroidBound.min[axis] + static_cast<float>(minCostIdx + 1) * (centroidBound.max[axis] - centroidBound.min[axis]) / numBins;
	return true;
}

void QBVHScene::Impl::PartitionPrimitives( const QBVHBuildData& data, unsigned int begin, unsigned int end, int axis, Math::Float splitPosition, unsigned int& splitTriIndex )
{
	splitTriIndex = begin;
	for (unsigned int i = begin; i < end; i++)
	{
		const unsigned int triIndex = triIndices[i];
		if (data.triBoundCentroids[triIndex][axis] <= splitPosition)
		{
			// Swap indices
			triIndices[i] = triIndices[splitTriIndex];
			triIndices[splitTriIndex] = triIndex;
			splitTriIndex++;
		}
	}
}

void QBVHScene::Impl::CreateLeafNode( unsigned int begin, unsigned int end, int parent, int child, const AABB& bound )
{
	// If the #parent is -1 the root is a leaf node
	// Note that in the case the root node is yet to be created
	if (parent < 0)
	{
		// Create node
		nodes.push_back(new QBVHNode());
		parent = 0;
	}

	// Set the value to the node
	auto& node = nodes[parent];
	node->SetBound(child, bound);

	// Initialize a leaf for #child
	// For now, # of quads section of the leaf data is replaced to # of triangles.
	// TODO : Replace it
	node->InitializeLeaf(child, end - begin, begin);
}

void QBVHScene::Impl::CreateIntermediateNode( int parent, int child, const AABB& bound, unsigned int& createdNodeIndex )
{
	// Create a new node
	createdNodeIndex = static_cast<unsigned int>(nodes.size());
	auto* node = new QBVHNode();
	nodes.push_back(node);
	
	// Set child data to the parent
	if (parent >= 0)
	{
		nodes[parent]->InitializeIntermediateNode(child, createdNodeIndex);
		nodes[parent]->SetBound(child, bound);
	}
}

bool QBVHScene::Impl::Intersect( Ray& ray, Intersection& isect ) const
{
	bool intersected = false;
	unsigned int intersectedTriIndex;
	Math::Vec2 intersectedTriB;

	// Some required data for intersection query
	Ray4 ray4(ray);

	__m128 invRayDir[3];
	invRayDir[0] = _mm_set1_ps(1.0f / ray.d.x);
	invRayDir[1] = _mm_set1_ps(1.0f / ray.d.y);
	invRayDir[2] = _mm_set1_ps(1.0f / ray.d.z);

	int rayDirSign[3];
	rayDirSign[0] = ray.d.x < 0.0f;
	rayDirSign[1] = ray.d.y < 0.0f;
	rayDirSign[2] = ray.d.z < 0.0f;

	// Stack for traversal
	std::vector<int> stack;
	stack.reserve(256);

	// Initial state
	stack.push_back(0);

	// Depth first traversal of QBVH
	while (!stack.empty())
	{
		int data = stack.back();
		stack.pop_back();

		if (data < 0)
		{
			// Leaf node
			
			// If the node is empty, ignore it
			if (data == QBVHNode::EmptyLeafNode) continue;

			// Intersection
			unsigned int size, offset;
			QBVHNode::ExtractLeafData(data, size, offset);
			for (unsigned int i = offset; i < offset + size; i++)
			{
				Math::Float t;
				Math::Vec2 b;
				if (triAccels[triIndices[i]].Intersect(ray, ray.minT, ray.maxT, b[0], b[1], t))
				{
					ray.maxT = t;
					intersectedTriIndex = triIndices[i];
					intersectedTriB = b;
					intersected = true;
				}
			}
		}
		else
		{
			// Intermediate node
			// Check intersection to 4 bounds simultaneously
			auto* node = nodes[data];
			int mask = node->Intersect(ray4, invRayDir, rayDirSign);
			if (mask & 0x1) stack.push_back(node->children[0]);
			if (mask & 0x2) stack.push_back(node->children[1]);
			if (mask & 0x4) stack.push_back(node->children[2]);
			if (mask & 0x8) stack.push_back(node->children[3]);
		}
	}

	if (intersected)
	{
		// Store some information to the intersection structure
		auto& triAccel = triAccels[intersectedTriIndex];
		self->StoreIntersectionFromBarycentricCoords(triAccel.primIndex, triAccel.shapeIndex, ray, intersectedTriB, isect);
		return true;
	}

	return false;
}

// --------------------------------------------------------------------------------

QBVHScene::QBVHScene()
	: p(new Impl(this))
{

}

QBVHScene::~QBVHScene()
{
	NANON_SAFE_DELETE(p);
}

bool QBVHScene::Build()
{
	return p->Build();
}

bool QBVHScene::Intersect( Ray& ray, Intersection& isect ) const
{
	return p->Intersect(ray, isect);
}

boost::signals2::connection QBVHScene::Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportBuildProgress(func);
}

bool QBVHScene::LoadImpl( const pugi::xml_node& node, const Assets& assets )
{
	return p->LoadImpl(node, assets);
}

NANON_NAMESPACE_END