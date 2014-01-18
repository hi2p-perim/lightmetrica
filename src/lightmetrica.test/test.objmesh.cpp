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
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica/objmesh.h>

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
		: mesh("test")
	{

	}

	pugi::xml_node GenerateNode(const std::string& path)
	{
		return LoadXMLBuffer(boost::str(boost::format(ObjMeshNode_Template) % path));
	}

	Math::Vec3 PositionFromIndex(unsigned int i)
	{
		const auto* p = mesh.Positions();
		return Math::Vec3(p[3*i], p[3*i+1], p[3*i+2]);
	}

protected:

	ObjMesh mesh;
	StubAssets assets;

};

TEST_F(ObjMeshTest, Load_Success)
{
	TemporaryFile tmp1("tmp1.obj", ObjMesh_Triangle_Success);
	TemporaryFile tmp2("tmp2.obj", ObjMesh_Polygon_Success);
	EXPECT_TRUE(mesh.Load(GenerateNode(tmp1.Path()), assets));
	ASSERT_EQ(6, mesh.NumFaces());
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 1), PositionFromIndex(mesh.Faces()[0])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), PositionFromIndex(mesh.Faces()[1])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 0, 1), PositionFromIndex(mesh.Faces()[2])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, 1), PositionFromIndex(mesh.Faces()[3])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 0, 1), PositionFromIndex(mesh.Faces()[4])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 1, 1), PositionFromIndex(mesh.Faces()[5])));
	EXPECT_TRUE(mesh.Load(GenerateNode(tmp2.Path()), assets));
}

TEST_F(ObjMeshTest, Load_Fail)
{
	TemporaryFile tmp1("tmp1.obj", ObjMesh_Fail_MissingIndex);
	EXPECT_FALSE(mesh.Load(LoadXMLBuffer(ObjMeshNode_Fail_MissingPathElement), assets));
	EXPECT_FALSE(mesh.Load(GenerateNode(tmp1.Path()), assets));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
