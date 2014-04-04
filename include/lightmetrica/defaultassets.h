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
		Register an asset factory.
		Register an asset factory which is used for creating assets.
		Fails if the factory with same name is already registered.
		\param entry An entry for register.
		\retval true Succeeded to register the asset factory.
		\retval false Failed to register the asset factory.
	*/
	bool RegisterAssetFactory(const AssetFactoryEntry& entry);

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