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
#ifndef __LIB_LIGHTMETRICA_CONFIG_H__
#define __LIB_LIGHTMETRICA_CONFIG_H__

#include "object.h"
#include "logger.h"
#include "math.types.h"
#include <string>

namespace pugi
{
	class xml_node;
};

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
	const ConfigNode Child(const std::string& name) const;
	
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

/*!
	Configuration of the renderer.
	The renderer is configured by a XML document.
	All configuration needed for rendering is contained the document.
*/
class LM_PUBLIC_API Config : public Object
{
public:

	Config();
	~Config();

private:

	LM_DISABLE_COPY_AND_MOVE(Config);
	
public:

	/*!
		Load the configuration file.
		\param path Path to the configuration file.
		\retval true Succeeded to load the configuration.
		\retval false Failed to load the configuration.
	*/
	bool Load(const std::string& path);

	/*!
		Load the configuration from a string.
		Use the function to load the configuration from a string.
		\param data Configuration string.
		\retval true Succeeded to load the configuration.
		\retval false Failed to load the configuration.
	*/
	bool LoadFromString(const std::string& data);

	/*!
	*/
	const ConfigNode Root() const;

	/*!
		Get the assets element.
		The function returns empty node if the configuration is not loaded.
		\return XML node of the assets element.
	*/
	const pugi::xml_node AssetsElement() const;

	/*!
		Get the scene element.
		The function returns empty node if the configuration is not loaded.
		\return XML node of the scene element.
	*/
	const pugi::xml_node SceneElement() const;

	/*!
		Get the renderer element.
		The function returns empty node if the configuration is not loaded.
		\return XML node of the renderer element.
	*/
	const pugi::xml_node RendererElement() const;

	/*!
		Get the scene type.
		Returns empty string if no type is specified.
		\return Scene type.
	*/
	std::string SceneType() const;

	/*!
		Get the renderer type.
		Returns empty string if no type is specified.
		\return Renderer type.
	*/
	std::string RendererType() const;

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_CONFIG_H__