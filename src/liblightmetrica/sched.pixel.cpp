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
#include <lightmetrica/film.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Deterministic multithreaded render process scheduler.
	Creates and schedules render processes among threads.
	Multi-threading is supported by OpenMP.
	We note that this scheduler requires DeterministicPixelBasedRenderProcess.
	\sa DeterministicPixelBasedRenderProcess.
*/
class DeterministicMTRenderProcessScheduler : public RenderProcessScheduler
{
public:

	LM_COMPONENT_IMPL_DEF("det.mt");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets);
	virtual void SetTerminationMode(TerminationMode mode, double time) {}
	virtual bool Render(Renderer& renderer, const Scene& scene) const;
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void(double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void(double, bool)> signal_ReportProgress;

private:

	int numThreads;

};

bool DeterministicMTRenderProcessScheduler::Configure(const ConfigNode& node, const Assets& assets)
{
	// Load parameters
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}

	// Set number of threads
	omp_set_num_threads(numThreads);

	return true;
}

bool DeterministicMTRenderProcessScheduler::Render(Renderer& renderer, const Scene& scene) const
{
	// # Create processes
	std::vector<std::unique_ptr<DeterministicPixelBasedRenderProcess>> processes;
	for (int i = 0; i < numThreads; i++)
	{
		// Create & check compatibility
		std::unique_ptr<RenderProcess> p(renderer.CreateRenderProcess(scene));
		if (dynamic_cast<DeterministicPixelBasedRenderProcess*>(p.get()) == nullptr)
		{
			LM_LOG_ERROR("Invalid render process type");
			return false;
		}

		// Add a process
		processes.emplace_back(p.release());
	}

	// --------------------------------------------------------------------------------

	// # Render loop

	auto* film = scene.MainCamera()->GetFilm();
	std::atomic<int> processedLines(0);

	signal_ReportProgress(0, false);

	#pragma omp parallel for
	for (int y = 0; y < film->Height(); y++)
	{
		auto& process = processes[omp_get_thread_num()];
		for (int x = 0; x < film->Width(); x++)
		{
			process->ProcessSinglePixel(scene, Math::Vec2i(x, y));
		}
	}

	signal_ReportProgress(1, true);

	// --------------------------------------------------------------------------------

	return true;
}

LM_COMPONENT_REGISTER_IMPL(DeterministicMTRenderProcessScheduler, RenderProcessScheduler);

LM_NAMESPACE_END