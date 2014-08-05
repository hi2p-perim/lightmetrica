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
#include <lightmetrica/texture.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/film.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/light.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
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
		<scene type="qbvh">
			<intersection_mode>sse</intersection_mode>
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

/*
class BPTMISWeightTest : public TestBase {};

// Checks if the conditions in p.260 [Veach 1997] are preserved.
TEST_F(BPTMISWeightTest, Conditions)
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
	std::vector<std::unique_ptr<BPTMISWeight>> misWeightFuncs;
	misWeightFuncs.emplace_back(ComponentFactory::Create<BPTMISWeight>("simple"));
	misWeightFuncs.emplace_back(ComponentFactory::Create<BPTMISWeight>("power"));
	misWeightFuncs.emplace_back(ComponentFactory::Create<BPTMISWeight>("powernaive"));

	const int Samples = 1<<0;
	for (int sample = 0; sample < Samples; sample++)
	{
		lightSubpath.Release(pool);
		eyeSubpath.Release(pool);
		lightSubpath.Sample(bptConfig, *scene, *rng, pool);
		eyeSubpath.Sample(bptConfig, *scene, *rng, pool);

		const int nL = lightSubpath.NumVertices();
		const int nE = eyeSubpath.NumVertices();

		for (int n = 2; n <= nE + nL; n++)
		{
			std::vector<Math::Float> sumWeights(misWeightFuncs.size(), Math::Float(0));
		
			const int minS = Math::Max(0, n-nE);
			const int maxS = Math::Min(nL, n);
			
			for (int s = minS; s <= maxS; s++)
			{
				const int t = n - s;
				BPTFullPath fullpath(s, t, lightSubpath, eyeSubpath);

				// Calculate contribution same as BPT implementation
				// in order to exclude zero-contribution cases.
				Math::Vec2 rasterPosition;
				auto Cstar = fullpath.EvaluateUnweightContribution(*scene, rasterPosition);
				if (Math::IsZero(Cstar))
				{
					continue;
				}

				for (size_t i = 0; i < misWeightFuncs.size(); i++)
				{
					sumWeights[i] += misWeightFuncs[i]->Evaluate(fullpath);
				}
			}

			// Check sums
			for (size_t i = 0; i < misWeightFuncs.size(); i++)
			{
				auto result = ExpectNear(Math::Float(1), sumWeights[i]);
				EXPECT_TRUE(result);
				if (!result)
				{
					LM_LOG_DEBUG("Type : " + misWeightFuncs[i]->ComponentImplTypeName());
					LM_LOG_DEBUG("Sum  : " + std::to_string(sumWeights[i]));
				}
			}
		}
	}
}
*/

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END