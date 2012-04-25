/*
fileowner.cpp

Кэш SID`ов и функция GetOwner
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "fileowner.hpp"
#include "pathmix.hpp"
#include "DList.hpp"
#include "privilege.hpp"
#include "elevation.hpp"

static char sddata[64*1024];

// эта часть - перспективная фигня, которая значительно ускоряет получение овнеров

struct SIDCacheItem
{
	PSID Sid;
	string strUserName;

	SIDCacheItem(const wchar_t *Computer,PSID InitSID)
	{
		Sid=xf_malloc(GetLengthSid(InitSID));
		if(Sid)
		{
			if(CopySid(GetLengthSid(InitSID),Sid,InitSID))
			{
				DWORD AccountLength=0,DomainLength=0;
				SID_NAME_USE snu;
				LookupAccountSid(Computer,Sid,nullptr,&AccountLength,nullptr,&DomainLength,&snu);
				if (AccountLength && DomainLength)
				{
					string strAccountName,strDomainName;
					LPWSTR AccountName=strAccountName.GetBuffer(AccountLength);
					LPWSTR DomainName=strDomainName.GetBuffer(DomainLength);
					if (AccountName && DomainName)
					{
						if(LookupAccountSid(Computer,Sid,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
						{
							strUserName=string(DomainName).Append(L"\\").Append(AccountName);
						}
					}
				}
				else
				{
					LPWSTR StrSid;
					if(ConvertSidToStringSid(Sid, &StrSid))
					{
						strUserName = StrSid;
						LocalFree(StrSid);
					}
				}
			}
		}

		if(strUserName.IsEmpty())
		{
			xf_free(Sid);
			Sid=nullptr;
		}
	}

	~SIDCacheItem()
	{
		if(Sid)
		{
			xf_free(Sid);
			Sid=nullptr;
		}
	}
};

DList<SIDCacheItem*>SIDCache;

void SIDCacheFlush()
{
	for(SIDCacheItem** i=SIDCache.First();i;i=SIDCache.Next(i))
	{
		delete *i;
	}
	SIDCache.Clear();
}

const wchar_t* AddSIDToCache(const wchar_t *Computer,PSID Sid)
{
	LPCWSTR Result=nullptr;
	SIDCacheItem* NewItem=new SIDCacheItem(Computer,Sid);
	if(NewItem->strUserName.IsEmpty())
	{
		delete NewItem;
	}
	else
	{
		Result=(*SIDCache.Push(&NewItem))->strUserName;
	}
	return Result;
}

const wchar_t* GetNameFromSIDCache(PSID Sid)
{
	LPCWSTR Result=nullptr;
	for(SIDCacheItem** i=SIDCache.First();i;i=SIDCache.Next(i))
	{
		if (EqualSid((*i)->Sid,Sid))
		{
			Result=(*i)->strUserName;
			break;
		}
	}
	return Result;
}


bool GetFileOwner(const wchar_t *Computer,const wchar_t *Name, string &strOwner)
{
	bool Result=false;
	/*
	if(!Owner)
	{
		SIDCacheFlush();
		return TRUE;
	}
	*/
	strOwner.Clear();
	SECURITY_INFORMATION si=OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;;
	DWORD LengthNeeded=0;
	NTPath strName(Name);
	PSECURITY_DESCRIPTOR sd=reinterpret_cast<PSECURITY_DESCRIPTOR>(sddata);

	if (GetFileSecurity(strName,si,sd,sizeof(sddata),&LengthNeeded) && LengthNeeded<=sizeof(sddata))
	{
		PSID pOwner;
		BOOL OwnerDefaulted;
		if (GetSecurityDescriptorOwner(sd,&pOwner,&OwnerDefaulted))
		{
			if (IsValidSid(pOwner))
			{
				const wchar_t *Owner=GetNameFromSIDCache(pOwner);
				if (!Owner)
				{
					Owner=AddSIDToCache(Computer,pOwner);
				}
				if (Owner)
				{
					strOwner=Owner;
					Result=true;
				}
			}
		}
	}
	return Result;
}

bool SetOwnerInternal(LPCWSTR Object, LPCWSTR Owner)
{
	bool Result = false;

	PSID Sid = nullptr;
	//в winapi от mingw.org неправильный тип параметра.
	if(!ConvertStringSidToSid((LPWSTR)Owner, &Sid))
	{
		SID_NAME_USE Use;
		DWORD cSid=0, ReferencedDomain=0;
		LookupAccountName(nullptr, Owner, nullptr, &cSid, nullptr, &ReferencedDomain, &Use);
		if(cSid)
		{
			Sid = LocalAlloc(LMEM_FIXED, cSid);
			if(Sid)
			{
				LPWSTR ReferencedDomainName = new WCHAR[ReferencedDomain];
				if(ReferencedDomainName)
				{
					if(LookupAccountName(nullptr, Owner, Sid, &cSid, ReferencedDomainName, &ReferencedDomain, &Use))
					{
					}
					delete[] ReferencedDomainName;
				}
			}
		}
	}
	if(Sid)
	{
		Privilege TakeOwnershipPrivilege(SE_TAKE_OWNERSHIP_NAME);
		Privilege RestorePrivilege(SE_RESTORE_NAME);
		DWORD dwResult = SetNamedSecurityInfo(const_cast<LPWSTR>(Object), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, Sid, nullptr, nullptr, nullptr);
		if(dwResult == ERROR_SUCCESS)
		{
			Result = true;
		}
		else
		{
			SetLastError(dwResult);
		}
	}
	if(Sid)
	{
		LocalFree(Sid);
	}
	return Result;
}


bool SetOwner(LPCWSTR Object, LPCWSTR Owner)
{
	NTPath strNtObject(Object);
	bool Result = SetOwnerInternal(strNtObject, Owner);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Elevation.fSetOwner(strNtObject, Owner);
	}
	return Result;
}
