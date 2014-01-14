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

struct QBVHNode
{

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
	// [#begin, #end) : Range of primitive indices
	// parent : 
	void Build(const QBVHBuildData& data, unsigned int begin, unsigned int end, int parent, int child, int depth);

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
	// Leaf node
	if (end - begin <= maxTriInLeaf)
	{
		
	}

	
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