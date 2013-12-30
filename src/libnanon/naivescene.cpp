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
#include <nanon/naivescene.h>
#include <nanon/ray.h>
#include <nanon/intersection.h>
#include <nanon/primitive.h>
#include <nanon/trianglemesh.h>
#include <nanon/triaccel.h>

NANON_NAMESPACE_BEGIN

class NaiveScene::Impl : public Object
{
public:

	Impl(NaiveScene* self);
	bool Build();
	bool Intersect(Ray& ray, Intersection& isect) const;

private:

	NaiveScene* self;
	std::vector<TriAccel> triAccels;

};

NaiveScene::Impl::Impl( NaiveScene* self )
	: self(self)
{

}

bool NaiveScene::Impl::Build()
{
	// Almost do nothing; simply creates a list of triangles from the primitives
	// as data structure, we used Wald's TriAccel.
	
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
				triAccels.push_back(TriAccel());
				triAccels.back().shapeIndex = j;
				triAccels.back().primIndex = i;
				unsigned int i1 = faces[3*j  ];
				unsigned int i2 = faces[3*j+1];
				unsigned int i3 = faces[3*j+2];
				Math::Vec3 p1(positions[3*i1], positions[3*i1+1], positions[3*i1+2]);
				Math::Vec3 p2(positions[3*i2], positions[3*i2+1], positions[3*i2+2]);
				Math::Vec3 p3(positions[3*i3], positions[3*i3+1], positions[3*i3+2]);
				triAccels.back().Load(p1, p2, p3);
			}
		}
	}
	
	return true;
}

bool NaiveScene::Impl::Intersect( Ray& ray, Intersection& isect ) const
{
	bool intersected = false;
	size_t minTriAccelIdx = 0;
	Math::Vec2 minB;

	for (size_t i = 0; i < triAccels.size(); i++)
	{
		Math::Float t;
		Math::Vec2 b;
		if (triAccels[i].Intersect(ray, ray.minT, ray.maxT, b[0], b[1], t))
		{
			ray.maxT = t;
			minTriAccelIdx = i;
			minB = b;
			intersected = true;
		}
	}

	if (intersected)
	{
		// Store required data for the intersection structure
		auto& triAccel = triAccels[minTriAccelIdx];
		self->StoreIntersectionFromBarycentricCoords(triAccel.primIndex, triAccel.shapeIndex, ray, minB, isect);
	}

	return intersected;
}

// --------------------------------------------------------------------------------

NaiveScene::NaiveScene()
	: p(new Impl(this))
{

}

NaiveScene::~NaiveScene()
{
	NANON_SAFE_DELETE(p);
}

bool NaiveScene::Build()
{
	return p->Build();
}

bool NaiveScene::Intersect( Ray& ray, Intersection& isect ) const
{
	return p->Intersect(ray, isect);
}

NANON_NAMESPACE_END