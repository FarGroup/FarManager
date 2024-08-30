#ifndef CHAR_WIDTH_HPP_D66C86AC_3415_4FD1_89DA_0AB843FFEEB8
#define CHAR_WIDTH_HPP_D66C86AC_3415_4FD1_89DA_0AB843FFEEB8
#pragma once

/*
char_width.hpp

Fullwidth support
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

// External:

//----------------------------------------------------------------------------

namespace char_width
{
	using codepoint = char32_t;

	[[nodiscard]]
	size_t get(codepoint Codepoint);

	[[nodiscard]]
	bool is_wide(codepoint Codepoint);

	void enable(int Value);

	[[nodiscard]]
	bool is_enabled();

	void invalidate();

	[[nodiscard]]
	bool is_half_width_surrogate_broken();
}

#endif // CHAR_WIDTH_HPP_D66C86AC_3415_4FD1_89DA_0AB843FFEEB8
