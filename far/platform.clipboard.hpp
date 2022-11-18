#ifndef PLATFORM_CLIPBOARD_HPP_49E6F44B_4F91_46FB_BDAA_CB8DF4D4764D
#define PLATFORM_CLIPBOARD_HPP_49E6F44B_4F91_46FB_BDAA_CB8DF4D4764D
#pragma once

/*
platform.clipboard.hpp

*/
/*
Copyright © 2022 Far Group
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

namespace os::clipboard
{
	void enable_ansi_to_unicode_conversion_workaround(bool Enable);

	bool open();
	bool close();
	bool clear();
	bool set_text(string_view Str);
	bool set_vtext(string_view Str);
	bool set_files(string_view NamesData, bool Move);
	bool get_text(string& Data);
	bool get_vtext(string& Data);

#ifdef ENABLE_TESTS
	namespace testing
	{
		class state;
		state* capture();
		void restore(state* State);
	}
#endif
}

#endif // PLATFORM_CLIPBOARD_HPP_49E6F44B_4F91_46FB_BDAA_CB8DF4D4764D
