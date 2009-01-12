#ifndef __MY_THREAD
#define __MY_THREAD

typedef void (*ThreadProc_t)( LPVOID ptr );

STRUCT( HThread )
#if defined(__HWIN__)
    HANDLE Handle;
    DWORD  Id;
#endif
  public:
    HThread( void );
    ~HThread();

    BOOL Create( ThreadProc_t cb, LPVOID ptr = NULL, BOOL Susp = FALSE );
    void Close( BOOL force = FALSE );
    void Wait( DWORD ms = INFINITE );
    BOOL isRun( void );
};

#endif
