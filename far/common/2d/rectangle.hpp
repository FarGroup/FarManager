#ifndef RECTANGLE_HPP_5553FC58_2069_43EC_AA37_98495C03BAD2
#define RECTANGLE_HPP_5553FC58_2069_43EC_AA37_98495C03BAD2
#pragma once

/*
rectangle.hpp

*/
/*
Copyright © 2018 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "point.hpp"

template<typename T>
struct rectangle_t
{
	T left;
	T top;
	T right;
	T bottom;

	rectangle_t() = default;

	rectangle_t(T const Left, T const Top, T const Right, T const Bottom):
		left(Left),
		top(Top),
		right(Right),
		bottom(Bottom)
	{
		//assert(left <= right);
		//assert(top <= bottom);
	}

	template<typename Y>
	rectangle_t(rectangle_t<Y> const Rectangle):
		rectangle_t(Rectangle.left, Rectangle.top, Rectangle.right, Rectangle.bottom)
	{
	}

	auto width() const { assert(left <= right); return right - left + 1; }
	auto height() const { assert(top <= bottom); return bottom - top + 1; }

	bool contains(point Point) const
	{
		return InRange(left, Point.x, right) && InRange(top, Point.y, bottom);
	}
};

using small_rectangle = rectangle_t<short>;
using rectangle = rectangle_t<int>;

#endif // RECTANGLE_HPP_5553FC58_2069_43EC_AA37_98495C03BAD2
