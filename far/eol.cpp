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

#include "headers.hpp"
#pragma hdrstop

#include "eol.hpp"

static const auto
	none_s = L""_sv,
	win_s = L"\r\n"_sv,       // DOS/Windows
	unix_s = L"\n"_sv,        // Unix
	mac_s = L"\r"_sv,         // Mac
	bad_win_s = L"\r\r\n"_sv; // result of <CR><LF> text mode conversion

string_view eol::str(type const Value)
{
	switch (Value)
	{
	case type::win:     return win_s;
	case type::unix:    return unix_s;
	case type::mac:     return mac_s;
	case type::bad_win: return bad_win_s;
	default:            return none_s;
	}
}

eol::type eol::parse(string_view const Value)
{
	return
		Value == win_s?     type::win :
		Value == unix_s?    type::unix :
		Value == mac_s?     type::mac :
		Value == bad_win_s? type::bad_win :
		                    type::none;
}
