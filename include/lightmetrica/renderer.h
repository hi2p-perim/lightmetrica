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
#ifndef LIB_LIGHTMETRICA_RENDERER_H
#define LIB_LIGHTMETRICA_RENDERER_H

#include "component.h"
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

class Assets;
class Scene;
class ConfigNode;

/*!
	Termination mode.
	Describes the termination mode of rendering.
*/
enum class RendererTerminationMode
{
	Samples,			//!< Terminate after specified number of samples.
	Time,				//!< Terminate after specified time.
};

/*!
	Renderer class.
	A base class of the renderer.
*/
class Renderer : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("renderer");

public:

	Renderer() {}
	virtual ~Renderer() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Renderer);

public:

	/*!
		Get renderer type.
		\return Renderer type.
	*/
	virtual std::string Type() const = 0;

	/*!
		Configure the renderer from XML element.
		This function is used internally or testing.
		\param node A XML element which consists of \a renderer element.
		\param assets Assets manager.
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
	virtual void SetTerminationMode(RendererTerminationMode mode, double time) = 0;

	/*!
		Preprocess the renderer.
		Preprocess required by some renderers are dispatched in this function.
		\param scene Scene.
		\retval true Succeeded to preprocess.
		\retval false Failed to preprocess.
	*/
	virtual bool Preprocess(const Scene& scene) = 0;

	/*!
		Start rendering.
		The function starts to render the #scene according to the current configuration.
		\param scene Scene.
		\param terminationMode Termination mode of the rendering.
		\retval true Succeeded to render the scene.
		\retval true Failed to render the scene.
	*/
	virtual bool Render(const Scene& scene) = 0;

public:

	/*!
		Connect to ReportProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RENDERER_H