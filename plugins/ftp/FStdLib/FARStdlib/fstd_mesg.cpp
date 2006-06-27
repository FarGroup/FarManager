#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

#define MSG_BUFFSIZE 10000

static char *messbuff = NULL;
static AbortProc ExitProc;

static void RTL_CALLBACK MSG_DelBuff( void )
  {
    if ( messbuff ) {
      delete[] messbuff;
      messbuff = NULL;
    }
    if ( ExitProc )
      ExitProc();
}

CONSTSTR DECLSPEC_PT Message( CONSTSTR patt,... )
  {  va_list  a;
     CONSTSTR m;

     va_start( a, patt );
       m = MessageV( patt,a );
     va_end( a );

 return m;
}

CONSTSTR DECLSPEC MessageV( CONSTSTR patt,va_list a )
  {
     if ( !messbuff ) {
       ExitProc = AtExit( MSG_DelBuff );
       messbuff = new char[MSG_BUFFSIZE];
     }

     VSNprintf( messbuff,MSG_BUFFSIZE,patt,a );

 return messbuff;
}
