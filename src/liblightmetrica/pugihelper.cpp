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
