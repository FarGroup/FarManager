#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

//-  QNX                                                                -
#if defined(__QNX__)

#define TIMER_MAGIC  MK_ID( 'T','i','M','r' )
#define TIMER_DUMMY   ((HANDLE)1)

STRUCTBASE( PRTimer, public HSafeObject )
  timer_t ID;
  pid_t   Proxy;
  sDWORD  Pause;

  PRTimer( void ) : HSafeObject(TIMER_MAGIC) {}

         void     Destroy( void ) { timer_delete( ID ); qnx_proxy_detach(Proxy); }
  static PPRTimer Cvt( HANDLE p ) { return (PPRTimer)HSafeObject::Convert(p,sizeof(PRTimer),TIMER_MAGIC); }
};

HANDLE MYRTLEXP PRTimerCreate( sDWORD ms )
  {  itimerspec wait_time;
     sigevent   timer_parm;
     PPRTimer   p;

    if ( ms <= 0 ) return TIMER_DUMMY;

    p = new PRTimer;
    p->Pause = ms;
    p->Proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
    timer_parm.sigev_signo = -p->Proxy;
    p->ID = timer_create( CLOCK_REALTIME, &timer_parm );
    wait_time.it_value.tv_sec     = (long)ms/1000;
    wait_time.it_value.tv_nsec    = 1000000L*(ms%1000);
    wait_time.it_interval.tv_sec  = (long)ms/1000;
    wait_time.it_interval.tv_nsec = 1000000L*(ms%1000);
    timer_settime( p->ID, 0, &wait_time, NULL );
 return (HANDLE)p;
}

void MYRTLEXP PRTimerDestroy( HANDLE _p )
  {  PPRTimer p;

     if (!_p || _p == TIMER_DUMMY) return;
     p = PRTimer::Cvt(_p);
     if (p) HSafeObject::Release(p);
}
BOOL MYRTLEXP PRTimerWait( HANDLE _p )
  {  PPRTimer p;

     if (!_p || _p == TIMER_DUMMY) return TRUE;

     p = PRTimer::Cvt(_p);
     pid_t    pid;
     if ( !p ) return TRUE;
     pid = Receive( p->Proxy,NULL,0 );
 return pid != -1;
}
#endif //QNX

//-  WIN                                                                -
#if defined(__HWIN32__)
HANDLE MYRTLEXP PRTimerCreate( sDWORD ms )     { return (HANDLE)ms;}
void   MYRTLEXP PRTimerDestroy( HANDLE /*p*/ ) {}
BOOL   MYRTLEXP PRTimerWait( HANDLE p )        { Sleep((DWORD)p); return TRUE; }
#endif

#if defined(__HWIN16__)
HANDLE MYRTLEXP PRTimerCreate( sDWORD ms )     { return (HANDLE)ms;}
void   MYRTLEXP PRTimerDestroy( HANDLE /*p*/ ) {}
BOOL   MYRTLEXP PRTimerWait( HANDLE p )
  {  DWORD dw = GetTickCount(),
           t;
     do
       t = GetTickCount();
     while( t-dw < (DWORD)p );

 return TRUE;
}
#endif
