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
#ifndef LIB_LIGHTMETRICA_TEST_TEST_SCENES_H
#define LIB_LIGHTMETRICA_TEST_TEST_SCENES_H

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

/*!
	Test scenes.
	A collection of test scenes.
	Returned string contains 'assets' and 'scene' element
	which can be passed to asset manager or scenes after parsing.
*/
class TestScenes
{
private:

	TestScenes();
	LM_DISABLE_COPY_AND_MOVE(TestScenes);

public:

	static std::string Simple03()
	{
		return LM_TEST_MULTILINE_LITERAL(
			<assets>
				<triangle_meshes>
					<triangle_mesh id="quad" type="raw">
						<positions>
							-0.1 0 -0.1
							-0.1 0 0.1
							0.1 0 0.1
							0.1 0 -0.1
						</positions>
						<normals>
							0 -1 0
							0 -1 0
							0 -1 0
							0 -1 0
						</normals>
						<faces>
							0 2 1
							0 3 2
						</faces>
					</triangle_mesh>
				</triangle_meshes>
				<bsdfs>
					<bsdf id="diffuse_white" type="diffuse">
						<diffuse_reflectance>
							<color>0.9 0.9 0.9</color>
						</diffuse_reflectance>
					</bsdf>
					<bsdf id="diffuse_black" type="diffuse">
						<diffuse_reflectance>
							<color>0 0 0</color>
						</diffuse_reflectance>
					</bsdf>
					<bsdf id="diffuse_red" type="diffuse">
						<diffuse_reflectance>
							<color>0.9 0.1 0.1</color>
						</diffuse_reflectance>
					</bsdf>
					<bsdf id="diffuse_green" type="diffuse">
						<diffuse_reflectance>
							<color>0.1 0.9 0.1</color>
						</diffuse_reflectance>
					</bsdf>
				</bsdfs>
				<films>
					<film id="film_1" type="hdr">
						<width>500</width>
						<height>500</height>
						<imagetype>radiancehdr</imagetype>
					</film>
				</films>
				<cameras>
					<camera id="camera_1" type="perspective">
						<film ref="film_1" />
						<fovy>45</fovy>
					</camera>
				</cameras>
				<lights>
					<light id="light_1" type="area">
						<luminance>2 2 2</luminance>
					</light>
				</lights>
			</assets>
			<scene type="naive">
				<root>
					<node>
						<transform>
							<lookat>
								<position>0 0.1 0.3</position>
								<center>0 0.1 0</center>
								<up>0 1 0</up>
							</lookat>
						</transform>
						<camera ref="camera_1" />
					</node>
					<node>
						<transform>
							<rotate>
								<angle>-90</angle>
								<axis>1 0 0</axis>
							</rotate>
							<translate>0 0.1 -0.1</translate>
						</transform>
						<triangle_mesh ref="quad" />
						<bsdf ref="diffuse_white" />
					</node>
					<node>
						<transform>
							<translate>0 0.2 0</translate>
						</transform>
						<triangle_mesh ref="quad" />
						<light ref="light_1" />
						<bsdf ref="diffuse_black" />
					</node>
				</root>
			</scene>
		);
	}

	static std::string Simple05()
	{
		return LM_TEST_MULTILINE_LITERAL(
			<assets>
				<triangle_meshes>
					<triangle_mesh id="quad" type="raw">
						<positions>
							-0.1 0 -0.1
							-0.1 0 0.1
							0.1 0 0.1
							0.1 0 -0.1
						</positions>
						<normals>
							0 -1 0
							0 -1 0
							0 -1 0
							0 -1 0
						</normals>
						<faces>
							0 2 1
							0 3 2
						</faces>
					</triangle_mesh>
				</triangle_meshes>
				<bsdfs>
					<bsdf id="diffuse_white" type="diffuse">
						<diffuse_reflectance>
							<color>0.9 0.9 0.9</color>
						</diffuse_reflectance>
					</bsdf>
					<bsdf id="glass" type="dielectric">
						<specular_reflectance>1 0.5 0.5</specular_reflectance>
						<specular_transmittance>1 0.5 0.5</specular_transmittance>
						<external_ior>1</external_ior>
						<internal_ior>1.458</internal_ior>
					</bsdf>
				</bsdfs>
				<films>
					<film id="film_1" type="hdr">
						<width>500</width>
						<height>500</height>
						<imagetype>radiancehdr</imagetype>
					</film>
				</films>
				<cameras>
					<camera id="camera_1" type="perspective">
						<film ref="film_1" />
						<fovy>45</fovy>
					</camera>
				</cameras>
				<lights>
					<light id="light_1" type="area">
						<luminance>2 2 2</luminance>
					</light>
				</lights>
			</assets>
			<scene type="naive">
				<root>
					<node>
						<transform>
							<lookat>
								<position>0 0.1 0.3</position>
								<center>0 0.1 0</center>
								<up>0 1 0</up>
							</lookat>
						</transform>
						<camera ref="camera_1" />
					</node>
					<node>
						<transform>
							<translate>0 0.2 0</translate>
						</transform>
						<triangle_mesh ref="quad" />
						<light ref="light_1" />
						<bsdf ref="diffuse_white" />
					</node>
					<node>
						<transform>
							<rotate>
								<angle>-90</angle>
								<axis>1 0 0</axis>
							</rotate>
							<translate>0 0.1 -0.1</translate>
						</transform>
						<triangle_mesh ref="quad" />
						<bsdf ref="diffuse_white" />
					</node>
					<node>
						<transform>
							<rotate>
								<angle>180</angle>
								<axis>1 0 0</axis>
							</rotate>
							<translate>0 0.02 0</translate>
							<scale>0.5 0.5 0.5</scale>
						</transform>
						<triangle_mesh ref="quad" />
						<bsdf ref="glass" />
					</node>
					<node>
						<transform>
							<rotate>
								<angle>180</angle>
								<axis>1 0 0</axis>
							</rotate>
						</transform>
						<triangle_mesh ref="quad" />
						<bsdf ref="diffuse_white" />
					</node>
				</root>
			</scene>
		);
	}

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_TEST_SCENES_H