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
#include <lightmetrica/sched.h>
#include <lightmetrica/renderproc.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/renderer.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Multithreaded render process scheduler.
	Creates and schedules render processes among threads.
	Multi-threading is supported by OpenMP.
	We note that this scheduler requires SamplingBasedRenderProcess.
	\sa SamplingBasedRenderProcess.
*/
class MTRenderProcessScheduler : public RenderProcessScheduler
{
public:

	LM_COMPONENT_IMPL_DEF("mt");

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

	long long numSamples;					//!< Number of samples
	int numThreads;							//!< Number of threads
	long long samplesPerBlock;				//!< Samples to be processed per block
	Math::Float progressImageInterval;		//!< Seconds between progress images' output (if -1, disabled)

};

bool MTRenderProcessScheduler::Configure(const ConfigNode& node, const Assets& assets)
{
	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
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
	node.ChildValueOrDefault("progress_image_interval", Math::Float(-1), progressImageInterval);

	// Set number of threads
	omp_set_num_threads(numThreads);

	return true;
}

bool MTRenderProcessScheduler::Render(Renderer& renderer, const Scene& scene) const
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);
	std::atomic<long long> processedSamples(0);

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// # Create processes
	std::vector<std::unique_ptr<SamplingBasedRenderProcess>> processes;
	for (int i = 0; i < numThreads; i++)
	{
		// Create & check compatibility
		std::unique_ptr<RenderProcess> p(renderer.CreateRenderProcess(scene));
		if (dynamic_cast<SamplingBasedRenderProcess*>(p.get()) == nullptr)
		{
			LM_LOG_ERROR("Invalid render process type");
			return false;
		}

		// Add a process
		processes.emplace_back(dynamic_cast<SamplingBasedRenderProcess*>(p.release()));
	}

	// --------------------------------------------------------------------------------

	// # Render loop

	bool cancel = false;
	bool done = false;
	auto startTime = std::chrono::high_resolution_clock::now();
	auto prevStartTime = startTime;
	int intermediateImageOutputCount = 0;

	while (true)
	{
		#pragma omp parallel for
		for (long long block = 0; block < blocks; block++)
		{
			#pragma omp flush (done)
			if (done)
			{
				continue;
			}

			// --------------------------------------------------------------------------------

			try
			{
				// Thread ID & process
				int threadId = omp_get_thread_num();
				auto& process = processes[threadId];

				// Sample range
				long long sampleBegin = samplesPerBlock * block;
				long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

				processedSamples += sampleEnd - sampleBegin;

				for (long long sample = sampleBegin; sample < sampleEnd; sample++)
				{
					process->ProcessSingleSample(scene);
				}
			}
			catch (const std::exception& e)
			{
				LM_LOG_ERROR(boost::str(boost::format("EXCEPTION (thread #%d) | %s") % omp_get_thread_num() % e.what()));
				cancel = done = true;
				#pragma omp flush (done)
			}

			// --------------------------------------------------------------------------------

			// Progress report
			processedBlocks++;
			if (terminationMode == TerminationMode::Samples)
			{
				auto progress = static_cast<double>(processedBlocks) / blocks;
				signal_ReportProgress(progress, false);
			}
			else if (terminationMode == TerminationMode::Time)
			{
				auto currentTime = std::chrono::high_resolution_clock::now();
				double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count()) / 1000.0;
				if (elapsed > terminationTime)
				{
					done = true;
					#pragma omp flush (done)
				}
				else
				{
					signal_ReportProgress(elapsed / terminationTime, false);
				}
			}
		}

		if (progressImageInterval > Math::Float(0))
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevStartTime).count()) / 1000.0;
			if (elapsed > static_cast<double>(progressImageInterval))
			{
				// Create output directory if it does not exists
				const std::string outputDir = "progress." + renderer.ComponentImplTypeName();
				if (!boost::filesystem::exists(outputDir))
				{
					LM_LOG_INFO("Creating directory : " + outputDir);
					if (!boost::filesystem::create_directory(outputDir))
					{
						LM_LOG_WARN("Failed to create output directory : " + outputDir);
					}
				}

				// Same intermediate image
				masterFilm->Clear();
				for (int i = 0; i < numThreads; i++)
				{
					masterFilm->AccumulateContribution(*processes[i]->GetFilm());
				}

				// Rescale & save
				intermediateImageOutputCount++;
				auto path = boost::filesystem::path(outputDir) / boost::str(boost::format("%010d") % intermediateImageOutputCount);
				dynamic_cast<BitmapFilm*>(masterFilm)->RescaleAndSave(path.string(), Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));

				LM_LOG_INFO("Saving : " + path.string());
				prevStartTime = currentTime;
			}
		}

		if (done || terminationMode == TerminationMode::Samples)
		{
			break;
		}
	}

	signal_ReportProgress(1, true);

	if (cancel)
	{
		LM_LOG_ERROR("Render operation has been canceled");
		return false;
	}

	// --------------------------------------------------------------------------------

	// # Accumulate rendered results for all threads to one film
	for (int i = 0; i < numThreads; i++)
	{
		masterFilm->AccumulateContribution(*processes[i]->GetFilm());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));

	// --------------------------------------------------------------------------------

	auto finishTime = std::chrono::high_resolution_clock::now();
	double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime - startTime).count()) / 1000.0;
	LM_LOG_INFO("Rendering completed in " + std::to_string(elapsed) + " seconds");
	LM_LOG_INFO("Processed number of samples : " + std::to_string(processedSamples));

	return true;
}

LM_COMPONENT_REGISTER_IMPL(MTRenderProcessScheduler, RenderProcessScheduler);

LM_NAMESPACE_END