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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/base.math.h>
#include <lightmetrica.test/stub.trianglemesh.h>
#include <lightmetrica.test/stub.bsdf.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/qbvhscene.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>

namespace
{

	const std::string SceneNode_Template = LM_TEST_MULTILINE_LITERAL(
		<scene type="qbvh">
			<intersection_mode>%s</intersection_mode>
		</scene>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

#if defined(LM_USE_SSE2) && defined(LM_SINGLE_PRECISION)

class QBVHSceneTest : public TestBase
{
public:

	QBVHSceneTest()
		: bsdf("test")
	{
		
	}

protected:

	void SetupScene(TriangleMesh* mesh, const std::string& mode)
	{
		// Primitives
		std::vector<Primitive*> primitives;
		primitives.push_back(new Primitive(Math::Mat4::Identity()));
		primitives.back()->mesh = mesh;
		primitives.back()->bsdf = &bsdf;
		
		// Load & configure & build
		scene.Reset();
		EXPECT_TRUE(scene.LoadPrimitives(primitives));
		EXPECT_TRUE(scene.Configure(config.LoadFromStringAndGetFirstChild(boost::str(boost::format(SceneNode_Template) % mode))));
		EXPECT_TRUE(scene.Build());
	}

protected:

	QBVHScene scene;
	StubBSDF bsdf;
	StubConfig config;

};

TEST_F(QBVHSceneTest, Intersect_Random)
{
	std::vector<std::string> modes;
	modes.push_back("sse");
	modes.push_back("triaccel");

	for (const auto& mode : modes)
	{
		// Setup scene
		std::unique_ptr<TriangleMesh> mesh(new StubTriangleMesh_Simple);
		SetupScene(mesh.get(), mode);

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

				ASSERT_TRUE(scene.Intersect(ray, isect));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, 0), isect.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.uv));
			}
		}
	}
}

#endif

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
