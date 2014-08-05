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
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bsdf.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

struct BPTSubpaths
{

	BPTSubpaths()
		: lightSubpath(TransportDirection::LE)
		, eyeSubpath(TransportDirection::EL)
	{

	}

	~BPTSubpaths()
	{
		pool.Release();
	}

	BPTPathVertexPool pool;
	BPTSubpath lightSubpath;
	BPTSubpath eyeSubpath;

};

class BPTFullpathTest : public TestBase
{
public:

	BPTFullpathTest()
		: light(ComponentFactory::Create<Light>("area"))
		, camera(ComponentFactory::Create<Camera>("perspective"))
		, diffuseBSDF_White(ComponentFactory::Create<BSDF>("diffuse"))
	{
		// Load assets
		{
			EXPECT_TRUE(light->Load(config.LoadFromStringAndGetFirstChild(
				LM_TEST_MULTILINE_LITERAL(
					<light id="light_1" type="area">
						<luminance>1 1 1</luminance>
						<testing>
							<area>1</area>
						</testing>
					</light>
				)
			), assets));
	
			EXPECT_TRUE(camera->Load(config.LoadFromStringAndGetFirstChild(
				LM_TEST_MULTILINE_LITERAL(
					<camera id="camera_1" type="perspective">
						<fovy>90</fovy>
						<testing>
							<aspect>1</aspect>
							<lookat>
								<position>2 1 0</position>
								<center>1 0 0</center>
								<up>0 1 0</up>
							</lookat>
						</testing>
					</camera>
				)
			), assets));
	
			EXPECT_TRUE(diffuseBSDF_White->Load(config.LoadFromStringAndGetFirstChild(
				LM_TEST_MULTILINE_LITERAL(
					<bsdf id="diffuse_white" type="diffuse">
						<diffuse_reflectance>1 1 1</diffuse_reflectance>
					</bsdf>
				)
			), assets));
		}

		// Create light sub-path and eye sub-path
		{
			// y0 : Light
			auto* y0 = subpaths.pool.Construct();
			y0->type = BPTPathVertexType::EndPoint;
			y0->transportDir = TransportDirection::LE;
			y0->geom.degenerated = false;
			y0->geom.p = Math::Vec3(0);
			y0->geom.gn = y0->geom.sn = Math::Vec3(0, 1, 0);
			y0->geom.ComputeTangentSpace();
			y0->wi = Math::Vec3();
			y0->wo = Math::Normalize(Math::Vec3(1, 1, 0));
			y0->emitter = light.get();
			y0->areaLight = light.get();
			y0->areaCamera = nullptr;
			y0->bsdf = light.get();
			y0->pdfP = light->EvaluatePositionPDF(y0->geom);
			y0->pdfD[TransportDirection::LE] = light->EvaluateDirectionPDF(
				GeneralizedBSDFEvaluateQuery(GeneralizedBSDFType::LightDirection, y0->transportDir, y0->wi, y0->wo), y0->geom);
			y0->pdfD[TransportDirection::EL] = Math::PDFEval();
			y0->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
			subpaths.lightSubpath.vertices.push_back(y0);

			// y1 : Terminated
			auto* y1 = subpaths.pool.Construct();
			y1->type = BPTPathVertexType::IntermediatePoint;
			y1->transportDir = TransportDirection::LE;
			y1->geom.degenerated = false;
			y1->geom.p = Math::Vec3(1, 1, 0);
			y1->geom.gn = y1->geom.sn = Math::Vec3(0, -1, 0);
			y1->geom.ComputeTangentSpace();
			y1->wi = Math::Normalize(Math::Vec3(-1, -1, 0));
			y1->wo = Math::Vec3();	// Invalid
			y1->emitter = nullptr;
			y1->areaLight = nullptr;
			y1->areaCamera = nullptr;
			y1->bsdf = diffuseBSDF_White.get();
			y1->pdfP = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::Area);
			y1->pdfD[TransportDirection::LE] = Math::PDFEval();		// Invalid
			y1->pdfD[TransportDirection::EL] = Math::PDFEval();		// Invalid
			y1->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
			subpaths.lightSubpath.vertices.push_back(y1);

			// z0 : Camera
			auto* z0 = subpaths.pool.Construct();
			z0->type = BPTPathVertexType::EndPoint;
			z0->transportDir = TransportDirection::EL;
			z0->geom.degenerated = true;
			z0->geom.p = Math::Vec3(2, 1, 0);
			z0->wi = Math::Vec3();	// Invalid
			z0->wo = Math::Normalize(Math::Vec3(-1, -1, 0));
			z0->emitter = camera.get();
			z0->areaLight = nullptr;
			z0->areaCamera = nullptr;
			z0->bsdf = camera.get();
			z0->pdfP = camera->EvaluatePositionPDF(z0->geom);
			z0->pdfD[TransportDirection::LE] = Math::PDFEval();
			z0->pdfD[TransportDirection::EL] = camera->EvaluateDirectionPDF(
				GeneralizedBSDFEvaluateQuery(GeneralizedBSDFType::EyeDirection, z0->transportDir, z0->wi, z0->wo), z0->geom);
			z0->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
			subpaths.eyeSubpath.vertices.push_back(z0);

			// z1 : Terminated
			auto* z1 = subpaths.pool.Construct();
			z1->type = BPTPathVertexType::IntermediatePoint;
			z1->transportDir = TransportDirection::EL;
			z1->geom.degenerated = false;
			z1->geom.p = Math::Vec3(1, 0, 0);
			z1->geom.gn = z1->geom.sn = Math::Vec3(0, 1, 0);
			z1->geom.ComputeTangentSpace();
			z1->wi = Math::Normalize(Math::Vec3(1, 1, 0));
			z1->wo = Math::Vec3();	// Invalid
			z1->emitter = nullptr;
			z1->areaLight = nullptr;
			z1->areaCamera = nullptr;
			z1->bsdf = diffuseBSDF_White.get();
			z1->pdfP = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::Area);
			z1->pdfD[TransportDirection::LE] = Math::PDFEval();		// Invalid
			z1->pdfD[TransportDirection::EL] = Math::PDFEval();		// Invalid
			z1->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
			subpaths.eyeSubpath.vertices.push_back(z1);
		}
	}

