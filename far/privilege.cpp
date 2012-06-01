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

Privilege::Privilege(LPCWSTR PrivilegeName):hToken(INVALID_HANDLE_VALUE),Changed(false)
{
	if (PrivilegeName && OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
	{
		TOKEN_PRIVILEGES NewState={1};
		if (LookupPrivilegeValue(nullptr,PrivilegeName,&NewState.Privileges->Luid))
		{
			NewState.Privileges->Attributes=SE_PRIVILEGE_ENABLED;
			DWORD ReturnLength=sizeof(SavedState);
			if (AdjustTokenPrivileges(hToken,FALSE,&NewState,sizeof(NewState),&SavedState,&ReturnLength) && GetLastError()==ERROR_SUCCESS)
			{
				Changed=true;
			}
		}
	}
}

Privilege::~Privilege()
{
	if(hToken!=INVALID_HANDLE_VALUE)
	{
		GuardLastError LE;
		if(Changed)
		{
			AdjustTokenPrivileges(hToken,FALSE,&SavedState,sizeof(SavedState),nullptr,nullptr);
		}
		CloseHandle(hToken);
	}
}

bool CheckPrivilege(LPCWSTR PrivilegeName)
{
	bool Result=false;
	TOKEN_PRIVILEGES State={1};
	if (LookupPrivilegeValue(nullptr,PrivilegeName,&State.Privileges->Luid))
	{
		HANDLE hToken=INVALID_HANDLE_VALUE;
		if (OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
		{
			DWORD TokenInformationLength=0;
			GetTokenInformation(hToken,TokenPrivileges,nullptr,0,&TokenInformationLength);
			if (TokenInformationLength)
			{
				PTOKEN_PRIVILEGES TokenInformation=static_cast<PTOKEN_PRIVILEGES>(xf_malloc(TokenInformationLength));
				if(TokenInformation)
				{
					if(GetTokenInformation(hToken,TokenPrivileges,TokenInformation,TokenInformationLength,&TokenInformationLength))
					{
						for(DWORD i=0;i<TokenInformation->PrivilegeCount;i++)
						{
							if (TokenInformation->Privileges[i].Luid.LowPart==State.Privileges->Luid.LowPart && TokenInformation->Privileges[i].Luid.HighPart==State.Privileges->Luid.HighPart)
							{
								Result = (TokenInformation->Privileges[i].Attributes&(SE_PRIVILEGE_ENABLED|SE_PRIVILEGE_ENABLED_BY_DEFAULT)) != 0;
								break;
							}
						}
					}
				}
			}
		}
	}
	return Result;
}
