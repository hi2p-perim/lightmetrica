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
#ifndef LIB_LIGHTMETRICA_EMITTER_SHAPE
#define LIB_LIGHTMETRICA_EMITTER_SHAPE

#include "component.h"

LM_NAMESPACE_BEGIN

/*!
	Emitter shape.
	Special shapes other than triangles associated with emitters.
	E.g., sphere is associated with environment lights.
*/
class EmitterShape : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("emittershape");

public:

	EmitterShape() {}
	virtual ~EmitterShape() {}

private:

	LM_DISABLE_COPY_AND_MOVE(EmitterShape);

public:

	

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_EMITTER_SHAPE