#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

HKEY WINAPI FP_CreateRegKey(const char *Key)
{
	HKEY  hKey;
	DWORD Disposition;
	char  name[ FAR_MAX_REG ];
	CHK_INITED

	if(Key && *Key)
		sprintf(name,"%s" SLASH_STR "%s",FP_PluginRootKey,Key);
	else
		sprintf(name,"%s",FP_PluginRootKey);

	SetLastError(RegOpenKey(HKEY_CURRENT_USER,name,&hKey));

	if(GetLastError() == ERROR_SUCCESS)
		return hKey;

	hKey = NULL;
	SetLastError(RegCreateKeyEx(HKEY_CURRENT_USER,name,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition));
	return hKey;
}
