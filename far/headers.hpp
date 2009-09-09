#ifndef __HEADERS_HPP__
#define __HEADERS_HPP__
/*
headers.hpp

Стандартные заголовки
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define STRICT

#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600
#if defined(__GNUC__)
#define WINVER 0x0500
#endif

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
 #if (defined(__GNUC__) || defined(_MSC_VER)) && !defined(_WIN64)
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

#ifdef __cplusplus
  #include <new>
#endif

#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <share.h>
#include <search.h>
#include <winioctl.h>
#include <mmsystem.h>
#include <wininet.h>
#include <wchar.h>
#include <setupapi.h>
#include <assert.h>

#define SECURITY_WIN32
#include <security.h>

#ifndef __GNUC__
#include <shobjidl.h>
#include <winternl.h>
#endif

#ifdef __GNUC__
 #define __NTDDK_H
 #include <ddk/cfgmgr32.h>
#else
 #include <cfgmgr32.h>
#endif

#ifdef __GNUC__
 #define ultoa _ultoa
 #include <ctype.h>
#endif

#ifdef __GNUC__
 #include <limits.h>
 #include <ntdef.h>
#endif

#define _wmemset wmemset

#define vsnprintf _vsnprintf
#define vsnwprintf _vsnwprintf

#define lfind _lfind

#define _export

#define setdisk(n) _chdrive((n)+1)

#define randomize() srand(67898)
#define random(x) ((int) (((x) *  rand()) / (RAND_MAX+1)) )

/*
  - Из-за различий в реализации функции getdisk в BC & VC
    не работал AltFx если панель имела UNC путь
    Сама функция находится в farrtl.cpp
*/
#ifdef  __cplusplus
extern "C" {
#endif
 int _cdecl getdisk();
#ifdef  __cplusplus
}
#endif
#if defined(_MSC_VER)
 #pragma warning (once:4018)
#endif

#if defined(__GNUC__)
 #define _strtoi64 strtoll
 #define _wcstoi64 wcstoll
 #define _abs64 llabs
#endif

#ifdef __GNUC__
 #define _i64(num)   num##ll
 #define _ui64(num)  num##ull
#else
 #define _i64(num)   num##i64
 #define _ui64(num)  num##ui64
#endif

#if defined(__GNUC__)
 #define TRY
 #define EXCEPT(a) if (0)
#else
 #define TRY    __try
 #define EXCEPT __except
#endif

#define countof(a) (sizeof(a)/sizeof(a[0]))

#define NullToEmpty(s) (s?s:L"")

#ifdef  __cplusplus
template <class T>
inline const T&Min(const T &a, const T &b) { return a<b?a:b; }

template <class T>
inline const T&Max(const T &a, const T &b) { return a>b?a:b; }

template <class T>
inline const T Round(const T &a, const T &b) { return a/b+(a%b*2>b?1:0); }
#endif

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8       0xBFBBEF

//#include <crtdbg.h>

#include "sdkpatches.hpp"
#include "farrtl.hpp"

#ifdef  __cplusplus
#include "UnicodeString.hpp"
#include "global.hpp"
#include "local.hpp"
#include "plugin.hpp"
#include "farwinapi.hpp"
#include "cvtname.hpp"
#endif

#endif // __HEADERS_HPP__
