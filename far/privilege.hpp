#ifndef PRIVILEGE_HPP_08ED45C7_0FD0_43D5_8838_F9B6F8EFD31C
#define PRIVILEGE_HPP_08ED45C7_0FD0_43D5_8838_F9B6F8EFD31C
#pragma once

/*
privilege.hpp

Privileges
*/
/*
Copyright © 2010 Far Group
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

namespace os
{
	namespace security
	{
		class privilege
		{
		public:
			NONCOPYABLE(privilege);
			MOVABLE(privilege);

			privilege(const std::initializer_list<const wchar_t*>& Names): privilege(Names.begin(), Names.size()) {}
			privilege(const std::vector<const wchar_t*>& Names): privilege(Names.data(), Names.size()) {}
			privilege(const wchar_t* const* Names, size_t Size);
			~privilege();

			template<class... args>
			static bool check(args&&... Args) { return check({ std::forward<args>(Args)... }); }
			static bool check(const std::initializer_list<const wchar_t*>& Names) { return check(make_range(Names.begin(), Names.size())); }
			static bool check(const range<const wchar_t* const*>& Names);

		private:
			handle m_Token;
			block_ptr<TOKEN_PRIVILEGES> m_SavedState;
			bool m_Changed {};
		};

	}
}
#endif // PRIVILEGE_HPP_08ED45C7_0FD0_43D5_8838_F9B6F8EFD31C
