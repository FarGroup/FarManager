#ifndef STRING_LITERAL_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
#define STRING_LITERAL_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
#pragma once

/*
string_literal.hpp
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

template<typename T>
using string_literal = range<const T*>;

constexpr auto operator "" _sl(const char* Data, size_t Size) noexcept { return string_literal<char>(Data, Data + Size); }
constexpr auto operator "" _sl(const wchar_t* Data, size_t Size) noexcept { return string_literal<wchar_t>(Data, Data + Size); }

template<typename T>
auto operator+(const std::basic_string<T>& Lhs, const string_literal<T>& Rhs)
{
	std::basic_string<T> Result;
	Result.reserve(Lhs.size() + Rhs.size());
	return Result.append(Lhs).append(Rhs.data(), Rhs.size());
}

template<typename T>
auto operator+(const string_literal<T>& Lhs, const std::basic_string<T>& Rhs)
{
	std::basic_string<T> Result;
	Result.reserve(Lhs.size() + Rhs.size());
	return Result.append(Lhs.data(), Lhs.size()).append(Rhs);
}

template<typename T>
auto operator+(const string_literal<T>& Lhs, const string_literal<T>& Rhs)
{
	std::basic_string<T> Result;
	Result.reserve(Lhs.size() + Rhs.size());
	return Result.append(Lhs.data(), Lhs.size()).append(Rhs.data(), Rhs.size());
}

#endif // STRING_LITERAL_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
