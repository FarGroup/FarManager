#ifndef FROM_STRING_HPP_B1AC0296_5353_4EFE_91BE_DD553796548A
#define FROM_STRING_HPP_B1AC0296_5353_4EFE_91BE_DD553796548A
#pragma once

/*
from_string.hpp
*/
/*
Copyright © 2019 Far Group
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

#include "exception.hpp"
#include "expected.hpp"
#include "function_traits.hpp"
#include "string_utils.hpp"

#include <system_error>
#include <utility>

//----------------------------------------------------------------------------

namespace detail
{
	template<typename result_type>
	std::errc from_string(std::wstring_view const Str, result_type& Value, size_t* const Pos, int const Base, auto const Converter)
	{
		if (Str.empty() || Str.front() == L' ' || Str.front() == L'+')
			return std::errc::invalid_argument;

		if constexpr(std::unsigned_integral<result_type>)
		{
			if (Str.front() == L'-')
				return std::errc::result_out_of_range;
		}

		const null_terminated Data(Str);
		const auto Ptr = Data.c_str();
		auto EndPtr = const_cast<wchar_t*>(Ptr);

		auto& Errno = errno; // Nonzero cost, pay it once
		const auto OldErrno = Errno;
		if (Errno)
			Errno = 0;

		const auto Result = Converter(Ptr, &EndPtr, Base);

		const auto NewErrno = Errno;
		if (!NewErrno && OldErrno)
			Errno = OldErrno;

		if (Ptr == EndPtr)
			return std::errc::invalid_argument;

		if (NewErrno == ERANGE)
			return std::errc::result_out_of_range;

		if (Pos != nullptr)
			*Pos = static_cast<size_t>(EndPtr - Ptr);

		Value = Result;

		return {};
	}

	template<std::integral T>
	auto get_converter()
	{
		if constexpr (sizeof(T) > sizeof(long))
		{
			if constexpr (std::is_signed_v<T>)
				return &std::wcstoll;
			else
				return &std::wcstoull;
		}
		else
		{
			if constexpr (std::is_signed_v<T>)
				return &std::wcstol;
			else
				return &std::wcstoul;
		}
	}

	template<std::integral I>
	auto from_string(std::wstring_view const Str, I& Value, size_t* const Pos, int const Base)
	{
		const auto Converter = get_converter<I>();

		typename function_traits<decltype(Converter)>::result_type LongValue;

		if (const auto Result = from_string(Str, LongValue, Pos, Base, Converter); Result != std::errc{})
			return Result;

		if (!std::in_range<I>(LongValue))
			return std::errc::result_out_of_range;

		Value = static_cast<I>(LongValue);
		return std::errc{};
	}

	inline auto from_string(std::wstring_view const Str, double& Value, size_t* const Pos, int const Base)
	{
		if (Base != 10)
			return std::errc::invalid_argument;

		return from_string(Str, Value, Pos, Base, [](wchar_t const* const StrPtr, wchar_t** const EndPtr, int) { return std::wcstod(StrPtr, EndPtr); });
	}
}

[[nodiscard]]
bool from_string(std::wstring_view const Str, auto& Value, size_t* const Pos = {}, int const Base = 10)
{
	return detail::from_string(Str, Value, Pos, Base) == std::errc{};
}

template<typename T>
[[nodiscard]]
T from_string(std::wstring_view const Str, size_t* const Pos = {}, int const Base = 10)
{
	using namespace std::string_view_literals;

	T Value;
	const auto Result = detail::from_string(Str, Value, Pos, Base);
	if (Result == std::errc{})
		return Value;

	switch (Result)
	{
	case std::errc::invalid_argument:
		throw_exception("invalid from_string argument"sv);

	case std::errc::result_out_of_range:
		throw_exception("from_string argument is out of range"sv);

	default:
		std::unreachable();
	}
}

template<typename T>
[[nodiscard]]
expected<T, std::errc> try_from_string(std::wstring_view const Str, size_t* const Pos = {}, int const Base = 10)
{
	using namespace std::string_view_literals;

	T Value;
	const auto Result = detail::from_string(Str, Value, Pos, Base);
	if (Result == std::errc{})
		return Value;

	return Result;
}

#endif // FROM_STRING_HPP_B1AC0296_5353_4EFE_91BE_DD553796548A
