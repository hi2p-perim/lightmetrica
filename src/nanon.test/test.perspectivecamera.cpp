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
#include "stub.assets.h"
#include <nanon/perspectivecamera.h>
#include <nanon/film.h>
#include <nanon/primitive.h>
#include <nanon/ray.h>

namespace
{

	const std::string PerspectiveCameraNode_Success = NANON_TEST_MULTILINE_LITERAL(
		<camera id="test" type="perspective">
			<film ref="stub" />
			<fovy>90</fovy>
		</camera>
	);

	const std::string PerspectiveCameraNode_Fail_InvalidType = NANON_TEST_MULTILINE_LITERAL(
		<camera id="test" type="perspect">
			<film ref="stub" />
			<fovy>90</fovy>
		</camera>
	);

	const std::string PerspectiveCameraNode_Fail_InvalidProperty = NANON_TEST_MULTILINE_LITERAL(
		<camera id="test" type="perspect">
			<film ref="stub" />
		</camera>
	);

}

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class StubFilm : public Film
{
public:

	StubFilm(const std::string& id) : Film(id) {}
	virtual int Width() const { return 200; }
	virtual int Height() const { return 100; }
	virtual bool Save() const { return true; }
	virtual void RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb ) {}
	virtual bool Load( const pugi::xml_node& node, const Assets& assets ) { return true; }
	virtual std::string Type() const { return "stub"; }

};

// --------------------------------------------------------------------------------

class PerspectiveCameraTest : public TestBase
{
public:

	PerspectiveCameraTest()
		: camera("test")
	{
		// Add assets
		assets.Add("stub", new StubFilm("stub"));
	}

protected:

	StubAssets assets;
	PerspectiveCamera camera;

};

TEST_F(PerspectiveCameraTest, Load_Success)
{
	EXPECT_TRUE(camera.Load(LoadXMLBuffer(PerspectiveCameraNode_Success), assets));
	EXPECT_EQ(assets.GetAssetByName("stub"), camera.GetFilm());
}

TEST_F(PerspectiveCameraTest, Load_Fail)
{
	EXPECT_FALSE(camera.Load(LoadXMLBuffer(PerspectiveCameraNode_Fail_InvalidType), assets));
	EXPECT_FALSE(camera.Load(LoadXMLBuffer(PerspectiveCameraNode_Fail_InvalidProperty), assets));
}

TEST_F(PerspectiveCameraTest, RasterPosToRay)
{
	EXPECT_TRUE(camera.Load(LoadXMLBuffer(PerspectiveCameraNode_Success), assets));

	Ray ray;

	// Primitive 1
	std::unique_ptr<Primitive> primitive1(new Primitive(Math::Mat4::Identity()));
	camera.RegisterPrimitive(primitive1.get());

	// Raster position (0.5, 0.5)
	// -> Ray { p = (0, 0, 0), d = (0, 0, -1) }
	camera.RasterPosToRay(Math::Vec2(0.5), ray);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(), ray.o));
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, -1), ray.d));

	// Raster position (1, 1)
	// -> Ray { p = (0, 0, 0), d = Normalize(2, 1, -1) }
	camera.RasterPosToRay(Math::Vec2(1), ray);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(), ray.o));
	EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(2, 1, -1)), ray.d));

	// Primitive 2
	std::unique_ptr<Primitive> primitive2(new Primitive(Math::LookAt(Math::Vec3(1), Math::Vec3(0), Math::Vec3(0, 0, 1))));
	camera.RegisterPrimitive(primitive2.get());
	
	// Raster position (0.5, 0.5)
	// -> Ray { p = (1, 1, 1), d = Normalize(-1, -1, -1) }
	camera.RasterPosToRay(Math::Vec2(0.5), ray);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1), ray.o));
	EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(-1)), ray.d));
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END