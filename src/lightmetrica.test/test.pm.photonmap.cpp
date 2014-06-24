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
	Photons photons;
	std::mt19937 gen(42);
	std::uniform_real_distribution<double> dist;
	const int Samples = 1<<7;
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
	const int Queries = 1<<7;
	for (int query = 0; query < Queries; query++)
	{
		// Generate query point
		Math::Vec3 p(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
		for (size_t i = 0; i < photonMapTypes.size(); i++)
		{
			for (size_t j = i+1; j < photonMapTypes.size(); j++)
			{
				const int N = 10;
				for (int n = 1; n < N; n++)
				{
					const int Steps = 5;
					const Math::Float Delta = Math::Float(1) / Steps;
					for (int step = 0; step <= Steps; step++)
					{
						auto maxDist = Delta * step;
						auto maxDist2 = maxDist * maxDist;

						typedef std::pair<const Photon*, Math::Float> CollectedPhotonInfo;
						const auto comp = [](const CollectedPhotonInfo& p1, const CollectedPhotonInfo& p2){ return p1.second < p2.second; };

						std::vector<CollectedPhotonInfo> psi;
						std::vector<CollectedPhotonInfo> psj;

						const auto collectFunc = [&n, &comp](std::vector<CollectedPhotonInfo>& collectedPhotons, const Math::Vec3& p, const Photon& photon, Math::Float& maxDist2)
						{
							auto dist2 = Math::Length2(photon.p - p);
							if (collectedPhotons.size() < (size_t)n)
							{
								collectedPhotons.emplace_back(&photon, dist2);
								if (collectedPhotons.size() == (size_t)n)
								{
									// Create heap
									std::make_heap(collectedPhotons.begin(), collectedPhotons.end(), comp);
									maxDist2 = collectedPhotons.front().second;
								}
							}
							else
							{
								// Update heap
								std::pop_heap(collectedPhotons.begin(), collectedPhotons.end(), comp);
								collectedPhotons.back() = std::make_pair(&photon, dist2);
								std::push_heap(collectedPhotons.begin(), collectedPhotons.end(), comp);
								maxDist2 = collectedPhotons.front().second;
							}
						};

						auto maxDist2_i = maxDist2;
						photonMaps[i]->CollectPhotons(p, maxDist2_i, std::bind(collectFunc, std::ref(psi), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

						auto maxDist2_j = maxDist2;
						photonMaps[j]->CollectPhotons(p, maxDist2_j, std::bind(collectFunc, std::ref(psj), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

						// Check distances
						for (const auto& info : psi)
						{
							EXPECT_LE(info.second, maxDist2_i);
						}
						for (const auto& info : psj)
						{
							EXPECT_LE(info.second, maxDist2_j);
						}

						// Compare two results
						auto result = ExpectNear(maxDist2_i, maxDist2_j);
						EXPECT_TRUE(result);
						if (!result)
						{
							LM_LOG_DEBUG("i : " + photonMapTypes[i]);
							LM_LOG_DEBUG("j : " + photonMapTypes[j]);
							LM_LOG_DEBUG("maxDist2_i : " + std::to_string(maxDist2_i));
							LM_LOG_DEBUG("maxDist2_j : " + std::to_string(maxDist2_j));
						}

						EXPECT_EQ(psi.size(), psj.size());
						if (psi.size() == psj.size())
						{
							// Sort two arrays according to distance to #p
							std::sort(psi.begin(), psi.end(), comp);
							std::sort(psj.begin(), psj.end(), comp);

							// Compare elements
							bool failed = false;
							for (size_t k = 0; k < psi.size(); k++)
							{
								const auto* pi = psi[k].first;
								const auto* pj = psj[k].first;
								auto result = ExpectVec3Near(pi->p, pj->p);
								EXPECT_TRUE(result);
								if (!result)
								{
									failed = true;
								}
							}

							if (failed)
							{
								// Show a few elements
								LM_LOG_DEBUG("i : " + photonMapTypes[i]);
								LM_LOG_DEBUG("j : " + photonMapTypes[j]);
								for (size_t k = 0; k < psi.size(); k++)
								{
									const auto* pi = psi[k].first;
									const auto* pj = psj[k].first;
									LM_LOG_DEBUG("k = " + std::to_string(k) + ":");
									LM_LOG_INDENTER();
									LM_LOG_DEBUG("ps_i   : " + std::to_string(pi->p.x) + ", " + std::to_string(pi->p.y) + ", " + std::to_string(pi->p.z));
									LM_LOG_DEBUG("dist_i : " + std::to_string(Math::Length2(pi->p - p)));
									LM_LOG_DEBUG("ps_j   : " + std::to_string(pj->p.x) + ", " + std::to_string(pj->p.y) + ", " + std::to_string(pj->p.z));
									LM_LOG_DEBUG("dist_j : " + std::to_string(Math::Length2(pj->p - p)));
								}
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