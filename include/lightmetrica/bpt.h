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
#ifndef LIB_LIGHTMETRICA_BPT_H
#define LIB_LIGHTMETRICA_BPT_H

#include "renderer.h"

LM_NAMESPACE_BEGIN

/*!
	Veach's bidirectional path trace renderer.
	An implementation of bidirectional path tracing (BPT) according to Veach's paper.
	Reference:
		Veach, E. and Guibas, L., Bidirectional estimators for light transport,
		In Proceedings of the Fifth Eurographics Workshop on Rendering, pp. 147-162, 1994.
*/
class LM_PUBLIC_API BidirectionalPathtraceRenderer : public Renderer
{
public:

	BidirectionalPathtraceRenderer();
	virtual ~BidirectionalPathtraceRenderer();

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual std::string Type() const { return "bpt"; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func);

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_H