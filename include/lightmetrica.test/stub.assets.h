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
#ifndef __LIB_LIGHTMETRICA_TEST_STUB_ASSETS_H__
#define __LIB_LIGHTMETRICA_TEST_STUB_ASSETS_H__

#include "common.h"
#include <lightmetrica/assets.h>
#include <string>
#include <memory>
#include <boost/unordered_map.hpp>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubAssets : public Assets
{
public:

	virtual Asset* GetAssetByName(const std::string& name) const { return assetInstanceMap.at(name).get(); }
	void Add(const std::string& id, Asset* asset) { assetInstanceMap[id] = std::unique_ptr<Asset>(asset); }
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return boost::signals2::connection(); }

protected:

	boost::unordered_map<std::string, std::unique_ptr<Asset>> assetInstanceMap;

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_TEST_STUB_ASSETS_H__