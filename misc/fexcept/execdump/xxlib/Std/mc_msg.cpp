#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__TEC32__)
  #define MSG_BUFFSIZE MAX_PATH_SIZE
  static char messbuff[ MAX_PATH_SIZE+1 ];
  static CRITICAL_SECTION cs;

  static void MYRTLEXP HMCInitialize( void ) { }
#else
  #define MSG_BUFFSIZE 10000

  static char *messbuff = NULL;
  static CRITICAL_SECTION cs;
  static AbortProc        aProcMsg;
  static void RTL_CALLBACK idDelBuff( void )
    {
      _Del( messbuff ); messbuff = NULL;
      DeleteCriticalSection( &cs );
      if ( aProcMsg )
        aProcMsg();
  }
  static void MYRTLEXP HMCInitialize( void )
    {
      if ( !messbuff ) {
        messbuff = (char*)_Alloc( MSG_BUFFSIZE+1 );
        if (!messbuff) HAbort( "!Alloc memory for message" );
        InitializeCriticalSection( &cs );
        aProcMsg = AtExit( idDelBuff );
      }
  }
#endif

//--------------------------------------------------------------------
CONSTSTR MYRTLEXP_PT Message( CONSTSTR patt,... )
  {  va_list  a;
     CONSTSTR m;

     va_start( a, patt );
     m = MessageV( patt,a );
     va_end( a );
 return m;
}

CONSTSTR MYRTLEXP MessageV( CONSTSTR patt,va_list a )
  {
     HMCInitialize();

     EnterCriticalSection( &cs );
       VSNprintf( messbuff,MSG_BUFFSIZE,patt,a );
     LeaveCriticalSection( &cs );
 return messbuff;
}
//--------------------------------------------------------------------
#if defined(__WINUNICODE__)
WCONSTSTR MYRTLEXP_PT Message( WCONSTSTR patt,... )
  {  va_list  a;
     WCONSTSTR m;

     va_start( a, patt );
     m = MessageV( patt,a );
     va_end( a );
 return m;
}

WCONSTSTR MYRTLEXP MessageV( WCONSTSTR patt,va_list a )
  {
     HMCInitialize();

     EnterCriticalSection( &cs );
       VSNprintf( (wchar_t*)messbuff, MSG_BUFFSIZE/(sizeof(wchar_t)/sizeof(char)),
                  patt, a );
     LeaveCriticalSection( &cs );
 return (WCONSTSTR)messbuff;
}

#endif
