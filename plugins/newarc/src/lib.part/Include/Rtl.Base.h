#pragma once

#if defined(_MSC_VER)
#pragma warning(disable:4201) // nameless unions
#pragma warning(disable:4121) // alignment of a member was sensitive to packing (i don't care)
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4100) // unreferenced formal parameter

#define _CRT_SECURE_NO_WARNINGS
#endif

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
 #if defined(__GNUC__) || defined(_MSC_VER)
  #if !defined(_WINCON_H) && !defined(_WINCON_)
    #define _WINCON_H
    #define _WINCON_ // to prevent including wincon.h
    #if defined(_MSC_VER)
     #pragma pack(push,2)
    #else
     #pragma pack(2)
    #endif
    #include<windows.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
    #undef _WINCON_
    #undef  _WINCON_H

    #if defined(_MSC_VER)
     #pragma pack(push,8)
    #else
     #pragma pack(8)
    #endif
    #include<wincon.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
  #endif
  #define _WINCON_
 #else
   #include<windows.h>
 #endif
#endif


#include <Rtl.Types.h>
#ifdef __GNUC__
#include <Rtl.Memory.h>
#endif
#include <Rtl.Strings.h>
#include <Rtl.Kernel.h>
#include <Rtl.Hook.h>
#include <Rtl.Options.h>
#include <Rtl.Thunks.h>
#include <Rtl.Misc.h>

#if defined(_MSC_VER)
#pragma comment(linker,"/merge:.CRT=.data")
#endif
