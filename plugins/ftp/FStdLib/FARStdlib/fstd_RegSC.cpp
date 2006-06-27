#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL DECLSPEC FP_SetRegKey(CONSTSTR Key,CONSTSTR ValueName,CONSTSTR ValueData)
  {  HKEY hKey=FP_CreateRegKey(Key);
     BOOL rc = hKey &&
               RegSetValueEx( hKey,ValueName,0,REG_SZ,(const BYTE *)ValueData,strLen(ValueData)+1 ) == ERROR_SUCCESS;
     RegCloseKey(hKey);
 return rc;
}
