#ifndef __PLATFOM_H
#define __PLATFOM_H

#define ERR_PLATFORM "Compilation not defined for current platform"

//-------------------------------------------------------------------
//Global compiller types
#undef __BORLAND    //Borland
#undef __SYMANTEC   //Symantec
#undef __MSOFT      //Microsoft
#undef __DMC        //Digital Mars Compoller
#undef __INTEL      //Intel compiller
#undef __CYGWIN32__ //Cygwin on Win32
#undef __MINGW32__  //MinGW on Win32

//Global targets models
#undef __HWIN__     //Any windows
#undef __HWIN32__   //Win32
#undef __HWIN16__   //Win16
#undef __HDOS__     //Any DOS
#undef __HUNIX__    //Any unix-compatible
//#undef __TEC32__   - MANUALLY defined in make file

//Specific
#undef HAS_ANONSTRUCT       //has anonimous structs
#undef __HAS_INT64__        //has __int64 doublelong type
#undef HAS_LONGARRAY        //Support <long> as array index
#undef __UNICODE__          //Support wide-char ptr

//Variants
#undef __REALDOS__
#undef __PROTDOS__

#if defined( __BORLANDC__)
  #include <Global/bcbDefs.h>
#else
  #define _HPACKAGE
#endif

//- PLATFORM
#if defined( __BORLANDC__ )
  #define __BORLAND    1

  #if defined( __WIN32__ )
    #define __HWIN__       1
    #define __HWIN32__     1
    #define __BCWIN32__    1
  #else
  #if defined( _Windows )
    #define __HWIN__     1
    #define __HWIN16__   1
    #define __BCWIN16__  1
  #else
  #if __BC__ || defined( __MSDOS__ )
    #define __HDOS__     1
    #define __REALDOS__  1
  #else
    #error "Undefined borland platform"
  #endif
  #endif
  #endif
#endif

#if defined(__SC__) && defined(DOS386)                          //Symantec DOSX
  #define HAS_ANONSTRUCT  1
  #define __HDOS__     1
  #define __SYMANTEC   1
  #define __PROTDOS__  1
#endif

#if defined(__SC__) && defined(_WINDOWS)                        //Symantec WIN32
  #define HAS_ANONSTRUCT  1
  #define __HWIN__     1
  #define __HWIN32__   1
  #define __SYMANTEC   1
  #define __SCWIN32__  1
#endif

#if defined(_MSC_VER)                                           //MS 6.0
  #define HAS_ANONSTRUCT  1
  #define __HWIN__     1
  #define __HWIN32__   1
  #define __MSOFT      1
  #define __MSWIN32__  1
#endif

#if defined(__DMC__) && defined(_WIN32)                         //DMC compiller
  #define HAS_ANONSTRUCT  1
  #define __HWIN__     1
  #define __HWIN32__   1
  #define __DMC        1
#endif

#if defined(__DMC__) && defined(_MSDOS)                         //DMC compiller
  #define HAS_ANONSTRUCT  1
  #define __HDOS__     1
  #define __DMC        1
  #define __REALDOS__  1
#endif

#if defined(__DMC__) && defined(DOS386)                         //DMC compiller
  #define HAS_ANONSTRUCT  1
  #define __HDOS__     1
  #define __DMC        1
  #define __PROTDOS__  1
#endif

#if defined(__INTEL_COMPILER)                                   //Intel
  #define HAS_ANONSTRUCT  1
  #define __HWIN__     1
  #define __HWIN32__   1
  #define __INTEL      1
#endif

//Windows GCC clons
#if defined(__CYGWIN32__) || defined(__MINGW32__)               //GCC
  #define __HUNIX__       1
  #define HAS_ANONSTRUCT  1
  #define __HWIN__        1
  #define __HWIN32__      1
#endif

#if defined(__GNUC__)                                          //UNIX
  #define __HUNIX__       1
#endif

//-------------------------------------------------------------------
//- DEBUG
#if defined(_DEBUG)           //BCB debug flag
  #undef __DEBUG__
  #define __DEBUG__ 1
#endif
#ifdef _DEBUG                 //VC debug flag
  #undef __DEBUG__
  #define __DEBUG__ 1
#endif
#ifdef NDEBUG                 //VC release
  #undef __DEBUG__
#endif
#if defined(NDEBUG)
  #undef __DEBUG__
#endif

//- VCL
#if defined(_RTLDLL) && defined(USEPACKAGES)
  //VCL must be used
#else
  #if defined(__NOVCL__)      //My flag to force no VCL
    #undef __VCL__
  #endif
  #ifdef _NO_VCL              //BCB Console vizard
    #undef __VCL__
  #endif
#endif

//- CONSOLE
#if defined(_CONSOLE) || defined(__CONSOLE__)  //VC || BCB
  #define __HCONSOLE__  1
#endif

#if defined(__WIN32__)
  #define __UNICODE__    1
  #define __WINUNICODE__ 1

  #define NO_SHLWAPI_STRFCNS 1
#endif

//- WARNINGS
#if defined(__MSOFT)
  #pragma warning(disable:4001)   // Single-line comment warnings
  #pragma warning(disable:4100)   //Disable: unreferenced formal parameter
  #pragma warning(disable:4115)   // Named type definition in parentheses
  #pragma warning(disable:4127)   // conditional expression is constant "while(1) {...}"
  #pragma warning(disable:4201)   // Nameless struct/union warning
  #pragma warning(disable:4214)   // Bit field types other than int warnings
  #pragma warning(disable:4514)   // Unreferenced inline function has been removed
  #pragma warning(disable:4166)   // illegal calling convention for constructor/destructor (warrn if __fastcall used)
  #pragma warning(disable:4800)   // forcing value to bool 'true' or 'false' (performance warning)
  #pragma warning(disable:4146)   // unary minus operator applied to unsigned type, result still unsigned
#endif
#if defined(__BORLAND)
  #pragma option -w-inl              //Disable: "cannot expand inline..." warnings
#endif

//- INT64
#if defined(__SYMANTEC) && defined(__PROTDOS__)      //SC protdos
  #define __HAS_INT64__ 1
#endif
#if defined(__SCWIN32__)                             //SC win32
  #define __HAS_INT64__ 1
#endif
#if defined(__BCWIN32__) && !defined(__BCB1__)       //BCB >1.0
  #define __HAS_INT64__ 1
#endif
#if defined(__MSWIN32__)                             //VC >=6.0
  #define __HAS_INT64__ 1
#endif
#if defined(__DMC) && defined(__HWIN__)              //DMC win32
  #define __HAS_INT64__ 1
#endif
#if defined(__DMC) && defined(__PROTDOS__)           //DMC protdos
  #define __HAS_INT64__ 1
#endif
#if defined(__INTEL) && defined(__HWIN__)            //Intel on VC
  #define __HAS_INT64__ 1
#endif

#if defined(__QNX__) || defined(__TEC32__)
  #define __HUNIX__ 1
#endif

#define HAS_LONGARRAY 1
#if defined(__BORLAND) && defined(__HWIN16__)
  #undef HAS_LONGARRAY
#endif

#undef HAS_INLINE_ENUM

#if defined(__BORLAND)
  #if __BORLANDC__ > __BCB10__
    #define HAS_INLINE_ENUM  1
  #endif
#else
#if !defined(__QNX__)
  #define HAS_INLINE_ENUM  1
#endif
#endif

#endif
