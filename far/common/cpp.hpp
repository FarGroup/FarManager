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

#include "compiler.hpp"

#include <version>

//----------------------------------------------------------------------------

#ifdef FAR_ENABLE_CORRECT_ISO_CPP_WCHAR_H_OVERLOADS
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

//----------------------------------------------------------------------------

#if COMPILER(GCC) && !defined(_GLIBCXX_HAS_GTHREADS)

namespace std::this_thread
{
	inline void yield() noexcept
	{
		Sleep(0);
	}
}
#endif

//----------------------------------------------------------------------------

#ifndef __cpp_lib_to_underlying
#include <type_traits>

namespace std
{
	template<class enum_type>
	[[nodiscard]]
	constexpr auto to_underlying(enum_type const Enum) noexcept
	{
		return static_cast<std::underlying_type_t<enum_type>>(Enum);
	}
}
#endif

#ifndef __cpp_lib_unreachable
#include <cassert>

namespace std
{
	[[noreturn]]
	inline void unreachable()
	{
		assert(false);

#if COMPILER(CL)
		__assume(0);
#else
		__builtin_unreachable();
#endif
	}
}
#endif

#ifndef __cpp_lib_ranges_fold
#include <algorithm>
#ifndef _LIBCPP___ALGORITHM_FOLD_H // as of March 2024 libc++ doesn't define __cpp_lib_ranges_fold
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace std::ranges
{
	struct fold_left_fn
	{
		template<std::input_iterator I, std::sentinel_for<I> S, typename T, typename F>
		constexpr auto operator()(I first, S last, T init, F f) const
		{
			using U = std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>;
			if (first == last)
				return U(std::move(init));
			U accum = std::invoke(f, std::move(init), *first);
			for (++first; first != last; ++first)
				accum = std::invoke(f, std::move(accum), *first);
			return std::move(accum);
		}

		template<ranges::input_range R, typename T, typename F>
		constexpr auto operator()(R&& r, T init, F f) const
		{
			return (*this)(ranges::begin(r), ranges::end(r), std::move(init), std::ref(f));
		}
	};

	inline constexpr fold_left_fn fold_left;
}
#endif
#endif

#ifndef __cpp_size_t_suffix

WARNING_PUSH()
WARNING_DISABLE_MSC(4455) // 'operator operator': literal suffix identifiers that do not start with an underscore are reserved

[[nodiscard]] consteval size_t operator""uz(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""Uz(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""uZ(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""UZ(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""zu(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""Zu(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""zU(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""ZU(unsigned long long const Value) noexcept { return Value; }

WARNING_POP()

#endif
//----------------------------------------------------------------------------

#endif // CPP_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
