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
#ifndef LIB_LIGHTMETRICA_PATH_UTILS_H
#define LIB_LIGHTMETRICA_PATH_UTILS_H

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN

class Config;

/*!
	Path utility.
	Helper class for path manipulation.
*/
class LM_PUBLIC_API PathUtils
{
private:

	PathUtils() {}
	LM_DISABLE_COPY_AND_MOVE(PathUtils);

public:

	/*!
		Resolve asset path.
		If the given #path is absolute, returns the path as it is.
		Otherwise, returns the path relative to the configuration file.
		\param config Configuration.
		\param path A path (absolute or relative).
		\return A resolved path.
	*/
	static std::string ResolveAssetPath(const Config& config, const std::string& path);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PATH_UTILS_H