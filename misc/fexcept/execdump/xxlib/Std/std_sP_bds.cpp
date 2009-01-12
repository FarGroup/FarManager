#include <all_lib.h>
#pragma hdrstop
#pragma package(smart_init)

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
    if ( reason == DLL_PROCESS_ATTACH ) {
      char nm[ MAX_PATH_SIZE ];
      nm[ GetModuleFileName(NULL,nm,sizeof(nm)-1) ] = 0;
      FILELog( "STDS(%08X) Loaded by: [%s]",GetHInstance(),nm );
    }
 return TRUE;
}

