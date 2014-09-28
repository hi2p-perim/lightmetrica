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
#ifndef LIB_LIGHTMETRICA_RENDER_PROC_H
#define LIB_LIGHTMETRICA_RENDER_PROC_H

#include "component.h"
#include <memory>

LM_NAMESPACE_BEGIN

class Scene;
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



};

/*!
	Sampling-based render process.
	Defines sampling-based render process, which is responsible for
	sampling-based renderers such as PT or MLT etc.
*/
class SamplingBasedRenderProcess : public RenderProcess
{
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

#endif // LIB_LIGHTMETRICA_RENDER_PROC_H