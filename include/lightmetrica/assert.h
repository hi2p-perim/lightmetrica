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
#ifndef __LM_CORE_ASSERT_H__
#define __LM_CORE_ASSERT_H__

#include "logger.h"
#include <cassert>
#include <string>

#if LM_DEBUG_MODE
	#define LM_ASSERT(cond) if (!(cond)) { LM_LOG_ERROR("Assertion failed : '" + std::string(#cond) + "'"); abort(); }
#else
	#define LM_ASSERT(cond)
#endif

#endif // __LM_CORE_ASSERT_H__