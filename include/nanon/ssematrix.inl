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

#include "ssematrix.h"

NANON_NAMESPACE_BEGIN

NANON_FORCE_INLINE TMat4<float>::TMat4()
{

}

NANON_FORCE_INLINE TMat4<float>::TMat4(const Mat4f& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
	v[3] = m.v[3];
}

NANON_FORCE_INLINE TMat4<float>::TMat4(float v)
{
	this->v[0] = TVec4<float>(v);
	this->v[1] = TVec4<float>(v);
	this->v[2] = TVec4<float>(v);
	this->v[3] = TVec4<float>(v);
}

NANON_FORCE_INLINE TMat4<float>::TMat4(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, const Vec4f& v3)
{

}

NANON_FORCE_INLINE TMat4<float>::TMat4(
	float v00, float v10, float v20, float v30,
	float v01, float v11, float v21, float v31,
	float v02, float v12, float v22, float v32,
	float v03, float v13, float v23, float v33)
{

}

NANON_FORCE_INLINE Mat4f TMat4<float>::Zero()
{

}

NANON_FORCE_INLINE Mat4f TMat4<float>::Diag(float v)
{

}

NANON_FORCE_INLINE Mat4f TMat4<float>::Identity()
{

}

NANON_FORCE_INLINE TVec4<float>& TMat4<float>::operator[](int i)
{

}

NANON_NAMESPACE_END