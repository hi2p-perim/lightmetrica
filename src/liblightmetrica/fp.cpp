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

#include "pch.h"
#include <lightmetrica/fp.h>
#include <lightmetrica/logger.h>
#if LM_PLATFORM_WINDOWS
#include <windows.h>
#endif

LM_NAMESPACE_BEGIN

bool FloatintPointUtils::EnableFPControl()
{
#if LM_STRICT_FP && LM_PLATFORM_WINDOWS

	errno_t error;

	// Restore current floating-point control word
	unsigned int currentFPState;
	if ((error = _controlfp_s(&currentFPState, 0, 0)) != 0)
	{
		LM_LOG_ERROR("_controlfp_s failed : " + std::string(strerror(error)));
		return false;
	}

	// Set a new control word
	// Unmask all floating-point exceptions
#if 0
	unsigned int newFPState = currentFPState &
							~(_EM_INVALID    |		// Invalid operation
							  _EM_DENORMAL   |		// Denormal operand 
							  _EM_ZERODIVIDE |		// Divide by zero
							  _EM_OVERFLOW   |		// Overflow
							  _EM_UNDERFLOW  |		// Underflow
							  _EM_INEXACT);			// Inexact result
#else
	unsigned int newFPState = currentFPState &
							~(_EM_INVALID    |		// Invalid operation
							  _EM_DENORMAL   |		// Denormal operand 
							  _EM_ZERODIVIDE);		// Divide by zero
#endif

	if ((error = _controlfp_s(&currentFPState, newFPState, _MCW_EM)) != 0)
	{
		LM_LOG_ERROR("_controlfp_s failed : " + std::string(strerror(error)));
		return false;
	}

	return true;

#else

	LM_LOG_ERROR("Unsupported platform");
	return false;

#endif
}

LM_NAMESPACE_END
