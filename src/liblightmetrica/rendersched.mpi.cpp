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

#include "pch.h"
#include <lightmetrica/rendersched.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/renderer.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/bitmap.h>
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
	MPI render process scheduler.
	Process scheduler for hybrid MPI + OpenMP parallelization.
*/
class MPIRenderProcessScheduler : public RenderProcessScheduler
{
public:

	LM_COMPONENT_IMPL_DEF("mpi");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets);
	virtual void SetTerminationMode(TerminationMode mode, double time) { terminationMode = mode; terminationTime = time; }
	virtual bool Render(Renderer& renderer, const Scene& scene) const;
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void(double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void(double, bool)> signal_ReportProgress;
	TerminationMode terminationMode;
	double terminationTime;
	
private:

	long long numSamples;									// Number of samples
	int numThreads;											// Number of threads
	long long samplesPerTask;								// Number of samples per MPI task
	long long samplesPerBlock;								// Samples to be processed per block

};

bool MPIRenderProcessScheduler::Configure(const ConfigNode& node, const Assets& assets)
{
	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
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

	// Set number of threads
	omp_set_num_threads(numThreads);

	return true;
}

bool MPIRenderProcessScheduler::Render(Renderer& renderer, const Scene& scene) const
{
	auto* masterFilm = scene.MainCamera()->GetFilm();

	// --------------------------------------------------------------------------------

	int rank;
	int numProcs;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

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
			long long samples = terminationMode == TerminationMode::Time ? samplesPerTask : Math::Min(samplesPerTask, numSamples - queriedSamples);
			MPI_Send(&samples, 1, MPI_LONG_LONG, i, TagType_AssignTask, MPI_COMM_WORLD);
			queriedSamples += samples;
		}

		// --------------------------------------------------------------------------------

		// ## Dispatch render tasks
		while (terminationMode == TerminationMode::Time ||
			  (terminationMode == TerminationMode::Samples && processedSamples < numSamples))
		{
			// Wait for result
			long long processedSamplesBySlave;
			MPI_Recv(&processedSamplesBySlave, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, TagType_TaskFinished, MPI_COMM_WORLD, &status);
			processedSamples += processedSamplesBySlave;

			// --------------------------------------------------------------------------------

			// Progress report
			if (terminationMode == TerminationMode::Samples)
			{
				auto progress = static_cast<double>(processedSamples) / numSamples;
				signal_ReportProgress(progress, false);
			}
			else if (terminationMode == TerminationMode::Time)
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
			if (terminationMode == TerminationMode::Time ||
			   (terminationMode == TerminationMode::Samples && queriedSamples < numSamples))
			{
				long long samples = terminationMode == TerminationMode::Time ? samplesPerTask : Math::Min(samplesPerTask, numSamples - queriedSamples);
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

		// ## Random number generators and films
		std::vector<std::unique_ptr<RenderProcess>> processes;
		for (int i = 0; i < numThreads; i++)
		{
			processes.emplace_back(renderer.CreateRenderProcess(scene));
		}

		// --------------------------------------------------------------------------------

		// ## Render loop
		while (true)
		{
			// ### Receive a task
			long long assignedSamples;
			MPI_Recv(&assignedSamples, 1, MPI_LONG_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if (status.MPI_TAG == TagType_Exit)
			{
				break;
			}

			// --------------------------------------------------------------------------------

			// ### Rendering
			std::atomic<long long> processedSamples(0);

			// Number of blocks to be separated
			long long blocks = (assignedSamples + samplesPerBlock) / samplesPerBlock;

			#pragma omp parallel for
			for (long long block = 0; block < blocks; block++)
			{
				// Thread ID & process
				int threadId = omp_get_thread_num();
				auto& process = processes[threadId];

				// Sample range
				long long sampleBegin = samplesPerBlock * block;
				long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, assignedSamples);

				processedSamples += sampleEnd - sampleBegin;

				for (long long sample = sampleBegin; sample < sampleEnd; sample++)
				{
					process->ProcessSingleSample(scene);
				}
			}

			// ### Send a result
			long long result = processedSamples;
			MPI_Send(&result, 1, MPI_LONG_LONG, 0, TagType_TaskFinished, MPI_COMM_WORLD);
		}

		// --------------------------------------------------------------------------------

		// ## Accumulate rendered results for all threads to one film
		for (int i = 0; i < numThreads; i++)
		{
			masterFilm->AccumulateContribution(*processes[i]->GetFilm());
		}
	}

	// --------------------------------------------------------------------------------

	// Reduce rendered images
	auto* bitmapFilm = dynamic_cast<BitmapFilm*>(masterFilm);
	auto* data = bitmapFilm->Bitmap().InternalData().data();
	int size = bitmapFilm->Width() * bitmapFilm->Height() * 3;
	if (rank == 0)
	{
		MPI_Reduce(MPI_IN_PLACE, data, size, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
		masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));
	}
	else
	{
		MPI_Reduce(data, data, size, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
	}

	return true;
}

LM_COMPONENT_REGISTER_IMPL(MPIRenderProcessScheduler, RenderProcessScheduler);

LM_NAMESPACE_END