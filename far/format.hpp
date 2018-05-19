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

WARNING_PUSH(3)

WARNING_DISABLE_MSC(4396) // https://msdn.microsoft.com/en-us/library/bb384968.aspx 'name': the inline specifier cannot be used when a friend declaration refers to a specialization of a function template
WARNING_DISABLE_MSC(4702) // https://msdn.microsoft.com/en-us/library/c26da40e.aspx unreachable code

WARNING_DISABLE_GCC("-Wctor-dtor-privacy")

#pragma push_macro("static_assert")
#undef static_assert

#include "thirdparty/fmt/format.h"
#include "thirdparty/fmt/ostream.h"

#pragma pop_macro("static_assert")

WARNING_POP()

namespace detail
{
	char get_incompatible_char(wchar_t);
	char get_incompatible_char(const wchar_t*);
	char get_incompatible_char(const string&);
	wchar_t get_incompatible_char(char);
	wchar_t get_incompatible_char(const char*);
	wchar_t get_incompatible_char(const std::string&);

	template<typename char_type>
	void check_char_compatibility(char_type) {}

	template<typename char_type, typename arg, typename... args>
	void check_char_compatibility(char_type, const arg&, const args&... Args)
	{
		static_assert((!std::is_convertible_v<arg, const char_type*>));
		static_assert((!std::is_convertible_v<arg, std::basic_string<char_type>>));
		static_assert((!std::is_convertible_v<arg, std::basic_string_view<char_type>>));

		check_char_compatibility(char_type{}, Args...);
	}
}

template<typename F, typename... args>
auto format(F&& Format, args&&... Args)
{
	detail::check_char_compatibility(decltype(detail::get_incompatible_char(Format)){}, Args...);
	return fmt::format(FWD(Format), FWD(Args)...);
}

template<typename T>
auto str(T&& Value)
{
	return fmt::to_wstring(FWD(Value));
}

template<typename T>
auto str(const T* Value)
{
	return format(L"0x{0:0{1}X}", reinterpret_cast<uintptr_t>(Value), sizeof(Value) * 2);
}

template<typename T>
auto str(T* Value)
{
	return str(static_cast<const T*>(Value));
}

string str(const char*) = delete;
string str(const wchar_t*) = delete;
string str(std::string) = delete;
string str(string) = delete;

#endif // FORMAT_HPP_27C3F464_170B_432E_9D44_3884DDBB95AC
