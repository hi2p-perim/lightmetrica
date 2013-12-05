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

#include "pch.h"
#include "base.h"
#include <nanon/assets.h>
#include <pugixml.hpp>

using namespace nanon;

namespace
{
	const std::string AssetsNode_Success = NANON_TEST_MULTILINE_LITERAL(
		<assets>
			<textures>
				<texture id="nanon_1" type="bitmap">
					<path>../../resources/nanon_1.jpg</path>
				</texture>
				<texture id="nanon_2" type="bitmap">
					<path>../../resources/nanon_2.jpg</path>
				</textures>
			</textures>
			<materials />
			<triangle_meshes />
			<films />
			<cameras />
			<lights />
		</assets>
	);

	const std::string AssetsNode_Fail_InvalidElementName = NANON_TEST_MULTILINE_LITERAL(
		<asset>
		</asset>
	);

	const std::string AssetsNode_Fail_SameID = NANON_TEST_MULTILINE_LITERAL(
		<assets>
			<textures>
				<texture id="wood" type="hdr" />
			</textures>
			<materials>
				<material id="wood" type="diffuse" />
			</materials>
		</assets>
	);
}

NANON_TEST_NAMESPACE_BEGIN

class AssetsTest : public TestBase
{
protected:

	pugi::xml_node LoadXMLBuffer(const std::string& data);

protected:

	Assets assets;
	pugi::xml_document doc;

};

pugi::xml_node AssetsTest::LoadXMLBuffer( const std::string& data )
{
	doc.load_buffer(static_cast<const void*>(data.c_str()), data.size());
	return doc.first_child();
}

TEST_F(AssetsTest, RegisterAssetFactory)
{
	
}

TEST_F(AssetsTest, RegisterAssetFactory_Failed)
{

}

TEST_F(AssetsTest, Load)
{
	EXPECT_TRUE(assets.Load(LoadXMLBuffer(AssetsNode_Success)));

	// TODO : Check if the assets are actually loaded
}

TEST_F(AssetsTest, Load_Failed)
{
	EXPECT_FALSE(assets.Load(LoadXMLBuffer(AssetsNode_Fail_InvalidElementName)));
	EXPECT_FALSE(assets.Load(LoadXMLBuffer(AssetsNode_Fail_SameID)));
}

NANON_TEST_NAMESPACE_END