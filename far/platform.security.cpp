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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.security.hpp"

// Internal:
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.concurrency.hpp"

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	const auto& lookup_privilege_value(const wchar_t* Name)
	{
		static unordered_string_map<std::optional<LUID>> s_Cache;
		static os::critical_section s_CS;

		SCOPED_ACTION(std::scoped_lock)(s_CS);

		const auto [Iterator, IsEmplaced] = s_Cache.try_emplace(Name);

		auto& [MapKey, MapValue] = *Iterator;

		if (IsEmplaced)
		{
			LUID Luid;
			if (LookupPrivilegeValue(nullptr, MapKey.c_str(), &Luid))
				MapValue = Luid;
			else
				LOGWARNING(L"LookupPrivilegeValue({}): {}"sv, MapKey, os::last_error());
		}

		return MapValue;
	}
}

static bool operator==(const LUID& a, const LUID& b)
{
	return a.LowPart == b.LowPart && a.HighPart == b.HighPart;
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

	handle open_current_process_token(DWORD const DesiredAccess)
	{
		HANDLE Handle;
		if (!OpenProcessToken(GetCurrentProcess(), DesiredAccess, &Handle))
			return {};

		return handle(Handle);
	}

	privilege::privilege(std::span<const wchar_t* const> const Names)
	{
		if (Names.empty())
			return;

		const block_ptr<TOKEN_PRIVILEGES> NewState(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES) * (Names.size() - 1));
		NewState->PrivilegeCount = 0;

		for (const auto& i: Names)
		{
			const auto& Luid = lookup_privilege_value(i);
			if (!Luid)
				continue;

			NewState->Privileges[NewState->PrivilegeCount++] = { *Luid, SE_PRIVILEGE_ENABLED };
		}

		const auto Token = open_current_process_token(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
		if (!Token)
		{
			LOGWARNING(L"open_current_process_token: {}"sv, last_error());
			return;
		}

		DWORD ReturnLength;
		m_SavedState.reset(NewState.size());
		m_Changed = AdjustTokenPrivileges(Token.native_handle(), FALSE, NewState.data(), static_cast<DWORD>(m_SavedState.size()), m_SavedState.data(), &ReturnLength) && m_SavedState->PrivilegeCount;

		if (m_SavedState->PrivilegeCount != NewState->PrivilegeCount)
			LOGWARNING(L"AdjustTokenPrivileges(): {}"sv, last_error());
	}

	privilege::~privilege()
	{
		if (!m_Changed)
			return;

		const auto Token = open_current_process_token(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
		if (!Token)
		{
			LOGWARNING(L"open_current_process_token: {}"sv, last_error());
			return;
		}

		SCOPED_ACTION(os::last_error_guard);

		if (!AdjustTokenPrivileges(Token.native_handle(), FALSE, m_SavedState.data(), 0, nullptr, nullptr))
			LOGWARNING(L"AdjustTokenPrivileges(): {}"sv, last_error());
	}

	static auto get_token_privileges(HANDLE TokenHandle)
	{
		block_ptr<TOKEN_PRIVILEGES> Result(1024);

		if (!os::detail::ApiDynamicReceiver(Result.bytes(),
			[&](std::span<std::byte> const Buffer)
			{
				DWORD LengthNeeded = 0;
				if (!GetTokenInformation(TokenHandle, TokenPrivileges, static_cast<TOKEN_PRIVILEGES*>(static_cast<void*>(Buffer.data())), static_cast<DWORD>(Buffer.size()), &LengthNeeded))
					return static_cast<size_t>(LengthNeeded);
				return Buffer.size();
			},
			[](size_t ReturnedSize, size_t AllocatedSize)
			{
				return ReturnedSize > AllocatedSize;
			},
			[](std::span<std::byte const>)
			{}
		))
		{
			Result.reset();
		}

		return Result;
	}

	bool privilege::check(std::span<const wchar_t* const> const Names)
	{
		const auto Token = open_current_process_token(TOKEN_QUERY);
		if (!Token)
		{
			LOGWARNING(L"open_current_process_token: {}"sv, last_error());
			return false;
		}

		const auto TokenPrivileges = get_token_privileges(Token.native_handle());
		if (!TokenPrivileges)
		{
			LOGWARNING(L"get_token_privileges: {}"sv, last_error());
			return false;
		}

		const std::span Privileges(TokenPrivileges->Privileges, TokenPrivileges->PrivilegeCount);

		return std::ranges::all_of(Names, [&](const wchar_t* const Name)
		{
			const auto& Luid = lookup_privilege_value(Name);
			if (!Luid)
				return false;

			const auto ItemIterator = std::ranges::find(Privileges, *Luid, &LUID_AND_ATTRIBUTES::Luid);
			return ItemIterator != Privileges.end() && ItemIterator->Attributes & (SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT);
		});
	}
}
