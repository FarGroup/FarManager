#ifndef __MY_EXCEPTION_HANDLING
#define __MY_EXCEPTION_HANDLING

#if defined(__HWIN32__)
  #define __EXCEPTIONS_USED__ 1
#endif

#if defined(__EXCEPTIONS_USED__)
  #include <Std/hwinmod.h>   //Process enumeration
  #include <Std/xxH.h>       //Procedure hooks
  #include <Std/xxSym.h>     //Symbol lists
  #include <Std/xxStack.h>   //Stack walk `n` stack frames

  enum _Except_ExceptionCodes {
   EXCEPTION_DELPHI_EXCEPTION    = 0x0EEDFADEUL,
   EXCEPTION_DELPHI_RERISE       = 0x0EEDFADFUL,
   EXCEPTION_DELPHI_EXCEPT       = 0x0EEDFAE0UL,
   EXCEPTION_DELPHI_FINALLY      = 0x0EEDFAE1UL,
   EXCEPTION_DELPHI_TERMINATE    = 0x0EEDFAE2UL,
   EXCEPTION_DELPHI_UNHANDLED    = 0x0EEDFAE3UL,
   EXCEPTION_NONDELPHI_EXCEPTION = 0x0EEDFAE4UL,
   EXCEPTION_DELPHI_EXITFINALLY  = 0x0EEDFAE5UL,
   EXCEPTION_DELPHI_DEBUGGER     = 0x0EEDFAE6UL,
   EXCEPTION_CPP_EXCEPTION       = 0x0EEFFACEUL,

   OS_EXCEPTION_FLAG             = 0x80000000UL
  };

  inline BOOL isOSException( DWORD x) { return IS_FLAG( x,OS_EXCEPTION_FLAG ); }

//[xxExceptU.cpp]
/*
    Returns text name of exception (static buffer)
*/
  HDECLSPEC CONSTSTR MYRTLEXP ExceptType2Str( DWORD ExceptionType );
  HDECLSPEC BOOL     MYRTLEXP IsNonContinuable( LPEXCEPTION_RECORD ex );

/*
    Create an expanded description of exception.
*/
  HDECLSPEC void     MYRTLEXP MakeExceptionInfo( HPrintProc_t PrintProcedure,
                                                 PEXCEPTION_RECORD record /*=NULL*/,
                                                 PCONTEXT context /*=NULL*/ );

  HDECLSPEC void     MYRTLEXP MakeContextInfo( HPrintProc_t print,
                                               PCONTEXT ctx /*=NULL*/ );

//[xxExcept.cpp]
/*
*/
  HDECLSPEC void     MYRTLEXP SetExceptionHandlerCPP( DWORD Flags /*set of STK_xxx*/ );
#endif

#endif
