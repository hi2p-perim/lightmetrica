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
#include <lightmetrica/confignode.h>
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

class ConfigNode::Impl : public Object
{
public:

	Impl() {}
	Impl(void* node, const Config* config);

public:

	pugi::xml_node node;
	const Config* config;

};

ConfigNode::Impl::Impl( void* node, const Config* config )
	: node(static_cast<pugi::xml_node_struct*>(node))
	, config(config)
{

}

// --------------------------------------------------------------------------------

ConfigNode::ConfigNode()
	: p(new Impl)
{

}

ConfigNode::ConfigNode( void* node, const Config* config )
	: p(new Impl(node, config))
{

}

ConfigNode::ConfigNode( const ConfigNode& config )
	: p(new Impl(config.p->node.internal_object(), config.p->config))
{

}

void ConfigNode::operator=( const ConfigNode& config )
{
	*p = *config.p;
}

ConfigNode::ConfigNode( ConfigNode&& config )
{
	p = config.p;
	config.p = nullptr;
}

void ConfigNode::operator=( ConfigNode&& config )
{
	LM_SAFE_DELETE(p);
	p = config.p;
	config.p = nullptr;
}

ConfigNode::~ConfigNode()
{
	LM_SAFE_DELETE(p);
}

bool ConfigNode::Empty() const
{
	return p->node.empty();
}

ConfigNode ConfigNode::Child( const std::string& name ) const
{
	return ConfigNode(p->node.child(name.c_str()).internal_object(), p->config);
}

ConfigNode ConfigNode::FirstChild() const
{
	return ConfigNode(p->node.first_child().internal_object(), p->config);
}

ConfigNode ConfigNode::NextChild() const
{
	return ConfigNode(p->node.next_sibling().internal_object(), p->config);
}

lightmetrica::ConfigNode ConfigNode::NextChild( const std::string& name ) const
{
	return ConfigNode(p->node.next_sibling(name.c_str()).internal_object(), p->config);
}

std::string ConfigNode::Value() const
{
	return p->node.child_value();
}

std::string ConfigNode::AttributeValue( const std::string& name ) const
{
	return p->node.attribute(name.c_str()).value();
}

std::string ConfigNode::Name() const
{
	return p->node.name();
}

template <>
std::string ConfigNode::Value<std::string>() const
{
	return Value();
}

template <>
int ConfigNode::Value<int>() const
{
	try
	{
		return std::stoi(Value());
	}
	catch (const std::invalid_argument& e)
	{
		LM_LOG_WARN(e.what());
		return 0;
	}
	catch (const std::out_of_range& e)
	{
		LM_LOG_WARN(e.what());
		return 0;
	}
}

template <>
Math::Float ConfigNode::Value<Math::Float>() const
{
	try
	{
		return Math::Float(std::stod(Value()));
	}
	catch (const std::invalid_argument& e)
	{
		LM_LOG_WARN(e.what());
		return Math::Float(0);
	}
	catch (const std::out_of_range& e)
	{
		LM_LOG_WARN(e.what());
		return Math::Float(0);
	}
}

template <>
Math::Vec3 ConfigNode::Value<Math::Vec3>() const
{
	// Parse vector elements (in double)
	std::vector<double> v;
	std::stringstream ss(Value());

	double t;
	while (ss >> t)
	{
		v.push_back(t);
	}
	if (v.size() != 3)
	{
		LM_LOG_WARN("Invalid number of elements in '" + Name() + "'");
		return Math::Vec3();
	}

	// Convert type and return
	return Math::Vec3(Math::Float(v[0]), Math::Float(v[1]), Math::Float(v[2]));
}

template <>
Math::Mat4 ConfigNode::Value<Math::Mat4>() const
{
	// Parse matrix elements (in double)
	std::vector<double> m;
	std::stringstream ss(Value());

	double t;
	while (ss >> t)
	{
		m.push_back(t);
	}
	if (m.size() != 16)
	{
		LM_LOG_WARN("Invalid number of elements in '" + Name() + "'");
		return Math::Mat4();
	}

	// Convert to Float and create matrix
	std::vector<Math::Float> m2(16);
	std::transform(m.begin(), m.end(), m2.begin(), [](double v){ return Math::Float(v); });
	return Math::Mat4(&m2[0]);
}


template <>
std::vector<Math::Float> ConfigNode::Value<std::vector<Math::Float>>() const
{
	std::vector<Math::Float> v;
	std::stringstream ss(Value());
	
	double t;
	while (ss >> t)
	{
		v.push_back(Math::Float(t));
	}

	return v;
}

template <>
std::vector<unsigned int> ConfigNode::Value<std::vector<unsigned int>>() const
{
	std::vector<unsigned int> v;
	std::stringstream ss(Value());

	unsigned int t;
	while (ss >> t)
	{
		v.push_back(t);
	}

	return v;
}

LM_NAMESPACE_END