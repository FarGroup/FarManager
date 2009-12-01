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

// эта часть - перспективная фигня, которая значительно ускоряет получение овнеров

struct SIDCacheRecord
{
	PSID sid;
	wchar_t *username;
	SIDCacheRecord *next;
};

static SIDCacheRecord *sid_cache=NULL;

void SIDCacheFlush()
{
	SIDCacheRecord *tmp_rec;

	while (sid_cache)
	{
		tmp_rec=sid_cache;
		sid_cache=sid_cache->next;
		xf_free(tmp_rec->sid);
		xf_free(tmp_rec->username);
		xf_free(tmp_rec);
	}
}

static const wchar_t *add_sid_cache(const wchar_t *computer,PSID sid)
{
	const wchar_t *res=NULL;
	SIDCacheRecord *new_rec=(SIDCacheRecord *)xf_malloc(sizeof(SIDCacheRecord));

	if (new_rec)
	{
		memset(new_rec,0,sizeof(SIDCacheRecord));
		new_rec->sid=(PSID)xf_malloc(GetLengthSid(sid));

		if (new_rec->sid)
		{
			CopySid(GetLengthSid(sid),new_rec->sid,sid);
			DWORD AccountLength=0,DomainLength=0;
			SID_NAME_USE snu;
			LookupAccountSid(computer,new_rec->sid,NULL,&AccountLength,NULL,&DomainLength,&snu);

			if (AccountLength && DomainLength)
			{
				wchar_t* AccountName=(wchar_t*)xf_malloc(AccountLength*sizeof(wchar_t));
				wchar_t* DomainName=(wchar_t*)xf_malloc(DomainLength*sizeof(wchar_t));

				if (AccountName && DomainName)
				{
					if (LookupAccountSid(computer,new_rec->sid,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
					{
						if ((new_rec->username=(wchar_t*)xf_malloc((AccountLength+DomainLength+16)*sizeof(wchar_t))) != NULL)
						{
							size_t Len=StrLength(wcscpy(new_rec->username,DomainName));
							new_rec->username[Len+1]=0;
							new_rec->username[Len]=L'\\';
							wcscat(new_rec->username,AccountName);
							res=new_rec->username;
							new_rec->next=sid_cache;
							sid_cache=new_rec;
						}
						else
						{
							xf_free(new_rec->sid);
							xf_free(new_rec);
						}
					}
					else
					{
						xf_free(new_rec->sid);
						xf_free(new_rec);
					}
				}

				if (AccountName) xf_free(AccountName);

				if (DomainName) xf_free(DomainName);
			}
			else
			{
				xf_free(new_rec->sid);
				xf_free(new_rec);
			}
		}
		else
			xf_free(new_rec);
	}

	return res;
}

static const wchar_t *get_sid_cache(PSID sid)
{
	wchar_t *res=NULL;
	SIDCacheRecord *tmp_rec=sid_cache;

	while (tmp_rec)
	{
		if (EqualSid(tmp_rec->sid,sid))
		{
			res=tmp_rec->username;
			break;
		}

		tmp_rec=tmp_rec->next;
	}

	return res;
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
					const wchar_t *SID=NULL;

					if (IsValidSid(pOwner))
					{
						SID=get_sid_cache(pOwner);

						if (!SID)
						{
							SID=add_sid_cache(Computer,pOwner);
						}
					}

					if (SID)
					{
						strOwner=SID;
						Result=true;
					}
				}
			}

			xf_free(sd);
		}
	}

	return Result;
}
