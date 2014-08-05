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

#include "pch.test.h"
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/base.math.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica/trianglemesh.h>

namespace
{
	
	const std::string ObjMesh_Triangle_Success = LM_TEST_MULTILINE_LITERAL(
		v 0 1 1 \n
		v 0 0 1 \n
		v 1 0 1 \n
		v 1 1 1 \n
		f 1 2 3 \n
		f 2 3 4 \n
	);

	const std::string ObjMesh_Polygon_Success = LM_TEST_MULTILINE_LITERAL(
		v 0 1 1 \n
		v 0 0 1 \n
		v 1 0 1 \n
		v 1 1 1 \n
		f 1 2 3 4 \n
	);

	const std::string ObjMesh_Fail_MissingIndex = LM_TEST_MULTILINE_LITERAL(
		v 0 1 1 \n
		v 0 0 1 \n
		v 1 0 1 \n
		f 1 2 4 \n
	);

	const std::string ObjMeshNode_Template = LM_TEST_MULTILINE_LITERAL(
		<triangle_mesh id="test" type="obj">
			<path>%s</path>
		</triangle_mesh>
	);

	const std::string ObjMeshNode_Fail_MissingPathElement = LM_TEST_MULTILINE_LITERAL(
		<triangle_mesh id="test" type="obj">
		</triangle_mesh>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class ObjMeshTest : public TestBase
{
public:

	ObjMeshTest()
		: mesh(ComponentFactory::Create<TriangleMesh>("obj"))
	{

	}

	ConfigNode GenerateNode(const std::string& path)
	{
		return config.LoadFromStringAndGetFirstChild(boost::str(boost::format(ObjMeshNode_Template) % path));
	}

	Math::Vec3 PositionFromIndex(unsigned int i)
	{
		const auto* p = mesh->Positions();
		return Math::Vec3(p[3*i], p[3*i+1], p[3*i+2]);
	}

protected:

	std::unique_ptr<TriangleMesh> mesh;
	StubAssets assets;
	StubConfig config;

};

TEST_F(ObjMeshTest, Load_Success)
{
	TemporaryTextFile tmp1("tmp1.obj", ObjMesh_Triangle_Success);
	TemporaryTextFile tmp2("tmp2.obj", ObjMesh_Polygon_Success);
	EXPECT_TRUE(mesh->Load(GenerateNode(tmp1.Path()), assets));
	ASSERT_EQ(6, mesh->NumFaces());
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 1), PositionFromIndex(mesh->Faces()[0])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), PositionFromIndex(mesh->Faces()[1])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 0, 1), PositionFromIndex(mesh->Faces()[2])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), PositionFromIndex(mesh->Faces()[3])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 0, 1), PositionFromIndex(mesh->Faces()[4])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 1, 1), PositionFromIndex(mesh->Faces()[5])));
	EXPECT_TRUE(mesh->Load(GenerateNode(tmp2.Path()), assets));
}

TEST_F(ObjMeshTest, Load_Fail)
{
	TemporaryTextFile tmp1("tmp1.obj", ObjMesh_Fail_MissingIndex);
	EXPECT_FALSE(mesh->Load(config.LoadFromStringAndGetFirstChild(ObjMeshNode_Fail_MissingPathElement), assets));
	EXPECT_FALSE(mesh->Load(GenerateNode(tmp1.Path()), assets));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