public:

	StubAssets assets;
	StubConfig config;

	std::unique_ptr<Light> light;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<BSDF> diffuseBSDF_White;

	BPTSubpaths subpaths;

};

TEST_F(BPTFullpathTest, EvaluateFullpathPDF)
{
	// Create full-path
	BPTFullPath fullpath(2, 2, subpaths.lightSubpath, subpaths.eyeSubpath);

	// Evaluate full-path PDF
	// For notations see p.303 in [Veach 1997].

	Math::Float expected;
	Math::Float actual;

	// Hand-calculated values:
	//   p_A(x_0) = 1
	//   p_A(x_3) = 1
	//   p_{\sigma^\bot}(x_3\to x_2) = 1/4
	//   p_{\sigma^\bot}(x_2\to x_1) = 1 / \pi
	//   p_{\sigma^\bot}(x_1\to x_0) = 1 / \pi
	//   p_{\sigma^\bot}(x_0\to x_1) = 1 / \pi
	//   p_{\sigma^\bot}(x_1\to x_2) = 1 / \pi
	//   p_{\sigma^\bot}(x_2\to x_3) = 0
	//   G(x_3\leftrightarrow x_2) = \sqrt{2} / 4
	//   G(x_2\leftrightarrow x_1) = 1
	//   G(x_1\leftrightarrow x_0) = 1 / 4

	//
	// (1)
	//
	// p_0 = p_{0,4} = p^L_0 * p^E_4
	//
	// Here,
	//   p^L_0 = 1
	//   p^E_4 = p_A(x_3)
	//         * p_{\sigma^\bot}(x_3\to x_2) * G(x_3\leftrightarrow x_2)
	//         * p_{\sigma^\bot}(x_2\to x_1) * G(x_2\leftrightarrow x_1)
	//         * p_{\sigma^\bot}(x_1\to x_0) * G(x_1\leftrightarrow x_0)
	// 
	// Then,
	//   p_0 = \sqrt{2} / (64 * \pi * \pi).
	// 
	expected = Math::Sqrt(Math::Float(2)) / (Math::Float(64) * Math::Constants::Pi() * Math::Constants::Pi());
	actual = fullpath.EvaluateFullpathPDF(0);
	EXPECT_TRUE(ExpectNear(expected, actual));

	//
	// (2)
	//
	// p_1 = p_{1,3} = p^L_1 * p^E_3
	//
	// Here,
	//   p^L_1 = p_A(x_0)
	//   p^E_3 = p_A(x_3)
	//         * p_{\sigma^\bot}(x_3\to x_2) * G(x_3\leftrightarrow x_2)
	//         * p_{\sigma^\bot}(x_2\to x_1) * G(x_2\leftrightarrow x_1)
	//
	// Then,
	//   p_1 = \sqrt{2} / (16 * \pi).
	//
	expected = Math::Sqrt(Math::Float(2)) / (Math::Float(16) * Math::Constants::Pi());
	actual = fullpath.EvaluateFullpathPDF(1);
	EXPECT_TRUE(ExpectNear(expected, actual));

	//
	// (3)
	//
	// p_2 = p_{2,2} = p^L_2 * p^E_2
	//
	// Here,
	//   p^L_2 = p_A(x_0)
	//         * p_{\sigma^\bot}(x_0\to x_1) * G(x_0\leftrightarrow x_1)
	//   p^E_2 = p_A(x_3)
	//         * p_{\sigma^\bot}(x_3\to x_2) * G(x_3\leftrightarrow x_2)
	//
	// Then,
	//   p_2 = \sqrt{2} / (64 * \pi).
	//
	expected = Math::Sqrt(Math::Float(2)) / (Math::Float(64) * Math::Constants::Pi());
	actual = fullpath.EvaluateFullpathPDF(2);
	EXPECT_TRUE(ExpectNear(expected, actual));

	//
	// (4)
	//
	// p_3 = p_{3,1} = p^L_3 * p^E_1
	//
	// Here,
	//   p^L_3 = p_A(x_0)
	//         * p_{\sigma^\bot}(x_0\to x_1) * G(x_0\leftrightarrow x_1)
	//         * p_{\sigma^\bot}(x_1\to x_2) * G(x_1\leftrightarrow x_2)
	//   p^E_1 = p_A(x_3)
	//
	// Then,
	//   p_3 = 1 / (4 * \pi * \pi).
	//
	expected = Math::Float(1) / (Math::Float(4) * Math::Constants::Pi() * Math::Constants::Pi());
	actual = fullpath.EvaluateFullpathPDF(3);
	EXPECT_TRUE(ExpectNear(expected, actual));

	//
	// (5)
	//
	// p_4 = p_{4,0} = p^L_4 * p^E_0
	//
	// Here,
	//   p^L_4 = p_A(x_0)
	//         * p_{\sigma^\bot}(x_0\to x_1) * G(x_0\leftrightarrow x_1)
	//         * p_{\sigma^\bot}(x_1\to x_2) * G(x_1\leftrightarrow x_2)
	//         * p_{\sigma^\bot}(x_2\to x_3) * G(x_2\leftrightarrow x_3)
	//   p^E_0 = 1
	//
	// Then,
	//   p_4 = 0
	//
	expected = Math::Float(0);
	actual = fullpath.EvaluateFullpathPDF(4);
	EXPECT_TRUE(ExpectNear(expected, actual));

}

