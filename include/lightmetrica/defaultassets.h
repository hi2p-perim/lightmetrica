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
#ifndef LIB_LIGHTMETRICA_DEFAULT_ASSETS_H
#define LIB_LIGHTMETRICA_DEFAULT_ASSETS_H

#include "assets.h"
#include <memory>
#include <vector>

LM_NAMESPACE_BEGIN

class AssetFactory;
class DefaultConfig;
class ConfigNode;

/*!
	An entry for the asset factory.
	This structure is used for registering asset factory to #Assets class.
	\sa Assets::RegisterAssetFactory
*/
struct AssetFactoryEntry
{

	AssetFactoryEntry() {}
	AssetFactoryEntry(const std::string& name, const std::string& child, int priority, const AssetFactory* factory)
		: name(name)
		, child(child)
		, priority(priority)
		, factory(factory)
	{}

	std::string name;					//!< Name of the asset corresponding to the element name under 'assets'.
	std::string child;					//!< Name of the child element of #name.
	int priority;						//!< Priority (smaller is better).
	const AssetFactory* factory;		//!< Instance of the asset factory.

};

/*!
	Default implementation of Assets.
	The class corresponds to the \a assets element in the configuration file.
*/
class LM_PUBLIC_API DefaultAssets : public Assets
{
public:

	DefaultAssets();
	virtual ~DefaultAssets();

public:

	virtual Asset* GetAssetByName(const std::string& name) const;
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func);

public:

	/*!
		Register an interface for assets.
		Register an component interface class for assets creation.
		The class must inherit #Assets and specify dependencies
		to the other asset types with LM_ASSET_DEPENDENCIES macro.
		Fails if the factory with same name is already registered.
		\param interfaceName Name of component interface class which is inherited from Asset.
		\param interfaceGroupName Name of interface group. e.g. \a triangle_meshes for \a triangle_mesh type.
		\param dependencies Dependent asset interface names.
		\retval true Succeeded to register.
		\retval false Failed to register.
		\sa LM_ASSET_DEPENDENCIES
	*/
	bool RegisterInterface(const std::string& interfaceName, const std::string& interfaceGroupName, const std::vector<std::string>& dependencies);

	/*!
		Register an interface for assets.
		Register an component interface class for assets creation.
		This template version statically checks the requirement for types.
		\tparam AssetInterfaceType Type of component interface class which is inherited from Asset.
		\retval true Succeeded to register.
		\retval false Failed to register.
		\sa LM_ASSET_DEPENDENCIES
	*/
	template <typename AssetInterfaceType>
	bool RegisterInterface()
	{
		LM_ASSET_CHECK_IS_VALID_INTERFACE(AssetInterfaceType);

		// Create dependency list
		size_t n;
		const char** deps = AssetInterfaceType::GetAssetDependencies(n);
		std::vector<std::string> dependencies;
		for (size_t i = 0; i < n; i++)
		{
			dependencies.push_back(deps[i]);
		}
		
		// Register
		std::string interfaceTypeName = AssetInterfaceType::InterfaceTypeName();
		std::string groupName = AssetInterfaceType::InterfaceGroupName();
		return RegisterInterface(interfaceTypeName, groupName, dependencies);
	}
	
	/*!
		Load assets from XML element.
		Parse the element #node and register assets.
		\param node A XML element which consists of the \a assets element.
		\retval true Succeeded to load assets.
		\retval false Failed to load assets.
	*/
	bool Load(const ConfigNode& node);

private:
	
	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DEFAULT_ASSETS_H