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
#ifndef LIB_LIGHTMETRICA_VERSION_H
#define LIB_LIGHTMETRICA_VERSION_H

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN

/*!
	Version information.
	The class is used to get version information of the library.
*/
class LM_PUBLIC_API Version
{
private:

	Version();
	~Version();

	LM_DISABLE_COPY_AND_MOVE(Version);

public:

	/*!
		Get the major version of the library.
		\return Major version.
	*/
	static std::string Major();

	/*!
		Get the minor version of the library.
		\return Minor version.
	*/
	static std::string Minor();

	/*!
		Get the patch version of the library.
		\return Patch version.
	*/
	static std::string Patch();

	/*!
		Get the revision number of the library.
		\return Revision.
	*/
	static std::string Revision();

	/*!
		Get the version codename of the library.
		\return Codename.
	*/
	static std::string Codename();

	/*!
		Get the build date of the library.
		\return Build date.
	*/
	static std::string BuildDate();
	
	/*!
		Get the formatted version of the library.
		Returns the formatted version in \a major.minor.patch.revision.
		\return Formatted version.
	*/
	static std::string Formatted();

	/*!
		Get the platform name.
		\return Platform name.
	*/
	static std::string Platform();

	/*!
		Get the architecture name.
		\return Architecture name.
	*/
	static std::string Archtecture();

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_VERSION_H
