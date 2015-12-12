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

static os::handle OpenCurrentProcessToken(DWORD DesiredAccess)
{
	HANDLE Handle;
	return os::handle(OpenProcessToken(GetCurrentProcess(), DesiredAccess, &Handle)? Handle : nullptr);
}

privilege::privilege(const std::vector<const wchar_t*>& PrivilegeNames):
	m_Token(OpenCurrentProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY)),
	m_Changed(false)
{
	if (m_Token)
	{
		block_ptr<TOKEN_PRIVILEGES> NewState(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES) * (PrivilegeNames.size() - 1));
		NewState->PrivilegeCount = static_cast<DWORD>(PrivilegeNames.size());

		std::transform(ALL_CONST_RANGE(PrivilegeNames), std::begin(NewState->Privileges), [](CONST_REFERENCE(PrivilegeNames) i) -> LUID_AND_ATTRIBUTES
		{
			LUID_AND_ATTRIBUTES laa = { {}, SE_PRIVILEGE_ENABLED };
			LookupPrivilegeValue(nullptr, i, &laa.Luid);
			// TODO: log if failed
			return laa;
		});

		m_SavedState.reset(NewState.size());

		DWORD ReturnLength;
		// TODO: log if failed
		if (AdjustTokenPrivileges(m_Token.native_handle(), FALSE, NewState.get(), static_cast<DWORD>(NewState.size()), m_SavedState.get(), &ReturnLength) && GetLastError() == ERROR_SUCCESS)
		{
			m_Changed = true;
		}
	}
}

privilege::~privilege()
{
	if(m_Token)
	{
		SCOPED_ACTION(GuardLastError);
		if(m_Changed)
		{
			AdjustTokenPrivileges(m_Token.native_handle(), FALSE, m_SavedState.get(), static_cast<DWORD>(m_SavedState.size()), nullptr, nullptr);
		}
	}
}

bool privilege::is_set(const wchar_t* PrivilegeName)
{
	bool Result=false;
	TOKEN_PRIVILEGES State={1};
	if (LookupPrivilegeValue(nullptr,PrivilegeName,&State.Privileges[0].Luid))
	{
		if (const auto Token = OpenCurrentProcessToken(TOKEN_QUERY))
		{
			DWORD TokenInformationLength=0;
			GetTokenInformation(Token.native_handle(), TokenPrivileges, nullptr, 0, &TokenInformationLength);
			if (TokenInformationLength)
			{
				block_ptr<TOKEN_PRIVILEGES> TokenInformation(TokenInformationLength);
				if(TokenInformation)
				{
					if (GetTokenInformation(Token.native_handle(), TokenPrivileges, TokenInformation.get(), TokenInformationLength, &TokenInformationLength))
					{
						const auto PrivilegesEnd = TokenInformation->Privileges + TokenInformation->PrivilegeCount;
						const auto ItemIterator = std::find_if(TokenInformation->Privileges, PrivilegesEnd, [&State](CONST_REFERENCE(TokenInformation->Privileges) i)
						{
							return i.Luid.LowPart == State.Privileges[0].Luid.LowPart && i.Luid.HighPart==State.Privileges[0].Luid.HighPart;
						});
						if (ItemIterator != PrivilegesEnd)
							Result = (ItemIterator->Attributes & (SE_PRIVILEGE_ENABLED|SE_PRIVILEGE_ENABLED_BY_DEFAULT)) != 0;
					}
				}
			}
		}
	}
	return Result;
}

}}
