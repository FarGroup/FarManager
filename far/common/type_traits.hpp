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

template<typename type, typename... args>
struct is_one_of;

template<typename type>
struct is_one_of<type>: std::false_type {};

template<typename type, typename... args>
struct is_one_of<type, type, args...>: std::true_type {};

template<typename type1, typename type2, typename... args>
struct is_one_of<type1, type2, args...>: is_one_of<type1, args...> {};

template<typename... args>
constexpr bool is_one_of_v = is_one_of<args...>::value;

#endif // TYPE_TRAITS_HPP_CC9B8497_9AF0_4882_A470_81FF9CBF6D7C
