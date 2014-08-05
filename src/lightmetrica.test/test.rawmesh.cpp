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

	const std::string RawMeshNode_Success = LM_TEST_MULTILINE_LITERAL(
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

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class RawMeshTest : public TestBase
{
public:

	RawMeshTest()
		: mesh(ComponentFactory::Create<TriangleMesh>("raw"))
	{

	}

	Math::Vec3 PositionFromIndex(unsigned int i)
	{
		const auto* p = mesh->Positions();
		return Math::Vec3(p[3*i], p[3*i+1], p[3*i+2]);
	}

	Math::Vec3 NormalFromIndex(unsigned int i)
	{
		const auto* n = mesh->Normals();
		return Math::Vec3(n[3*i], n[3*i+1], n[3*i+2]);
	}

protected:

	std::unique_ptr<TriangleMesh> mesh;
	StubAssets assets;
	StubConfig config;

};

TEST_F(RawMeshTest, Load)
{
	EXPECT_TRUE(mesh->Load(config.LoadFromStringAndGetFirstChild(RawMeshNode_Success), assets));
	ASSERT_EQ(6, mesh->NumFaces());
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 0), PositionFromIndex(mesh->Faces()[0])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 1), PositionFromIndex(mesh->Faces()[1])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 1, 0), PositionFromIndex(mesh->Faces()[2])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 0), PositionFromIndex(mesh->Faces()[3])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 1, 1), PositionFromIndex(mesh->Faces()[4])));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1, 1, 1), PositionFromIndex(mesh->Faces()[5])));
	for (int i = 0; i < 6; i++)
	{
		EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, -1, 0), NormalFromIndex(mesh->Faces()[i])));
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END