#ifndef __MY_ASSERT
#define __MY_ASSERT

#if defined(__HWIN__)
 #if defined(__VCL__)
  #define THROW_ERROR(err,fl,nm)      throw Exception( AnsiString("Assert: [" err "] in " fl ":")+nm )
 #else
  #define THROW_ERROR(err,fl,nm)      HAbort( "Assert [%s] fail at \"%s:%d\"",err,fl,nm)
 #endif
#else
#if defined( __REALDOS__ )
  #define THROW_ERROR(err,fl,nm)      do{ fprintf( stderr,"Assert: [" err "] in " fl ":%d",nm); asm int 3; }while(0)
#else
#if defined( __QNX__ )
  #define THROW_ERROR(err,fl,nm)      HAbort( "Assert [%s] fail at \"%s:%d\"",err,fl,nm )
#else
#if defined( __PROTDOS__ )
  #define THROW_ERROR(err,fl,nm)      do{ fprintf( stderr,"Assert: [" err "] in " fl ":%d",nm); asm int 3; }while(0)
#else
#if defined( __TEC32__ )
  #define THROW_ERROR(err,fl,nm)      HAbort( "Assert [%s] fail at \"%s:%d\"",err,fl,nm )
#else
#if defined( __GNUC__ )
  #define THROW_ERROR(err,fl,nm)      HAbort( "Assert [%s] fail at \"%s:%d\"",err,fl,nm )
#else
#error ERR_PLATFORM
#endif  //GNUC
#endif  //TEC32
#endif  //PROTDOS
#endif  //QNX
#endif  //MSDOS
#endif  //WIN32

#if defined(__HWIN__)
  #if defined(__HCONSOLE__)
    #define OSMessage( v )  ((printf v) != 0 )
  #else
    #define OSMessage( v )  (MessageBox( NULL,Message v,"Assertion message...",MB_ICONHAND|MB_YESNO) == IDYES)
  #endif
  #define TraceAssert(p)    do{                                                                                  \
                              if ( (p) == 0 ) {                                                                  \
                                __HeapIgnoreErrors = TRUE;                                                       \
                                if ( OSMessage(( "Assertion fail: [%s]\nAt \"%s:%d\""                            \
                                                 "\nGenerating call-stack to %s...",                             \
                                                 #p, __FILE__ , __LINE__, GetLogFullFileName() )) ) {            \
                                  FILELog( "Assertion fail: [%s] At \"%s:%d\"", #p, __FILE__ , __LINE__ );       \
                                  MakeStackWalkListCUR( FILELog, 0, STK_DEFAULT );                               \
                                }                                                                                \
                                HAbort( "Assertion fail: %s", #p );                                              \
                              }                                                                                  \
                            }while(0)

#else
  #define TraceAssert       Assert
#endif

#if defined(__USE_TRAPLOGER__)
  #define Assert              TraceAssert
#else
#if defined(__DEBUG__)
  #define Assert( p )         do{ if ( (p) == 0 ) THROW_ERROR( #p , __FILE__ , __LINE__ ); }while(0)
#else
  #define Assert(p)
  #undef  TraceAssert
  #define TraceAssert(p)
#endif
#endif

#endif
