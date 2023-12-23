#ifndef PLATFORM_SECURITY_HPP_08ED45C7_0FD0_43D5_8838_F9B6F8EFD31C
#define PLATFORM_SECURITY_HPP_08ED45C7_0FD0_43D5_8838_F9B6F8EFD31C
#pragma once

/*
platform.security.hpp

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

// Internal:

// Platform:
#include "platform.hpp"

// Common:
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::security
{
	namespace detail
	{
		struct sid_deleter
		{
			void operator()(PSID Sid) const noexcept;
		};
	}

	using sid_ptr = std::unique_ptr<std::remove_pointer_t<PSID>, detail::sid_deleter>;

	[[nodiscard]]
	sid_ptr make_sid(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, BYTE SubAuthorityCount, DWORD SubAuthority0 = 0, DWORD SubAuthority1 = 0, DWORD SubAuthority2 = 0, DWORD SubAuthority3 = 0, DWORD SubAuthority4 = 0, DWORD SubAuthority5 = 0, DWORD SubAuthority6 = 0, DWORD SubAuthority7 = 0);

	[[nodiscard]]
	bool is_admin();

	[[nodiscard]]
	handle open_current_process_token(DWORD DesiredAccess);

	class privilege
	{
	public:
		NONCOPYABLE(privilege);
		MOVE_CONSTRUCTIBLE(privilege);

		privilege(const std::initializer_list<const wchar_t* const>& Names): privilege(std::span(Names)) {}
		explicit privilege(std::span<const wchar_t* const> Names);
		~privilege();

		[[nodiscard]]
		static bool check(auto&&... Args) { return check({{ FWD(Args)... }}); }

		[[nodiscard]]
		static bool check(std::span<const wchar_t* const> Names);

	private:
		block_ptr<TOKEN_PRIVILEGES> m_SavedState;
		bool m_Changed{};
	};

	using descriptor = block_ptr<SECURITY_DESCRIPTOR, 512>;
}

#endif // PLATFORM_SECURITY_HPP_08ED45C7_0FD0_43D5_8838_F9B6F8EFD31C
