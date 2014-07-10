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
#ifndef LIB_LIGHTMETRICA_NAIVE_SCENE_H
#define LIB_LIGHTMETRICA_NAIVE_SCENE_H

#include "scene.h"

LM_NAMESPACE_BEGIN

/*!
*/
class LM_PUBLIC_API NaiveScene : public Scene
{
public:

	NaiveScene();
	virtual ~NaiveScene();

public:

	virtual bool Build();
	virtual bool Intersect(Ray& ray, Intersection& isect) const;
	virtual std::string Type() const { return "naive"; }
	virtual boost::signals2::connection Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func);
	virtual bool Configure( const ConfigNode& node ) { return true; }
	virtual void ResetScene() {}

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_NAIVE_SCENE_H