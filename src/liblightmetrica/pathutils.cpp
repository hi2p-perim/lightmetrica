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
	else
	{
		LM_LOG_ERROR("Invalid path : " + path);
		return path;
	}
}

LM_NAMESPACE_END