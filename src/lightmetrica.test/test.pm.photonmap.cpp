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
#include <lightmetrica/pm.photonmap.h>
#include <random>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class PhotonMapTest : public TestBase {};

TEST_F(PhotonMapTest, Consistency)
{
	std::vector<std::string> photonMapTypes;
	photonMapTypes.emplace_back("naive");
	photonMapTypes.emplace_back("kdtree");

	// Create photon map with random photons
	std::vector<std::unique_ptr<PhotonMap>> photonMaps;
	std::vector<Photon> photons;
	std::mt19937 gen(42);
	std::uniform_real_distribution<double> dist;
	const int Samples = 1<<10;
	for (int i = 0; i < Samples; i++)
	{
		Photon photon;
		photon.p = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
		photons.push_back(photon);
	}
	for (auto& type : photonMapTypes)
	{
		photonMaps.emplace_back(ComponentFactory::Create<PhotonMap>(type));
		photonMaps.back()->Build(photons);
	}

	// Compare results for sample queries
	const int Queries = 1<<5;
	for (int query = 0; query < Queries; query++)
	{
		// Generate query point
		Math::Vec3 p(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
		for (size_t i = 0; i < photonMapTypes.size(); i++)
		{
			for (size_t j = i+1; j < photonMapTypes.size(); j++)
			{
				const int N = 10;
				for (int n = 0; n < N; n++)
				{
					const int Steps = 10;
					const Math::Float Delta = Math::Float(1) / Steps;
					for (int step = 0; step <= Steps; step++)
					{
						auto maxDist = Delta * step;
						auto maxDist2 = maxDist * maxDist;
						std::vector<const Photon*> psi;
						std::vector<const Photon*> psj;
						photonMaps[i]->CollectPhotons(n, p, psi, maxDist2);
						photonMaps[j]->CollectPhotons(n, p, psj, maxDist2);

						// Compare two results
						EXPECT_EQ(psi.size(), psj.size());
						if (psi.size() == psj.size())
						{
							// Sort two arrays according to distance to #p
							const auto comp = [&p](const Photon* p1, const Photon* p2)
							{
								return Math::Length2(p1->p - p) < Math::Length2(p2->p - p);
							};
							std::sort(psi.begin(), psi.end(), comp);
							std::sort(psj.begin(), psj.end(), comp);

							// Compare elements
							for (size_t k = 0; k < psi.size(); k++)
							{
								EXPECT_EQ(psi[i], psj[j]);
							}
						}
					}
				}
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END