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

#ifndef __LIB_NANON_SSE_MATRIX_H__
#define __LIB_NANON_SSE_MATRIX_H__

#include "matrix.h"
#include "ssevector.h"

NANON_NAMESPACE_BEGIN

/*!
	SSE optimized 4x4 matrix.
	Specialized version of TMat4 optimized by SSE.
*/
template <>
struct NANON_ALIGN_16 TMat4<float>
{
	
	TVec4<float> v[4];

	NANON_FORCE_INLINE TMat4();
	NANON_FORCE_INLINE TMat4(const Mat4f& m);
	NANON_FORCE_INLINE TMat4(float v);
	NANON_FORCE_INLINE TMat4(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, const Vec4f& v3);
	NANON_FORCE_INLINE TMat4(
		float v00, float v10, float v20, float v30,
		float v01, float v11, float v21, float v31,
		float v02, float v12, float v22, float v32,
		float v03, float v13, float v23, float v33);

	static NANON_FORCE_INLINE Mat4f Zero();
	static NANON_FORCE_INLINE Mat4f Diag(float v);
	static NANON_FORCE_INLINE Mat4f Identity();

	NANON_FORCE_INLINE TVec4<float>& operator[](int i);

};

NANON_NAMESPACE_END

#include "ssematrix.inl"

#endif //__LIB_NANON_SSE_MATRIX_H__