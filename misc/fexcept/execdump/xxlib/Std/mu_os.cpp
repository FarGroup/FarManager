#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/* ----------------------------------------------------------------------------
   ToOEM
   FromOEM

   OS: HWIN
   ---------------------------------------------------------------------------- */
#if defined(__HWIN32__)
CONSTSTR MYRTLEXP ToOEM( CONSTSTR s )
  {  static char str[ MAX_PATH_SIZE ];
     CharToOem( s, str );
 return str;
}
CONSTSTR MYRTLEXP FromOEM( CONSTSTR s )
  {  static char str[ MAX_PATH_SIZE ];
     OemToChar( s, str );
 return str;
}
#endif

/* ----------------------------------------------------------------------------
   GetHInstance

   OS: all
   ---------------------------------------------------------------------------- */
HANDLE MYRTLEXP GetHInstance( void )
  {
#if defined(__VCL__)
  return (HANDLE)HInstance;
#else
#if defined(__HWIN32__)
  return (HANDLE)GetCurrentThreadId();
#else
#if defined(__HWIN16__)
  return (HANDLE)1;
#else
#if defined(__QNX__)
  return (HANDLE)getpid();
#else
  return (HANDLE)1;
#endif // QNX
#endif // WIN16
#endif // WIN32
#endif // VCL
}

/* ----------------------------------------------------------------------------
   random
   randomize

   OS: QNX
   ---------------------------------------------------------------------------- */
#if defined(__QNX__)
int _cdecl random( int val )
  {
    val = Abs(val);
return (val<=1)?0:((val-1)*rand()/RAND_MAX);
}

void _cdecl randomize( void )
  {  time_t t;
      time(&t);
    srand( (int)t );
}
#endif

/* ----------------------------------------------------------------------------
   FreeSlice

   OS: all
   ---------------------------------------------------------------------------- */
#if defined(__GNUC__)

void MYRTLEXP FreeSlice( void )  { }

#else
#if defined(__QNX__)

void MYRTLEXP FreeSlice( void )  { delay(1); }

#else
#if defined(__HWIN__)

void MYRTLEXP FreeSlice( void )
  {  MSG msg;
     if ( PeekMessage( &msg,(HWND)0,0,0,PM_REMOVE ) ) {
       TranslateMessage( &msg );
       DispatchMessage( &msg );
     }
#if defined(__HWIN32__)
     Sleep(1);
#endif
}

#else
#if defined(__TEC32__)

void MYRTLEXP FreeSlice( void )  {}

#else
#if defined(__HDOS__)

void MYRTLEXP FreeSlice( void )
  {  static int freeSlice = 0;

     if ( freeSlice > 30 ) {
       asm{ mov ax,1680h; int 2fh } // Free system sice under windows
       freeSlice = 0;
     }
     freeSlice++;
}

#endif  //HDOS
#endif  //TEC
#endif  //HWIN
#endif  //QNX
#endif  //GNUC

/* ----------------------------------------------------------------------------
   ErrorBeep

   OS: all
   ---------------------------------------------------------------------------- */
void MYRTLEXP ErrorBeep( void )
  {
#if defined(__HWIN__)
    MessageBeep( MB_ICONEXCLAMATION );
#else
#if defined(__PROTDOS__)
    sound_beep( 1000 );
#else
#if defined(__QNX__) || defined(__REALDOS__)
    sound( 1000 );
    delay( 1000 );
    nosound();
#else
#if defined(__TEC32__)
    ;
#else
#if defined(__GNUC__)
    ;
#else
  #error ERR_PLATFORM
#endif //GNUC
#endif //TEC
#endif //QNX || REALDOS
#endif //PROTDOS
#endif //HWIN
}

/* ----------------------------------------------------------------------------
   AtExit, CallAtExit

   OS: all
   ---------------------------------------------------------------------------- */
#if !defined(__TEC32__)
static int RegisterAtExit();
static int _AtExitVal = RegisterAtExit();
static int RegisterAtExit() { return atexit( CallAtExit ); }
#endif

static AbortProc CTAbortProc = NULL;

void RTL_CALLBACK CallAtExit( void )
  {
     if ( CTAbortProc ) {
       CTAbortProc();
       CTAbortProc = NULL;
     }
}

AbortProc MYRTLEXP AtExit( AbortProc p )
  {  AbortProc old = CTAbortProc;
     CTAbortProc = p;
 return old;
}

/* ----------------------------------------------------------------------------
   __ConAbort, __ConAbortV

   OS: all
   ---------------------------------------------------------------------------- */
void MYRTLEXP_PT __ConAbort( CONSTSTR msg,... )
  {
     va_list a;
     va_start( a,msg );
     __ConAbortV( msg,a );
     va_end( a );
}

#if !defined(__TEC32__)
void MYRTLEXP __ConAbortV( CONSTSTR msg,va_list a )
  {  FIO_ERR_TYPE err = FIO_ERRORN;

     CallAtExit();

     if ( msg ) {
       printf( "\nABORT: " );
       vprintf( msg,a );
       if ( err != ERROR_SUCCESS ) {
         FIO_SETERRORN( err );
         printf( "\nStdError: %s",FIO_ERROR );
       }
       printf( "\n" );
     }

     exit(1);
}
#endif

