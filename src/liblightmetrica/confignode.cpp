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

const Config* ConfigNode::GetConfig() const
{
	return p->config;
}

template <>
LM_PUBLIC_API std::string ConfigNode::Value<std::string>() const
{
	return Value();
}

template <>
LM_PUBLIC_API int ConfigNode::Value<int>() const
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
LM_PUBLIC_API long long ConfigNode::Value<long long>() const
{
	try
	{
		return std::stoll(Value());
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
LM_PUBLIC_API bool ConfigNode::Value<bool>() const
{
	if (Value() == "true")
	{
		return true;
	}
	else if (Value() == "false")
	{
		return false;
	}
	else
	{
		LM_LOG_WARN("Invalid boolean value, forced to 'false'");
		return false;
	}
}

template <>
LM_PUBLIC_API Math::Float ConfigNode::Value<Math::Float>() const
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
LM_PUBLIC_API Math::Vec3 ConfigNode::Value<Math::Vec3>() const
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
LM_PUBLIC_API Math::Mat4 ConfigNode::Value<Math::Mat4>() const
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
LM_PUBLIC_API std::vector<Math::Float> ConfigNode::Value<std::vector<Math::Float>>() const
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
LM_PUBLIC_API std::vector<unsigned int> ConfigNode::Value<std::vector<unsigned int>>() const
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