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
#include <lightmetrica/renderer.h>
#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/random.h>
#include <lightmetrica/align.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/light.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/align.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

typedef std::pair<Math::Vec2, Math::Vec3> Splat;
typedef std::vector<Splat, aligned_allocator<Splat, std::alignment_of<Splat>::value>> Splats;

/*!
	Primary sample space Metropolis light transport renderer.
	PSSMLT implementation with BPT as an underlying light path sampler.
	Reference:
		Kelemen, C., Szirmay-Kalos, L., Antal, G., and Csonka, F.,
		A simple and robust mutation strategy for the metropolis light transport algorithm,
		In Computer Graphics Forum. pp. 531–540, 2002.
*/
class BPTPSSMLTRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pssmlt.bpt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& scene );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { boost::signals2::signal<void (double, bool)> signal_ReportProgress; }

private:

	void SampleAndEvaluateBPTPaths(const Scene& scene, PSSMLTSampler& sampler, Splats& splats) const;

public:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of sample mutations
	int rrDepth;					// Depth of beginning RR
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block
	std::string rngType;			// Type of random number generator

	long long numSeedSamples;		// Number of seed samples
	Math::Float largeStepProb;		// Large step mutation probability
	Math::Float kernelSizeS1;		// Minimum kernel size
	Math::Float kernelSizeS2;		// Maximum kernel size

};

bool BPTPSSMLTRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}
	node.ChildValueOrDefault("samples_per_block", 100LL, samplesPerBlock);
	if (samplesPerBlock <= 0)
	{
		LM_LOG_ERROR("Invalid value for 'samples_per_block'");
		return false;
	}
	node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
	if (!ComponentFactory::CheckRegistered<Random>(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}
	node.ChildValueOrDefault("num_seed_samples", 1LL, numSeedSamples);
	node.ChildValueOrDefault("large_step_prob", Math::Float(0.1), largeStepProb);
	node.ChildValueOrDefault("kernel_size_s1", Math::Float(1.0 / 1024.0), kernelSizeS1);
	node.ChildValueOrDefault("kernel_size_s2", Math::Float(1.0 / 64.0), kernelSizeS2);

	return true;
}

bool BPTPSSMLTRenderer::Preprocess( const Scene& scene )
{
	signal_ReportProgress(0, false);

	// Take #numSeedSamples samples with BPT and generate seeds for each thread
	std::vector<std::pair<int, Math::Float>> candidates;
	std::unique_ptr<Random> rng(ComponentFactory::Create<Random>(rngType));
	PSSMLTRestorableSampler sampler(rng.get(), static_cast<int>(std::time(nullptr)));

	for (long long sample = 0; sample < numSeedSamples; sample++)
	{
		// Current sample index
		int index = sampler.Index();

		// Sample light paths
		// Note that BPT might generate samples over multiple raster position
		
	}







}

bool BPTPSSMLTRenderer::Render( const Scene& scene )
{

}

void BPTPSSMLTRenderer::SampleAndEvaluateBPTPaths( const Scene& scene, PSSMLTSampler& sampler, Splats& splats ) const
{

}

LM_COMPONENT_REGISTER_IMPL(BPTPSSMLTRenderer, Renderer);

LM_NAMESPACE_END