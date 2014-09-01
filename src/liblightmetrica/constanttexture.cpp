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
#include <lightmetrica/texture.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

/*!
	Constant color textures.
	The texture which is evaluated to constant color value.
*/
class ConstantTexture : public Texture
{
public:

	LM_COMPONENT_IMPL_DEF("constant");

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets )
	{
		node.ChildValueOrDefault("color", Math::Vec3(Math::Float(1)), C);
		return true;
	}

public:

	virtual Math::Vec3 Evaluate( const Math::Vec2& uv ) const
	{
		return C;
	}

private:

	Math::Vec3 C;

};

LM_COMPONENT_REGISTER_IMPL(ConstantTexture, Texture);

LM_NAMESPACE_END