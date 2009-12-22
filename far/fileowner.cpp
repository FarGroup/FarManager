/*
fileowner.cpp

Кэш SID`ов и функция GetOwner
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

// эта часть - перспективная фигня, которая значительно ускоряет получение овнеров

struct SIDCacheItem
{
	PSID SID;
	string strUserName;

	SIDCacheItem(const wchar_t *Computer,PSID InitSID)
	{
		SID=xf_malloc(GetLengthSid(InitSID));
		if(SID)
		{
			if(CopySid(GetLengthSid(InitSID),SID,InitSID))
			{
				DWORD AccountLength=0,DomainLength=0;
				SID_NAME_USE snu;
				LookupAccountSid(Computer,SID,NULL,&AccountLength,NULL,&DomainLength,&snu);
				if (AccountLength && DomainLength)
				{
					string strAccountName,strDomainName;
					LPWSTR AccountName=strAccountName.GetBuffer(AccountLength);
					LPWSTR DomainName=strDomainName.GetBuffer(DomainLength);
					if (AccountName && DomainName)
					{
						if(LookupAccountSid(Computer,SID,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
						{
							strUserName=string(DomainName).Append(L"\\").Append(AccountName);
						}
					}
				}
			}
		}

		if(strUserName.IsEmpty())
		{
			xf_free(SID);
			SID=NULL;
		}
	}

	~SIDCacheItem()
	{
		if(SID)
		{
			xf_free(SID);
			SID=NULL;
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

const wchar_t* AddSIDToCache(const wchar_t *Computer,PSID SID)
{
	LPCWSTR Result=NULL;
	SIDCacheItem* NewItem=new SIDCacheItem(Computer,SID);
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

const wchar_t* GetNameFromSIDCache(PSID sid)
{
	LPCWSTR Result=NULL;
	for(SIDCacheItem** i=SIDCache.First();i;i=SIDCache.Next(i))
	{
		if (EqualSid((*i)->SID,sid))
		{
			Result=(*i)->strUserName;
			break;
		}
	}
	return Result;
}


bool WINAPI GetFileOwner(const wchar_t *Computer,const wchar_t *Name, string &strOwner)
{
	bool Result=false;
	/*
	if(!Owner)
	{
		SIDCacheFlush();
		return(TRUE);
	}
	*/
	strOwner.Clear();
	SECURITY_INFORMATION si=OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;;
	DWORD LengthNeeded=0;
	string strName(NTPath(Name).Str);
	GetFileSecurity(strName,si,NULL,0,&LengthNeeded);

	if (LengthNeeded)
	{
		PSECURITY_DESCRIPTOR sd=reinterpret_cast<PSECURITY_DESCRIPTOR>(xf_malloc(LengthNeeded));

		if (sd)
		{
			if (GetFileSecurity(strName,si,sd,LengthNeeded,&LengthNeeded))
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
			xf_free(sd);
		}
	}
	return Result;
}
