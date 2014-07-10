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
#ifndef LIB_LIGHTMETRICA_TRIANGLE_REF_H
#define LIB_LIGHTMETRICA_TRIANGLE_REF_H

/*!
	Reference to triangle information.
	The structure is used as intermediate data for some processes, e.g., QBVH construction.
	A entry of the structure points to a triangle in a mesh in one of a primitives.
*/
struct TriangleRef
{

	int primitiveIndex;		//!< Index of a primitive. -1 specifies no reference
	int faceIndex;			//!< Index of a face in the primitive

	LM_FORCE_INLINE TriangleRef()
		: primitiveIndex(-1)
	{

	}

};

#endif // LIB_LIGHTMETRICA_TRIANGLE_REF_H