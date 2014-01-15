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
#include <nanon.test/base.h>
#include <nanon.test/base.math.h>
#include <nanon.test/stub.bsdf.h>
#include <nanon.test/stub.trianglemesh.h>
#include <nanon/scenefactory.h>
#include <nanon/naivescene.h>
#include <nanon/primitive.h>
#include <nanon/ray.h>
#include <nanon/intersection.h>
#include <nanon/math.functions.h>
#include <random>

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

// {(x, y, z) : 0<=x,y<=1, z=0,-1}
class StubTriangleMesh_Simple : public StubTriangleMesh
{
public:

	StubTriangleMesh_Simple()
		: StubTriangleMesh("simple")
	{
		const double ps[] =
		{
			0, 0, 0,
			1, 0, 0,
			1, 1, 0,
			0, 1, 0,
			0, 0, -1,
			1, 0, -1,
			1, 1, -1,
			0, 1, -1
		};

		const double ns[] =
		{
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1
		};

		const double ts[] =
		{
			0, 0,
			1, 0,
			1, 1,
			0, 1,
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};

		const unsigned int fs[] =
		{
			0, 1, 2,
			0, 2, 3,
			4, 5, 6,
			4, 6, 7
		};

		for (int i = 0; i < 8; i++)
		{
			positions.push_back(Math::Float(ps[3*i  ]));
			positions.push_back(Math::Float(ps[3*i+1]));
			positions.push_back(Math::Float(ps[3*i+2]));
			normals.push_back(Math::Float(ns[3*i  ]));
			normals.push_back(Math::Float(ns[3*i+1]));
			normals.push_back(Math::Float(ns[3*i+2]));
			texcoords.push_back(Math::Float(ts[2*i  ]));
			texcoords.push_back(Math::Float(ts[2*i+1]));
		}

		for (int i = 0; i < 4; i++)
		{
			faces.push_back(fs[3*i]);
			faces.push_back(fs[3*i+1]);
			faces.push_back(fs[3*i+2]);
		}
	}

};

// {(x, y, z) : 0<=x,y<=1, x=-z}
class StubTriangleMesh_Simple2 : public StubTriangleMesh
{
public:

	StubTriangleMesh_Simple2()
		: StubTriangleMesh("simple2")
	{
		const double ps[] =
		{
			0, 0, 0,
			1, 0, -1,
			1, 1, -1,
			0, 1, 0
		};

		const double ts[] =
		{
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};

		const unsigned int fs[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		auto n = Math::Normalize(Math::Vec3(1, 0, 1));

		for (int i = 0; i < 4; i++)
		{
			positions.push_back(Math::Float(ps[3*i  ]));
			positions.push_back(Math::Float(ps[3*i+1]));
			positions.push_back(Math::Float(ps[3*i+2]));
			normals.push_back(n[0]);
			normals.push_back(n[1]);
			normals.push_back(n[2]);
			texcoords.push_back(Math::Float(ts[2*i  ]));
			texcoords.push_back(Math::Float(ts[2*i+1]));
		}

		for (int i = 0; i < 2; i++)
		{
			faces.push_back(fs[3*i]);
			faces.push_back(fs[3*i+1]);
			faces.push_back(fs[3*i+2]);
		}
	}

};

// Many triangles in [0, 1]^3
class StubTriangleMesh_Random : public StubTriangleMesh
{
public:

	StubTriangleMesh_Random()
		: StubTriangleMesh("random")
	{
		// Fix seed
		std::mt19937 gen(42);
		std::uniform_real_distribution<double> dist;

		const int FaceCount = 1000;
		for (int i = 0; i < FaceCount; i++)
		{
			auto p1 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
			auto p2 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
			auto p3 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));

			positions.push_back(p1[0]);
			positions.push_back(p1[1]);
			positions.push_back(p1[2]);
			positions.push_back(p2[0]);
			positions.push_back(p2[1]);
			positions.push_back(p2[2]);
			positions.push_back(p3[0]);
			positions.push_back(p3[1]);
			positions.push_back(p3[2]);

			auto n = Math::Cross(p2 - p1, p3 - p1);
			for (int j = 0; j < 3; j++)
			{
				normals.push_back(n[0]);
				normals.push_back(n[1]);
				normals.push_back(n[2]);
			}

			faces.push_back(3*i);
			faces.push_back(3*i+1);
			faces.push_back(3*i+2);
		}
	}

};

// --------------------------------------------------------------------------------

class SceneIntersectionTest : public TestBase
{
public:

	SceneIntersectionTest()
	{
		// List of scene types to be tested
		sceneTypes.push_back("naive");
		sceneTypes.push_back("bvh");
#if defined(NANON_USE_SSE2) && defined(NANON_SINGLE_PRECISION)
		sceneTypes.push_back("qbvh");
#endif

		// BSDF
		bsdf = new StubBSDF("test");
	}

	~SceneIntersectionTest()
	{
		NANON_SAFE_DELETE(bsdf);
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
				ray.minT = Math::Constants::Zero;
				ray.maxT = Math::Constants::Inf;

				ASSERT_TRUE(scene->Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, 0), isect.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.uv));
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
				ray.minT = Math::Constants::Zero;
				ray.maxT = Math::Constants::Inf;

				ASSERT_TRUE(scene->Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, -x), isect.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(1, 0, 1)), isect.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(1, 0, 1)), isect.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.uv));
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

			// Load & build
			EXPECT_TRUE(scene->LoadPrimitives(primitives));
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
					ray.minT = Math::Constants::Zero;
					ray.maxT = Math::Constants::Inf;

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
				for (int k = 0; k < isectsI.size(); k++)
				{
					auto& isectIK = isectsI[k];
					auto& isectJK = isectsJ[k];
					EXPECT_EQ(isectIK.primitive, isectJK.primitive);
					EXPECT_EQ(isectIK.primitiveIndex, isectJK.primitiveIndex);
					EXPECT_EQ(isectIK.triangleIndex, isectJK.triangleIndex);
					EXPECT_TRUE(ExpectVec3Near(isectIK.p, isectJK.p));
					EXPECT_TRUE(ExpectVec3Near(isectIK.gn, isectJK.gn));
					EXPECT_TRUE(ExpectVec3Near(isectIK.sn, isectJK.sn));
					EXPECT_TRUE(ExpectVec3Near(isectIK.ss, isectJK.ss));
					EXPECT_TRUE(ExpectVec3Near(isectIK.st, isectJK.st));
					EXPECT_TRUE(ExpectVec2Near(isectIK.uv, isectJK.uv));
				}
			}
		}
	}
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END