/* ----------------------------------------------------------------------------
   __WinAbort, __WinAbortV

   OS: HWIN
   ---------------------------------------------------------------------------- */
#if defined(__HWIN__)
void MYRTLEXP_PT __WinAbort( CONSTSTR msg,... )
  {  va_list a;

     va_start( a,msg );
     __WinAbortV( msg,a );
     va_end( a );
}
void MYRTLEXP __WinAbortV( CONSTSTR msg,va_list a )
  {  char  str[ 1000 ];
     FIO_ERR_TYPE err = FIO_ERRORN;

     CallAtExit();

     if (msg) {
       VSNprintf( str,sizeof(str),msg,a );

       if ( err != ERROR_SUCCESS ) {
         FIO_SETERRORN( err );
         StrCat( str, "\nStdError: ", sizeof(str) );
         StrCat( str, FIO_ERROR, sizeof(str) );
       }
       MessageBox(
#if defined(__VCL__)
         Application?Application->Handle:NULL,
#else
         NULL,
#endif
         str,"Programm aborted:",MB_OK|MB_ICONHAND );
     }

     exit(1);
}
#endif

/* ----------------------------------------------------------------------------
   OSVersion, OSPlatform

   OS: all
   ---------------------------------------------------------------------------- */
WORD MYRTLEXP OSVersion( void )
  {
#if defined(__QNX__)
  struct _osinfo osi;
  return  (qnx_osinfo(0,&osi) == -1)?0:osi.version;
#else
#if defined(__HDOS__)
    return _osmajor*100 +_osminor;
#else
#if defined(__HWIN__)
  return (WORD)GetVersion();
#else
#if defined(__TEC32__)
  return (WORD)0x4008;
#else
#if defined(__GNUC__)
  return 0;
#else
#error ERR_PLATFORM
#endif  //GNUC
#endif  //TEC
#endif  //HWIN
#endif  //HDOS
#endif  //QNX
}

const char *MYRTLEXP OSPlatform( void )
  {
#if defined(__QNX__)
  #if defined(CONSOLE_ONLY)
    #if defined(CONSCREENOUT)
      return "QNX direct screen";
    #else
      return "QNX console";
    #endif
  #else
    return "QNX terminal";
  #endif
#else
#if defined(__REALDOS__)
  return "DOS 16";
#else
#if defined(__PROTDOS__)
  return "DOS 32";
#else
#if defined(__HWIN16__)
  switch( GetVersion() ) {
    case        /*VER_PLATFORM_WIN32s*/1: return "Win32 on Win3.x";
    case 0/*VER_PLATFORM_WIN32_WINDOWS*/: return "WinNT";
                                 default: return "Win16";
  }
#else
#if defined(__HWIN32__)
  OSVERSIONINFO os;
  os.dwOSVersionInfoSize = sizeof(os);
  if ( !GetVersionEx(&os) )
    return "Win32";

  if (os.dwPlatformId==VER_PLATFORM_WIN32s) return "Win32s";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) && (LOWORD(os.dwBuildNumber)==950))   return "Win95";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) && (LOWORD(os.dwBuildNumber)==1111))  return "Win95OSR";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) && (LOWORD(os.dwBuildNumber)==1998))  return "Win98";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) && (LOWORD(os.dwBuildNumber)==2222))  return "Win98SE";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_NT)      && (LOWORD(os.dwBuildNumber)<2195))   return "WinNT";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_NT)      && (LOWORD(os.dwBuildNumber)==2195))  return "Win2000";
  if ((os.dwPlatformId==VER_PLATFORM_WIN32_NT)      && (LOWORD(os.dwBuildNumber)>=2600))  return "WinXP";
return "Win32";
#else
#if defined(__TEC32__)
  return "Ordinal board";
#else
#if defined(__GNUC__)
  return "GnuC";
#else
  #error ERR_PLATFORM
#endif  //GNUC
#endif  //TEC
#endif  //WIN16
#endif  //WIN32
#endif  //PROTDOS
#endif  //REALDOS
#endif  //QNX
}

/* ----------------------------------------------------------------------------
   CheckReadable

   OS: all
   ---------------------------------------------------------------------------- */
BOOL MYRTLEXP CheckReadable( const LPVOID ptr,SIZE_T sz )
  {
#if defined(__HWIN__)
     BOOL rc;
     static LPVOID last = 0;

     if ( ptr == last )
       return FALSE;

     rc = IsBadReadPtr( ptr,sz + (4-sz%4) );

     if ( rc )
       last = ptr;

 return !rc;
#else
  return TRUE;
#endif
}

/* ----------------------------------------------------------------------------
   ! Auto call
   ---------------------------------------------------------------------------- */
#if defined(__BORLAND) || defined(__MSOFT) || defined(__SYMANTEC) || defined(__INTEL)
/* Sets defaut file IO mode to BINARY because some
   RTL set it to TEXT by default.
*/
static int SetFMode( void );
int _dummy_fmode = SetFMode();
int SetFMode( void )
  {
    _fmode = O_BINARY;
  return 0;
}
#endif
