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

#include "pch.plugin.h"
#include <lightmetrica/renderer.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/bitmap.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/light.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <thread>
#include <atomic>
#include <omp.h>
#include <mpi.h>

#ifndef LM_MPI
#error "MPI must be enabled"
#endif

LM_NAMESPACE_BEGIN

enum MPIPTTagType
{
	TagType_Result			= 0,
	TagType_AssignTask		= 1,
	TagType_TaskFinished	= 2,
	TagType_GatherImage		= 3,
	TagType_Exit			= 4,
};

/*!
	Path trace renderer using MPI.
	An implementation of path tracing using MPI.
*/
class MPIPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pt.mpi");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void SetTerminationMode( RendererTerminationMode mode, double time ) { terminationMode = mode; terminationTime = time; }
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(1, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }
	
private:

	void ProcessRenderSingleSample(const Scene& scene, Sampler& sampler, Film& film) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;
	RendererTerminationMode terminationMode;
	double terminationTime;

private:

	long long numSamples;									// Number of samples
	int rrDepth;											// Depth of beginning RR
	int maxPathVertices;									// Maximum number of light path vertices
	int numThreads;											// Number of threads
	long long samplesPerTask;								// Number of samples per MPI task
	long long samplesPerBlock;								// Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;	// Sampler

};

bool MPIPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("max_path_vertices", -1, maxPathVertices);
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}
	node.ChildValueOrDefault("samples_per_mpi_task", 1000000LL, samplesPerTask);
	node.ChildValueOrDefault("samples_per_block", 100LL, samplesPerBlock);
	if (samplesPerBlock <= 0)
	{
		LM_LOG_ERROR("Invalid value for 'samples_per_block'");
		return false;
	}

	// Sampler
	auto samplerNode = node.Child("sampler");
	initialSampler.reset(ComponentFactory::Create<ConfigurableSampler>(samplerNode.AttributeValue("type")));
	if (initialSampler == nullptr || !initialSampler->Configure(samplerNode, assets))
	{
		LM_LOG_ERROR("Invalid sampler");
		return false;
	}

	// Set number of threads
	omp_set_num_threads(numThreads);

	return true;
}

bool MPIPathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();

	// --------------------------------------------------------------------------------

	int rank;
	int numProcs;
	int procNameLen;
	char procName[MPI_MAX_PROCESSOR_NAME];

	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(procName, &procNameLen);

	// --------------------------------------------------------------------------------

	// Random number generators and films
	std::vector<std::unique_ptr<Sampler>> samplers;
	std::vector<std::unique_ptr<Film>> films;
	if (rank > 0)
	{
		for (int i = 0; i < numThreads; i++)
		{
			samplers.emplace_back(initialSampler->Clone());
			samplers.back()->SetSeed(initialSampler->NextUInt());
			films.emplace_back(masterFilm->Clone());
		}
	}

	// --------------------------------------------------------------------------------

	MPI_Status status;
	long long processedSamples = 0;

	if (rank == 0)
	{
		// # Master process
		signal_ReportProgress(0, false);
		auto startTime = std::chrono::high_resolution_clock::now();

		// Number of samples queried
		long long queriedSamples = 0;

		// --------------------------------------------------------------------------------

		// ## Assign initial tasks to slave processes
		for (int i = 1; i < numProcs; i++)
		{
			// Number of samples to be processed by this task
			long long samples = terminationMode == RendererTerminationMode::Time ? samplesPerTask : Math::Min(samplesPerTask, numSamples - queriedSamples);
			MPI_Send(&samples, 1, MPI_LONG_LONG, i, TagType_AssignTask, MPI_COMM_WORLD);
			queriedSamples += samples;
		}

		// --------------------------------------------------------------------------------

		// ## Dispatch render tasks
		while (terminationMode == RendererTerminationMode::Time || (terminationMode == RendererTerminationMode::Samples && processedSamples < numSamples))
		{
			// Wait for result
			long long processedSamplesBySlave;
			MPI_Recv(&processedSamplesBySlave, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, TagType_TaskFinished, MPI_COMM_WORLD, &status);
			processedSamples += processedSamplesBySlave;

			// --------------------------------------------------------------------------------

			// Progress report
			if (terminationMode == RendererTerminationMode::Samples)
			{
				auto progress = static_cast<double>(processedSamples) / numSamples;
				signal_ReportProgress(progress, false);
			}
			else if (terminationMode == RendererTerminationMode::Time)
			{
				auto currentTime = std::chrono::high_resolution_clock::now();
				double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count()) / 1000.0;

				if (elapsed > terminationTime)
				{
					// Wait for remaining tasks
					while (processedSamples < queriedSamples)
					{
						MPI_Recv(&processedSamplesBySlave, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, TagType_TaskFinished, MPI_COMM_WORLD, &status);
						processedSamples += processedSamplesBySlave;
					}

					break;
				}
				else
				{
					signal_ReportProgress(elapsed / terminationTime, false);
				}
			}

			// --------------------------------------------------------------------------------

			// Assign next task if necessary
			if (terminationMode == RendererTerminationMode::Time || (terminationMode == RendererTerminationMode::Samples && queriedSamples < numSamples))
			{
				long long samples = terminationMode == RendererTerminationMode::Time ? samplesPerTask : Math::Min(samplesPerTask, numSamples - queriedSamples);
				MPI_Send(&samples, 1, MPI_LONG_LONG, status.MPI_SOURCE, TagType_AssignTask, MPI_COMM_WORLD);
				queriedSamples += samples;
			}
		}

		// --------------------------------------------------------------------------------

		// ## Exit slaves
		for (int i = 1; i < numProcs; i++)
		{
			MPI_Send(NULL, 0, MPI_INT, i, TagType_Exit, MPI_COMM_WORLD);
		}

		// --------------------------------------------------------------------------------

		// ## Print rendering time
		auto finishTime = std::chrono::high_resolution_clock::now();
		double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime - startTime).count()) / 1000.0;
		LM_LOG_INFO("Rendering completed in " + std::to_string(elapsed) + " seconds");
		LM_LOG_INFO("Processed number of samples : " + std::to_string(processedSamples));

		signal_ReportProgress(1, true);
	}
	else
	{
		// # Worker process
		while (true)
		{
			// ## Receive a task
			long long assignedSamples;
			MPI_Recv(&assignedSamples, 1, MPI_LONG_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if (status.MPI_TAG == TagType_Exit)
			{
				break;
			}

			// --------------------------------------------------------------------------------

			// ## Rendering
			std::atomic<long long> processedSamples(0);

			// Number of blocks to be separated
			long long blocks = (assignedSamples + samplesPerBlock) / samplesPerBlock;

			#pragma omp parallel for
			for (long long block = 0; block < blocks; block++)
			{
				// Thread ID
				int threadId = omp_get_thread_num();
				auto& sampler = samplers[threadId];
				auto& film = films[threadId];

				// Sample range
				long long sampleBegin = samplesPerBlock * block;
				long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, assignedSamples);

				processedSamples += sampleEnd - sampleBegin;

				for (long long sample = sampleBegin; sample < sampleEnd; sample++)
				{
					ProcessRenderSingleSample(scene, *sampler, *film);
				}
			}

			// ## Send a result
			long long result = processedSamples;
			MPI_Send(&result, 1, MPI_LONG_LONG, 0, TagType_TaskFinished, MPI_COMM_WORLD);
		}
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(*f.get());
	}

	// --------------------------------------------------------------------------------

	// Reduce rendered images
	auto* bitmapFilm = dynamic_cast<BitmapFilm*>(masterFilm);
	auto* data = bitmapFilm->Bitmap().InternalData().data();
	int size = bitmapFilm->Width() * bitmapFilm->Height() * 3;
	if (rank == 0)
	{
		MPI_Reduce(MPI_IN_PLACE, data, size, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Reduce(data, data, size, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	}

	// --------------------------------------------------------------------------------

	if (rank == 0)
	{
		// Rescale master film
		masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));
	}

	return true;
}

