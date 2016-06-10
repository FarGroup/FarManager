/*
privilege.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "privilege.hpp"
#include "lasterror.hpp"

namespace os { namespace security {

static handle OpenCurrentProcessToken(DWORD DesiredAccess)
{
	HANDLE Handle;
	return handle(OpenProcessToken(GetCurrentProcess(), DesiredAccess, &Handle)? Handle : nullptr);
}

privilege::privilege(const wchar_t* const* Names, size_t Size)
{
	if (!Size)
		return;

	m_Token = OpenCurrentProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);

	// TODO: log if failed
	if (!m_Token)
		return;

	block_ptr<TOKEN_PRIVILEGES> NewState(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES) * (Size - 1));
	NewState->PrivilegeCount = static_cast<DWORD>(Size);

	std::transform(Names, Names + Size, std::begin(NewState->Privileges), [](const auto& i)
	{
		LUID_AND_ATTRIBUTES laa = { {}, SE_PRIVILEGE_ENABLED };
		LookupPrivilegeValue(nullptr, i, &laa.Luid);
		// TODO: log if failed
		return laa;
	});

	m_SavedState.reset(NewState.size());

	DWORD ReturnLength;
	// TODO: log if failed
	m_Changed = AdjustTokenPrivileges(m_Token.native_handle(), FALSE, NewState.get(), static_cast<DWORD>(NewState.size()), m_SavedState.get(), &ReturnLength) && m_SavedState->PrivilegeCount;
}

privilege::~privilege()
{
	if (!m_Token || !m_Changed)
		return;

	SCOPED_ACTION(GuardLastError);
	AdjustTokenPrivileges(m_Token.native_handle(), FALSE, m_SavedState.get(), static_cast<DWORD>(m_SavedState.size()), nullptr, nullptr);
}

bool privilege::check(const wchar_t* const* Names, size_t Size)
{
	const auto Token{ OpenCurrentProcessToken(TOKEN_QUERY) };
	if (!Token)
		return false;

	DWORD TokenInformationLength{};
	if (!GetTokenInformation(Token.native_handle(), TokenPrivileges, nullptr, 0, &TokenInformationLength) || !TokenInformationLength)
		return false;

	block_ptr<TOKEN_PRIVILEGES> TokenInformation{ TokenInformationLength };
	if (!GetTokenInformation(Token.native_handle(), TokenPrivileges, TokenInformation.get(), TokenInformationLength, &TokenInformationLength))
		return false;

	const auto PrivilegesEnd{ TokenInformation->Privileges + TokenInformation->PrivilegeCount };

	TOKEN_PRIVILEGES State{ static_cast<DWORD>(Size) };

	for (size_t i = 0; i != Size; ++i)
	{
		auto& Luid = State.Privileges[i].Luid;

		if (!LookupPrivilegeValue(nullptr, Names[i], &Luid))
			return false;

		const auto ItemIterator = std::find_if(TokenInformation->Privileges, PrivilegesEnd, [&Luid](const auto& Item)
		{
			return Item.Luid.LowPart == Luid.LowPart && Item.Luid.HighPart == Luid.HighPart;
		});

		if (ItemIterator == PrivilegesEnd || !(ItemIterator->Attributes & (SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT)))
			return false;
	}

	return true;
}

}}
