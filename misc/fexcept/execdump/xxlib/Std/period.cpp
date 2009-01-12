#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define PER_ID MK_ID( 'P','e','R',0 )

LOCALSTRUCT( PRPeriod_t )
  DWORD     Id;
  TIME_TYPE b,e;
  DWORD     Period;
};

HANDLE MYRTLEXP PRPeriodCreate( DWORD ms )
  {  PPRPeriod_t p = new PRPeriod_t;
     p->Id = PER_ID;
     GET_TIME( p->b );

     p->e        = p->b;
     p->Period   = ms;

 return (HANDLE)p;
}

BOOL MYRTLEXP PRPeriodEnd( HANDLE _p )
  {  PPRPeriod_t p = (PPRPeriod_t)_p;
     BOOL      res;

     if (!p || p->Id != PER_ID )
       return FALSE;

     DWORD ld = PRPeriodTime( _p );

     res = ld >= p->Period;
     if (res)
       p->b = p->e;

  return res;
}

DWORD MYRTLEXP PRPeriodTime( HANDLE _p )
  {  PPRPeriod_t p = (PPRPeriod_t)_p;
     if (!p || p->Id != PER_ID ) return 0;

     GET_TIME( p->e );
     CMP_TIME_TYPE diff = CMP_TIME( p->e, p->b );

  return TIME_DIFF_MS( diff );
}

DWORD MYRTLEXP PRPeriodPeriod( HANDLE _p )
  {  PPRPeriod_t p = (PPRPeriod_t)_p;
     if (!p || p->Id != PER_ID ) return 0;
  return p->Period;
}

void MYRTLEXP PRPeriodDestroy( HANDLE _p )
  {  PPRPeriod_t p = (PPRPeriod_t)_p;
     if (p && p->Id == PER_ID ) {
       p->Id = 0;
       delete p;
     }
}
void MYRTLEXP PRPeriodReset( HANDLE _p )
  {  PPRPeriod_t p = (PPRPeriod_t)_p;
     if (p && p->Id == PER_ID)
       GET_TIME( p->b );
}
