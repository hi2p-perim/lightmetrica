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
#include "simdsupport.h"
#include <lightmetrica/object.h>
#include <lightmetrica/align.h>

LM_NAMESPACE_BEGIN

void* Object::operator new(std::size_t size) throw (std::bad_alloc) 
{
#if LM_SINGLE_PRECISION && LM_SSE2
	void* p = aligned_malloc(size, 16);
#elif LM_DOUBLE_PRECISION && LM_AVX
	void* p = aligned_malloc(size, 32);
#else
	void* p = malloc(size);
#endif
	if (!p) throw std::bad_alloc();
	return p;
}

void Object::operator delete(void* p)
{
#if (LM_SINGLE_PRECISION && LM_SSE2) || (LM_DOUBLE_PRECISION && LM_AVX)
	aligned_free(p);
#else
	free(p);
#endif
}

LM_NAMESPACE_END
