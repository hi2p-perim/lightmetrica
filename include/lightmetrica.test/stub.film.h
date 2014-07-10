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
#ifndef LIB_LIGHTMETRICA_TEST_STUB_FILM_H
#define LIB_LIGHTMETRICA_TEST_STUB_FILM_H

#include "common.h"
#include <lightmetrica/film.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubFilm : public Film
{
public:

	LM_COMPONENT_IMPL_DEF("stub");

public:

	virtual int Width() const { return 200; }
	virtual int Height() const { return 100; }
	virtual bool Save(const std::string& path) const { return true; }
	virtual bool RescaleAndSave( const std::string& path, const Math::Float& weight ) const { return true; }
	virtual void RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb ) {}
	virtual void AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb ) {}
	virtual void AccumulateContribution( const Film& film ) {}
	virtual void Rescale( const Math::Float& weight ) {}
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }
	virtual Film* Clone() const { return nullptr; }

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_STUB_FILM_H