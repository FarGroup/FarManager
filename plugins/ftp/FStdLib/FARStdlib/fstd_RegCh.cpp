#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_CheckRegKey(const char *Key)
{
	HKEY hKey = FP_OpenRegKey(Key);

	if(hKey!=NULL)
		RegCloseKey(hKey);

	return hKey != NULL;
}

BOOL WINAPI FP_CheckRegKeyFull(const char *Key)
{
	HKEY hKey;

	if(RegOpenKeyEx(HKEY_CURRENT_USER,Key,0,KEY_READ,&hKey) != ERROR_SUCCESS)
		return FALSE;
	else
	{
		RegCloseKey(hKey);
		return TRUE;
	}
}
