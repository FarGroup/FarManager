/*
eol.cpp

Line endings
*/
/*
Copyright © 2017 Far Group
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

// Self:
#include "eol.hpp"

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

static const auto
	none_s = L""sv,
	win_s = L"\r\n"sv,       // DOS/Windows
	unix_s = L"\n"sv,        // Unix
	mac_s = L"\r"sv,         // Mac
	bad_win_s = L"\r\r\n"sv; // result of <CR><LF> text mode conversion

#define EOL(type) eol::type(eol_type::type)

eol const
	EOL(none),
	EOL(win),
	EOL(unix),
	EOL(mac),
	EOL(bad_win),
	eol::std = unix,
	eol::system = win;

#undef EOL

static eol::eol_type parse_type(string_view const Value)
{
#define EOL_FROM_STR(type) if (Value == type ## _s) return eol::eol_type::type

	EOL_FROM_STR(win);
	EOL_FROM_STR(unix);
	EOL_FROM_STR(mac);
	EOL_FROM_STR(bad_win);

	return eol::eol_type::none;

#undef EOL_FROM_STR
}

eol eol::parse(string_view const Value)
{

	return eol(parse_type(Value));
}

string_view eol::str() const
{
	switch (m_Type)
	{
#define EOL_TO_STR(type) case eol_type::type: return type ## _s

		EOL_TO_STR(win);
		EOL_TO_STR(unix);
		EOL_TO_STR(mac);
		EOL_TO_STR(bad_win);
		default: return none_s;

#undef EOL_TO_STR
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("eol")
{
	static const struct
	{
		eol const& Eol;
		string_view EolStr;
	}
	Tests[]
	{
#define EOL_STR(type) {eol::type,    type ## _s }
		EOL_STR(none),
		EOL_STR(win),
		EOL_STR(unix),
		EOL_STR(mac),
		EOL_STR(bad_win),
#undef EOL_STR
		{eol::std,    unix_s },
		{eol::system, win_s },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Eol.str() == i.EolStr);
		REQUIRE(eol::parse(i.EolStr) == i.Eol);
	}

	REQUIRE(eol::parse(L"banana"sv) == eol::none);
}
#endif
