#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BYTE *DECLSPEC FP_GetRegKey( const char *Key, const char *ValueName, BYTE *ValueData, LPCBYTE Default, DWORD DataSize)
  {  HKEY  hKey;
     DWORD Type,sz = DataSize;
     int   ExitCode;

     Assert( ValueData );

     hKey = FP_OpenRegKey(Key);

     if ( hKey ) {
       ExitCode = RegQueryValueEx(hKey,ValueName,0,&Type,ValueData,&sz);
       RegCloseKey(hKey);
     }

     if ( !hKey ||
          (ExitCode != ERROR_SUCCESS && ExitCode != ERROR_MORE_DATA) ) {
       if (Default)
         memmove( ValueData, Default, DataSize );
        else
         memset( ValueData, 0, DataSize );
     }

  return ValueData;
}

char *DECLSPEC FP_GetRegKey( const char *Key, const char *ValueName, char *ValueData, CONSTSTR Default, DWORD DataSize)
  {  HKEY  hKey;
     DWORD Type,sz = DataSize;
     int   ExitCode;

     Assert( ValueData );

     hKey = FP_OpenRegKey(Key);

     if ( hKey ) {
       ExitCode = RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)ValueData,&sz);
       RegCloseKey(hKey);
     }

     if ( !hKey ||
          (ExitCode != ERROR_SUCCESS && ExitCode != ERROR_MORE_DATA) ) {
       if (Default)
         StrCpy( ValueData, Default, DataSize );
        else
         memset( ValueData, 0, DataSize );
     }

  return ValueData;
}
