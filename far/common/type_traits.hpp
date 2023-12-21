#ifndef TYPE_TRAITS_HPP_CC9B8497_9AF0_4882_A470_81FF9CBF6D7C
#define TYPE_TRAITS_HPP_CC9B8497_9AF0_4882_A470_81FF9CBF6D7C
#pragma once

/*
type_traits.hpp
*/
/*
Copyright © 2017 Far Group
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

#include <concepts>
#include <type_traits>

//----------------------------------------------------------------------------

template<typename type, typename... args>
using is_one_of = std::disjunction<std::is_same<type, args>...>;

template<typename type, typename... args>
inline constexpr bool is_one_of_v = is_one_of<type, args...>::value;

template<typename T> requires std::integral<T> || std::is_enum_v<T>
auto sane_to_underlying(T Value)
{
	if constexpr (std::is_enum_v<T>)
		return static_cast<std::underlying_type_t<T>>(Value);
	else
		return Value;
}

template<typename T>
using sane_underlying_type = decltype(sane_to_underlying(std::declval<T&>()));

#endif // TYPE_TRAITS_HPP_CC9B8497_9AF0_4882_A470_81FF9CBF6D7C
