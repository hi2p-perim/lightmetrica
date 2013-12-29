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

#pragma once
#ifndef __LIB_NANON_VERSION_H__
#define __LIB_NANON_VERSION_H__

#include "common.h"
#include <string>

NANON_NAMESPACE_BEGIN

/*!
	Version information.
	The class is used to get version information of the library.
*/
class NANON_PUBLIC_API Version
{
private:

	Version();
	~Version();

	NANON_DISABLE_COPY_AND_MOVE(Version);

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

NANON_NAMESPACE_END

#endif // __LIB_NANON_VERSION_H__