#ifndef CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
#define CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
#pragma once

/*
cpp.hpp

Some workarounds & emulations for C++ features, missed in currently used compilers & libraries.

Here be dragons
*/
/*
Copyright © 2013 Far Group
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

#include "common/compiler.hpp"
#include "common/preprocessor.hpp"

#if COMPILER == C_GCC
// These inline implementations in gcc/cwchar are wrong and non-compilable if _CONST_RETURN is defined.
namespace std
{
	inline wchar_t* wcschr(wchar_t* p, wchar_t c)
	{
		return const_cast<wchar_t*>(wcschr(const_cast<const wchar_t*>(p), c));
	}

	inline wchar_t* wcspbrk(wchar_t* s1, const wchar_t* s2)
	{
		return const_cast<wchar_t*>(wcspbrk(const_cast<const wchar_t*>(s1), s2));
	}

	inline wchar_t* wcsrchr(wchar_t* p, wchar_t c)
	{
		return const_cast<wchar_t*>(wcsrchr(const_cast<const wchar_t*>(p), c));
	}

	inline wchar_t* wcsstr(wchar_t* s1, const wchar_t* s2)
	{
		return const_cast<wchar_t*>(wcsstr(const_cast<const wchar_t*>(s1), s2));
	}

	inline wchar_t* wmemchr(wchar_t* p, wchar_t c, size_t n)
	{
		return const_cast<wchar_t*>(wmemchr(const_cast<const wchar_t*>(p), c, n));
	}
}

using std::wcschr;
using std::wcspbrk;
using std::wcsrchr;
using std::wcsstr;
using std::wmemchr;

#endif

#if VS_OLDER_THAN(1910)
namespace std
{
	namespace detail
	{
		template <class F, class Tuple, size_t... I>
		constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
		{
			return std::invoke(FWD(f), std::get<I>(FWD(t))...);
		}
	}

	template <class F, class Tuple>
	constexpr decltype(auto) apply(F&& f, Tuple&& t)
	{
		return detail::apply_impl(FWD(f), FWD(t), std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
	}
}
#endif


#if COMPILER == C_CL && _MSC_VER < 1910
#define DETAIL_STATIC_ASSERT_2(expression, message) static_assert(expression, message)
#define DETAIL_STATIC_ASSERT_1(expression) DETAIL_STATIC_ASSERT_2(expression, #expression)
#define DETAIL_STATIC_ASSERT_GET_MACRO(_1, _2, NAME, ...) NAME
#define static_assert(...) EXPAND(DETAIL_STATIC_ASSERT_GET_MACRO(__VA_ARGS__, DETAIL_STATIC_ASSERT_2, DETAIL_STATIC_ASSERT_1)(__VA_ARGS__))
#endif


#if VS_OLDER_THAN(1911)
namespace std
{
	namespace detail
	{
		template <typename void_type, typename, typename...>
		struct invoke_result {};

		template <typename callable, typename... args>
		struct invoke_result<decltype(void(std::invoke(std::declval<callable>(), std::declval<args>()...))), callable, args...>
		{
			using type = decltype(std::invoke(std::declval<callable>(), std::declval<args>()...));
		};
	}

	template <typename callable, typename... args>
	struct invoke_result: detail::invoke_result<void, callable, args...> {};

	template <typename callable, typename... args>
	using invoke_result_t = typename invoke_result<callable, args...>::type;
}
#endif


#if VS_OLDER_THAN(1910)
#include "common/any.hpp"

namespace std
{
	using ::any_impl::any;
	using ::any_impl::any_cast;
	using ::any_impl::bad_any_cast;
}
#else
#include <any>
#endif

#if VS_OLDER_THAN(1910)
#include "common/string_view.hpp"

namespace std
{
	using ::string_view_impl::basic_string_view;
	using ::string_view_impl::string_view;
	using ::string_view_impl::wstring_view;

	inline namespace literals
	{
		inline namespace string_view_literals
		{
			using namespace ::string_view_impl::string_view_literals;
		}
	}
}
#else
#include <string_view>
#endif

#endif // CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
