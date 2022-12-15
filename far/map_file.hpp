#ifndef MAP_FILE_HPP_29032DD0_A55A_4E8B_97AC_C991B24BBBFE
#define MAP_FILE_HPP_29032DD0_A55A_4E8B_97AC_C991B24BBBFE
#pragma once

/*
map_file.hpp
*/
/*
Copyright © 2021 Far Group
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
#include "common/preprocessor.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

class map_file
{
public:
	NONCOPYABLE(map_file);

	explicit map_file(string_view ModuleName);
	~map_file();

	struct info
	{
		string_view File;
		string_view Symbol;
		size_t Displacement;

		bool operator==(const info&) const = default;
	};

	info get(uintptr_t Address);

	struct line;

private:
	void read(std::istream& Stream);

	std::map<uintptr_t, line> m_Symbols;
	unordered_string_set m_Files;
};

#endif // MAP_FILE_HPP_29032DD0_A55A_4E8B_97AC_C991B24BBBFE
