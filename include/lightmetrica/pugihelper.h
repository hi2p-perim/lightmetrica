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
#ifndef LIB_LIGHTMETRICA_PUGI_HELPER_H
#define LIB_LIGHTMETRICA_PUGI_HELPER_H

#include "common.h"
#include "math.types.h"
#include <string>

namespace pugi
{
	class xml_node;
};

LM_NAMESPACE_BEGIN

/*!
	Pugi helper.
	Helper class for pugixml.
*/
class LM_PUBLIC_API PugiHelper
{
private:

	PugiHelper();
	LM_DISABLE_COPY_AND_MOVE(PugiHelper);

public:

	/*!
		Get the XML element in string.
		\param node A XML node.
	*/
	static std::string ElementInString(const pugi::xml_node& node);

	/*!
		Get the start element of the given XML element in string.
		\param node A XML node.
	*/
	static std::string StartElementInString(const pugi::xml_node& node);

	/*!
		Parse Math::Vec3.
		\param node A XML node.
	*/
	static Math::Vec3 ParseVec3(const pugi::xml_node& node);

	/*!
		Parse Math::Mat4.
		\param node A XML node.
	*/
	static Math::Mat4 ParseMat4(const pugi::xml_node& node);

};


LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PUGI_HELPER_H