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

//----------------------------------------------------------------------------

namespace detail
{
	namespace handler
	{
		[[noreturn]]
		inline void invalid_argument()
		{
			throw std::invalid_argument("invalid from_string argument");
		}

		[[noreturn]]
		inline void out_of_range()
		{
			throw std::out_of_range("from_string argument out of range");
		}
	}

	template<typename result_type, typename converter_type>
	void from_string(std::wstring_view const Str, result_type& Value, size_t* Pos, int Base, converter_type Converter)
	{
		if (Str.empty() || Str.front() == L' ' || Str.front() == L'+')
			handler::invalid_argument();

		if constexpr(std::is_unsigned_v<result_type>)
		{
			if (Str.front() == L'-')
				handler::out_of_range();
		}

		auto& Errno = errno; // Nonzero cost, pay it once

		const null_terminated Data(Str);
		const auto Ptr = Data.c_str();
		auto EndPtr = const_cast<wchar_t*>(Ptr);

		Errno = 0;
		const auto Result = Converter(Ptr, &EndPtr, Base);

		if (Ptr == EndPtr)
			handler::invalid_argument();

		if (Errno == ERANGE)
			handler::out_of_range();

		if (Pos != nullptr)
			*Pos = static_cast<size_t>(EndPtr - Ptr);

		Value = Result;
	}

	inline void from_string(std::wstring_view const Str, long& Value, size_t* Pos, int Base)
	{
		from_string(Str, Value, Pos, Base, std::wcstol);
	}

	inline void from_string(std::wstring_view const Str, unsigned long& Value, size_t* Pos, int Base)
	{
		from_string(Str, Value, Pos, Base, std::wcstoul);
	}

	inline void from_string(std::wstring_view const Str, long long& Value, size_t* Pos, int Base)
	{
		from_string(Str, Value, Pos, Base, std::wcstoll);
	}

	inline void from_string(std::wstring_view const Str, unsigned long long& Value, size_t* Pos, int Base)
	{
		from_string(Str, Value, Pos, Base, std::wcstoull);
	}

	inline void from_string(std::wstring_view const Str, int& Value, size_t* Pos, int Base)
	{
		static_assert(sizeof(int) == sizeof(long));
		long LongValue;
		from_string(Str, LongValue, Pos, Base);
		Value = static_cast<int>(LongValue);
	}

	inline void from_string(std::wstring_view const Str, unsigned int& Value, size_t* Pos, int Base)
	{
		static_assert(sizeof(unsigned int) == sizeof(unsigned long));
		unsigned long LongValue;
		from_string(Str, LongValue, Pos, Base);
		Value = static_cast<unsigned int>(LongValue);
	}

	inline void from_string(std::wstring_view const Str, short& Value, size_t* Pos, int Base)
	{
		long LongValue;
		from_string(Str, LongValue, Pos, Base);
		if (LongValue < std::numeric_limits<short>::min() || LongValue > std::numeric_limits<short>::max())
			handler::out_of_range();

		Value = static_cast<short>(LongValue);
	}

	inline void from_string(std::wstring_view const Str, unsigned short& Value, size_t* Pos, int Base)
	{
		unsigned long LongValue;
		from_string(Str, LongValue, Pos, Base);
		if (LongValue > std::numeric_limits<unsigned short>::max())
			handler::out_of_range();

		Value = static_cast<unsigned short>(LongValue);
	}

	inline void from_string(std::wstring_view const Str, double& Value, size_t* Pos, int)
	{
		from_string(Str, Value, Pos, {}, [](const wchar_t* const StrPtr, wchar_t** const EndPtr, int) { return std::wcstod(StrPtr, EndPtr); });
	}
}

template<typename T>
[[nodiscard]]
bool from_string(std::wstring_view const Str, T& Value, size_t* Pos = nullptr, int Base = 10)
{
	try
	{
		detail::from_string(Str, Value, Pos, Base);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

template<typename T>
[[nodiscard]]
T from_string(std::wstring_view const Str, size_t* Pos = nullptr, int Base = 10)
{
	T Value;
	detail::from_string(Str, Value, Pos, Base);
	return Value;
}

#endif // FROM_STRING_HPP_B1AC0296_5353_4EFE_91BE_DD553796548A
