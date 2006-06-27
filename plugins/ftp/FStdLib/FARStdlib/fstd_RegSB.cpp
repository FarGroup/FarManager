#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL DECLSPEC FP_SetRegKey( CONSTSTR Key,CONSTSTR ValueName,LPCBYTE ValueData,DWORD ValueSize)
  {  HKEY hKey = FP_CreateRegKey(Key);
     BOOL rc   = hKey &&
                 RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize) == ERROR_SUCCESS;
     RegCloseKey(hKey);
 return rc;
}
