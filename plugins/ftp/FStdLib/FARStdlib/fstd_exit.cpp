#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static AbortProc CTAbortProc = NULL;

AbortProc DECLSPEC AtExit( AbortProc p )
  {  AbortProc old = CTAbortProc;
     CTAbortProc = p;
 return old;
}

void DECLSPEC CallAtExit( void )
  {
    if ( CTAbortProc ) {
      CTAbortProc();
      CTAbortProc = NULL;
    }
}
