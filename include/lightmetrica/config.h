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
#ifndef LIB_LIGHTMETRICA_CONFIG_H
#define LIB_LIGHTMETRICA_CONFIG_H

#include "object.h"
#include <string>

LM_NAMESPACE_BEGIN

class ConfigNode;

/*!
	Configuration of the renderer.
	The renderer is configured by a XML document.
	All configuration needed for rendering is contained the document.
*/
class LM_PUBLIC_API Config : public Object
{
public:

	Config() {}
	virtual ~Config() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Config);
	
public:

	/*!
		Load the configuration file.
		\param path Path to the configuration file.
		\retval true Succeeded to load the configuration.
		\retval false Failed to load the configuration.
	*/
	virtual bool Load(const std::string& path) = 0;

	/*!
		Load the configuration file.
		\param path Path to the configuration file.
		\param path Base path for asset loading.
		\retval true Succeeded to load the configuration.
		\retval false Failed to load the configuration.
	*/
	virtual bool Load(const std::string& path, const std::string& basePath) = 0;

	/*!
		Load the configuration from a string.
		Use the function to load the configuration from a string.
		\param data Configuration string.
		\param basePath Base path for asset loading.
		\retval true Succeeded to load the configuration.
		\retval false Failed to load the configuration.
	*/
	virtual bool LoadFromString(const std::string& data, const std::string& basePath) = 0;

	/*!
		Get the root node.
		\return Root node.
	*/
	virtual ConfigNode Root() const = 0;

	/*!
		Get base path.
		Returns the base path of the assets.
		\return Base path.
	*/
	virtual std::string BasePath() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_CONFIG_H