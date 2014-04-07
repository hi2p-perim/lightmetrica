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
#include <lightmetrica/pathutils.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/config.h>

LM_NAMESPACE_BEGIN

std::string PathUtils::ResolveAssetPath( const Config& config, const std::string& path )
{
	// Convert the path to the absolute path if required
	boost::filesystem::path tmpPath(path);
	if (tmpPath.is_absolute())
	{
		// If the 'path' is absolute use the value as it is
		// This is not a recommended style so we display a warning message
		LM_LOG_WARN("Using absolute path may break compatibility between environments.");
		return path;
	}
	else if (tmpPath.is_relative())
	{
		// If the 'path' is relative, use the path relative to the configuration file
		return boost::filesystem::canonical(tmpPath, config.BasePath()).string();
	}
}

LM_NAMESPACE_END