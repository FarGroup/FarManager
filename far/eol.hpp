#ifndef EOL_HPP_F17D2D67_EAFD_480A_A1DA_92894204FB5A
#define EOL_HPP_F17D2D67_EAFD_480A_A1DA_92894204FB5A
#pragma once

/*
eol.hpp

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

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class eol
{
public:
	eol();

	static const eol
		none,
		win,      // <CR><LF>      \r\n
		unix,     // <LF>          \n
		mac,      // <CR>          \r
		bad_win,  // <CR><CR><LF>  \r\r\n
		std,      // unix
		system;   // win

	static eol parse(string_view Value);
	string_view str() const;
	bool operator==(const eol& rhs) const;
	bool operator!=(const eol& rhs) const;

private:
	explicit eol(char Type);

	char m_Type;
};

#endif // EOL_HPP_F17D2D67_EAFD_480A_A1DA_92894204FB5A
