#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_SetRegKey(LPCSTR Key,LPCSTR ValueName,const BYTE * ValueData,DWORD ValueSize)
{
	HKEY hKey = FP_CreateRegKey(Key);
	BOOL rc   = hKey &&
	            RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize) == ERROR_SUCCESS;
	RegCloseKey(hKey);
	return rc;
}
