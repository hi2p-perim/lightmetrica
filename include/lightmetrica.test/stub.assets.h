/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef LIB_LIGHTMETRICA_TEST_STUB_ASSETS_H
#define LIB_LIGHTMETRICA_TEST_STUB_ASSETS_H

#include "common.h"
#include <lightmetrica/assets.h>
#include <lightmetrica/asset.h>
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

#endif // LIB_LIGHTMETRICA_TEST_STUB_ASSETS_H
