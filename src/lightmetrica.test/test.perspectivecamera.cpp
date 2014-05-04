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
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica.test/stub.film.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/math.transform.h>
#include <lightmetrica/surfacegeometry.h>

namespace
{

	const std::string PerspectiveCameraNode_Success = LM_TEST_MULTILINE_LITERAL(
		<camera id="test" type="perspective">
			<film ref="stub" />
			<fovy>90</fovy>
		</camera>
	);

	const std::string PerspectiveCameraNode_Fail_InvalidProperty = LM_TEST_MULTILINE_LITERAL(
		<camera id="test" type="perspective">
			<film ref="stub" />
		</camera>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class PerspectiveCameraTest : public TestBase
{
public:

	PerspectiveCameraTest()
		: camera(ComponentFactory::Create<Camera>("perspective"))
	{
		// Add assets
		assets.Add("stub", new StubFilm);
	}

protected:

	StubAssets assets;
	StubConfig config;
	std::unique_ptr<Camera> camera;

};

TEST_F(PerspectiveCameraTest, Load)
{
	EXPECT_TRUE(camera->Load(config.LoadFromStringAndGetFirstChild(PerspectiveCameraNode_Success), assets));
	EXPECT_EQ(assets.GetAssetByName("stub"), camera->GetFilm());
}

TEST_F(PerspectiveCameraTest, Load_Fail)
{
	EXPECT_FALSE(camera->Load(config.LoadFromStringAndGetFirstChild(PerspectiveCameraNode_Fail_InvalidProperty), assets));
}

TEST_F(PerspectiveCameraTest, SampleRay)
{
	EXPECT_TRUE(camera->Load(config.LoadFromStringAndGetFirstChild(PerspectiveCameraNode_Success), assets));

	Math::PDFEval _;
	SurfaceGeometry geom;
	std::vector<Primitive*> primitives;
	GeneralizedBSDFSampleQuery bsdfSQ;
	GeneralizedBSDFSampleResult bsdfSR;

	// --------------------------------------------------------------------------------

	// Primitive 1
	std::unique_ptr<Primitive> primitive1(new Primitive(Math::Mat4::Identity()));
	primitives.clear();
	primitives.push_back(primitive1.get());
	camera->RegisterPrimitives(primitives);

	// Raster position (0.5, 0.5)
	// -> Ray { p = (0, 0, 0), d = (0, 0, -1) }
	
	camera->SamplePosition(Math::Vec2(), geom, _);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(), geom.p));
	
	bsdfSQ.sample = Math::Vec2(0.5);
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	camera->SampleDirection(bsdfSQ, geom, bsdfSR);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(0, 0, -1), bsdfSR.wo));

	// Raster position (1, 1)
	// -> Ray { p = (0, 0, 0), d = Normalize(2, 1, -1) }

	camera->SamplePosition(Math::Vec2(), geom, _);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(), geom.p));

	bsdfSQ.sample = Math::Vec2(1);
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	camera->SampleDirection(bsdfSQ, geom, bsdfSR);
	EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(2, 1, -1)), bsdfSR.wo));

	// --------------------------------------------------------------------------------

	// Primitive 2
	std::unique_ptr<Primitive> primitive2(new Primitive(Math::LookAt(Math::Vec3(1), Math::Vec3(0), Math::Vec3(0, 0, 1))));
	primitives.clear();
	primitives.push_back(primitive2.get());
	camera->RegisterPrimitives(primitives);
	
	// Raster position (0.5, 0.5)
	// -> Ray { p = (1, 1, 1), d = Normalize(-1, -1, -1) }

	camera->SamplePosition(Math::Vec2(), geom, _);
	EXPECT_TRUE(ExpectVec3Near(Math::Vec3(1), geom.p));

	bsdfSQ.sample = Math::Vec2(0.5);
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	camera->SampleDirection(bsdfSQ, geom, bsdfSR);
	EXPECT_TRUE(ExpectVec3Near(Math::Normalize(Math::Vec3(-1)), bsdfSR.wo));
}

TEST_F(PerspectiveCameraTest, RayToRasterPosition)
{
	EXPECT_TRUE(camera->Load(config.LoadFromStringAndGetFirstChild(PerspectiveCameraNode_Success), assets));

	Math::Vec2 rasterPosition;
	std::vector<Primitive*> primitives;

	// --------------------------------------------------------------------------------

	// Primitive 1
	std::unique_ptr<Primitive> primitive1(new Primitive(Math::Mat4::Identity()));
	primitives.clear();
	primitives.push_back(primitive1.get());
	camera->RegisterPrimitives(primitives);

	// Ray { p = (0, 0, 0), d = (0, 0, -1) }
	// -> Raster position (0.5, 0.5)
	EXPECT_TRUE(camera->RayToRasterPosition(Math::Vec3(), Math::Normalize(Math::Vec3(0, 0, -1)), rasterPosition));
	EXPECT_TRUE(ExpectVec2Near(Math::Vec2(0.5), rasterPosition));

	// Ray { p = (0, 0, 0), d = Normalize(2, 1, -1) }
	// -> Raster position (1, 1)
	EXPECT_TRUE(camera->RayToRasterPosition(Math::Vec3(), Math::Normalize(Math::Vec3(2, 1, -1)), rasterPosition));
	EXPECT_TRUE(ExpectVec2Near(Math::Vec2(1), rasterPosition));
	
	// --------------------------------------------------------------------------------

	// Primitive 2
	std::unique_ptr<Primitive> primitive2(new Primitive(Math::LookAt(Math::Vec3(1), Math::Vec3(0), Math::Vec3(0, 0, 1))));
	primitives.clear();
	primitives.push_back(primitive2.get());
	camera->RegisterPrimitives(primitives);

	// Ray { p = (1, 1, 1), d = Normalize(-1, -1, -1) }
	// -> Raster position (0.5, 0.5)
	EXPECT_TRUE(camera->RayToRasterPosition(Math::Vec3(1), Math::Normalize(Math::Vec3(-1)), rasterPosition));
	EXPECT_TRUE(ExpectVec2Near(Math::Vec2(0.5), rasterPosition));
}

//TEST_F(PerspectiveCameraTest, SamplePosition)
//{
//	FAIL() << "Not implemented";
//}
//
//TEST_F(PerspectiveCameraTest, EvaluateWe)
//{
//	FAIL() << "Not implemented";
//}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END