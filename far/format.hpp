#ifndef FORMAT_HPP_27C3F464_170B_432E_9D44_3884DDBB95AC
#define FORMAT_HPP_27C3F464_170B_432E_9D44_3884DDBB95AC
#pragma once

/*
format.hpp

Форматирование строк
*/
/*
Copyright © 2009 Far Group
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

// Internal:

// Platform:

// Common:
#include "common/preprocessor.hpp"
#include "common/type_traits.hpp"

// External:

//----------------------------------------------------------------------------

WARNING_PUSH(3)

WARNING_DISABLE_GCC("-Warray-bounds")
WARNING_DISABLE_GCC("-Wctor-dtor-privacy")
WARNING_DISABLE_GCC("-Wdangling-reference")
WARNING_DISABLE_GCC("-Wstringop-overflow")

WARNING_DISABLE_CLANG("-Weverything")

#define FMT_CONSTEVAL consteval
#define FMT_HAS_CONSTEVAL

#define FMT_STATIC_THOUSANDS_SEPARATOR
#include "thirdparty/fmt/fmt/format.h"
#include "thirdparty/fmt/fmt/xchar.h"
#undef FMT_STATIC_THOUSANDS_SEPARATOR

WARNING_POP()

namespace far
{
	template<typename... args>
	using format_string = fmt::wformat_string<args...>;

	template<typename... args>
	using char_format_string = fmt::format_string<args...>;

	template <typename... args>
	auto format(format_string<args...> const Format, args const&... Args)
	{
		return fmt::vformat(fmt::wstring_view(Format), fmt::make_wformat_args(Args...));
	}

	template <typename... args>
	auto format(char_format_string<args...> const Format, args const&... Args)
	{
		return fmt::vformat(fmt::string_view(Format), fmt::make_format_args(Args...));
	}

	// Don't "auto" it yet, ICE in VS2019
	template <typename... args>
	auto vformat(string_view const Format, args const&... Args)
	{
		return fmt::vformat(fmt::wstring_view(Format), fmt::make_wformat_args(Args...));
	}

	template<typename I, typename... args> requires std::output_iterator<I, wchar_t>
	auto format_to(I const Iterator, format_string<args...> const Format, args const&... Args)
	{
		return fmt::vformat_to(Iterator, fmt::wstring_view(Format), fmt::make_wformat_args(Args...));
	}

	template<typename... args>
	auto format_to(string& Str, format_string<args...> const Format, args const&... Args)
	{
		return fmt::vformat_to(std::back_inserter(Str), fmt::wstring_view(Format), fmt::make_wformat_args(Args...));
	}
}

#define FSTR(str) FMT_STRING(str)

auto str(const auto& Value)
{
	return fmt::to_wstring(Value);
}

inline auto str(const void* Value)
{
	return far::format(L"0x{:0{}X}"sv, std::bit_cast<uintptr_t>(Value), sizeof(Value) * 2);
}

inline auto str(void* Value)
{
	return str(static_cast<void const*>(Value));
}

string str(const char*) = delete;
string str(const wchar_t*) = delete;
string str(std::string) = delete;
string str(string) = delete;
string str(std::string_view) = delete;
string str(string_view) = delete;

namespace format_helpers
{
	struct parse_no_spec
	{
		constexpr auto parse(auto& ctx) const
		{
			return ctx.begin();
		}
	};

	template<typename object_type>
	struct format_no_spec
	{
		// Don't "auto" it yet, ICE in VS2019
		template<typename FormatContext>
		auto format(object_type const& Value, FormatContext& ctx) const
		{
			return fmt::format_to(ctx.out(), L"{}"sv, fmt::formatter<object_type, wchar_t>::to_string(Value));
		}
	};

	template<typename object_type>
	struct no_spec: parse_no_spec, format_no_spec<object_type>
	{
	};
}

template<typename object_type>
struct formattable;

namespace detail
{
	template<typename type>
	constexpr inline auto is_formattable = requires
	{
		formattable<type>{};
	};

	template<typename type>
	constexpr inline auto has_to_string = requires(type t)
	{
		t.to_string();
	};
}

template<typename object_type> requires detail::is_formattable<object_type> || detail::has_to_string<object_type>
struct fmt::formatter<object_type, wchar_t>: format_helpers::no_spec<object_type>
{
	static string to_string(object_type const& Value)
	{
		if constexpr(::detail::is_formattable<object_type>)
			return formattable<object_type>::to_string(Value);
		else
			return Value.to_string();
	}
};

// fmt 9 deprecated implicit enums formatting :(
template<typename enum_type> requires std::is_enum_v<enum_type>
auto format_as(enum_type const Value)
{
	return std::to_underlying(Value);
}

#endif // FORMAT_HPP_27C3F464_170B_432E_9D44_3884DDBB95AC
