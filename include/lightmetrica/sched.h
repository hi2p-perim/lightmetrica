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

#pragma once
#ifndef LIB_LIGHTMETRICA_RENDER_SCHED_H
#define LIB_LIGHTMETRICA_RENDER_SCHED_H

#include "component.h"
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

/*!
	Termination mode.
	Describes the termination mode of rendering.
*/
enum class TerminationMode
{
	Samples,			//!< Terminate after specified number of samples.
	Time,				//!< Terminate after specified time.
};

// --------------------------------------------------------------------------------

class ConfigNode;
class Assets;
class Renderer;
class Scene;

/*!
	Render process scheduler.
	A base class for Render process scheduler.
	Render process scheduler is responsible for dispatch render processes
	according to internal implementations, e.g. multi-threaded, MPI.
*/
class RenderProcessScheduler : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("rendersched");

public:

	RenderProcessScheduler() {}
	virtual ~RenderProcessScheduler() {}

private:

	LM_DISABLE_COPY_AND_MOVE(RenderProcessScheduler);

public:

	/*!
		Configure scheduler from XML element.
		\param node A XML element which consists of \a render_scheduler element.
		\param assets Assets manager.
		\param scene Scene.
		\retval true Succeeded to configure.
		\retval false Failed to configure.
	*/
	virtual bool Configure(const ConfigNode& node, const Assets& assets) = 0;

	/*!
		Set termination mode.
		Configures termination mode of the renderer and its parameters.
		\param terminationMode Termination mode.
		\param time Termination time for Time mode (in seconds).
	*/
	virtual void SetTerminationMode(TerminationMode mode, double time) = 0;

	/*!
		Start rendering.
		The function starts to render the #scene according to the current configuration.
		\param scene Scene.
		\retval true Succeeded to render the scene.
		\retval true Failed to render the scene.
	*/
	virtual bool Render(Renderer& renderer, const Scene& scene) const = 0;

	/*!
		Connect to ReportProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) = 0;

};

// --------------------------------------------------------------------------------

/*!
	Sampling based render process scheduler.
	A base class for Render process scheduler for sampling-based rendering techniques.
*/
class SamplingBasedRenderProcessScheduler : public RenderProcessScheduler
{
public:

	SamplingBasedRenderProcessScheduler() {}
	virtual ~SamplingBasedRenderProcessScheduler();

private:

	/*!
		Get number of samples.
		This function is valid only if the termination mode is \a Samples.
		\return Number of samples.
	*/
	virtual long long NumSamples() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RENDER_SCHED_H