void MPIPathtraceRenderer::ProcessRenderSingleSample( const Scene& scene, Sampler& sampler, Film& film ) const
{
	// Raster position
	auto rasterPos = sampler.NextVec2();

	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfP;
	scene.MainCamera()->SamplePosition(sampler.NextVec2(), geomE, pdfP);

	// Sample ray direction
	GeneralizedBSDFSampleQuery bsdfSQ;
	GeneralizedBSDFSampleResult bsdfSR;
	bsdfSQ.sample = rasterPos;
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	auto We_Estimated = scene.MainCamera()->SampleAndEstimateDirection(bsdfSQ, geomE, bsdfSR);

	// Construct initial ray
	Ray ray;
	ray.o = geomE.p;
	ray.d = bsdfSR.wo;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf();

	Math::Vec3 throughput = We_Estimated;
	Math::Vec3 L;
	int numPathVertices = 1;

	while (true)
	{
		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}
					
		const auto* light = isect.primitive->light;
		if (light)
		{
			// Evaluate Le
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.type = GeneralizedBSDFType::LightDirection;
			bsdfEQ.wo = -ray.d;
			auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
			auto LeP = light->EvaluatePosition(isect.geom);
			L += throughput * LeD * LeP;
		}

		// --------------------------------------------------------------------------------

		// Sample BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler.NextVec2();
		bsdfSQ.uComp = sampler.Next();
		bsdfSQ.type = GeneralizedBSDFType::AllBSDF;
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.wi = -ray.d;
		
		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = isect.primitive->bsdf->SampleAndEstimateDirection(bsdfSQ, isect.geom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;

		// Setup next ray
		ray.d = bsdfSR.wo;
		ray.o = isect.geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// --------------------------------------------------------------------------------

		if (rrDepth != -1 && numPathVertices >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}

		numPathVertices++;

		if (maxPathVertices != -1 && numPathVertices >= maxPathVertices)
		{
			break;
		}
	}

	film.AccumulateContribution(rasterPos, L);
}

LM_COMPONENT_REGISTER_PLUGIN_IMPL(MPIPathtraceRenderer, Renderer);

LM_NAMESPACE_END
