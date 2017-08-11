#ifndef STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
#define STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
#pragma once

/*
string_utils.hpp
*/
/*
Copyright © 2016 Far Group
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

#include "preprocessor.hpp"

namespace detail
{
	inline void append_one(string& Str, wchar_t Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, const wchar_t* Arg, size_t Size) { Str.append(Arg, Size); }
	inline void append_one(string& Str, const string& Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, const string_view& Arg, size_t) { Str.append(Arg.data(), Arg.size()); }

	inline void append_impl(string&, const size_t*) {}

	template<typename arg, typename... args>
	void append_impl(string& Str, const size_t* Sizes, arg&& Arg, args&&... Args)
	{
		append_one(Str, FWD(Arg), *Sizes);
		append_impl(Str, Sizes + 1, FWD(Args)...);
	}

	template<typename T, REQUIRES(std::is_same<T, wchar_t>::value)>
	size_t size_one(T) { return 1; }

	template<typename T, REQUIRES(std::is_same<T, const wchar_t*>::value)>
	size_t size_one(T Str) { return wcslen(Str); }

	template<typename T, REQUIRES(std::is_same<T, string>::value)>
	size_t size_one(const T& Str) { return Str.size(); }

	template<size_t N>
	size_t size_one(const wchar_t(&Str)[N]) { return Str[N - 1]? N : N - 1; }

	inline size_t size_one(const string_view& Str) { return Str.size(); }

	inline size_t size_impl(size_t*) { return 0; }

	template<typename arg, typename... args>
	size_t size_impl(size_t* Sizes, arg&& Arg, args&&... Args)
	{
		*Sizes = size_one(FWD(Arg));
		return *Sizes + size_impl(Sizes + 1, FWD(Args)...);
	}
}

template<typename... args>
void append(string& Str, args&&... Args)
{
	size_t Sizes[sizeof...(Args)];
	Str.reserve(Str.size() + detail::size_impl(Sizes, FWD(Args)...));
	detail::append_impl(Str, Sizes, FWD(Args)...);
}

template<typename... args>
auto concat(args&&... Args)
{
	string Str;
	append(Str, FWD(Args)...);
	return Str;
}

namespace inplace
{
	inline auto& cut_left(string& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.erase(0, Str.size() - MaxWidth);
		return Str;
	}

	inline auto& cut_right(string& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.resize(MaxWidth);
		return Str;
	}

	inline auto& pad_left(string& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.insert(0, MinWidth - Str.size(), Padding);
		return Str;
	}

	inline auto& pad_right(string& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.append(MinWidth - Str.size(), Padding);
		return Str;
	}

	inline auto& fit_to_left(string& Str, size_t Size)
	{
		return Str.size() < Size? pad_right(Str, Size) : cut_right(Str, Size);
	}

	inline auto& fit_to_center(string& Str, size_t Size)
	{
		const auto StrSize = Str.size();
		return Str.size() < Size? pad_right(pad_left(Str, StrSize + (Size - StrSize) / 2), Size) : cut_right(Str, Size);
	}

	inline auto& fit_to_right(string& Str, size_t Size)
	{
		return Str.size() < Size? pad_left(Str, Size) : cut_right(Str, Size);
	}

	inline auto& unquote(string& Str)
	{
		Str.erase(std::remove(ALL_RANGE(Str), L'"'), Str.end());
		return Str;
	}

	inline auto& quote(string& Str)
	{
		if (Str.empty() || Str.front() != L'"')
			Str.insert(0, 1, L'"');

		if (Str.size() == 1 || Str.back() != L'"')
			Str.push_back(L'"');

		return Str;
	}

	inline auto& quote_unconditional(string& Str)
	{
		Str.insert(0, 1, L'"');
		Str.push_back(L'"');
		return Str;
	}

	inline auto& quote_normalise(string& Str)
	{
		return quote(unquote(Str));
	}
}

inline auto cut_left(string Str, size_t MaxWidth)
{
	return inplace::cut_left(Str, MaxWidth);
}

inline auto cut_right(string Str, size_t MaxWidth)
{
	return inplace::cut_right(Str, MaxWidth);
}

inline auto pad_left(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	return inplace::pad_left(Str, MinWidth, Padding);
}

inline auto pad_right(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	return inplace::pad_right(Str, MinWidth, Padding);
}

inline auto fit_to_left(string Str, size_t Size)
{
	return inplace::fit_to_left(Str, Size);
}

inline auto fit_to_center(string Str, size_t Size)
{
	return inplace::fit_to_center(Str, Size);
}

inline auto fit_to_right(string Str, size_t Size)
{
	return inplace::fit_to_right(Str, Size);
}

inline auto unquote(string Str)
{
	return inplace::unquote(Str);
}

inline auto quote(string Str)
{
	return inplace::quote(Str);
}

inline auto quote_unconditional(string Str)
{
	return inplace::quote_unconditional(Str);
}

inline auto quote_normalise(string Str)
{
	return inplace::quote_normalise(Str);
}

inline bool equal(const string_view& Str1, const string_view& Str2)
{
	return Str1 == Str2;
}

inline bool starts_with(const string_view& Str, const string_view& Prefix)
{
	return Str.size() >= Prefix.size() && equal({ Str.data(), Prefix.size() }, Prefix);
}

inline bool ends_with(const string_view& Str, const string_view& Suffix)
{
	return Str.size() >= Suffix.size() && equal({ Str.data() + Str.size() - Suffix.size(), Suffix.size() }, Suffix);
}

inline bool contains(const string_view& Str, const string_view& Token)
{
	return std::search(ALL_CONST_RANGE(Str), ALL_CONST_RANGE(Token)) != Str.cend();
}


#endif // STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
