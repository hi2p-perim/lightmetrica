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

#pragma once
#ifndef __LIB_LIGHTMETRICA_CONFIG_NODE_H__
#define __LIB_LIGHTMETRICA_CONFIG_NODE_H__

#include "object.h"
#include "logger.h"
#include "math.types.h"
#include <vector>

LM_NAMESPACE_BEGIN

class Config;

/*!
	Config node.
	Represents a XML node of the configuration file.
*/
class LM_PUBLIC_API ConfigNode : public Object
{
public:

	ConfigNode();
	~ConfigNode();

public:

	ConfigNode(const ConfigNode& config);
	void operator=(const ConfigNode& config);
	ConfigNode(ConfigNode&& config);
	void operator=(ConfigNode&& config);

public:

	/*!
		Internal constructor.
		The constructor is used internally.
		\param node pugixml's internal object for xml_node.
		\param Config.
	*/
	ConfigNode(void* node, const Config* config);

public:

	/*!
		Check if the node is empty.
		\retval true Node is empty.
		\retval false Node is not empty.
	*/
	bool Empty() const;

	/*!
		Get child node by element name.
		Returns empty node is the node specified by #name does not exist.
		\param name Element name.
		\return Child node or empty node.
	*/
	ConfigNode Child(const std::string& name) const;
	
	/*!
		Get the first child node.
		\return First child node.
	*/
	ConfigNode FirstChild() const;
	
	/*!
		Get the next child node.
		\return Next child node.
	*/
	ConfigNode NextChild() const;

	/*!
		Get the next child node with given name.
		\param name Node name.
		\return Next child node.
	*/
	ConfigNode NextChild(const std::string& name) const;

	/*!
		Get the name of the node.
		\return Name of the node.
	*/
	std::string Name() const;

	/*!
		Get the value of the node by string.
		\return Value of the node in string.
	*/
	std::string Value() const;

	/*!
		Get the value of the node by type.
		\tparam T Return type.
		\return Value of the node with type #T.
	*/
	template <typename T>
	T Value() const;

	/*!
		Get the value of the attribute by name.
		\param name Attribute name.
		\return Value of the attribute.
	*/
	std::string AttributeValue(const std::string& name) const;

	/*!
		Get the value of the child with default value.
		Obtains the value of the child node specified by #name if the node exists.
		If not, the value specified by #defaultValue is returned.
		\param name Name of the child node.
		\param value Value to be obtained.
		\retval true Child node is found.
		\retval false Child node is not found.
	*/
	template <typename T>
	bool ChildValueOrDefault(const std::string& name, const T& defaultValue, T& value) const
	{
		const auto child = Child(name);
		if (child.Empty())
		{
			LM_LOG_WARN("Missing '" + name + "' element. Using default.");
			value = defaultValue;
			return false;
		}

		value = child.Value<T>();
		return true;
	}

	/*!
		Get the value of the child.
		Obtains the value of the child node specified by #name.
		Returns false if the child node does not exist.
		\param name Name of the child node.
		\param value Value to be obtained.
		\retval true Child node is found.
		\retval false Child node is not found.
	*/
	template <typename T>
	bool ChildValue(const std::string& name, T& value) const
	{
		const auto child = Child(name);
		if (child.Empty())
		{
			LM_LOG_ERROR("Missing '" + name + "' element");
			return false;
		}

		value = child.Value<T>();
		return true;
	}

private:

	class Impl;
	Impl* p;

};

extern template LM_PUBLIC_API std::string ConfigNode::Value<std::string>() const;
extern template LM_PUBLIC_API int ConfigNode::Value<int>() const;
extern template LM_PUBLIC_API Math::Float ConfigNode::Value<Math::Float>() const;
extern template LM_PUBLIC_API Math::Vec3 ConfigNode::Value<Math::Vec3>() const;
extern template LM_PUBLIC_API Math::Mat4 ConfigNode::Value<Math::Mat4>() const;
extern template LM_PUBLIC_API std::vector<Math::Float> ConfigNode::Value<std::vector<Math::Float>>() const;
extern template LM_PUBLIC_API std::vector<unsigned int> ConfigNode::Value<std::vector<unsigned int>>() const;

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_CONFIG_NODE_H__