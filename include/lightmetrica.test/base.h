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
#ifndef LIB_LIGHTMETRICA_TEST_BASE_H
#define LIB_LIGHTMETRICA_TEST_BASE_H

#include "common.h"
#include <lightmetrica/object.h>
#include <gtest/gtest.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class TestBase : public ::testing::Test
{
public:

	// Default timeout in milliseconds
	static const long long OutputProcessTimeout;

protected:

	virtual void SetUp();
	virtual void TearDown();

};

class TemporaryFile
{
public:

	virtual ~TemporaryFile();

	// Get the full path of the filename
	std::string Path() const { return path; }

protected:

	std::string path;

};

class TemporaryTextFile : public TemporaryFile
{
public:

	TemporaryTextFile(const std::string& filename, const std::string& content);

};

class TemporaryBinaryFile : public TemporaryFile
{
public:

	TemporaryBinaryFile(const std::string& filename, const unsigned char* content, unsigned int length);

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_BASE_H