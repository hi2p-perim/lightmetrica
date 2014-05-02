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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/math.linalgebra.h>
#include <lightmetrica/arealight.h>
#include <lightmetrica/diffuse.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class BPTMISTest : public TestBase
{
public:

	BPTMISTest()
		//: lightSubpath(TransportDirection::LE)
		//, eyeSubpath(TransportDirection::EL)
	{
		/*
		misWeightTypes.emplace_back("bpt.mis.simple");
		misWeightTypes.emplace_back("bpt.mis.powerheuristics");

		// Create light sub-path
		// y0 :
		//   Type     : Emitter
		//   Emitter  : Area light
		//   BSDF     : Diffuse (black)
		//   Position : (0, 0, 0)
		//   Normal   : (0, 1, 0)
		//     
		BPTPathVertex y0;
		y0.type = BPTPathVertexType::EndPoint;
		y0.geom.degenerated = false;
		y0.geom.p = Math::Vec3(0);
		y0.geom.gn = y0.geom.sn = Math::Vec3(0, 1, 0);
		y0.geom.ComputeTangentSpace();
		y0.areaLight = &areaLight;
		y0.bsdf
		*/
	}

protected:

	//std::vector<std::string> misWeightTypes;
	//StubAssets assets;
	
	//AreaLight areaLight;
	//DiffuseBSDF diffuseBSDF_Black;

	//BPTSubpath lightSubpath, eyeSubpath;
	//BPTFullPath fullPath;

};

/*
	Checks if the condition W1
	(p.260 on Veach's thesis) is preserved.
*/
TEST_F(BPTMISTest, Condition_W1)
{
	//for (const auto& type : misWeightTypes)
	//{
		// Create MIS weighting function
		//std::unique_ptr<BPTMISWeight> misWeight(ComponentFactory::Create<BPTMISWeight>(type));
		//EXPECT_TRUE(misWeight->Configure(ConfigNode(), assets));

		//misWeight->Evaluate()
	//}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END