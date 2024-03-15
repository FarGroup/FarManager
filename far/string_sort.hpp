#ifndef STRING_SORT_HPP_CAE94F71_4292_45B5_9D34_C40E43E8C2AF
#define STRING_SORT_HPP_CAE94F71_4292_45B5_9D34_C40E43E8C2AF
#pragma once

/*
string_sort.hpp

*/
/*
Copyright © 2018 Far Group
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

// External:

//----------------------------------------------------------------------------

class SQLiteDb;
class pluginapi_sort_accessor;

namespace string_sort
{
	// Default comparison entry point.
	// Exact behaviour is controlled by the user settings.
	[[nodiscard]]
	std::strong_ordering compare(string_view, string_view);

	void adjust_comparer(size_t Collation, bool CaseSensitive, bool DigitsAsNumbers);

	constexpr inline struct less_t
	{
		using is_transparent = void;

		[[nodiscard]]
		bool operator()(string_view Str1, string_view Str2) const
		{
			return std::is_lt(compare(Str1, Str2));
		}
	}
	less;

	constexpr inline struct less_icase_t
	{
		using is_transparent = void;

		[[nodiscard]]
		bool operator()(string_view Str1, string_view Str2) const;
	}
	less_icase;

	class keyhole
	{
		friend SQLiteDb;
		friend pluginapi_sort_accessor;
		static std::strong_ordering compare_ordinal_icase(string_view Str1, string_view Str2);
		static std::strong_ordering compare_ordinal_numeric(string_view Str1, string_view Str2);
	};

	namespace detail
	{
		template<typename ordering>
		concept sane_ordering =
			std::bit_cast<signed char>(ordering::less) == -1 &&
			std::bit_cast<signed char>(ordering::equal) == 0 &&
			std::bit_cast<signed char>(ordering::greater) == 1;

		constexpr auto ordering_as_int(sane_ordering auto const Value) noexcept
		{
			return std::bit_cast<signed char>(Value);
		}

		constexpr auto ordering_as_int(auto const Value) noexcept
		{
			return std::is_lt(Value)? -1 : !std::is_eq(Value);
		}
	}

	constexpr int ordering_as_int(std::strong_ordering const Value) noexcept
	{
		return detail::ordering_as_int(Value);
	}
}

#endif // STRING_SORT_HPP_CAE94F71_4292_45B5_9D34_C40E43E8C2AF
