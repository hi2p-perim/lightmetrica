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

#include "pch.test.h"
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/base.math.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica.test/stub.film.h>
#include <lightmetrica/camera.h>

namespace
{

	const std::string ThinLensCameraNode_Success = LM_TEST_MULTILINE_LITERAL(
		<camera id="test" type="thinlens">
			<film ref="stub" />
			<fovy>90</fovy>

		</camera>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class ThinLensCameraTest : public TestBase
{
public:

	ThinLensCameraTest()
		: camera(ComponentFactory::Create<Camera>("thinlens"))
	{
		// Add assets
		assets.Add("stub", new StubFilm);
	}

protected:

	StubAssets assets;
	StubConfig config;
	std::unique_ptr<Camera> camera;

};

TEST_F(ThinLensCameraTest, Load)
{
	
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END