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

#ifndef __LIB_NANON_SCENE_H__
#define __LIB_NANON_SCENE_H__

#include "common.h"
#include <string>

namespace pugi
{
	class xml_node;
};

NANON_NAMESPACE_BEGIN

class Assets;
class NanonConfig;

/*!
	Scene class.
	A base class of the scene.
*/
class NANON_PUBLIC_API Scene
{
public:

	Scene();
	virtual ~Scene();

private:

	NANON_DISABLE_COPY_AND_MOVE(Scene);

public:

	/*!
	*/
	bool Load(const pugi::xml_node& node, const Assets& assets);

	/*!
	*/
	bool Load(const NanonConfig& config, const Assets& assets);

	/*!
	*/
	virtual std::string Type() = 0;

private:

	class Impl;
	Impl* p;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_SCENE_H__