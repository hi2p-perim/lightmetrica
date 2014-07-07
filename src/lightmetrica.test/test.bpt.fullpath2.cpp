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
#include <lightmetrica/sampler.h>
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

class BPTFullpathTest2 : public TestBase {};

TEST_F(BPTFullpathTest2, Consistency)
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

	std::unique_ptr<Sampler> sampler(ComponentFactory::Create<Sampler>("random"));
	ASSERT_TRUE(sampler->Configure(ConfigNode(), assets));
	sampler->SetSeed(1);

	const int Samples = 1<<10;
	for (int sample = 0; sample < Samples; sample++)
	{
		pool.Release();
		lightSubpath.Clear();
		eyeSubpath.Clear();
		lightSubpath.Sample(bptConfig, *scene, *sampler, pool);
		eyeSubpath.Sample(bptConfig, *scene, *sampler, pool);

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
				
				// Between termination vertices geometry term must be positive
				// Otherwise, EvaluateFullpathPDFRatio is invalid due to offsetting of geometry terms.
				Math::Float connGeom(-1);
				if (s > 0 && t > 0)
				{
					connGeom = RenderUtils::GeneralizedGeometryTermWithVisibility(*scene, lightSubpath.GetVertex(s-1)->geom, eyeSubpath.GetVertex(t-1)->geom);
					if (Math::Abs(connGeom) < Math::Constants::Eps())
					{
						continue;
					}
				}

				BPTFullPath fullpath(s, t, lightSubpath, eyeSubpath);
				auto ps = fullpath.EvaluateFullpathPDF(s);
				if (Math::Abs(ps) < Math::Constants::Eps())
				{
					// EvaluateFullpathPDFRatio is invalid if p_s is zero
					continue;
				}

				for (int i = 0; i < n; i++)
				{
					auto pi		= fullpath.EvaluateFullpathPDF(i);
					auto piNext	= fullpath.EvaluateFullpathPDF(i+1);
					auto ratio	= fullpath.EvaluateFullpathPDFRatio(i);

					bool piIsZero		= Math::Abs(pi) < Math::Constants::Eps();
					bool piNextIsZero	= Math::Abs(piNext) < Math::Constants::Eps();

					// We note that we have only to check the case with p_i and p_{i+1} are both non-zero
					// because in actual weight calculation (cf. bpt.mis.power.cpp)
					// calculation of the ratio is aborted immediately after p_i or p_{i+1} is found to be non-zero.
					bool cond = false;
					if (piIsZero && piNextIsZero)
					{
						continue;
					}
					else if (piIsZero)
					{
						auto result = ExpectNear(Math::Float(0), ratio);
						EXPECT_TRUE(result);
						cond = result;
					}
					else if (piNextIsZero)
					{
						auto result = ExpectNear(Math::Float(0), ratio);
						EXPECT_TRUE(result);
						cond = result;
					}
					else
					{
						auto result = ExpectNearRelative(ratio, piNext / pi, Math::Constants::EpsLarge());
						EXPECT_TRUE(result);
						cond = result;
					}

					if (!cond)
					{
						LM_LOG_DEBUG("Evaluating i = " + std::to_string(i));
						LM_LOG_DEBUG("connGeom = " + std::to_string(connGeom));
						LM_LOG_DEBUG("ps       = " + std::to_string(ps));
						LM_LOG_DEBUG("pi       = " + std::to_string(pi));
						LM_LOG_DEBUG("piNext   = " + std::to_string(piNext));
						LM_LOG_DEBUG("ratio    = " + std::to_string(ratio));
						fullpath.DebugPrint();
					}
				}
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END