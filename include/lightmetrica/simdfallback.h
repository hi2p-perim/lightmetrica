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

#ifndef __LIB_NANON_SIMD_FALLBACK_H__
#define __LIB_NANON_SIMD_FALLBACK_H__

template <typename T>
struct sv4
{
	T d[4];
};

typedef sv4<double> sv4d;
typedef sv4<float> sv4f;

template <typename T>
sv4<T> sv4_add(const sv4<T>& v1, const sv4<T>& v2)
{
	return sv4<T>();
}

template<>
sv4d sv4_add<double>(const sv4d& v1, const sv4d& v2)
{
	sv4d r;
	r.d[0] = v1.d[0] + v2.d[0];
	r.d[1] = v1.d[1] + v2.d[1];
	r.d[2] = v1.d[2] + v2.d[2];
	r.d[3] = v1.d[3] + v2.d[3];
	return r;
}

#endif // __LIB_NANON_SIMD_FALLBACK_H__