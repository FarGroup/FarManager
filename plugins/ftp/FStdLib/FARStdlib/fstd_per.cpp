#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

STRUCT( PRPeriod )
  TIME_TYPE b,e;
  DWORD     Period;
  DWORD     LastDiff;
};

/** @brief Create time period.

    @param ms Period length.

    @return handle of new period or NULL on error.
*/
HANDLE DECLSPEC FP_PeriodCreate( DWORD ms )
  {  PPRPeriod p = new PRPeriod;
     GET_TIME( p->b );
     p->Period = ms;
 return (HANDLE)p;
}

void DECLSPEC FP_PeriodDestroy( HANDLE& _p )
  {
  delete ((PPRPeriod)_p);
  _p = NULL;
}

BOOL DECLSPEC FP_PeriodEnd( HANDLE _p )
  {  PPRPeriod p = (PPRPeriod)_p;
     BOOL      res;
     if (!p) return FALSE;
     GET_TIME( p->e );
     p->LastDiff = (DWORD)(CMP_TIME(p->e,p->b)*1000.);
     res = p->LastDiff >= p->Period;
     if (res) p->b = p->e;
  return res;
}

DWORD DECLSPEC FP_PeriodTime( HANDLE _p )
  {  PPRPeriod p = (PPRPeriod)_p;
     if (!p) return 0;
  return p->LastDiff;
}

void DECLSPEC FP_PeriodReset( HANDLE _p )
  {  PPRPeriod p = (PPRPeriod)_p;
     if (p) GET_TIME( p->b );
}
