#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define PRTHREAD_MAGIC  MK_ID('T','h','R','d')

//-  WIN32                                                               -
#if defined( __HWIN__ )
  STRUCTBASE( PRThread,public HSafeObject )
    HANDLE Handle;
    DWORD  ID;
    PRThread( void ) : HSafeObject(PRTHREAD_MAGIC) { Handle = NULL; }
    static PPRThread Cvt( HANDLE p ) { return (PPRThread)HSafeObject::Convert(p,sizeof(PRThread),PRTHREAD_MAGIC); }
  };

HANDLE MYRTLEXP PRThreadCreate( PRThreadProc proc,BOOL Suspended,LPVOID param )
  {  PPRThread p = new PRThread;
     if (!p) return NULL;
     p->Handle = CreateThread( NULL,0,proc,param,(Suspended)?CREATE_SUSPENDED:0,&p->ID );
     if ( !p->Handle ) { HSafeObject::Release(p); return NULL; }
 return p;
}
void MYRTLEXP PRThreadTerminate( HANDLE  _p )
  {  PPRThread p = PRThread::Cvt( _p );
     HANDLE    th;
     DWORD     id;
     if ( !p ) return;
     th = p->Handle;
     id = p->ID;
     HSafeObject::Release(p);
     if ( GetCurrentThreadId() != id ) {
       TerminateThread(th,0);
       CloseHandle(th);
     } else {
       ExitThread(0);
     }
}
BOOL MYRTLEXP PRThreadSuspend( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
     if ( !p ) return FALSE;
 return SuspendThread( p->Handle ) != MAX_DWORD;
}
BOOL MYRTLEXP PRThreadResume( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
     if ( !p ) return FALSE;
 return ResumeThread( p->Handle ) != MAX_DWORD;
}
void MYRTLEXP PRThreadExit( DWORD val )
  {
    ExitThread( val );
}
BOOL MYRTLEXP PRThreadValid( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
     if ( !p ) return FALSE;
     if ( WaitForSingleObject(p->Handle,0) != WAIT_TIMEOUT ) {
       PRThreadTerminate( p );
       return FALSE;
     } else
       return TRUE;
}
DWORD MYRTLEXP PRThreadID( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
 return (!p)?0:p->ID;
}
DWORD MYRTLEXP PRThreadCurrentID( void )
  {
 return GetCurrentThreadId();
}
#endif

//-  QNX                                                                -
#if defined(__QNX__)
  #define THREAD_STACK      80000

  STRUCTBASE( PRThread,public HSafeObject )
    __QNX_THREADPara Par;
    int              Handle;
    char            *Stack;
    PRThread( void ) : HSafeObject(PRTHREAD_MAGIC) { Handle = NULL; }
    static PPRThread Cvt( HANDLE p ) { return (PPRThread)HSafeObject::Convert(p,sizeof(PRThread),PRTHREAD_MAGIC); }
  };

HANDLE MYRTLEXP PRThreadCreate( PRThreadProc proc,BOOL Suspended,LPVOID param )
  {  PPRThread p    = new PRThread;

     p->Par.Suspended = Suspended;
     p->Par.Param     = param;
     p->Par.Started   = FALSE;

     p->Stack  = (char*)_Alloc( THREAD_STACK+1 );
     if ( !p->Stack ) { HSafeObject::Release(p); return NULL; }
     p->Handle = tfork( p->Stack,THREAD_STACK,proc,&p->Par,0 );

     if ( p->Handle == -1 ) {
       _Del( p->Stack );
       HSafeObject::Release(p);
       return NULL;
     } else {
       while( !p->Par.Started );
       return (HANDLE)p;
     }
}
void MYRTLEXP PRThreadTerminate( HANDLE  _p )
  {  PPRThread p = PRThread::Cvt( _p );
     int       th;
     if ( !p ) return;

     th = p->Handle;
     _Del( p->Stack );
     HSafeObject::Release(p);
     if ( getpid() == th )
       exit(0);
     else
     if ( getprio(th) != -1 ) {
       kill( th,THREAD_KILL );
       waitpid( th,NULL,0 );
     }
}
BOOL MYRTLEXP PRThreadSuspend( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
     if ( !p ) return FALSE;
 return kill( p->Handle,THREAD_SUSPEND ) == 0;
}
BOOL MYRTLEXP PRThreadResume( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
     if ( !p ) return FALSE;
 return kill( p->Handle,THREAD_RESUME ) == 0;
}
void MYRTLEXP PRThreadExit( DWORD val )
  {
    exit( (int)val );
}
BOOL MYRTLEXP PRThreadValid( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
     if ( !p ) return FALSE;
     if ( waitpid( p->Handle,NULL,WNOHANG ) != 0 ) {
       return FALSE;
     } else
       return TRUE;
}
DWORD MYRTLEXP PRThreadID( HANDLE _p )
  {  PPRThread p = PRThread::Cvt( _p );
 return (DWORD)((!p)?0:p->Handle);
}
DWORD MYRTLEXP PRThreadCurrentID( void )
  {
 return (DWORD)getpid();
}
#endif
