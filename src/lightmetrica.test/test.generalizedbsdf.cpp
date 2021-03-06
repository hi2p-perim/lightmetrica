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
					<diffuse_reflectance><color>1 1 1</color></diffuse_reflectance>
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
	SurfaceGeometry geom;
	geom.degenerated = false;
	geom.p = Math::Vec3();
	geom.sn = geom.gn = Math::Vec3(0, 1, 0);
	geom.ComputeTangentSpace();

	for (auto& bsdf : bsdfs)
	{
		LM_LOG_DEBUG("Testing generalized BSDF type '" + bsdf->ComponentImplTypeName() + " (" + bsdf->ComponentInterfaceTypeName() + ")'");
		
		const int Samples = 1<<10;
		for (int sample = 0; sample < Samples; sample++)
		{
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = (bsdf->BSDFTypes() & GeneralizedBSDFType::LightDirection) != 0 ? TransportDirection::LE : TransportDirection::EL;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = Math::Normalize(Math::Vec3(Math::Float(1)));

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

TEST_F(GeneralizedBSDFTest, Consistency_SampleAndEstimateDirection)
{
	SurfaceGeometry geom;
	geom.degenerated = false;
	geom.p = Math::Vec3();
	geom.sn = geom.gn = Math::Vec3(0, 1, 0);
	geom.ComputeTangentSpace();

	for (auto& bsdf : bsdfs)
	{
		LM_LOG_DEBUG("Testing generalized BSDF type '" + bsdf->ComponentImplTypeName() + " (" + bsdf->ComponentInterfaceTypeName() + ")'");

		const int Samples = 1<<10;
		for (int sample = 0; sample < Samples; sample++)
		{
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = (bsdf->BSDFTypes() & GeneralizedBSDFType::LightDirection) != 0 ? TransportDirection::LE : TransportDirection::EL;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = Math::Normalize(Math::Vec3(Math::Float(1)));

			GeneralizedBSDFSampleResult bsdfSR;
			EXPECT_TRUE(bsdf->SampleDirection(bsdfSQ, geom, bsdfSR));
			EXPECT_FALSE(Math::IsZero(bsdfSR.pdf.v));
			EXPECT_EQ(bsdfSR.pdf.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);
			auto fs = bsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geom);
			EXPECT_FALSE(Math::IsZero(fs));
			auto w1 = fs / bsdfSR.pdf.v;
			
			GeneralizedBSDFSampleResult bsdfSR2;
			auto w2 = bsdf->SampleAndEstimateDirection(bsdfSQ, geom, bsdfSR2);
			EXPECT_FALSE(Math::IsZero(w2));
			EXPECT_FALSE(Math::IsZero(bsdfSR2.pdf.v));
			EXPECT_EQ(bsdfSR2.pdf.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			EXPECT_TRUE(ExpectNear(bsdfSR.pdf.v, bsdfSR2.pdf.v));
			auto result = ExpectVec3Near(w1, w2, Math::Float(1e-2));
			EXPECT_TRUE(result);
			if (!result)
			{
				LM_LOG_DEBUG("w1 = " + std::to_string(w1.x) + " " + std::to_string(w1.y) + " " +std::to_string(w1.z));
				LM_LOG_DEBUG("w2 = " + std::to_string(w2.x) + " " + std::to_string(w2.y) + " " +std::to_string(w2.z));
			}
		}
	}
}

TEST_F(GeneralizedBSDFTest, Consistency_SampleAndEstimateDirectionBidir)
{
	SurfaceGeometry geom;
	geom.degenerated = false;
	geom.p = Math::Vec3();
	geom.sn = geom.gn = Math::Vec3(0, 1, 0);
	geom.ComputeTangentSpace();

	for (auto& bsdf : bsdfs)
	{
		LM_LOG_DEBUG("Testing generalized BSDF type '" + bsdf->ComponentImplTypeName() + " (" + bsdf->ComponentInterfaceTypeName() + ")'");
		
		const int Samples = 1<<9;
		for (int sample = 0; sample < Samples; sample++)
		{
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = (bsdf->BSDFTypes() & GeneralizedBSDFType::LightDirection) != 0 ? TransportDirection::LE : TransportDirection::EL;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = Math::Normalize(Math::Vec3(Math::Float(1)));

			GeneralizedBSDFSampleBidirResult bsdfSR;
			EXPECT_TRUE(bsdf->SampleAndEstimateDirectionBidir(bsdfSQ, geom, bsdfSR));
			EXPECT_EQ(bsdfSR.pdf[bsdfSQ.transportDir].measure, Math::ProbabilityMeasure::ProjectedSolidAngle);
			EXPECT_EQ(bsdfSR.pdf[1-bsdfSQ.transportDir].measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			GeneralizedBSDFSampleResult bsdfSR2;
			auto w2_0 = bsdf->SampleAndEstimateDirection(bsdfSQ, geom, bsdfSR2);
			EXPECT_EQ(bsdfSR2.pdf.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			EXPECT_TRUE(ExpectNear(bsdfSR.pdf[bsdfSQ.transportDir].v, bsdfSR2.pdf.v, Math::Float(1e-2)));
			EXPECT_TRUE(ExpectVec3Near(bsdfSR.weight[bsdfSQ.transportDir], w2_0, Math::Float(1e-2)));

			// Opposite direction
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = bsdfSR2.sampledType;
			bsdfEQ.transportDir = TransportDirection(1 - bsdfSQ.transportDir);
			bsdfEQ.wi = bsdfSR2.wo;
			bsdfEQ.wo = bsdfSQ.wi;
			auto fs2_1 = bsdf->EvaluateDirection(bsdfEQ, geom);
			auto pdfD2_1 = bsdf->EvaluateDirectionPDF(bsdfEQ, geom);
			EXPECT_EQ(pdfD2_1.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);
			auto w2_1 = Math::IsZero(pdfD2_1.v) ? Math::Vec3() : fs2_1 / pdfD2_1.v;

			EXPECT_TRUE(ExpectNear(bsdfSR.pdf[1-bsdfSQ.transportDir].v, pdfD2_1.v, Math::Float(1e-2)));
			EXPECT_TRUE(ExpectVec3Near(bsdfSR.weight[1-bsdfSQ.transportDir], w2_1, Math::Float(1e-2)));
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
