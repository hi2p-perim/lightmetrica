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
#if LM_PLATFORM_WINDOWS
	return "Windows";
#elif LM_PLATFORM_LINUX
	return "Linux";
#endif
}

std::string Version::Archtecture()
{
#if LM_ARCH_X64
	return "x64";
#elif LM_ARCH_X86
	return "x86";
#endif
}

std::string Version::Codename()
{
	return LM_VERSION_CODENAME;
}

LM_NAMESPACE_END
