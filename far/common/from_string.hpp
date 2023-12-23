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

#include "string_utils.hpp"

#include <stdexcept>
#include <utility>

//----------------------------------------------------------------------------

namespace detail
{
	enum class result
	{
		ok,
		invalid_argument,
		out_of_range,
	};

	template<typename result_type>
	result from_string(std::wstring_view const Str, result_type& Value, size_t* const Pos, int const Base, auto const Converter)
	{
		if (Str.empty() || Str.front() == L' ' || Str.front() == L'+')
			return result::invalid_argument;

		if constexpr(std::unsigned_integral<result_type>)
		{
			if (Str.front() == L'-')
				return result::out_of_range;
		}

		auto& Errno = errno; // Nonzero cost, pay it once

		const null_terminated Data(Str);
		const auto Ptr = Data.c_str();
		auto EndPtr = const_cast<wchar_t*>(Ptr);

		Errno = 0;
		const auto Result = Converter(Ptr, &EndPtr, Base);

		if (Ptr == EndPtr)
			return result::invalid_argument;

		if (Errno == ERANGE)
			return result::out_of_range;

		if (Pos != nullptr)
			*Pos = static_cast<size_t>(EndPtr - Ptr);

		Value = Result;

		return result::ok;
	}

	inline auto from_string(std::wstring_view const Str, long& Value, size_t* const Pos, int const Base)
	{
		return from_string(Str, Value, Pos, Base, std::wcstol);
	}

	inline auto from_string(std::wstring_view const Str, unsigned long& Value, size_t* const Pos, int const Base)
	{
		return from_string(Str, Value, Pos, Base, std::wcstoul);
	}

	inline auto from_string(std::wstring_view const Str, long long& Value, size_t* const Pos, int const Base)
	{
		return from_string(Str, Value, Pos, Base, std::wcstoll);
	}

	inline auto from_string(std::wstring_view const Str, unsigned long long& Value, size_t* const Pos, int const Base)
	{
		return from_string(Str, Value, Pos, Base, std::wcstoull);
	}

	template<typename L, typename S>
	auto from_string_long(std::wstring_view const Str, S& Value, size_t* const Pos, int const Base)
	{
		L LongValue;
		if (const auto Result = from_string(Str, LongValue, Pos, Base); Result != result::ok)
			return Result;

		if (LongValue < std::numeric_limits<S>::min() || LongValue > std::numeric_limits<S>::max())
			return result::out_of_range;

		Value = static_cast<S>(LongValue);
		return result::ok;
	}

	inline auto from_string(std::wstring_view const Str, int& Value, size_t* const Pos, int const Base)
	{
		return from_string_long<long>(Str, Value, Pos, Base);
	}

	inline auto from_string(std::wstring_view const Str, unsigned int& Value, size_t* const Pos, int const Base)
	{
		return from_string_long<unsigned long>(Str, Value, Pos, Base);
	}

	inline auto from_string(std::wstring_view const Str, short& Value, size_t* const Pos, int const Base)
	{
		return from_string_long<long>(Str, Value, Pos, Base);
	}

	inline auto from_string(std::wstring_view const Str, unsigned short& Value, size_t* const Pos, int const Base)
	{
		return from_string_long<unsigned long>(Str, Value, Pos, Base);
	}

	inline auto from_string(std::wstring_view const Str, double& Value, size_t* const Pos, int const Base)
	{
		if (Base != 10)
			return result::invalid_argument;

		return from_string(Str, Value, Pos, Base, [](wchar_t const* const StrPtr, wchar_t** const EndPtr, int) { return std::wcstod(StrPtr, EndPtr); });
	}
}

[[nodiscard]]
bool from_string(std::wstring_view const Str, auto& Value, size_t* const Pos = {}, int const Base = 10)
{
	return detail::from_string(Str, Value, Pos, Base) == detail::result::ok;
}

template<typename T>
[[nodiscard]]
T from_string(std::wstring_view const Str, size_t* const Pos = {}, int const Base = 10)
{
	switch (T Value; detail::from_string(Str, Value, Pos, Base))
	{
	case detail::result::ok:
		return Value;

	case detail::result::invalid_argument:
		throw std::invalid_argument("invalid from_string argument");

	case detail::result::out_of_range:
		throw std::out_of_range("from_string argument out of range");

	default:
		std::unreachable();
	}
}

#endif // FROM_STRING_HPP_B1AC0296_5353_4EFE_91BE_DD553796548A
