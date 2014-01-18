/*
	L I G H T  M E T R I C A

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
#include <lightmetrica/version.h>

LM_NAMESPACE_BEGIN

std::string Version::Major()
{
	return LM_VERSION_MAJOR;
}

std::string Version::Minor()
{
	return LM_VERSION_MINOR;
}

std::string Version::Patch()
{
	return LM_VERSION_PATCH;
}

std::string Version::Revision()
{
	return LM_VERSION_REVISION;
}

std::string Version::BuildDate()
{
	return LM_BUILD_DATE;
}

std::string Version::Formatted()
{
	return boost::str(boost::format("%s.%s.%s.%s")
		% LM_VERSION_MAJOR
		% LM_VERSION_MINOR
		% LM_VERSION_PATCH
		% LM_VERSION_REVISION);
}

std::string Version::Platform()
{
#ifdef LM_PLATFORM_WINDOWS
	return "Windows";
#elif defined(LM_PLATFORM_LINUX)
	return "Linux";
#endif
}

std::string Version::Archtecture()
{
#ifdef LM_ARCH_X64
	return "x64";
#elif defined(LM_ARCH_X86)
	return "x86";
#endif
}

std::string Version::Codename()
{
	return LM_VERSION_CODENAME;
}

LM_NAMESPACE_END
