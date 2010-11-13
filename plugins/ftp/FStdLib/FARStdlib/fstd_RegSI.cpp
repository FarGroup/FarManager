#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_SetRegKey(LPCSTR Key,LPCSTR ValueName,DWORD ValueData)
{
	HKEY hKey = FP_CreateRegKey(Key);
	BOOL rc = hKey &&
	          RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData)) == ERROR_SUCCESS;
	RegCloseKey(hKey);
	return rc;
}
