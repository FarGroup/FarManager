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

WARNING_DISABLE_GCC("-Wctor-dtor-privacy")

WARNING_DISABLE_CLANG("-Weverything")

#define FMT_CONSTEVAL // Not yet

#define FMT_STATIC_THOUSANDS_SEPARATOR
#include "thirdparty/fmt/fmt/format.h"
#include "thirdparty/fmt/fmt/xchar.h"
#include "thirdparty/fmt/fmt/ostream.h"
#undef FMT_STATIC_THOUSANDS_SEPARATOR

WARNING_POP()

namespace detail
{
	template<typename F>
	void validate_format()
	{
		static_assert(!std::is_array_v<F>, "Use FSTR or string_view instead of string literals");
	}
}

template<typename F, typename... args>
auto format(F&& Format, args&&... Args)
{
	detail::validate_format<F>();

	return fmt::format(FWD(Format), FWD(Args)...);
}

template<typename I, typename F, typename... args>
auto format_to(I&& Iterator, F&& Format, args&&... Args)
{
	detail::validate_format<F>();

	return fmt::format_to(FWD(Iterator), FWD(Format), FWD(Args)...);
}

template<typename F, typename... args>
auto format_to(string& Str, F&& Format, args&&... Args)
{
	detail::validate_format<F>();

	return fmt::format_to(std::back_inserter(Str), FWD(Format), FWD(Args)...);
}

#define FSTR(str) FMT_STRING(str)

template<typename T>
auto str(const T& Value)
{
	return fmt::to_wstring(Value);
}

inline auto str(const void* Value)
{
	return format(FSTR(L"0x{:0{}X}"sv), reinterpret_cast<uintptr_t>(Value), sizeof(Value) * 2);
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
		template<typename ParseContext>
		constexpr auto parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};

	template<typename object_type>
	struct format_no_spec
	{
		template <typename FormatContext>
		auto format(object_type const& Value, FormatContext& ctx)
		{
			return fmt::format_to(ctx.out(), FSTR(L"{}"sv), fmt::formatter<object_type, wchar_t>::to_string(Value));
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
	IS_DETECTED(is_formattable, formattable<T>{});
	IS_DETECTED(has_to_string, std::declval<T&>().to_string());
}

template<typename object_type>
struct fmt::formatter<object_type, wchar_t, std::enable_if_t<detail::is_formattable<object_type>>>: format_helpers::no_spec<object_type>
{
	static string to_string(object_type const& Value)
	{
		return formattable<object_type>::to_string(Value);
	}
};

template<typename object_type>
struct fmt::formatter<object_type, wchar_t, std::enable_if_t<detail::has_to_string<object_type>>>: format_helpers::no_spec<object_type>
{
	static string to_string(object_type const& Value)
	{
		return Value.to_string();
	}
};

// fmt 9 deprecated implicit enums formatting :(
template <typename object_type, typename char_type>
struct fmt::formatter<object_type, char_type, std::enable_if_t<std::is_enum_v<object_type>>>: fmt::formatter<std::underlying_type_t<object_type>, char_type>
{
	template <typename FormatContext>
	auto format(object_type const& Value, FormatContext& ctx) const
	{
		return formatter<std::underlying_type_t<object_type>, char_type>::format(std::to_underlying(Value), ctx);
	}
};

#endif // FORMAT_HPP_27C3F464_170B_432E_9D44_3884DDBB95AC
