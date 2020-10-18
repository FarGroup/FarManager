#ifndef FNPARCE_HPP_4E73DE55_DA35_4962_86C4_EC0DBDE2E229
#define FNPARCE_HPP_4E73DE55_DA35_4962_86C4_EC0DBDE2E229
#pragma once

/*
fnparce.hpp

Парсер файловых ассоциаций
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

// External:

//----------------------------------------------------------------------------

class delayed_deleter;

class subst_context
{
public:
	subst_context(string_view NameStr, string_view ShortNameStr);

	string_view Name;
	string_view ShortName;
	string_view Path;

	std::unordered_map<string, string> mutable Variables;
};

bool SubstFileName(
	string &Str,
	const subst_context& Context,
	delayed_deleter* ListNames = nullptr,
	bool* PreserveLongName = nullptr,
	bool IgnoreInput = false,
	string_view DlgTitle = {},
	bool EscapeAmpersands = false
);

#endif // FNPARCE_HPP_4E73DE55_DA35_4962_86C4_EC0DBDE2E229
