#ifndef POINT_HPP_5D641DB2_6406_4A9E_8D64_C642ECCF9790
#define POINT_HPP_5D641DB2_6406_4A9E_8D64_C642ECCF9790
#pragma once

/*
point.hpp

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

#include "../preprocessor.hpp"
#include "../rel_ops.hpp"

//----------------------------------------------------------------------------

struct point: public rel_ops<point>
{
	int x{};
	int y{};

	point() = default;

	point(int const X, int const Y) noexcept:
		x(X),
		y(Y)
	{
	}

	template<typename T, REQUIRES(sizeof(T::X) && sizeof(T::Y))>
	point(T const& Coord) noexcept:
		point(Coord.X, Coord.Y)
	{
	}

	[[nodiscard]]
	bool operator==(point const& rhs) const noexcept
	{
		return x == rhs.x && y == rhs.y;
	}
};

#endif // POINT_HPP_5D641DB2_6406_4A9E_8D64_C642ECCF9790
