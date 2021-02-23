﻿#ifndef PLATFORM_ENV_HPP_0266D13F_F208_49D6_B3FF_A87D0131D2D3
#define PLATFORM_ENV_HPP_0266D13F_F208_49D6_B3FF_A87D0131D2D3
#pragma once

/*
platform.env.hpp

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
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::env
{
	namespace provider
	{
		class strings
		{
		public:
			strings();
			~strings();

			[[nodiscard]]
			const wchar_t* data() const;

		private:
			wchar_t* m_Data{};
		};

		class block
		{
		public:
			block();
			~block();

			[[nodiscard]]
			const wchar_t* data() const;

		private:
			wchar_t* m_Data{};
		};
	}

	bool get(string_view Name, string& Value);

	[[nodiscard]]
	string get(string_view Name);

	bool set(string_view Name, string_view Value);

	bool del(string_view Name);

	[[nodiscard]]
	string expand(string_view Str);

	[[nodiscard]]
	string get_pathext();
}

#endif // PLATFORM_ENV_HPP_0266D13F_F208_49D6_B3FF_A87D0131D2D3
