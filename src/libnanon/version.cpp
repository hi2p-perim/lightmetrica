/*
	nanon : A research-oriented renderer

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
#include "versiondef.h"
#include <nanon/version.h>

NANON_NAMESPACE_BEGIN

std::string Version::Major()
{
	return NANON_VERSION_MAJOR;
}

std::string Version::Minor()
{
	return NANON_VERSION_MINOR;
}

std::string Version::Patch()
{
	return NANON_VERSION_PATCH;
}

std::string Version::Revision()
{
	return NANON_VERSION_REVISION;
}

std::string Version::BuildDate()
{
	return NANON_BUILD_DATE;
}

std::string Version::Formatted()
{
	return boost::str(boost::format("%s.%s.%s.%s")
		% NANON_VERSION_MAJOR
		% NANON_VERSION_MINOR
		% NANON_VERSION_PATCH
		% NANON_VERSION_REVISION);
}

std::string Version::Platform()
{
#ifdef NANON_PLATFORM_WINDOWS
	return "Windows";
#elif NANON_PLATFORM_LINUX
	return "Linux";
#endif
}

std::string Version::Archtecture()
{
#ifdef NANON_ARCH_X64
	return "x64";
#elif NANON_ARCH_X86
	return "x86";
#endif
}

NANON_NAMESPACE_END