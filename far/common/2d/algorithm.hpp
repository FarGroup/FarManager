#ifndef ALGORITHM_HPP_6F8540EC_CCA6_4932_8DE1_D3BEDFF24453
#define ALGORITHM_HPP_6F8540EC_CCA6_4932_8DE1_D3BEDFF24453
#pragma once

/*
algorithm.hpp

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

#include "rectangle.hpp"
#include "../function_traits.hpp"

//----------------------------------------------------------------------------

template<class P>
void for_submatrix(auto& Matrix, rectangle Rect, P Predicate)
{
	for (auto i = Rect.top; i <= Rect.bottom; ++i)
	{
		for (auto j = Rect.left; j <= Rect.right; ++j)
		{
			if constexpr (function_traits<P>::arity == 2)
				Predicate(Matrix[i][j], point{ j - Rect.left, i - Rect.top });
			else
				Predicate(Matrix[i][j]);
		}
	}
}

template<typename T, size_t Width, size_t Height>
static consteval auto column_major_iota()
{
	std::array<T, Width* Height> Result;
	static_assert(Result.size() - 1 <= std::numeric_limits<T>::max());

	for (size_t Row = 0; Row != Height; ++Row)
		for (size_t Col = 0; Col != Width; ++Col)
			Result[Col + Row * Width] = static_cast<T>(Row + Col * Height);

	return Result;
}

#endif // ALGORITHM_HPP_6F8540EC_CCA6_4932_8DE1_D3BEDFF24453
