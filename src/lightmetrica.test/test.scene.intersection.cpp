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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/base.math.h>
#include <lightmetrica.test/stub.bsdf.h>
#include <lightmetrica.test/stub.trianglemesh.h>
#include <lightmetrica/scenefactory.h>
#include <lightmetrica/naivescene.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/math.functions.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class SceneIntersectionTest : public TestBase
{
public:

	SceneIntersectionTest()
	{
		// List of scene types to be tested
		sceneTypes.push_back("naive");
		sceneTypes.push_back("bvh");
#if LM_SSE2 && LM_SINGLE_PRECISION
		sceneTypes.push_back("qbvh");
#endif

		// BSDF
		bsdf = new StubBSDF;
	}

	~SceneIntersectionTest()
	{
		LM_SAFE_DELETE(bsdf);
	}

protected:

	std::shared_ptr<Scene> CreateAndSetupScene(const std::string& type, TriangleMesh* mesh)
	{
		// Create scene
		auto scene = factory.Create(type);

		// Primitives for this test
		std::vector<Primitive*> primitives;
		primitives.push_back(new Primitive(Math::Mat4::Identity()));
		primitives.back()->mesh = mesh;
		primitives.back()->bsdf = bsdf;

		// Load & build
		EXPECT_TRUE(scene->LoadPrimitives(primitives));
		EXPECT_TRUE(scene->Configure(ConfigNode()));
		EXPECT_TRUE(scene->Build());

		return std::shared_ptr<Scene>(scene);
	}

protected:

	std::vector<std::string> sceneTypes;
	StubBSDF* bsdf;
	SceneFactory factory;

};

TEST_F(SceneIntersectionTest, Intersect_Simple)
{
	for (const auto& type : sceneTypes)
	{
		// Triangle mesh and scene
		std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Simple());
		auto scene = CreateAndSetupScene(type, mesh.get());

		// Trace rays in the region of [0, 1]^2
		Ray ray;
		Intersection isect;
		const int Steps = 10;
		const Math::Float Delta = Math::Float(1) / Math::Float(Steps);
		for (int i = 1; i < Steps; i++)
		{
			const Math::Float y = Delta * Math::Float(i);
			for (int j = 1; j < Steps; j++)
			{
				const Math::Float x = Delta * Math::Float(j);

				// Intersection query
				ray.o = Math::Vec3(0, 0, 1);
				ray.d = Math::Normalize(Math::Vec3(x, y, 0) - ray.o);
				ray.minT = Math::Constants::Zero();
				ray.maxT = Math::Constants::Inf();

				ASSERT_TRUE(scene->Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, 0), isect.geom.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.geom.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.geom.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.geom.uv));
			}
		}
	}
}

TEST_F(SceneIntersectionTest, Intersect_Simple2)
{
	for (const auto& type : sceneTypes)
	{
		// Triangle mesh and scene
		std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Simple2());
		auto scene = CreateAndSetupScene(type, mesh.get());

		// Trace rays in the region of [0, 1]^2
		Ray ray;
		Intersection isect;
		const int Steps = 10;
		const Math::Float Delta = Math::Float(1) / Math::Float(Steps);
		for (int i = 1; i < Steps; i++)
		{
			const Math::Float y = Delta * Math::Float(i);
			for (int j = 1; j < Steps; j++)
			{
				const Math::Float x = Delta * Math::Float(j);

				// Intersection query
				ray.o = Math::Vec3(x, y, 1);
				ray.d = Math::Vec3(0, 0, -1);
				ray.minT = Math::Constants::Zero();
				ray.maxT = Math::Constants::Inf();

				ASSERT_TRUE(scene->Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, -x), isect.geom.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(1, 0, 1)), isect.geom.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(1, 0, 1)), isect.geom.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.geom.uv));
			}
		}
	}
}

// Check if all implementation returns the same result
TEST_F(SceneIntersectionTest, Consistency)
{
	// This test requires at least two implementations
	if (sceneTypes.size() >= 2)
	{
		// Triangle mesh
		std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Random());

		// Primitives for this test
		std::vector<Primitive*> primitives;
		primitives.push_back(new Primitive(Math::Mat4::Identity()));
		primitives.back()->mesh = mesh.get();
		primitives.back()->bsdf = bsdf;

		// Result for each type (primitive ID)
		std::vector<std::vector<
			Intersection,
			aligned_allocator<Intersection, std::alignment_of<Intersection>::value>
		>> results(sceneTypes.size());

		Ray ray;
		Intersection isect;

		for (size_t typeIdx = 0; typeIdx < sceneTypes.size(); typeIdx++)
		{
			const int Steps = 10;
			const Math::Float Delta = Math::Float(1) / Math::Float(Steps);

			// Create scene
			auto scene = factory.Create(sceneTypes[typeIdx]);

			// Load & configure & build
			EXPECT_TRUE(scene->LoadPrimitives(primitives));
			EXPECT_TRUE(scene->Configure(ConfigNode()));
			EXPECT_TRUE(scene->Build());

			for (int i = 1; i < Steps; i++)
			{
				const Math::Float y = Delta * Math::Float(i);

				for (int j = 1; j < Steps; j++)
				{
					const Math::Float x = Delta * Math::Float(j);

					// Intersection query
					ray.o = Math::Vec3(x, y, 1);
					ray.d = Math::Vec3(0, 0, -1);
					ray.minT = Math::Constants::Zero();
					ray.maxT = Math::Constants::Inf();

					if (scene->Intersect(ray, isect))
					{
						// Store the intersection to the result
						results[typeIdx].push_back(isect);
					}
				}
			}
		}

		// Check consistency
		for (size_t i = 0; i < sceneTypes.size(); i++)
		{
			for (size_t j = i+1; j < sceneTypes.size(); j++)
			{
				auto& isectsI = results[i];
				auto& isectsJ = results[j];

				// Number of intersected triangles
				ASSERT_EQ(isectsI.size(), isectsJ.size());

				// For each intersections, check values if two of them are same
				for (size_t k = 0; k < isectsI.size(); k++)
				{
					auto& isectIK = isectsI[k];
					auto& isectJK = isectsJ[k];
					EXPECT_EQ(isectIK.primitive, isectJK.primitive);
					EXPECT_EQ(isectIK.primitiveIndex, isectJK.primitiveIndex);
					EXPECT_EQ(isectIK.triangleIndex, isectJK.triangleIndex);
					EXPECT_TRUE(ExpectVec3Near(isectIK.geom.p, isectJK.geom.p));
					EXPECT_TRUE(ExpectVec3Near(isectIK.geom.gn, isectJK.geom.gn));
					EXPECT_TRUE(ExpectVec3Near(isectIK.geom.sn, isectJK.geom.sn));
					EXPECT_TRUE(ExpectVec3Near(isectIK.geom.ss, isectJK.geom.ss));
					EXPECT_TRUE(ExpectVec3Near(isectIK.geom.st, isectJK.geom.st));
					EXPECT_TRUE(ExpectVec2Near(isectIK.geom.uv, isectJK.geom.uv));
				}
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
