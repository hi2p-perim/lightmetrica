/*
	nanon : A research-oriented renderer

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

#ifndef __LIB_NANON_ASSETS_H__
#define __LIB_NANON_ASSETS_H__

#include "common.h"
#include <memory>

namespace pugi
{
	class xml_node;
};

NANON_NAMESPACE_BEGIN

class Asset;
class AssetFactory;
class NanonConfig;

/*!
	An entry for the asset factory.
	This structure is used for registering asset factory to #Assets class.
	\sa Assets::RegisterAssetFactory
*/
struct AssetFactoryEntry
{

	AssetFactoryEntry() {}
	AssetFactoryEntry(const std::string& name, const std::string& child, int priority, const std::shared_ptr<AssetFactory>& factory)
		: name(name)
		, child(child)
		, priority(priority)
		, factory(factory)
	{}

	std::string name;						//!< Name of the asset corresponding to the element name under 'assets'.
	std::string child;						//!< Name of the child element of #name.
	int priority;							//!< Priority (smaller is better).
	std::shared_ptr<AssetFactory> factory;	//!< Instance of the asset factory.

};

/*!
	Collection of assets.
	The asset collection class for the asset management.
	The class corresponds to the \a assets element in the configuration file.
*/
class NANON_PUBLIC_API Assets
{
public:

	Assets();
	~Assets();

private:

	NANON_DISABLE_COPY_AND_MOVE(Assets);

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
	bool Load(const pugi::xml_node& node);

	/*!
		Load assets from the configuration.
		Get the \a assets element from the configuration and load the assets.
		\param config Configuration.
		\retval true Succeeded to load assets.
		\retval false Failed to load assets.
	*/
	bool Load(const NanonConfig& config);

	/*!
		Get an asset by name. 
		Returns nullptr if not found.
		\param name Name of the asset.
		\return Asset instance. 
	*/
	std::shared_ptr<Asset> GetAssetByName(const std::string& name);

private:
	
	class Impl;
	Impl* p;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_ASSETS_H__