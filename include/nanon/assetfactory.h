/*
	L I G H T  M E T R I C A

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
#ifndef __LIB_LIGHTMETRICA_ASSET_FACTORY_H__
#define __LIB_LIGHTMETRICA_ASSET_FACTORY_H__

#include "object.h"
#include <string>
#include <memory>

LM_NAMESPACE_BEGIN

class Asset;

/*!
	Asset factory.
	The base class of the asset factories.
	The class is used for generating assets under the 'assets' elements
	in the configuration file.
*/
class LM_PUBLIC_API AssetFactory : public Object
{
public:

	AssetFactory();
	virtual ~AssetFactory();

private:

	LM_DISABLE_COPY_AND_MOVE(AssetFactory);

public:
	
	/*!
		Create instance of an asset.
		The function creates an instance of the asset specified by #type.
		\param id ID of the asset to be created.
		\param type Type of the asset.
		\return An instance of the asset.
	*/
	virtual Asset* Create(const std::string& id, const std::string& type) const = 0;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_ASSET_FACTORY_H__