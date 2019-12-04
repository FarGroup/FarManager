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

namespace string_sort
{
	// Default comparison enrty point.
	// Exact behaviour is controlled by the user settings.
	[[nodiscard]]
	int compare(string_view, string_view);

	void adjust_comparer();

	[[nodiscard]]
	inline bool less(string_view Str1, string_view Str2)
	{
		return compare(Str1, Str2) < 0;
	}

	struct [[nodiscard]] less_t
	{
		[[nodiscard]]
		bool operator()(string_view Str1, string_view Str2) const
		{
			return less(Str1, Str2);
		}
	};

	class keyhole
	{
		friend SQLiteDb;
		static int compare_ordinal_numeric(string_view Str1, string_view Str2);
	};
}

#endif // STRING_SORT_HPP_CAE94F71_4292_45B5_9D34_C40E43E8C2AF
