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
#include <memory>
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

class Assets;
class Scene;
class ConfigNode;
class RenderProcess;

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
		\param node A XML element which consists of \a renderer element.
		\param assets Assets manager.
		\param scene Scene.
		\retval true Succeeded to configure.
		\retval false Failed to configure.
	*/
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene) = 0;

	/*!
		Preprocess the renderer.
		Preprocess required by some renderers are dispatched in this function.
		\param scene Scene.
		\retval true Succeeded to preprocess.
		\retval false Failed to preprocess.
	*/
	virtual bool Preprocess(const Scene& scene) = 0;

	/*!
		Create a render process.
		Creates a new instance of the render process associated with the renderer.
		This function called from the render process scheduler.
		Ownership of the created instance is deletgated to the caller.
		\param scene Scene.
		\return An instance of render process.
	*/
	virtual RenderProcess* CreateRenderProcess(const Scene& scene) const = 0;

public:

	/*!
		Connect to ReportProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) = 0;

};

// --------------------------------------------------------------------------------

class Film;

/*!
	Render process.
	A base class for render process, which is responsible
	for process some part of the entire samples.
	This class is used for parallelization of renderers.
*/
class RenderProcess
{
public:

	RenderProcess() {};
	virtual ~RenderProcess() {}

private:
	
	LM_DISABLE_COPY_AND_MOVE(RenderProcess);

public:

	/*!
		Process single sample.
		\param scene Scene.
	*/
	virtual void ProcessSingleSample(const Scene& scene) = 0;

	/*!
		Get film.
		Gets internal film associate with the process.
		\return Film.
	*/
	virtual const Film* GetFilm() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RENDERER_H