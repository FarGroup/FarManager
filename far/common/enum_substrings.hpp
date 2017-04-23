#ifndef ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
#define ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
#pragma once

/*
enum_substrings.hpp
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

#include "enumerator.hpp"
#include "iterator_range.hpp"

// Enumerator for string1\0string2\0string3\0...stringN\0\0

template<class char_type>
auto enum_substrings(char_type* Data)
{
	size_t Offset = 0;
	using value_type = range<char_type*>;
	return make_inline_enumerator<value_type>([Offset, Data](size_t Index, value_type& Value) mutable
	{
		if (!Index)
			Offset = 0;

		const auto Begin = Data + Offset;
		auto End = Begin;
		while (*End)
			++End;

		if (End == Begin)
			return false;

		Value = { Begin, End };
		Offset += Value.size() + 1;
		return true;
	});
}

void enum_tokens(const string&&, const wchar_t*) = delete;
inline auto enum_tokens(const string& Data, const wchar_t* Separators)
{
	size_t Offset = 0;
	using value_type = string_view;
	return make_inline_enumerator<value_type>([Offset, &Data, Separators](size_t Index, value_type& Value) mutable
	{
		if (!Index)
			Offset = 0;

		if (Offset >= Data.size())
			return false;

		auto Pos = Data.find_first_of(Separators, Offset);
		if (Pos == string::npos)
			Pos = Data.size();

		Value = { Data.data() + Offset, Data.data() + Pos };
		Offset = Pos + 1;
		return true;
	});
}

#endif // ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
