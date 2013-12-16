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

#ifndef __LIB_NANON_ASSET_H__
#define __LIB_NANON_ASSET_H__

#include "common.h"

namespace pugi
{
	class xml_node;
};

NANON_NAMESPACE_BEGIN

/*!
	Asset.
	A base class for assets.
*/
class NANON_PUBLIC_API Asset
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

	NANON_DISABLE_COPY_AND_MOVE(Asset);

public:

	/*!
		Load an asset.
		Configure and initialize the asset by the XML elements given by #node.
		\param id ID of the asset.
		\param node XML node for the configuration.
	*/
	virtual bool Load(const pugi::xml_node& node) = 0;

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

	/*!
		Get ID of the asset.
		\return ID of the asset.
	*/
	std::string ID() const;

private:

	class Impl;
	Impl* p;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_ASSET_H__