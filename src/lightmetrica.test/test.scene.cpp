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
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/primitive.h>

namespace
{

	const std::string SceneNode_Success = LM_TEST_MULTILINE_LITERAL(
		<scene type="stub">
			<root>
				<node id="node1">
					<triangle_mesh ref="mesh1" />
					<bsdf ref="bsdf1" />
				</node>
				<node id="node2">
					<triangle_mesh ref="mesh2" />
					<bsdf ref="bsdf2" />
				</node>
			</root>
		</scene>
	);

	// Specify transform by matrix
	const std::string SceneNode_Success_WithTransformByMatrix = LM_TEST_MULTILINE_LITERAL(
		<scene type="stub">
			<root>
				<transform>
					<matrix>
						1 0 0 0
						0 1 0 0
						0 0 1 0
						1 2 3 1
					</matrix>
				</transform>
				<node id="node1">
					<transform>
						<matrix>
							2 0 0 0
							0 2 0 0
							0 0 2 0
							0 0 0 1
						</matrix>
					</transform>
					<triangle_mesh ref="mesh1" />
					<bsdf ref="bsdf1" />
				</node>
			</root>
		</scene>
	);

	const std::string SceneNode_Success_WithTransform = LM_TEST_MULTILINE_LITERAL(
		<scene type="stub">
			<root>
				<node id="node1">
					<transform>
						<translate>1 1 1</translate>
						<rotate>
							<angle>45</angle>
							<axis>0 1 0</axis>
						</rotate>
						<scale>2 2 2</scale>
					</transform>
					<triangle_mesh ref="mesh1" />
					<bsdf ref="bsdf1" />
				</node>
			</root>
		</scene>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubScene : public Scene
{
public:

	virtual bool Build() { return true; }
	virtual bool Intersect( Ray& ray, Intersection& isect ) const { return false; }
	virtual std::string Type() const { return "stub"; }
	virtual boost::signals2::connection Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func) { return boost::signals2::connection(); }
	virtual bool Configure( const ConfigNode& node ) { return true; }
	virtual void ResetScene() {}

};

class SceneTest : public TestBase
{
public:

	SceneTest()
	{
		assets.Add("mesh1", new StubTriangleMesh);
		assets.Add("mesh2", new StubTriangleMesh);
		assets.Add("bsdf1", new StubBSDF);
		assets.Add("bsdf2", new StubBSDF);
	}

protected:

	StubAssets assets;
	StubScene scene;
	StubConfig config;

};

TEST_F(SceneTest, Load)
{
	EXPECT_TRUE(scene.Load(config.LoadFromStringAndGetFirstChild(SceneNode_Success), assets));

	const auto* node1 = scene.PrimitiveByID("node1");
	ASSERT_NE(nullptr, node1);
	EXPECT_EQ("stub", node1->mesh->ComponentImplTypeName());
	EXPECT_EQ("stub", node1->bsdf->ComponentImplTypeName());
	EXPECT_TRUE(ExpectMat4Near(Math::Mat4::Identity(), node1->transform));
	
	const auto* node2 = scene.PrimitiveByID("node2");
	ASSERT_NE(nullptr, node2);
	EXPECT_EQ("stub", node2->mesh->ComponentImplTypeName());
	EXPECT_EQ("stub", node2->bsdf->ComponentImplTypeName());
	EXPECT_TRUE(ExpectMat4Near(Math::Mat4::Identity(), node2->transform));
}

TEST_F(SceneTest, Load_WithTransformByMatrix)
{
	EXPECT_TRUE(scene.Load(config.LoadFromStringAndGetFirstChild(SceneNode_Success_WithTransformByMatrix), assets));
	
	const auto* node1 = scene.PrimitiveByID("node1");
	ASSERT_NE(nullptr, node1);

	auto transform = node1->transform;
	Math::Mat4 expected(
		2, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 2, 0,
		1, 2, 3, 1);

	ASSERT_TRUE(ExpectMat4Near(expected, transform));
}

TEST_F(SceneTest, Load_WithTransform)
{
	EXPECT_TRUE(scene.Load(config.LoadFromStringAndGetFirstChild(SceneNode_Success_WithTransform), assets));

	const auto* node1 = scene.PrimitiveByID("node1");
	ASSERT_NE(nullptr, node1);

	Math::Vec4 t = node1->transform * Math::Vec4(Math::Float(1));
	Math::Vec4 expected(Math::Sqrt(Math::Float(2)) * Math::Float(2) + Math::Float(1), Math::Float(3), Math::Float(1), Math::Float(1));
	ASSERT_TRUE(ExpectVec4Near(expected, t));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END