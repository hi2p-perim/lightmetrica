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

#pragma once
#ifndef __LIB_NANON_QBVH_SCENE_H__
#define __LIB_NANON_QBVH_SCENE_H__

#include "scene.h"

NANON_NAMESPACE_BEGIN

/*!
	QBVH scene.
	An implementation of Quad-BVH (QBVH).
	Reference:
		Dammertz, H., Shallow Bounding Volume Hierarchies for Fast SIMD Ray Tracing of Incoherent Rays,
		EGSR'08 Proceedings, 2008.
	Partially based on the implementation of
	- LuxRender's QBVHAccel
	- http://d.hatena.ne.jp/ototoi/20090925/p1
*/
class NANON_PUBLIC_API QBVHScene : public Scene
{
public:

	QBVHScene();
	virtual ~QBVHScene();

public:

	virtual bool Build();
	virtual bool Intersect( Ray& ray, Intersection& isect ) const;
	virtual std::string Type() const { return "qbvh"; }
	virtual boost::signals2::connection Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func);
	virtual bool LoadImpl( const pugi::xml_node& node, const Assets& assets );

private:

	class Impl;
	Impl* p;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_QBVH_SCENE_H__