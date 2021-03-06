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
#ifndef LIB_LIGHTMETRICA_LIGHT_H
#define LIB_LIGHTMETRICA_LIGHT_H

#include "emitter.h"

LM_NAMESPACE_BEGIN

/*!
	Light.
	A base class of the lights.
*/
class Light : public Emitter
{
public:

	LM_ASSET_INTERFACE_DEF("light", "lights");
	LM_ASSET_DEPENDENCIES("texture");

public:

	Light() {}
	virtual ~Light() {}

public:

	/*!
		Check if the light is environment light.
		\retval true The light is environment light.
		\retval false The light is not environment light.
	*/
	virtual bool EnvironmentLight() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_LIGHT_H