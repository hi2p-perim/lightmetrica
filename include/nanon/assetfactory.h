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

#ifndef __LIB_NANON_ASSET_FACTORY_H__
#define __LIB_NANON_ASSET_FACTORY_H__

#include "common.h"

NANON_NAMESPACE_BEGIN

class Asset;

/*!
*/
class NANON_PUBLIC_API AssetFactory
{
public:

	AssetFactory();
	virtual ~AssetFactory();

private:

	NANON_DISABLE_COPY_AND_MOVE(AssetFactory);

public:
	
	/*!
	*/
	virtual std::shared_ptr<Asset> Create(const std::string& type) = 0;

};

/*!
*/
class NANON_PUBLIC_API TextureFactory : public AssetFactory
{
public:

	TextureFactory() {}
	virtual ~TextureFactory() {}
	virtual std::shared_ptr<Asset> Create(const std::string& type) { return nullptr; }

};

/*!
*/
class NANON_PUBLIC_API MaterialFactory : public AssetFactory
{
public:

	MaterialFactory() {}
	virtual ~MaterialFactory() {}
	virtual std::shared_ptr<Asset> Create(const std::string& type) { return nullptr; }

};

/*!
*/
class NANON_PUBLIC_API TriangleMeshFactory : public AssetFactory
{
public:

	TriangleMeshFactory() {}
	virtual ~TriangleMeshFactory() {}
	virtual std::shared_ptr<Asset> Create(const std::string& type) { return nullptr; }

};

/*!
*/
class NANON_PUBLIC_API FilmFactory : public AssetFactory
{
public:

	FilmFactory() {}
	virtual ~FilmFactory() {}
	virtual std::shared_ptr<Asset> Create(const std::string& type) { return nullptr; }

};

/*!
*/
class NANON_PUBLIC_API CameraFactory : public AssetFactory
{
public:

	CameraFactory() {}
	virtual ~CameraFactory() {}
	virtual std::shared_ptr<Asset> Create(const std::string& type) { return nullptr; }

};

/*!
*/
class NANON_PUBLIC_API LightFactory : public AssetFactory
{
public:

	LightFactory() {}
	virtual ~LightFactory() {}
	virtual std::shared_ptr<Asset> Create(const std::string& type) { return nullptr; }

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_ASSET_FACTORY_H__