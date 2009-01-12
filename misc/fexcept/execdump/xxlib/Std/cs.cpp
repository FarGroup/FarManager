#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if 0
  #define Log(v) FILELog v
#else
  #define Log(v)
#endif

//! CriticalSection                                     .
//-  QNX                                                .
#if defined(__QNX__)
BOOL MYRTLEXP InitializeCriticalSection( LPCRITICAL_SECTION cs )
  {
  return sem_init( cs,1,1 ) == 0;
}
BOOL MYRTLEXP DeleteCriticalSection( LPCRITICAL_SECTION cs )
  {
 return sem_destroy(cs) == 0;
}
void MYRTLEXP EnterCriticalSection( LPCRITICAL_SECTION cs )
  {
     sem_wait(cs);
}
BOOL MYRTLEXP LeaveCriticalSection( LPCRITICAL_SECTION cs )
  {
 return sem_post(cs) == 0;
}
BOOL MYRTLEXP TryEnterCriticalSection( LPCRITICAL_SECTION cs )
  {
 return sem_trywait(cs) == 0;
}
#endif

//-  SINGLE-TASK enviropment                            .
#if defined(__TEC32__) || defined(__REALDOS__) || defined(__PROTDOS__)
BOOL MYRTLEXP InitializeCriticalSection( LPCRITICAL_SECTION cs ) { *cs = FALSE; return TRUE; }
BOOL MYRTLEXP DeleteCriticalSection( LPCRITICAL_SECTION cs )     { *cs = FALSE; return TRUE; }
void MYRTLEXP EnterCriticalSection( LPCRITICAL_SECTION cs )      { while( *cs ) FreeSlice(); *cs = TRUE; }
BOOL MYRTLEXP LeaveCriticalSection( LPCRITICAL_SECTION cs )      { *cs = FALSE; return TRUE; }
BOOL MYRTLEXP TryEnterCriticalSection( LPCRITICAL_SECTION cs )   { return *cs == FALSE; }
#endif

//GNUC
#if defined(__GNUC__)
BOOL MYRTLEXP InitializeCriticalSection( LPCRITICAL_SECTION cs ) { *cs = FALSE; return TRUE; }
BOOL MYRTLEXP DeleteCriticalSection( LPCRITICAL_SECTION cs )     { *cs = FALSE; return TRUE; }
void MYRTLEXP EnterCriticalSection( LPCRITICAL_SECTION cs )      { while( *cs ) FreeSlice(); *cs = TRUE; }
BOOL MYRTLEXP LeaveCriticalSection( LPCRITICAL_SECTION cs )      { *cs = FALSE; return TRUE; }
BOOL MYRTLEXP TryEnterCriticalSection( LPCRITICAL_SECTION cs )   { return *cs == FALSE; }
#endif

//---------------------------------------------------------------------------
#if defined(__HWIN__)
//! RW access                                       .
DEF_SAFEOBJECT( HAccess,MK_ID( 'A','c','c',0 ) )
  CRITICAL_SECTION m_cs;              // Permits exclusive access to other members
  HANDLE           m_hsemReaders;     // Readers wait on this if a writer has access
  HANDLE           m_hsemWriters;     // Writers wait on this if a reader has access
  int              m_nWaitingReaders; // Number of readers waiting for access
  int              m_nWaitingWriters; // Number of writers waiting for access
  int              m_nActive;         // Number of threads currently with access
                                      //   (0=no threads, >0=# of readers, -1=1 writer)
  virtual void Destroy( void );
};

void HAccess::Destroy( void )
  {
     if (m_nActive) {
      Log(( "ACC::still active %p[%d,%d,%d]\n",this,m_nWaitingReaders,m_nWaitingWriters,m_nActive ));
      return;
     }
     m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
     DeleteCriticalSection(&m_cs);
     CloseHandle(m_hsemReaders);
     CloseHandle(m_hsemWriters);
}

HANDLE MYRTLEXP PRAccessCreate( void )
  {  PHAccess p = new HAccess;

// Initially no readers want access, no writers want access, and
// no threads are accessing the resource
     p->m_nWaitingReaders =
     p->m_nWaitingWriters =
     p->m_nActive         = 0;
     p->m_hsemReaders     = CreateSemaphore(NULL, 0, MAXLONG, NULL);
     p->m_hsemWriters     = CreateSemaphore(NULL, 0, MAXLONG, NULL);
     InitializeCriticalSection(&p->m_cs);
 return (HANDLE)p;
}

