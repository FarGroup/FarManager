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

template<class type, template<class> class operation, class = void>
struct is_valid: std::false_type {};

template<class type, template<class> class operation>
struct is_valid<type, operation, std::void_t<operation<type>>>: std::true_type {};

template<typename type, template<class> class operation>
constexpr bool is_valid_v = is_valid<type, operation>::value;


template<typename type, typename... args>
using is_one_of = std::disjunction<std::is_same<type, args>...>;

template<typename type, typename... args>
constexpr bool is_one_of_v = is_one_of<type, args...>::value;

namespace detail
{
	template<typename type>
	using try_begin = decltype(std::begin(std::declval<type&>()));

	template<typename type>
	using try_end = decltype(std::end(std::declval<type&>()));
}

template<class type>
using is_range = std::conjunction<is_valid<type, detail::try_begin>, is_valid<type, detail::try_end>>;

template<class type>
constexpr bool is_range_v = is_range<type>::value;


#endif // TYPE_TRAITS_HPP_CC9B8497_9AF0_4882_A470_81FF9CBF6D7C
