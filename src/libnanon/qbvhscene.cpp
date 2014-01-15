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

NANON_NAMESPACE_BEGIN

// QBVH node (128 bytes)
struct QBVHNode
{
	
	// Constant which indicates a empty leaf node
	static const int EmptyLeafNode = 0xffffffff;

	// Bounds for 4 nodes in SOA structure
	AABB bounds[2];
	//__m128 bounds[2][3];

	/*
		Child nodes
		If the node is a leaf, the reference to the primitive is encoded to
		 - [31:31] : 1
		 - [30:27] : # of triangles in the leaf
		 - [26: 0] : An index of the first quad triangles
		If the node is a intermediate node, 
		 - [31:31] : 0
		 - [30: 0] : An index of the child node
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
		Set bounds to the node.
		\param i Child index.
		\param bound A bound to be set
	*/
	NANON_FORCE_INLINE void SetBound(int i, const AABB& bound)
	{
		for (int i = 0; i < 3; i++)
		{
			
		}
	}

	

};

// The structure is used on QBVHScene::Build
struct QBVHBuildData
{
	std::vector<AABB> triBounds;				// Bounds of the triangles
	std::vector<Math::Vec3> triBoundCentroids;	// Centroids of the bounds of the triangles
};

// --------------------------------------------------------------------------------

class QBVHScene::Impl : public Object
{
public:

	Impl(QBVHScene* self);
	bool Build();
	bool Intersect( Ray& ray, Intersection& isect ) const;
	boost::signals2::connection Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func) { return signal_ReportBuildProgress.connect(func); }

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

	void CreateLeafNode(int parent, int child, unsigned int begin, unsigned int end, const AABB& bound);
	void CreateIntermediateNode(int parent, int child, const AABB& bound);

private:

	QBVHScene* self;
	boost::signals2::signal<void (double, bool)> signal_ReportBuildProgress;
	int numProcessedTris;

	const int maxTriInLeaf;					// Maximum # of triangle in a node (4 bit : 64 triangles)
	std::vector<TriAccel> triAccels;		// List of triaccels
	std::vector<unsigned int> triIndices;	// List of triangle indices. The list is rearranged through build process.
	std::vector<QBVHNode*> nodes;			// List of QBVH nodes

};

QBVHScene::Impl::Impl( QBVHScene* self )
	: self(self)
	, maxTriInLeaf(64)
{

}

bool QBVHScene::Impl::Build()
{
	QBVHBuildData data;

	{
		// TODO : replace triaccel with SSE optimized quad triangle intersection
		NANON_LOG_INFO("Creating triaccels");
		NANON_LOG_INDENTER();

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
					size_t triIdx = triAccels.size();
				
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
		Build(data, 0, static_cast<int>(triAccels.size()));
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
	for (int i = begin; i < end; i++)
	{
		bound = bound.Union(data.triBounds[triIndices[i]]);
	}
	
	// Leaf node
	if (end - begin <= maxTriInLeaf)
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
	int current, left, right;
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
		//CreateIntermediateNode();
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
	for (int i = begin; i < end; i++)
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

	// Simply use mean index
	// TODO : Replace with bucket sorted SAH optimized split
	splitPosition = data.triBoundCentroids[(end - begin) / 2][axis];
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

void QBVHScene::Impl::CreateLeafNode( int parent, int child, unsigned int begin, unsigned int end, const AABB& bound )
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
	node->SetAABB();
}

void QBVHScene::Impl::CreateIntermediateNode( int parent, int child, const AABB& bound )
{

}

bool QBVHScene::Impl::Intersect( Ray& ray, Intersection& isect ) const
{
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

NANON_NAMESPACE_END