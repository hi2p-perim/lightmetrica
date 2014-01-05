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
#include <nanon.test/stub.assets.h>
#include <nanon/rawmesh.h>

namespace
{

	const std::string RawMeshNode_Success = NANON_TEST_MULTILINE_LITERAL(
		<triangle_mesh id="quad" type="raw">
			<positions>
				0 1 0
				0 1 1
				1 1 0
				1 1 1
			</positions>
			<normals>
				0 -1 0
				0 -1 0
				0 -1 0
				0 -1 0
			</normals>
			<faces>
				0 1 2
				0 1 3
			</faces>
		</triangle_mesh>
	);

}

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class RawMeshTest : public TestBase
{
public:

	RawMeshTest()
		: mesh("test")
	{

	}

	Math::Vec3 PositionFromIndex(unsigned int i)
	{
		const auto* p = mesh.Positions();
		return Math::Vec3(p[3*i], p[3*i+1], p[3*i+2]);
	}

	Math::Vec3 NormalFromIndex(unsigned int i)
	{
		const auto* n = mesh.Normals();
		return Math::Vec3(n[3*i], n[3*i+1], n[3*i+2]);
	}

protected:

	RawMesh mesh;
	StubAssets assets;

};

TEST_F(RawMeshTest, Load_Success)
{
	EXPECT_TRUE(mesh.Load(LoadXMLBuffer(RawMeshNode_Success), assets));
	ASSERT_EQ(6, mesh.NumFaces());
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 0), PositionFromIndex(mesh.Faces()[0])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 1), PositionFromIndex(mesh.Faces()[1])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 1, 0), PositionFromIndex(mesh.Faces()[2])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 0), PositionFromIndex(mesh.Faces()[3])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 1), PositionFromIndex(mesh.Faces()[4])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 1, 1), PositionFromIndex(mesh.Faces()[5])));
	for (int i = 0; i < 6; i++)
	{
		EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, -1, 0), NormalFromIndex(mesh.Faces()[i])));
	}
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END