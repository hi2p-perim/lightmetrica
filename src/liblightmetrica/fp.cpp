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
#include "pch.h"
#include <lightmetrica/fp.h>
#include <lightmetrica/logger.h>

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
	unsigned int newFPState = currentFPState &
							~(_EM_INVALID			// Invalid operation
							| _EM_DENORMAL			// Denormal operand 
							| _EM_ZERODIVIDE		// Divide by zero
							| _EM_OVERFLOW			// Overflow
							| _EM_UNDERFLOW);		// Underflow
	//						| _EM_INEXACT);			// Inexact result

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
