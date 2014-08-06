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
#include <lightmetrica.test/testscenes.h>
#include <lightmetrica/assets.h>
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
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/primitives.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class BPTFullpathTest2 : public TestBase {};

TEST_F(BPTFullpathTest2, Consistency)
{
	StubConfig config;
	ASSERT_TRUE(config.LoadFromString(TestScenes::Simple03(), ""));

	std::unique_ptr<Assets> assets(ComponentFactory::Create<Assets>());
	ASSERT_TRUE(assets->RegisterInterface<Texture>());
	ASSERT_TRUE(assets->RegisterInterface<BSDF>());
	ASSERT_TRUE(assets->RegisterInterface<TriangleMesh>());
	ASSERT_TRUE(assets->RegisterInterface<Film>());
	ASSERT_TRUE(assets->RegisterInterface<Camera>());
	ASSERT_TRUE(assets->RegisterInterface<Light>());
	ASSERT_TRUE(assets->Load(config.Root().Child("assets")));
	
	std::unique_ptr<Primitives> primitives(ComponentFactory::Create<Primitives>());
	ASSERT_TRUE(primitives->Load(config.Root().Child("scene"), *assets));
	std::unique_ptr<Scene> scene(ComponentFactory::Create<Scene>(config.Root().Child("scene").AttributeValue("type")));
	ASSERT_NE(scene, nullptr);
	scene->Load(primitives.release());
	ASSERT_TRUE(scene->Configure(config.Root().Child("scene")));
	ASSERT_TRUE(scene->Build());

	BPTPathVertexPool pool;
	BPTSubpath subpathL(TransportDirection::LE);
	BPTSubpath subpathE(TransportDirection::EL);

	std::unique_ptr<ConfigurableSampler> sampler(ComponentFactory::Create<ConfigurableSampler>("random"));
	ASSERT_TRUE(sampler->Configure(ConfigNode(), *assets));
	sampler->SetSeed(1);

	const int Samples = 1<<10;
	for (int sample = 0; sample < Samples; sample++)
	{
		pool.Release();
		subpathL.Clear();
		subpathE.Clear();
		subpathL.Sample(*scene, *sampler, pool, 3, -1);
		subpathE.Sample(*scene, *sampler, pool, 3, -1);

		const int nL = subpathL.NumVertices();
		const int nE = subpathE.NumVertices();
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
					connGeom = RenderUtils::GeneralizedGeometryTermWithVisibility(*scene, subpathL.Vertex(s-1)->geom, subpathE.Vertex(t-1)->geom);
					if (Math::Abs(connGeom) < Math::Constants::Eps())
					{
						continue;
					}
				}

				BPTFullPath fullpath(s, t, subpathL, subpathE);
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