#ifndef NAMELIST_HPP_09E5F1A3_91E5_416F_B810_48B92D126FF5
#define NAMELIST_HPP_09E5F1A3_91E5_416F_B810_48B92D126FF5
#pragma once

/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

// External:

//----------------------------------------------------------------------------

class NamesList
{
public:
	NONCOPYABLE(NamesList);
	MOVABLE(NamesList);

	NamesList(): CurPos(Names.end()) {}

	void AddName(string_view Name);
	std::optional<string_view> GetNextName();
	std::optional<string_view> GetPrevName();
	bool SetCurName(string_view Name);

	bool empty() const { return Names.empty(); }
	auto size() const { return Names.size(); }

private:
	using names_list = std::vector<string>;
	names_list Names;
	names_list::const_iterator CurPos;
};

#endif // NAMELIST_HPP_09E5F1A3_91E5_416F_B810_48B92D126FF5
