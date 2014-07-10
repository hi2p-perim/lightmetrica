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
#ifndef LIB_LIGHTMETRICA_DEFAULT_CONFIG_H
#define LIB_LIGHTMETRICA_DEFAULT_CONFIG_H

#include "config.h"

LM_NAMESPACE_BEGIN

/*!
	Default config.
	Default implementation of the config class.
*/
class LM_PUBLIC_API DefaultConfig : public Config
{
public:

	DefaultConfig();
	~DefaultConfig();

public:

	virtual bool Load( const std::string& path );
	virtual bool Load( const std::string& path, const std::string& basePath );
	virtual bool LoadFromString( const std::string& data, const std::string& basePath );
	virtual ConfigNode Root() const;
	virtual std::string BasePath() const;

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DEFAULT_CONFIG_H