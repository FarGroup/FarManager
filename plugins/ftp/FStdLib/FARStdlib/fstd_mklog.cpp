#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//DATA
CRITICAL_SECTION    PLOG_cs;

BOOL DECLSPEC LOGInit( void )
  {  static BOOL inited = FALSE;
  if (inited) return FALSE;
  InitializeCriticalSection(&PLOG_cs);
  inited=TRUE;
#if defined(__QNX__)
  setbuf( stdout,NULL );
#endif
 return TRUE;
}

CONSTSTR DECLSPEC FP_GetLogFullFileName( void )
  {  static char str[MAX_PATH_SIZE] = "";
     CONSTSTR  m;
     char     *tmp;

     if ( !str[0] ) {
       m = FP_GetPluginLogName();
       if (!m || !m[0])
         return "";

       str[ GetModuleFileName(FP_HModule,str,sizeof(str)) ] = 0;
       tmp = strrchr( str,SLASH_CHAR );
         if (tmp) {
           tmp[1] = 0;
           strcat( str,m );
         } else
           strcpy( str,m );
     }

 return str;
}
