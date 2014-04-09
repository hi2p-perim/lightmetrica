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

#include "pch.h"
#include <lightmetrica/scenefactory.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/naivescene.h>
#include <lightmetrica/bvhscene.h>
#if LM_SSE2 && LM_SINGLE_PRECISION
#include <lightmetrica/qbvhscene.h>
#endif

LM_NAMESPACE_BEGIN

SceneFactory::SceneFactory()
{

}

SceneFactory::~SceneFactory()
{

}

Scene* SceneFactory::Create( const std::string& type ) const
{
	if (type == "naive")
	{
		return new NaiveScene();
	}
	else if (type == "bvh")
	{
		return new BVHScene();
	}
	else if (type == "qbvh")
	{
#if LM_SSE2 && LM_SINGLE_PRECISION
		return new QBVHScene();
#else
		LM_LOG_ERROR("QBVH implementation requires SSE2 support and single precision mode");
		return nullptr;
#endif
	}
	else
	{
		LM_LOG_ERROR("Invalid scene type '" + type + "'");
		return nullptr;
	}
}

LM_NAMESPACE_END
