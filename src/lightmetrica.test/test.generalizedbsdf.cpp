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
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/random.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class GeneralizedBSDFTest : public TestBase
{
public:

	GeneralizedBSDFTest()
		: rng(ComponentFactory::Create<Random>("sfmt"))
	{
		
	}

public:

	virtual void SetUp()
	{
		TestBase::SetUp();
		
		// Random number generator
		rng->SetSeed(1);

		// Generalized BSDFs
		std::vector<std::tuple<std::string, std::string, std::string>> bsdfTypes;

		bsdfTypes.emplace_back(
			"bsdf",
			"dielectric",
			LM_TEST_MULTILINE_LITERAL(
				<bsdf id="glass" type="dielectric">
					<specular_reflectance>1 1 1</specular_reflectance>
					<specular_transmittance>1 1 1</specular_transmittance>
					<external_ior>1</external_ior>
					<internal_ior>1.458</internal_ior>
				</bsdf>
			));

		bsdfTypes.emplace_back(
			"bsdf",
			"diffuse",
			LM_TEST_MULTILINE_LITERAL(
				<bsdf type="diffuse_white" type="diffuse">
					<diffuse_reflectance>1 1 1</diffuse_reflectance>
				</bsdf>
			));

		bsdfTypes.emplace_back(
			"bsdf",
			"mirror",
			LM_TEST_MULTILINE_LITERAL(
				<bsdf id="mirror" type="mirror">
					<specular_reflectance>1 1 1</specular_reflectance>
				</bsdf>
			));

		bsdfTypes.emplace_back(
			"light",
			"area",
			LM_TEST_MULTILINE_LITERAL(
				<light id="light_1" type="area">
					<luminance>1 1 1</luminance>
					<testing>
						<area>1</area>
					</testing>
				</light>
			));

		bsdfTypes.emplace_back(
			"camera",
			"perspective",
			LM_TEST_MULTILINE_LITERAL(
				<camera id="camera_1" type="perspective">
					<fovy>90</fovy>
					<testing>
						<aspect>1</aspect>
						<lookat>
							<position>0 0 0</position>
							<center>0 0 -1</center>
							<up>0 1 0</up>
						</lookat>
					</testing>
				</camera>
			));

		for (auto& bsdfType : bsdfTypes)
		{
			bsdfs.emplace_back(dynamic_cast<GeneralizedBSDF*>(ComponentFactory::Create(std::get<0>(bsdfType), std::get<1>(bsdfType))));
			EXPECT_TRUE(bsdfs.back()->Load(config.LoadFromStringAndGetFirstChild(std::get<2>(bsdfType)), assets));
		}
	}

protected:

	std::vector<std::unique_ptr<GeneralizedBSDF>> bsdfs;
	std::unique_ptr<Random> rng;
	StubAssets assets;	
	StubConfig config;

};

TEST_F(GeneralizedBSDFTest, Consistency_PDF)
{
	for (auto& bsdf : bsdfs)
	{
		LM_LOG_DEBUG("Testing generalized BSDF type '" + bsdf->ComponentImplTypeName() + " (" + bsdf->ComponentInterfaceTypeName() + ")'");
		
		const int Samples = 1<<10;
		for (int sample = 0; sample < Samples; sample++)
		{
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = TransportDirection::EL;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = Math::Normalize(Math::Vec3(Math::Float(1)));

			SurfaceGeometry geom;
			geom.degenerated = false;
			geom.p = Math::Vec3();
			geom.sn = geom.gn = Math::Vec3(0, 1, 0);
			geom.ComputeTangentSpace();

			GeneralizedBSDFSampleResult bsdfSR;
			EXPECT_TRUE(bsdf->SampleDirection(bsdfSQ, geom, bsdfSR));
			EXPECT_FALSE(Math::IsZero(bsdfSR.pdf.v));
			EXPECT_EQ(bsdfSR.pdf.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			auto pdfD = bsdf->EvaluateDirectionPDF(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geom);
			EXPECT_FALSE(Math::IsZero(pdfD.v));
			EXPECT_EQ(pdfD.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			auto result = ExpectNear(bsdfSR.pdf.v, pdfD.v, Math::Float(1e-2));
			EXPECT_TRUE(result);
			if (!result)
			{
				LM_LOG_DEBUG("bsdfSR.pdf = " + std::to_string(bsdfSR.pdf.v));
				LM_LOG_DEBUG("pdfD.v     = " + std::to_string(pdfD.v));
			}
		}
	}
}

TEST_F(GeneralizedBSDFTest, Consistency_Fs)
{
	for (auto& bsdf : bsdfs)
	{
		LM_LOG_DEBUG("Testing generalized BSDF type '" + bsdf->ComponentImplTypeName() + " (" + bsdf->ComponentInterfaceTypeName() + ")'");

		const int Samples = 1<<10;
		for (int sample = 0; sample < Samples; sample++)
		{
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = TransportDirection::EL;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = Math::Normalize(Math::Vec3(Math::Float(1)));

			SurfaceGeometry geom;
			geom.degenerated = false;
			geom.p = Math::Vec3();
			geom.sn = geom.gn = Math::Vec3(0, 1, 0);
			geom.ComputeTangentSpace();

			GeneralizedBSDFSampleResult bsdfSR;
			EXPECT_TRUE(bsdf->SampleDirection(bsdfSQ, geom, bsdfSR));
			ASSERT_FALSE(Math::IsZero(bsdfSR.pdf.v));
			EXPECT_EQ(bsdfSR.pdf.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			auto fs = bsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geom);
			auto fsDivP1 = fs / bsdfSR.pdf.v;
			auto fsDivP2 = bsdf->SampleAndEstimateDirection(bsdfSQ, geom, bsdfSR);
			EXPECT_FALSE(Math::IsZero(fsDivP1));
			EXPECT_FALSE(Math::IsZero(fsDivP2));

			auto result = ExpectVec3Near(fsDivP1, fsDivP2, Math::Float(1e-2));
			EXPECT_TRUE(result);
			if (!result)
			{
				LM_LOG_DEBUG("fsDivP1 = " + std::to_string(fsDivP1.x) + " " + std::to_string(fsDivP1.y) + " " +std::to_string(fsDivP1.z));
				LM_LOG_DEBUG("fsDivP2 = " + std::to_string(fsDivP2.x) + " " + std::to_string(fsDivP2.y) + " " +std::to_string(fsDivP2.z));
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