void MYRTLEXP PRAccessDestroy( HANDLE acc )
  {  PHAccess p = HAccess::Cvt(acc);
     if (p) HSafeObject::Release(p);
}

BOOL MYRTLEXP PRAccessEnter( HANDLE acc,AccTypes op )
  {  PHAccess p = HAccess::Cvt(acc);
     BOOL     res;
     if (!p || !op) return FALSE;

   EnterCriticalSection(&p->m_cs);
     if ( IS_FLAG(op,ACC_WRITE) ) {
       res = p->m_nActive != 0;
       if (res)
         p->m_nWaitingWriters++;
        else
         p->m_nActive = -1;
     } else
     if ( IS_FLAG(op,ACC_READ) ) {
       res = p->m_nWaitingWriters || (p->m_nActive < 0);
       if ( res )
         p->m_nWaitingReaders++;
        else
         p->m_nActive++;
     } else
       res = FALSE;
   LeaveCriticalSection(&p->m_cs);

     if ( IS_FLAG(op,ACC_WRITE) )
       return (!res) ? TRUE : (WaitForSingleObject(p->m_hsemWriters, INFINITE) == WAIT_OBJECT_0);
      else
     if ( IS_FLAG(op,ACC_READ) )
       return (!res) ? TRUE : (WaitForSingleObject(p->m_hsemReaders, INFINITE) == WAIT_OBJECT_0);
      else
       return res;
}

void MYRTLEXP PRAccessLeave( HANDLE acc )
  {  PHAccess p = HAccess::Cvt(acc);
     HANDLE   hsem = NULL;
     LONG     lCount = 0;

     if (!p) return;

   EnterCriticalSection(&p->m_cs);
     if (p->m_nActive > 0) p->m_nActive--; else p->m_nActive++;
     if (p->m_nActive == 0) {
      // No thread has access, who should wake up?
      // NOTE: It is possible that readers could never get access
      //       if there are always writers wanting to write
        if (p->m_nWaitingWriters > 0) {  // Writers are waiting and they take priority over readers
           p->m_nActive = -1;                // A writer will get access
           hsem         = p->m_hsemWriters;  // Writers wait on this semaphore
           lCount       = 1;                 // Assume only 1 waiter wakes; always TRUE for writers
           p->m_nWaitingWriters--;           // One less writer will be waiting
           // NOTE: The semaphore will release only 1 writer thread
        } else
        if (p->m_nWaitingReaders > 0) {  // Readers are waiting and no writers are waiting
           p->m_nActive         = p->m_nWaitingReaders; // All readers will get access
           p->m_nWaitingReaders = 0;                    // No readers will be waiting
           hsem                 = p->m_hsemReaders;     // Readers wait on this semaphore
           lCount               = p->m_nActive;         // Semaphore releases all readers
        } else {
           // There are no threads waiting at all; no semaphore gets released
           hsem = NULL;
        }
      }
   LeaveCriticalSection(&p->m_cs);

   if (hsem) ReleaseSemaphore(hsem, lCount, NULL);
}
//---------------------------------------------------------------------------
//  PRcs
//---------------------------------------------------------------------------
PRcs::PRcs( void )          { InitializeCriticalSection( &cs ); }
PRcs::~PRcs()               { DeleteCriticalSection( &cs ); }
void PRcs::Enter( void )    { EnterCriticalSection( &cs ); }
BOOL PRcs::Try( void )      { return TryEnterCriticalSection( &cs ); }
void PRcs::Leave( void )    { LeaveCriticalSection( &cs ); }
//---------------------------------------------------------------------------
//  PRAccess
//---------------------------------------------------------------------------
PRAccess::PRAccess( void )           { Handle = PRAccessCreate(); }
PRAccess::~PRAccess()                { PRAccessDestroy( Handle ); Handle = NULL; }
BOOL PRAccess::Enter( AccTypes op )  { return Handle ? PRAccessEnter( Handle, op ) : FALSE; }
void PRAccess::Leave( void )         { if ( Handle ) PRAccessLeave( Handle ); }
#endif
