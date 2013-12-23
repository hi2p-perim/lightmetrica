/*
	nanon : A research-oriented renderer

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

#ifndef __LIB_NANON_RENDERER_H__
#define __LIB_NANON_RENDERER_H__

#include "common.h"
#include <string>

namespace pugi
{
	class xml_node;
};

NANON_NAMESPACE_BEGIN

class Assets;
class Scene;
class NanonConfig;

/*!
	Renderer class.
	A base class of the renderer.
*/
class NANON_PUBLIC_API Renderer
{
public:

	Renderer();
	virtual ~Renderer();

private:

	NANON_DISABLE_COPY_AND_MOVE(Renderer);

public:

	/*!
		Configure the renderer from XML element.
		This function is used internally or testing.
		\param node A XML element which consists of \a renderer element.
		\retval true Succeeded to configure.
		\retval false Failed to configure.
	*/
	virtual bool Configure(const pugi::xml_node& node, const Assets& assets) = 0;

	/*!
		Configure the renderer from the configuration.
		Get the \a renderer element from the configuration and configure the renderer.
		\retval config Configuration.
		\retval true Succeeded to configure.
		\retval false Failed to configure.
	*/
	bool Configure(const NanonConfig& config, const Assets& assets);

	/*!
		Get the scene type.
		\return Scene type.
	*/
	virtual std::string Type() const = 0;

	/*!
		Start rendering.
		The function starts to render the #scene according to the current configuration.
		\param scene Scene.
		\retval true Succeeded to render the scene.
	*/
	virtual bool Render(const Scene& scene) = 0;

private:

	class Impl;
	Impl* p;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_RENDERER_H__