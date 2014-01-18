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
#ifndef __LIB_LIGHTMETRICA_ASSET_H__
#define __LIB_LIGHTMETRICA_ASSET_H__

#include "object.h"

namespace pugi
{
	class xml_node;
};

LM_NAMESPACE_BEGIN

class Assets;

/*!
	Asset.
	A base class for assets.
*/
class LM_PUBLIC_API Asset : public Object
{
public:

	/*!
		Constructor.
		\param id ID of the asset.
	*/
	Asset(const std::string& id);

	//! Destructor.
	virtual ~Asset();

private:

	LM_DISABLE_COPY_AND_MOVE(Asset);

public:

	/*!
		Load an asset.
		Configure and initialize the asset by the XML elements given by #node.
		Some assets have references to the other assets, so #assets is also required.
		Dependent asset must be loaded beforehand.
		\param node XML node for the configuration.
		\param assets Asset manager.
	*/
	bool Load(const pugi::xml_node& node, const Assets& assets);

	/*!
		Get ID of the asset.
		\return ID of the asset.
	*/
	std::string ID() const;

public:

	/*!
		Get the name of the asset.
		\return Name of the asset.
	*/
	virtual std::string Name() const = 0;

	/*!
		Get the type of the asset.
		\return Type of the asset.
	*/
	virtual std::string Type() const = 0;

protected:

	/*!
		Implementation specific load function.
		\param node XML node for the configuration.
		\param assets Asset manager.
	*/
	virtual bool LoadAsset(const pugi::xml_node& node, const Assets& assets) = 0;

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_ASSET_H__