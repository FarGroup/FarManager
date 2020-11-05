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

enum eol_type: char
{
	none = 0,
	win,      // <CR><LF>      \r\n
	unix,     // <LF>          \n
	mac,      // <CR>          \r
	bad_win,  // <CR><CR><LF>  \r\r\n
};

static const auto
	none_s = L""sv,
	win_s = L"\r\n"sv,       // DOS/Windows
	unix_s = L"\n"sv,        // Unix
	mac_s = L"\r"sv,         // Mac
	bad_win_s = L"\r\r\n"sv; // result of <CR><LF> text mode conversion

const eol eol::none(eol_type::none);
const eol eol::win(eol_type::win);
const eol eol::unix(eol_type::unix);
const eol eol::mac(eol_type::mac);
const eol eol::bad_win(eol_type::bad_win);
const eol eol::std = unix;
const eol eol::system = win;

eol::eol():
	m_Type(eol_type::none)
{
}

eol eol::parse(string_view const Value)
{
	return eol(
		Value == win_s?     eol_type::win :
		Value == unix_s?    eol_type::unix :
		Value == mac_s?     eol_type::mac :
		Value == bad_win_s? eol_type::bad_win :
		                    eol_type::none
	);
}

string_view eol::str() const
{
	switch (m_Type)
	{
	case eol_type::win:     return win_s;
	case eol_type::unix:    return unix_s;
	case eol_type::mac:     return mac_s;
	case eol_type::bad_win: return bad_win_s;
	default:                return none_s;
	}
}

bool eol::operator==(const eol& rhs) const
{
	return m_Type == rhs.m_Type;
}

bool eol::operator!=(const eol& rhs) const
{
	return !(*this == rhs);
}

eol::eol(char const Type):
	m_Type(Type)
{
}
