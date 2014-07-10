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

#if LM_SSE2 && LM_SINGLE_PRECISION

class QBVHSceneTest : public TestBase
{
public:

	QBVHSceneTest()
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
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(x, y, 0), isect.geom.p));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.geom.gn));
				EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), isect.geom.sn));
				EXPECT_TRUE(ExpectVec2Near(Math::Vec2(x, y), isect.geom.uv));
			}
		}
	}
}

#endif

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
