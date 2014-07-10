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