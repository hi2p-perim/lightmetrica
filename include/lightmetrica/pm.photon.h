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
#ifndef LIB_LIGHTMETRICA_PM_PHOTON_H
#define LIB_LIGHTMETRICA_PM_PHOTON_H

#include "common.h"
#include "math.types.h"
#include "align.h"

LM_NAMESPACE_BEGIN

/*!
	Photon.
	Represents single photon.
*/
struct Photon
{
	Math::Vec3 p;				// Surface point
	Math::Vec3 throughput;		// Current throughput
	Math::Vec3 wi;				// Incident ray direction
};

// Vector type for Photon
typedef std::vector<Photon, aligned_allocator<Photon, std::alignment_of<Photon>::value>> Photons;

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PM_PHOTON_H