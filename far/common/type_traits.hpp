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

//----------------------------------------------------------------------------

namespace detail
{
	template<typename void_type, template<typename...> typename operation, typename... args>
	struct is_detected : std::false_type{};

	template<template<typename...> typename operation, typename... args>
	struct is_detected<std::void_t<operation<args...>>, operation, args...> : std::true_type{};
}

template <template<typename...> typename operation, typename... args>
using is_detected = typename detail::is_detected<void, operation, args...>::type;

template<template<typename...> typename operation, typename... args>
inline constexpr bool is_detected_v = is_detected<operation, args...>::value;


template<typename type, typename... args>
using is_one_of = std::disjunction<std::is_same<type, args>...>;

template<typename type, typename... args>
inline constexpr bool is_one_of_v = is_one_of<type, args...>::value;

namespace detail
{
#define DETAIL_TRY_(What) \
	template<typename type> \
	using try_ ## What = decltype(std::What(std::declval<type&>()))

	DETAIL_TRY_(begin);
	DETAIL_TRY_(end);
	DETAIL_TRY_(data);
	DETAIL_TRY_(size);

#undef DETAIL_TRY_
}

template<class type>
using is_range = std::conjunction<is_detected<detail::try_begin, type>, is_detected<detail::try_end, type>>;

template<class type>
inline constexpr bool is_range_v = is_range<type>::value;

template<class type>
using is_span = std::conjunction<is_detected<detail::try_data, type>, is_detected<detail::try_size, type>>;

template<class type>
inline constexpr bool is_span_v = is_span<type>::value;


#endif // TYPE_TRAITS_HPP_CC9B8497_9AF0_4882_A470_81FF9CBF6D7C
