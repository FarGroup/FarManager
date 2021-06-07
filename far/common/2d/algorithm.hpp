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
#include "../type_traits.hpp"

//----------------------------------------------------------------------------

namespace detail
{
	template<typename P, typename T, typename I>
	using try_2_args = decltype(std::declval<P&>()(std::declval<T&>(), std::declval<I&>()));

	template<typename P, typename T, typename I>
	inline constexpr bool has_2_args = is_detected_v<try_2_args, P, T, I>;
}

template<class T, class P>
void for_submatrix(T& Matrix, rectangle Rect, P Predicate)
{
	for (auto i = Rect.top; i <= Rect.bottom; ++i)
	{
		for (auto j = Rect.left; j <= Rect.right; ++j)
		{
			if constexpr (detail::has_2_args<P, decltype(Matrix[i][j]), point>)
				Predicate(Matrix[i][j], point{ j - Rect.left, i - Rect.top });
			else
				Predicate(Matrix[i][j]);
		}
	}
}

#endif // ALGORITHM_HPP_6F8540EC_CCA6_4932_8DE1_D3BEDFF24453
