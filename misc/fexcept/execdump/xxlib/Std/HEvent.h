#ifndef __MY_EVENT
#define __MY_EVENT

#if defined(__HWIN32__)
LOCALSTRUCT( HManualEvent )
    BOOL   Self;
    HANDLE Handle;
  public:
    HManualEvent( void )        { Handle = NULL; }
    HManualEvent( BOOL state )  { Self = TRUE; Handle = CreateEvent(NULL,TRUE,state,NULL); }
    HManualEvent( HANDLE ev )   { Self = FALSE; Handle = ev; }
    ~HManualEvent()             { Close(); }

    void Close( void )          { if (Self) CloseHandle(Handle); Handle = NULL; }
    BOOL operator!() const      { return Handle == NULL; }
    operator int() const        { return Handle != NULL; }
    operator HANDLE()           { return Handle; }

    void Assign( BOOL state )   { Close(); Self = TRUE; Handle = CreateEvent(NULL,TRUE,state,NULL); }
    void Assign( HANDLE ev )    { Close(); Self = FALSE; Handle = ev; }

    BOOL WaitSignaled( DWORD tm = INFINITE )   { return WaitForSingleObject(Handle,tm) == WAIT_OBJECT_0; }
    BOOL WaitUnsignaled( void ) { if (IsError()) return FALSE; while( IsSet() ) Sleep(1); return TRUE; }
    BOOL IsSet( void )          { return WaitForSingleObject(Handle,0) == WAIT_OBJECT_0; }
    BOOL IsError( void )        { return !Handle || WaitForSingleObject(Handle,0) == WAIT_ABANDONED; }

    void Set( void )            { SetEvent(Handle); }
    void Reset( void )          { ResetEvent(Handle); }
    void Pulse( void )          { PulseEvent(Handle); }
};
#endif

#endif
