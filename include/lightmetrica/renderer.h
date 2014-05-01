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
		Start rendering.
		The function starts to render the #scene according to the current configuration.
		\param scene Scene.
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