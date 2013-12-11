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

#include "vector.h"

NANON_NAMESPACE_BEGIN

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4()
	: x(T(0))
	, y(T(0))
	, z(T(0))
	, w(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(v.w)
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const T& v)
	: x(v)
	, y(v)
	, z(v)
	, w(v)
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const T& x, const T& y, const T& z, const T& w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{

}

template <typename T>
NANON_FORCE_INLINE T& TVec4<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE const T& TVec4<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator=(const TVec4<T>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
	return *this;
}

NANON_NAMESPACE_END