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
#ifndef LM_CORE_ASSERT_H
#define LM_CORE_ASSERT_H

#include "logger.h"
#include <cassert>
#include <string>

#if LM_DEBUG_MODE
	#define LM_ASSERT(cond) do { if (!(cond)) { LM_LOG_ERROR("Assertion failed : '" + std::string(#cond) + "'"); abort(); } } while (0);
#else
	#define LM_ASSERT(cond) ((void)0)
#endif

#if LM_DEBUG_MODE
	#define LM_UNREACHABLE() LM_ASSERT(false)
#else
	#if LM_COMPILER_GCC
		#define LM_UNREACHABLE() __builtin_unreachable()
	#elif LM_COMPILER_MSVC
		#define LM_UNREACHABLE() __assume(0)
	#endif
#endif

#endif // LM_CORE_ASSERT_H