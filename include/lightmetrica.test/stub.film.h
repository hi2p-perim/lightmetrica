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

	StubFilm(const std::string& id) : Film(id) {}
	virtual int Width() const { return 200; }
	virtual int Height() const { return 100; }
	virtual bool Save(const std::string& path) const { return true; }
	virtual bool RescaleAndSave( const std::string& path, const Math::Float& weight ) const { return true; }
	virtual void RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb ) {}
	virtual void AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb ) {}
	virtual void AccumulateContribution( const Film& film ) {}
	virtual void Rescale( const Math::Float& weight ) {}
	virtual bool LoadAsset( const ConfigNode& node, const Assets& assets ) { return true; }
	virtual std::string Type() const { return "stub"; }
	virtual Film* Clone() const { return nullptr; }

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_STUB_FILM_H