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
#ifndef __LIB_LIGHTMETRICA_LIGHT_TRACE_H__
#define __LIB_LIGHTMETRICA_LIGHT_TRACE_H__

#include "renderer.h"

LM_NAMESPACE_BEGIN

/*!
	Light trace renderer.
	An implementation of light tracing (a.k.a. inverse path tracing).
	Reference:
		Arvo, J., Backward ray tracing, Developments in Ray Tracing,
		Computer Graphics, Proc. of ACM SIGGRAPH 86 Course Notes, 1986.
*/
class LM_PUBLIC_API LighttraceRenderer : public Renderer
{
public:

	LighttraceRenderer();
	virtual ~LighttraceRenderer();

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual std::string Type() const { return "lighttrace"; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func);

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_LIGHT_TRACE_H__