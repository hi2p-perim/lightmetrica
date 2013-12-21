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
#include "base.h"
#include "base.math.h"
#include "stub.assetfactory.h"
#include <nanon/scene.h>
#include <nanon/assets.h>
#include <nanon/trianglemesh.h>
#include <nanon/bsdf.h>
#include <nanon/primitive.h>

using namespace nanon;

namespace
{

	const std::string SceneNode_Success = NANON_TEST_MULTILINE_LITERAL(
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
	const std::string SceneNode_Success_WithTransformByMatrix = NANON_TEST_MULTILINE_LITERAL(
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

	// Specify transform
	//const std::string SceneNode_Success_WithTransform = NANON_TEST_MULTILINE_LITERAL(
	//	<scene type="stub">
	//		<root>
	//			<node id="node1">
	//				<transform>
	//					<translate>1 1 1</translate>
	//					<rotate>1 0 0 1</rotate>
	//					<scale>2 2 2</scale>
	//				</transform>
	//				<triangle_mesh ref="mesh1" />
	//				<bsdf ref="bsdf1" />
	//			</node>
	//		</root>
	//	</scene>
	//);

	// Invalid number of element in 'matrix'
	const std::string SceneNode_Fail_MatrixInvalidNumberOfElements = NANON_TEST_MULTILINE_LITERAL(
		<scene type="stub">
			<root>
				<node id="node1">
					<transform>
						<matrix>
							1 0 0 0
							0 1 0 0
							0 0 1 0
							0 0 0
						</matrix>
					</transform>
					<triangle_mesh ref="mesh1" />
					<bsdf ref="bsdf1" />
				</node>
			</root>
		</scene>
	);

}

NANON_TEST_NAMESPACE_BEGIN

class StubTriangleMesh : public TriangleMesh
{
public:

	StubTriangleMesh(const std::string& id) : TriangleMesh(id) {}
	virtual bool Load(const pugi::xml_node& node) { return true; }
	virtual std::string Type() const { return "stub"; }
	virtual int NumVertices() const { return 0; }
	virtual int NumFaces() const { return 0; }
	virtual const Math::Vec3* Positions() const { return nullptr; }
	virtual const Math::Vec3* Normals() const { return nullptr; }
	virtual const Math::Vec2* TexCoords() const { return nullptr; }
	virtual const Math::Vec3i* Faces() const { return nullptr; }

};

class StubBSDF : public BSDF
{
public:

	StubBSDF(const std::string& id) : BSDF(id) {}
	virtual bool Load(const pugi::xml_node& node) { return true; }
	virtual std::string Type() const { return "stub"; }

};

class StubAssets : public Assets
{
public:

	StubAssets()
	{
		assetInstanceMap["mesh1"] = std::make_shared<StubTriangleMesh>("mesh1");
		assetInstanceMap["mesh2"] = std::make_shared<StubTriangleMesh>("mesh2");
		assetInstanceMap["bsdf1"] = std::make_shared<StubBSDF>("bsdf1");
		assetInstanceMap["bsdf2"] = std::make_shared<StubBSDF>("bsdf2");
	}

	virtual Asset* GetAssetByName(const std::string& name)
	{
		return assetInstanceMap[name].get();
	}

private:

	boost::unordered_map<std::string, std::shared_ptr<Asset>> assetInstanceMap;

};

class StubScene : public Scene
{
public:

	virtual bool Build() { return true; }
	virtual bool Intersect( Ray& ray, Intersection& isect )  { return false; }
	virtual std::string Type() { return "stub"; }

};

// --------------------------------------------------------------------------------

class SceneTest : public TestBase
{
protected:

	StubAssets assets;
	StubScene scene;

};

TEST_F(SceneTest, Load)
{
	EXPECT_TRUE(scene.Load(LoadXMLBuffer(SceneNode_Success), assets));

	const auto* node1 = scene.PrimitiveByID("node1");
	ASSERT_NE(nullptr, node1);
	EXPECT_EQ("stub", node1->mesh->Type());
	EXPECT_EQ("stub", node1->bsdf->Type());
	EXPECT_TRUE(ExpectMat4Near(Math::Mat4::Identity(), node1->transform));
	
	const auto* node2 = scene.PrimitiveByID("node2");
	ASSERT_NE(nullptr, node2);
	EXPECT_EQ("stub", node2->mesh->Type());
	EXPECT_EQ("stub", node2->bsdf->Type());
	EXPECT_TRUE(ExpectMat4Near(Math::Mat4::Identity(), node2->transform));
}

TEST_F(SceneTest, Load_WithTransformByMatrix)
{
	EXPECT_TRUE(scene.Load(LoadXMLBuffer(SceneNode_Success_WithTransformByMatrix), assets));
	
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

TEST_F(SceneTest, Load_Fail_MatrixInvalidNumberOfElements)
{
	EXPECT_FALSE(scene.Load(LoadXMLBuffer(SceneNode_Fail_MatrixInvalidNumberOfElements), assets));
}

NANON_TEST_NAMESPACE_END