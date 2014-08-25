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
#include <lightmetrica/bsdf.h>
#include <lightmetrica/random.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class SpecularBSDFTest : public TestBase {};

TEST_F(SpecularBSDFTest, FLoatingPointComp)
{
	// Assets & config
	StubAssets assets;	
	StubConfig config;

	// Random number generator
	std::unique_ptr<Random> rng(ComponentFactory::Create<Random>("sfmt"));
	rng->SetSeed(42);

	// Surface geometry
	SurfaceGeometry geom;
	geom.degenerated = false;
	geom.p = Math::Vec3();
	geom.sn = geom.gn = Math::Vec3(0, 1, 0);
	geom.ComputeTangentSpace();

	// --------------------------------------------------------------------------------

	// Create BSDFs
	std::vector<std::unique_ptr<BSDF>> bsdfs;
	
	bsdfs.emplace_back(ComponentFactory::Create<BSDF>("mirror"));
	EXPECT_TRUE(bsdfs.back()->Load(
		config.LoadFromStringAndGetFirstChild(
			LM_TEST_MULTILINE_LITERAL(
				<bsdf id="_" type="mirror">
					<specular_reflectance>1 1 1</specular_reflectance>
				</bsdf>
			)
		), assets));

	bsdfs.emplace_back(ComponentFactory::Create<BSDF>("dielectric"));
	EXPECT_TRUE(bsdfs.back()->Load(
		config.LoadFromStringAndGetFirstChild(
			LM_TEST_MULTILINE_LITERAL(
				<bsdf id="_" type="dielectric">
					<specular_reflectance>1 1 1</specular_reflectance>
					<specular_transmittance>1 1 1</specular_transmittance>
					<external_ior>1</external_ior>
					<internal_ior>1.458</internal_ior>
				</bsdf>
			)
		), assets));

	// --------------------------------------------------------------------------------

	for (auto& bsdf : bsdfs)
	{
		const int Samples = 1<<1;
		for (int sample = 0; sample < Samples; sample++)
		{
			GeneralizedBSDFSampleQuery bsdfSQ;
			bsdfSQ.sample = rng->NextVec2();
			bsdfSQ.uComp = rng->Next();
			bsdfSQ.transportDir = TransportDirection::LE;
			bsdfSQ.type = GeneralizedBSDFType::All;
			bsdfSQ.wi = Math::Normalize(Math::Vec3(Math::Float(1)));

			GeneralizedBSDFSampleResult bsdfSR;
			EXPECT_TRUE(bsdf->SampleDirection(bsdfSQ, geom, bsdfSR));
			EXPECT_FALSE(Math::IsZero(bsdfSR.pdf.v));
			EXPECT_EQ(bsdfSR.pdf.measure, Math::ProbabilityMeasure::ProjectedSolidAngle);

			// Check if EvaluateDirection returns non-zero value
			EXPECT_FALSE(Math::IsZero(bsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geom)));
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
