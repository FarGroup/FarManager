#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_SetRegKey(LPCSTR Key,LPCSTR ValueName,LPCSTR ValueData)
{
	HKEY hKey=FP_CreateRegKey(Key);
	BOOL rc = hKey &&
	          RegSetValueEx(hKey,ValueName,0,REG_SZ,(const BYTE *)ValueData,strLen(ValueData)+1) == ERROR_SUCCESS;
	RegCloseKey(hKey);
	return rc;
}
