#ifndef __MY_THREADS
#define __MY_THREADS

/* -----------------------------------------------------
   WIN
*/
#if defined(__HWIN32__)
    #define DECL_ENTRY_PROC(nm,param)    void nm( LPVOID param ) {
    #define END_ENTRY_PROC               exit(0); }
    #define SPAWN_CHILD(child,param)     { DWORD id; if ( !CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)child,param,0,&id) ) HAbort("Spawn"); }

    typedef DWORD (DLL_CALLBACK *PRThreadProc)( LPVOID param );

    #define BEGIN_THREAD_PROC( nm,param )                     DWORD DLL_CALLBACK nm( LPVOID param ) {
    #define END_THREAD_PROC                                   return 0; }
    #define DEF_THREAD_PROC(nm) DWORD DLL_CALLBACK nm( LPVOID param )
#else
/* -----------------------------------------------------
   QNX
*/
#if defined(__QNX__)
    #define DECL_ENTRY_PROC(nm,param)    void nm( LPVOID param ) {
    #define END_ENTRY_PROC               exit(0); }
    #define SPAWN_CHILD(child,param)     switch( fork() ) { case -1: HAbort( "Spawn" ); case 0: child(param); }

    #define THREAD_SUSPEND    SIGSTOP
    #define THREAD_RESUME     SIGCONT
    #define THREAD_KILL       SIGKILL
    STRUCT( __QNX_THREADPara )
      BOOL   Suspended;
      LPVOID Param;
      BOOL   Started;
    };
    typedef int (*PRThreadProc)( LPVOID param );
    #define BEGIN_THREAD_PROC( nm,pnm )          int nm( void *_Th_p ) {                                 \
                                                   LPVOID pnm = ((P__QNX_THREADPara)_Th_p)->Param;       \
                                                   USEARG(pnm);                                          \
                                                   ((P__QNX_THREADPara)_Th_p)->Started = TRUE;           \
                                                   if ( ((P__QNX_THREADPara)_Th_p)->Suspended )          \
                                                     kill(getpid(),THREAD_SUSPEND);

    #define END_THREAD_PROC                      exit(0); return 0; }
    #define DEF_THREAD_PROC(nm) int nm( void *param )
#else
/* -----------------------------------------------------
   System is single-task
*/
    #define DECL_ENTRY_PROC(nm,param)                         void nm( LPVOID param ) {
    #define END_ENTRY_PROC                                    }
    #define SPAWN_CHILD(child,param)                          child(param)

    typedef int (*PRThreadProc)( LPVOID param );

    #define BEGIN_THREAD_PROC( nm,param )                     DWORD DLL_CALLBACK nm( LPVOID param ) {
    #define END_THREAD_PROC                                   return 0; }
    #define DEF_THREAD_PROC(nm)                               DWORD DLL_CALLBACK nm( LPVOID param )
#endif
#endif

HDECLSPEC HANDLE MYRTLEXP PRThreadCreate( PRThreadProc proc,BOOL Suspended,LPVOID param );
HDECLSPEC void   MYRTLEXP PRThreadTerminate( HANDLE p );
HDECLSPEC BOOL   MYRTLEXP PRThreadSuspend( HANDLE p );
HDECLSPEC BOOL   MYRTLEXP PRThreadResume( HANDLE p );
HDECLSPEC void   MYRTLEXP PRThreadExit( DWORD retVal );
HDECLSPEC BOOL   MYRTLEXP PRThreadValid( HANDLE p );
HDECLSPEC DWORD  MYRTLEXP PRThreadID( HANDLE p );
HDECLSPEC DWORD  MYRTLEXP PRThreadCurrentID( void );

#endif