TEST_F(BPTFullpathTest, EvaluateFullpathPDFRatio)
{
	BPTFullPath fullpath(2, 2, subpaths.lightSubpath, subpaths.eyeSubpath);
	Math::Float expected;
	Math::Float actual;

	// (1)
	expected = Math::Float(4) * Math::Constants::Pi();
	actual = fullpath.EvaluateFullpathPDFRatio(0);
	EXPECT_TRUE(ExpectNear(expected, actual));

	// (2)
	expected = Math::Float(1) / Math::Float(4);
	actual = fullpath.EvaluateFullpathPDFRatio(1);
	EXPECT_TRUE(ExpectNear(expected, actual));

	// (3)
	expected = Math::Float(8) * Math::Sqrt(Math::Float(2)) / Math::Constants::Pi();
	actual = fullpath.EvaluateFullpathPDFRatio(2);
	EXPECT_TRUE(ExpectNear(expected, actual));

	// (4)
	expected = Math::Float(0);
	actual = fullpath.EvaluateFullpathPDFRatio(3);
	EXPECT_TRUE(ExpectNear(expected, actual));
}

TEST_F(BPTFullpathTest, Consistency)
{
	for (int s = 0; s <= 2; s++)
	{
		for (int t = 0; t <= 2; t++)
		{
			const int n = s + t;
			if (n < 2)
			{
				continue;
			}

			BPTFullPath fullpath(s, t, subpaths.lightSubpath, subpaths.eyeSubpath);
			for (int i = 0; i < n; i++)
			{
				auto pi		= fullpath.EvaluateFullpathPDF(i);
				auto piNext	= fullpath.EvaluateFullpathPDF(i+1);
				auto ratio	= fullpath.EvaluateFullpathPDFRatio(i);
				if (Math::Abs(pi) < Math::Constants::Eps())
				{
					EXPECT_TRUE(Math::Abs(ratio) < Math::Constants::Eps());
				}
				else
				{
					EXPECT_TRUE(ExpectNear(ratio, piNext / pi));
				}
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END