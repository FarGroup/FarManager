#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/HThread.h>

// ---------------------------------------------------------------------------------
// HWIN
// ---------------------------------------------------------------------------------
#if defined(__HWIN32__)

LOCALSTRUCT( ThreadParams )
  ThreadProc_t cb;
  LPVOID       Param;
};

static DWORD WINAPI idThreadFunc( PThreadParams p )
  {
     p->cb( p->Param );
     delete p;

 return 0;
}

HThread::HThread( void )
  {
     Handle = NULL;
     Id     = 0;
}

HThread::~HThread()
  {
    Close( TRUE );
}

void HThread::Close( BOOL force /*= FALSE*/ )
  {
     if ( Handle ) {
       if ( force && isRun() ) TerminateThread( Handle, 0 );
       CloseHandle( Handle );
       Handle = NULL;
     }
}

BOOL HThread::Create( ThreadProc_t cb, LPVOID ptr, BOOL Susp /*=FALSE*/ )
  {  PThreadParams p = new ThreadParams;
     p->cb    = cb;
     p->Param = ptr;

     Handle = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)idThreadFunc, p, Susp ? CREATE_SUSPENDED : 0, &Id );

     if ( Handle && Handle != INVALID_HANDLE_VALUE )
       return TRUE;

     delete p;
     Handle = NULL;
 return FALSE;
}

BOOL HThread::isRun( void )
  {
 return Handle && WaitForSingleObject( Handle,0 ) == WAIT_TIMEOUT;
}

void HThread::Wait( DWORD ms /*= INFINITE*/ )
  {
    if ( Handle ) WaitForSingleObject( Handle,ms );
}
#endif
// ---------------------------------------------------------------------------------
// HDOS
// ---------------------------------------------------------------------------------
#if defined(__HDOS__)
#endif
