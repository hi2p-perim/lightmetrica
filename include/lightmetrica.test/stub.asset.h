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
#ifndef __LIB_LIGHTMETRICA_TEST_STUB_ASSET_H__
#define __LIB_LIGHTMETRICA_TEST_STUB_ASSET_H__

#include "common.h"
#include <lightmetrica/asset.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubAsset : public Asset
{
public:

	StubAsset(const std::string& id) : Asset(id) {}
	virtual ~StubAsset() {}
	virtual std::string Name() const { return "asset"; }

};

class StubAsset_Success : public StubAsset
{
public:
	
	StubAsset_Success(const std::string& id) : StubAsset(id) {}
	virtual bool LoadAsset( const ConfigNode& node, const Assets& assets ) { return true; }
	virtual std::string Type() const { return "success"; }

};

class StubAsset_FailOnCreate : public StubAsset
{
public:

	StubAsset_FailOnCreate(const std::string& id) : StubAsset(id) {}
	virtual bool LoadAsset( const ConfigNode& node, const Assets& assets ) { return false; }
	virtual std::string Type() const { return "fail_on_create"; }

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_TEST_STUB_ASSET_H__
