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
#include <lightmetrica/defaultassets.h>
#include <lightmetrica/texture.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/film.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/light.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/scenefactory.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/bpt.config.h>
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/random.h>
#include <lightmetrica/renderutils.h>

namespace
{

	const std::string SceneFile = LM_TEST_MULTILINE_LITERAL(
		<assets>
			<triangle_meshes>
				<triangle_mesh id="quad" type="raw">
					<positions>
						-0.1 0 -0.1
						-0.1 0 0.1
						0.1 0 0.1
						0.1 0 -0.1
					</positions>
					<normals>
						0 -1 0
						0 -1 0
						0 -1 0
						0 -1 0
					</normals>
					<faces>
						0 2 1
						0 3 2
					</faces>
				</triangle_mesh>
			</triangle_meshes>
			<bsdfs>
				<bsdf id="diffuse_white" type="diffuse">
					<diffuse_reflectance>0.9 0.9 0.9</diffuse_reflectance>
				</bsdf>
				<bsdf id="diffuse_black" type="diffuse">
					<diffuse_reflectance>0 0 0</diffuse_reflectance>
				</bsdf>
				<bsdf id="diffuse_red" type="diffuse">
					<diffuse_reflectance>0.9 0.1 0.1</diffuse_reflectance>
				</bsdf>
				<bsdf id="diffuse_green" type="diffuse">
					<diffuse_reflectance>0.1 0.9 0.1</diffuse_reflectance>
				</bsdf>
			</bsdfs>
			<films>
				<film id="film_1" type="hdr">
					<width>500</width>
					<height>500</height>
					<imagetype>radiancehdr</imagetype>
				</film>
			</films>
			<cameras>
				<camera id="camera_1" type="perspective">
					<film ref="film_1" />
					<fovy>45</fovy>
				</camera>
			</cameras>
			<lights>
				<light id="light_1" type="area">
					<luminance>2 2 2</luminance>
				</light>
			</lights>
		</assets>
		<scene type="naive">
			<root>
				<node>
					<transform>
						<lookat>
							<position>0 0.1 0.3</position>
							<center>0 0.1 0</center>
							<up>0 1 0</up>
						</lookat>
					</transform>
					<camera ref="camera_1" />
				</node>
				<node>
					<transform>
						<rotate>
							<angle>-90</angle>
							<axis>1 0 0</axis>
						</rotate>
						<translate>0 0.1 -0.1</translate>
					</transform>
					<triangle_mesh ref="quad" />
					<bsdf ref="diffuse_white" />
				</node>
				<node>
					<transform>
						<translate>0 0.2 0</translate>
					</transform>
					<triangle_mesh ref="quad" />
					<light ref="light_1" />
					<bsdf ref="diffuse_black" />
				</node>
			</root>
		</scene>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class BPTPowerHeuristicsMISWeightTest : public TestBase {};

TEST_F(BPTPowerHeuristicsMISWeightTest, Consistency)
{
	StubConfig config;
	ASSERT_TRUE(config.LoadFromString(SceneFile, ""));

	DefaultAssets assets;
	assets.RegisterInterface<Texture>();
	assets.RegisterInterface<BSDF>();
	assets.RegisterInterface<TriangleMesh>();
	assets.RegisterInterface<Film>();
	assets.RegisterInterface<Camera>();
	assets.RegisterInterface<Light>();
	ASSERT_TRUE(assets.Load(config.Root().Child("assets")));

	SceneFactory sceneFactory;
	std::unique_ptr<Scene> scene(sceneFactory.Create(config.Root().Child("scene").AttributeValue("type")));
	ASSERT_NE(scene, nullptr);
	ASSERT_TRUE(scene->Load(config.Root().Child("scene"), assets));
	ASSERT_TRUE(scene->Configure(config.Root().Child("scene")));
	ASSERT_TRUE(scene->Build());

	BPTPathVertexPool pool;
	BPTSubpath lightSubpath(TransportDirection::LE);
	BPTSubpath eyeSubpath(TransportDirection::EL);

	BPTConfig bptConfig;
	bptConfig.rrDepth = 3;
	bptConfig.enableExperimentalMode = false;

	std::unique_ptr<Random> rng(ComponentFactory::Create<Random>("sfmt"));
	rng->SetSeed(1);

	// BPT weights
	std::unique_ptr<BPTMISWeight> misWeightFunc_Power(ComponentFactory::Create<BPTMISWeight>("power"));
	std::unique_ptr<BPTMISWeight> misWeightFunc_PowerNaive(ComponentFactory::Create<BPTMISWeight>("powernaive"));

	const int Samples = 1<<12;
	for (int sample = 0; sample < Samples; sample++)
	{
		lightSubpath.Release(pool);
		eyeSubpath.Release(pool);
		lightSubpath.Sample(bptConfig, *scene, *rng, pool);
		eyeSubpath.Sample(bptConfig, *scene, *rng, pool);

		const int nL = lightSubpath.NumVertices();
		const int nE = eyeSubpath.NumVertices();
		for (int s = 0; s <= nL; s++)
		{
			for (int t = 0; t <= nE; t++)
			{
				// # of vertices must be no less than 2
				const int n = s + t;
				if (n < 2)
				{
					continue;
				}

				BPTFullPath fullpath(s, t, lightSubpath, eyeSubpath);

				// Calculate contribution same as BPT implementation
				// in order to exclude zero-contribution cases.
				Math::Vec2 rasterPosition;
				auto Cstar = fullpath.EvaluateUnweightContribution(*scene, rasterPosition);
				if (Math::IsZero(Cstar))
				{
					continue;
				}

				// p_s should be non-zero (such cases are supposed to be excluded in above)
				auto ps = fullpath.EvaluateFullpathPDF(s);
				bool psIsZero = Math::Abs(ps) < Math::Constants::Eps();
				EXPECT_FALSE(psIsZero);

				// Calculate weights using different implementation
				auto weight_Power		= misWeightFunc_Power->Evaluate(fullpath);
				auto weight_PowerNaive	= misWeightFunc_PowerNaive->Evaluate(fullpath);

				// Compare weights
				auto result = ExpectNear(weight_Power, weight_PowerNaive);
				EXPECT_TRUE(result);
				if (!result)
				{
					LM_LOG_DEBUG("s     = " + std::to_string(s));
					LM_LOG_DEBUG("t     = " + std::to_string(t));
					LM_LOG_DEBUG("Cstar = (" + std::to_string(Cstar.x) + ", " + std::to_string(Cstar.y) + ", " + std::to_string(Cstar.z) + ")");
					//auto weight_Power		= misWeightFunc_Power->Evaluate(fullpath);
					//auto weight_PowerNaive	= misWeightFunc_PowerNaive->Evaluate(fullpath);
					for (int i = 0; i <= s + t; i++)
					{
						auto pi = fullpath.EvaluateFullpathPDF(i);
						LM_LOG_DEBUG(boost::str(boost::format("p%02d   = %f") % i % pi));
					}
					fullpath.DebugPrint();
				}
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END