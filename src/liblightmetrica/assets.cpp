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

#include "pch.h"
#include <lightmetrica/assets.h>
#include <lightmetrica/asset.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

Assets::Assets()
{

}

Assets::~Assets()
{

}

Asset* Assets::ResolveReferenceToAsset( const pugi::xml_node& node, const std::string& name ) const
{
	// The element must have 'ref' attribute
	auto refAttr = node.attribute("ref");
	if (!refAttr)
	{
		LM_LOG_ERROR(boost::str(boost::format("'%s' element in 'node' must have 'ref' attribute") % name));
		LM_LOG_ERROR(PugiHelper::StartElementInString(node));
		return nullptr;
	}

	// Find the light specified by 'ref'
	auto* asset = GetAssetByName(refAttr.as_string());
	if (!asset)
	{
		LM_LOG_ERROR(boost::str(boost::format("The asset referenced by '%s' is not found") % refAttr.as_string()));
		LM_LOG_ERROR(PugiHelper::StartElementInString(node));
		return nullptr;
	}
	else if (asset->Name() != name)
	{
		LM_LOG_ERROR(boost::str(boost::format("Invalid asset name '%s' (expected '%s')") % asset->Name() % name));
		LM_LOG_ERROR(PugiHelper::StartElementInString(node));
		return nullptr;
	}

	return asset;
}

LM_NAMESPACE_END