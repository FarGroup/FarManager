#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

void DECLSPEC_PT __WinAbort( CONSTSTR msg,... )
  {  va_list a;
     char    pnm[MAX_PATH_SIZE],
             str[ 1000 ];
     int     l;

     if (!msg) exit(1);
//Message
     va_start( a,msg );
     VSNprintf( str,sizeof(str),msg,a );
     va_end( a );

//Plugin name
     strcpy( pnm,"Assertion in \"" );
     l = strlen(pnm);
     pnm[ l + GetModuleFileName(FP_HModule,pnm+l,sizeof(pnm)-l)] = 0;
     strcat( pnm,"\" !" );

     MessageBox( NULL,str,pnm,MB_OK|MB_ICONHAND);

     TerminateProcess( GetCurrentProcess(),0 );
}
