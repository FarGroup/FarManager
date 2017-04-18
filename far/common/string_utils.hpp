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

namespace detail
{
	inline void append_one(string& Str, wchar_t Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, const wchar_t* Arg, size_t Size) { Str.append(Arg, Size); }
	inline void append_one(string& Str, const string& Arg, size_t) { Str += Arg; }

	inline void append_impl(string&, const size_t*) {}

	template<typename arg, typename... args>
	void append_impl(string& Str, const size_t* Sizes, arg&& Arg, args&&... Args)
	{
		append_one(Str, std::forward<arg>(Arg), *Sizes);
		append_impl(Str, Sizes + 1, std::forward<args>(Args)...);
	}

	inline size_t size_one(wchar_t) { return 1; }
	inline size_t size_one(const wchar_t* Str) { return wcslen(Str); }
	inline size_t size_one(const string& Str) { return Str.size(); }

	inline size_t size_impl(size_t* Sizes) { return 0; }

	template<typename arg, typename... args>
	size_t size_impl(size_t* Sizes, arg&& Arg, args&&... Args)
	{
		*Sizes = size_one(std::forward<arg>(Arg));
		return *Sizes + size_impl(Sizes + 1, std::forward<args>(Args)...);
	}
}

template<typename... args>
void append(string& Str, args&&... Args)
{
	size_t Sizes[sizeof...(Args)];
	Str.reserve(Str.size() + detail::size_impl(Sizes, std::forward<args>(Args)...));
	detail::append_impl(Str, Sizes, std::forward<args>(Args)...);
}

template<typename... args>
auto concat(args&&... Args)
{
	string Str;
	append(Str, std::forward<args>(Args)...);
	return Str;
}

inline auto cut_left(string Str, size_t MaxWidth)
{
	if (Str.size() > MaxWidth)
		Str.erase(0, Str.size() - MaxWidth);
	return Str;
}

inline auto cut_right(string Str, size_t MaxWidth)
{
	if (Str.size() > MaxWidth)
		Str.resize(MaxWidth);
	return Str;
}

inline auto pad_left(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	if (Str.size() < MinWidth)
		Str.insert(0, MinWidth - Str.size(), Padding);
	return Str;
}

inline auto pad_right(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	if (Str.size() < MinWidth)
		Str.append(MinWidth - Str.size(), Padding);
	return Str;
}

inline auto fit_to_left(string Str, size_t Size)
{
	return Str.size() < Size? pad_right(std::move(Str), Size) : cut_right(std::move(Str), Size);
}

inline auto fit_to_center(string Str, size_t Size)
{
	const auto StrSize = Str.size();
	return Str.size() < Size? pad_right(pad_left(std::move(Str), StrSize + (Size - StrSize) / 2), Size) : cut_right(std::move(Str), Size);
}

inline auto fit_to_right(string Str, size_t Size)
{
	return Str.size() < Size? pad_left(std::move(Str), Size) : cut_right(std::move(Str), Size);
}

#endif // STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
