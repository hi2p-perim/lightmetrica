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
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/primitives.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/triaccel.h>

LM_NAMESPACE_BEGIN

class NaiveScene : public Scene
{
public:

	LM_COMPONENT_IMPL_DEF("naive");

public:

	virtual bool Build();
	virtual bool IntersectTriangles(Ray& ray, Intersection& isect) const;
	virtual AABB GetAABBTriangles() const { return aabbTris; }
	virtual boost::signals2::connection Connect_ReportBuildProgress(const std::function<void (double, bool)>& func) { return signal_ReportBuildProgress.connect(func); }
	virtual bool Configure( const ConfigNode& node ) { return true; }

private:

	std::vector<TriAccel> triAccels;
	AABB aabbTris;
	boost::signals2::signal<void (double, bool)> signal_ReportBuildProgress;

};

bool NaiveScene::Build()
{
	// Almost do nothing; simply creates a list of triangles from the primitives
	// as data structure, we used Wald's TriAccel.
	
	signal_ReportBuildProgress(0, false);

	int numPrimitives = primitives->NumPrimitives();
	for (int i = 0; i < numPrimitives; i++)
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
				// Create a triaccel
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

				// Entire bound
				aabbTris = aabbTris.Union(p1);
				aabbTris = aabbTris.Union(p2);
				aabbTris = aabbTris.Union(p3);
			}
		}

		signal_ReportBuildProgress(static_cast<double>(i) / numPrimitives, i == numPrimitives-1);
	}
	
	return true;
}

bool NaiveScene::IntersectTriangles( Ray& ray, Intersection& isect ) const
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
		StoreIntersectionFromBarycentricCoords(triAccel.primIndex, triAccel.shapeIndex, ray, minB, isect);
	}

	return intersected;
}

LM_COMPONENT_REGISTER_IMPL(NaiveScene, Scene);

LM_NAMESPACE_END