#include <all_far.h>
#pragma hdrstop

#include "Int.h"

#if 0
#undef Log
#undef PROC
#define Log(v)  INProc::Say v
#define PROC(v) INProc _inp v ;
#endif

//---------------------------------------------------------------------------------
EnumHost::EnumHost(char *HostsPath)
{
	Log(("EnumHost::EnumHost","%p %s",this,HostsPath));
	Assign(HostsPath);
}

EnumHost::~EnumHost()
{
	Log(("EnumHost::~EnumHost"));

	if(hEnum!=NULL)
		RegCloseKey(hEnum);
}

BOOL EnumHost::Assign(char *HostsPath)
{
	PROC(("EnumHost::Assign","%s",HostsPath))
	HostPos = 0;
	StrCpy(RootKey, FTPHost::MkHost(NULL,HostsPath), ARRAYSIZE(RootKey));
	hEnum = FP_OpenRegKey(RootKey);
	Log(("rc = %d",hEnum != NULL));
	return hEnum != NULL;
}

BOOL EnumHost::GetNextHost(FTPHost* p)
{
	PROC(("EnumHost::GetNextHost",NULL))
	char     SubKey[FAR_MAX_REG];
	DWORD    Size = sizeof(SubKey)-1;
	FILETIME lw;

	if(!hEnum)
		return FALSE;

	if(RegEnumKeyEx(hEnum,HostPos,SubKey,&Size,NULL,NULL,NULL,&lw) != ERROR_SUCCESS)
	{
		Log(("!enum keys"));
		return FALSE;
	}

	p->Init();
	strcpy(p->RegKey, p->MkHost(RootKey,SubKey));
	p->LastWrite = lw;
	HostPos++;
	Log(("SetKey %p to: %s", p, p->RegKey));
	return TRUE;
}
