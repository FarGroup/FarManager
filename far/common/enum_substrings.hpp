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

#include <optional>
#include <string_view>

//----------------------------------------------------------------------------

// Enumerator for string1\0string2\0string3\0...stringN\0\0
// Stops on \0\0 or once Str + Size is reached.

template<class char_type>
[[nodiscard]]
auto enum_substrings(char_type const* const Str, std::optional<size_t> const Size = {})
{
	using value_type = std::basic_string_view<char_type>;
	return inline_enumerator<value_type>([Iterator = Str, Str, Size](const bool Reset, value_type& Value) mutable
	{
		if (Reset)
			Iterator = Str;
		else
		{
			if (Size && Iterator == Str + *Size)
				return false;

			++Iterator;
		}

		const auto NewIterator = Size?
			std::find(Iterator, Str + *Size, char_type{}) :
			Iterator + std::char_traits<char_type>::length(Iterator);

		if (NewIterator == Iterator)
			return false;

		Value = { Iterator, static_cast<size_t>(NewIterator - Iterator) };
		Iterator = NewIterator;
		return true;
	});
}

template<class string_type> requires (!std::is_pointer_v<string_type>) && (!std::is_rvalue_reference_v<string_type>)
[[nodiscard]]
auto enum_substrings(string_type&& Str)
{
	return enum_substrings(Str.data(), Str.size());
}

#endif // ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
