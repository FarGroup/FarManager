/*
platform.security.cpp

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

#include "platform.security.hpp"

#include "lasterror.hpp"

#include "platform.concurrency.hpp"
#include "platform.reg.hpp"

namespace
{
	static os::handle OpenCurrentProcessToken(DWORD DesiredAccess)
	{
		HANDLE Handle;
		return os::handle(OpenProcessToken(GetCurrentProcess(), DesiredAccess, &Handle)? Handle : nullptr);
	}

	static bool lookup_privilege_value(const wchar_t* Name, LUID& Value)
	{
		using value_type = std::pair<LUID, bool>;
		static std::unordered_map<string, value_type> s_Cache;
		static os::critical_section s_CS;

		SCOPED_ACTION(os::critical_section_lock)(s_CS);

		auto Result = s_Cache.try_emplace(Name);

		const auto& MapKey = Result.first->first;
		auto& MapValue = Result.first->second;

		if (Result.second)
		{
			MapValue.second = LookupPrivilegeValue(nullptr, MapKey.c_str(), &MapValue.first) != FALSE;
		}

		Value = MapValue.first;
		return MapValue.second;
	}

	static bool operator==(const LUID& a, const LUID& b)
	{
		return a.LowPart == b.LowPart && a.HighPart == b.HighPart;
	}
}

namespace os::security
{
	void detail::sid_deleter::operator()(PSID Sid) const noexcept
	{
		FreeSid(Sid);
	}

	sid_ptr make_sid(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, BYTE SubAuthorityCount, DWORD SubAuthority0, DWORD SubAuthority1, DWORD SubAuthority2, DWORD SubAuthority3, DWORD SubAuthority4, DWORD SubAuthority5, DWORD SubAuthority6, DWORD SubAuthority7)
	{
		PSID Sid;
		return sid_ptr(AllocateAndInitializeSid(IdentifierAuthority, SubAuthorityCount, SubAuthority0, SubAuthority1, SubAuthority2, SubAuthority3, SubAuthority4, SubAuthority5, SubAuthority6, SubAuthority7, &Sid)? Sid : nullptr);
	}

	bool is_admin()
	{
		static const auto Result = []
		{
			SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
			const auto AdministratorsGroup = make_sid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
			if (!AdministratorsGroup)
				return false;

			BOOL IsMember;
			return CheckTokenMembership(nullptr, AdministratorsGroup.get(), &IsMember) && IsMember;
		}();

		return Result;
	}

	privilege::privilege(range<const wchar_t* const*> const Names)
	{
		if (Names.empty())
			return;

		block_ptr<TOKEN_PRIVILEGES> NewState(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES) * (Names.size() - 1));
		NewState->PrivilegeCount = 0;

		for (const auto& i : Names)
		{
			LUID_AND_ATTRIBUTES laa = { {}, SE_PRIVILEGE_ENABLED };
			if (lookup_privilege_value(i, laa.Luid))
			{
				NewState->Privileges[NewState->PrivilegeCount++] = laa;
			}
			// TODO: log if failed
		}

		m_SavedState.reset(NewState.size());

		const auto Token = OpenCurrentProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
		if (!Token)
			// TODO: log
			return;

		DWORD ReturnLength;
		m_Changed = AdjustTokenPrivileges(Token.native_handle(), FALSE, NewState.get(), static_cast<DWORD>(m_SavedState.size()), m_SavedState.get(), &ReturnLength) && m_SavedState->PrivilegeCount;
		// TODO: log if failed
	}

	privilege::~privilege()
	{
		if (!m_Changed)
			return;

		const auto Token = OpenCurrentProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
		if (!Token)
			// TODO: log
			return;

		SCOPED_ACTION(GuardLastError);
		AdjustTokenPrivileges(Token.native_handle(), FALSE, m_SavedState.get(), 0, nullptr, nullptr);
		// TODO: log if failed
	}

	bool privilege::check(range<const wchar_t* const*> const Names)
	{
		const auto Token = OpenCurrentProcessToken(TOKEN_QUERY);
		if (!Token)
			return false;

		DWORD TokenInformationLength = 0;
		if (!GetTokenInformation(Token.native_handle(), TokenPrivileges, nullptr, 0, &TokenInformationLength) || TokenInformationLength)
			return false;

		block_ptr<TOKEN_PRIVILEGES> TokenInformation{ TokenInformationLength };
		if (!GetTokenInformation(Token.native_handle(), TokenPrivileges, TokenInformation.get(), TokenInformationLength, &TokenInformationLength))
			return false;

		const auto Privileges = make_range(TokenInformation->Privileges, TokenInformation->PrivilegeCount);

		for (const auto& Name: Names)
		{
			LUID Luid;
			if (!lookup_privilege_value(Name, Luid))
				return false;

			const auto ItemIterator = std::find_if(ALL_CONST_RANGE(Privileges), [&](const auto& Item) { return Item.Luid == Luid; });
			if (ItemIterator == Privileges.end() || !(ItemIterator->Attributes & (SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT)))
				return false;
		}

		return true;
	}

	fs::drives_set allowed_drives_mask()
	{
		// It's good enough to read it once.
		static const auto AllowedDrivesMask = []
		{
			for (const auto& i: { &os::reg::key::local_machine, &os::reg::key::current_user })
			{
				unsigned NoDrives;
				if (i->get(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"sv, L"NoDrives"sv, NoDrives))
					return ~NoDrives;
			}

			return ~0u;
		}();

		return AllowedDrivesMask;
	}
}
