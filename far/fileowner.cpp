/*
fileowner.cpp

Кэш SID`ов и функция GetOwner

*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"

#if 0

/* $ 07.09.2000 SVS
   Функция GetFileOwner тоже доступна плагинам :-)
*/
int WINAPI GetFileOwner(const char *Computer,const char *Name,char *Owner)
{
	SECURITY_INFORMATION si;
	SECURITY_DESCRIPTOR *sd;
	char sddata[500];
	DWORD Needed;
	*Owner=0;
	si=OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;
	sd=(SECURITY_DESCRIPTOR *)sddata;
	char AnsiName[NM];
	FAR_OemToChar(Name,AnsiName);
	SetFileApisTo(APIS2ANSI);
	int GetCode=GetFileSecurity(AnsiName,si,sd,sizeof(sddata),&Needed);
	SetFileApisTo(APIS2OEM);

	/* $ 21.02.2001 VVM
	    ! Под НТ/2000 переменная Needed устанавливается независимо от результат. */
	if (!GetCode || (Needed>sizeof(sddata)))
		return(FALSE);

	/* VVM $ */
	PSID pOwner;
	BOOL OwnerDefaulted;

	if (!GetSecurityDescriptorOwner(sd,&pOwner,&OwnerDefaulted))
		return(FALSE);

	char AccountName[200],DomainName[200];
	DWORD AccountLength=sizeof(AccountName),DomainLength=sizeof(DomainName);
	SID_NAME_USE snu;

	if (!LookupAccountSid(Computer,pOwner,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
		return(FALSE);

	FAR_CharToOem(AccountName,Owner);
	return(TRUE);
}
/* SVS $*/

void SIDCacheFlush(void)
{
	return;
}


#else

// эта часть - перспективная фигня, которая значительно ускоряет получение овнеров

struct SIDCacheRecord
{
	PSID sid;
	char *username;
	SIDCacheRecord *next;
};

static struct SIDCacheRecord *sid_cache=NULL;

void SIDCacheFlush(void)
{
	SIDCacheRecord *tmp_rec;

	while (sid_cache)
	{
		tmp_rec=sid_cache;
		sid_cache=sid_cache->next;
		free(tmp_rec->sid);
		free(tmp_rec->username);
		free(tmp_rec);
	}
}

static const char *add_sid_cache(const char *computer,PSID sid)
{
	const char *res=NULL;
	SIDCacheRecord *new_rec=(SIDCacheRecord *)malloc(sizeof(SIDCacheRecord));

	if (new_rec)
	{
		memset(new_rec,0,sizeof(sizeof(SIDCacheRecord)));
		new_rec->sid=(PSID)malloc(GetLengthSid(sid));

		if (new_rec->sid)
		{
			CopySid(GetLengthSid(sid),new_rec->sid,sid);
			char AccountName[200],DomainName[200];
			DWORD AccountLength=sizeof(AccountName),DomainLength=sizeof(DomainName);
			SID_NAME_USE snu;

			if (LookupAccountSid(computer,new_rec->sid,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
			{
				if ((new_rec->username=(char *)malloc(AccountLength+DomainLength+16)) != NULL)
				{
					int Len=(int)strlen(strcpy(new_rec->username,DomainName));
					new_rec->username[Len+1]=0;
					new_rec->username[Len]='\\';
					strcat(new_rec->username,AccountName);
					FAR_CharToOem(new_rec->username,new_rec->username);
					res=new_rec->username;
					new_rec->next=sid_cache;
					sid_cache=new_rec;
				}
				else
				{
					free(new_rec->sid);
					free(new_rec);
				}
			}
			else
			{
				free(new_rec->sid);
				free(new_rec);
			}
		}
		else
			free(new_rec);
	}

	return res;
}

static const char *get_sid_cache(PSID sid)
{
	char *res=NULL;
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


/* $ 07.09.2000 SVS
   Функция GetFileOwner тоже доступна плагинам :-)
*/
int WINAPI GetFileOwner(const char *Computer,const char *Name,char *Owner)
{
	if (!Owner)
	{
		SIDCacheFlush();
		return(TRUE);
	}

	SECURITY_INFORMATION si;
	SECURITY_DESCRIPTOR *sd;
	char sddata[500];
	DWORD Needed;
	*Owner=0;
	si=OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;
	sd=(SECURITY_DESCRIPTOR *)sddata;
	char AnsiName[NM];
	FAR_OemToChar(Name,AnsiName);
	SetFileApisTo(APIS2ANSI);
	int GetCode=GetFileSecurity(AnsiName,si,sd,sizeof(sddata),&Needed);
	SetFileApisTo(APIS2OEM);

	/* $ 21.02.2001 VVM
	    ! Под НТ/2000 переменная Needed устанавливается независимо от результат. */
	if (!GetCode || (Needed>sizeof(sddata)))
		return(FALSE);

	/* VVM $ */
	PSID pOwner;
	BOOL OwnerDefaulted;

	if (!GetSecurityDescriptorOwner(sd,&pOwner,&OwnerDefaulted))
		return(FALSE);

#if 1
	Owner[0]=0;
	const char *SID=NULL;

	if (IsValidSid(pOwner))
	{
		SID=get_sid_cache(pOwner);

		if (!SID)
			SID=add_sid_cache(Computer,pOwner);
	}

	if (!SID)
		return(FALSE);

	strcpy(Owner,SID);
#else
char AccountName[200],DomainName[200];
DWORD AccountLength=sizeof(AccountName),DomainLength=sizeof(DomainName);
SID_NAME_USE snu;

if (!LookupAccountSid(Computer,pOwner,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
	return(FALSE);

FAR_CharToOem(AccountName,Owner);
#endif
	return(TRUE);
}
/* SVS $*/

#endif
