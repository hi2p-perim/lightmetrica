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
#include <lightmetrica/pugihelper.h>
#include <lightmetrica/logger.h>
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

std::string PugiHelper::ElementInString( const pugi::xml_node& node )
{
	std::stringstream ss;
	node.print(ss);
	return ss.str();
}

std::string PugiHelper::StartElementInString( const pugi::xml_node& node )
{
	std::stringstream ss;
	node.print(ss);
	std::string line;
	std::getline(ss, line);
	return line;
}

Math::Vec3 PugiHelper::ParseVec3( const pugi::xml_node& node )
{
	// Parse vector elements (in double)
	std::vector<double> v;
	std::stringstream ss(node.child_value());

	double t;
	while (ss >> t) v.push_back(t);
	if (v.size() != 3)
	{
		LM_LOG_WARN("Invalid number of elements in '" + std::string(node.name()) + "'");
		return Math::Vec3();
	}

	// Convert type and return
	return Math::Vec3(Math::Float(v[0]), Math::Float(v[1]), Math::Float(v[2]));
}

Math::Mat4 PugiHelper::ParseMat4( const pugi::xml_node& node )
{
	// Parse matrix elements (in double)
	std::vector<double> m;
	std::stringstream ss(node.child_value());

	double t;
	while (ss >> t) m.push_back(t);
	if (m.size() != 16)
	{
		LM_LOG_WARN("Invalid number of elements in '" + std::string(node.name()) + "'");
		return Math::Mat4::Identity();
	}

	// Convert to Float and create matrix
	std::vector<Math::Float> m2(16);
	std::transform(m.begin(), m.end(), m2.begin(), [](double v){ return Math::Float(v); });
	return Math::Mat4(&m2[0]);
}

LM_NAMESPACE_END
