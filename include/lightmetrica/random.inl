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

#include "random.h"

LM_NAMESPACE_BEGIN

LM_FORCE_INLINE Math::Float Random::Next()
{
	return Math::Float(NextUInt() * (1.0/4294967296.0));
}

LM_FORCE_INLINE Math::Vec2 Random::NextVec2()
{
	// Note : according to C++ standard, evaluation order of the arguments are undefined
	// so we avoid the implementation like Vec2(Next(), Next()).
	auto u1 = Next();
	auto u2 = Next();
	return Math::Vec2(u1, u2);
}

LM_NAMESPACE_END