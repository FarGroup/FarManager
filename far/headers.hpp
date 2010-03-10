#pragma once

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

#ifdef _MSC_VER
#include <sdkddkver.h>
#if _WIN32_WINNT<0x0601
#error Windows SDK v7.0 (or higher) required
#endif
#endif //_MSC_VER

#ifdef __GNUC__
#include <w32api.h>
#if __W32API_MAJOR_VERSION<3 || (__W32API_MAJOR_VERSION==3&&(__W32API_MINOR_VERSION<13))
#error w32api-3.13 (or higher) required
#endif
#endif // __GNUC__

#ifdef __GNUC__
#define WINVER       0x0601
#define _WIN32_WINNT 0x0601
#define _WIN32_IE    0x0601
#endif // __GNUC__

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h

#include <windows.h>
#include <winioctl.h>
#include <mmsystem.h>
#include <wininet.h>
#include <setupapi.h>
#include <Aclapi.h>
#define SECURITY_WIN32
#include <security.h>

#undef WIN32_NO_STATUS


#ifdef _MSC_VER
#include <ntstatus.h>
#include <shobjidl.h>
#include <winternl.h>
#include <cfgmgr32.h>
#include <ntddscsi.h>
#endif // _MSC_VER

#ifdef __GNUC__
#define __NTDDK_H
#include <ddk/ntstatus.h>
#include <ddk/cfgmgr32.h>
struct _ADAPTER_OBJECT;
typedef struct _ADAPTER_OBJECT ADAPTER_OBJECT,*PADAPTER_OBJECT;
#include <ddk/ntddscsi.h>
#include <ntdef.h>
#endif // __GNUC__


#include "SDK/sdk.common.h"

#ifdef _MSC_VER
#include "SDK/sdk.vc.h"
#endif // _MSC_VER

#ifdef __GNUC__
#include "SDK/sdk.gcc.h"
#endif // __GNUC__



#ifdef __cplusplus
#include <new>
#endif // __cplusplus

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
#include <wchar.h>
#include <assert.h>
#include <math.h>

#ifdef __GNUC__
#include <ctype.h>
#include <limits.h>
#endif  //__GNUC__

#define vsnprintf _vsnprintf
#define vsnwprintf _vsnwprintf

#define _export

#ifdef __GNUC__
#define _strtoi64 strtoll
#define _wcstoi64 wcstoll
#define _abs64 llabs
#endif // __GNUC__

#ifdef __GNUC__
#define __try
#define __except(a) if(0)
#endif // __GNUC__

#define countof(a) (sizeof(a)/sizeof(a[0]))

#define NullToEmpty(s) (s?s:L"")

#ifdef  __cplusplus
template <class T>
inline const T&Min(const T &a, const T &b) { return a<b?a:b; }

template <class T>
inline const T&Max(const T &a, const T &b) { return a>b?a:b; }

template <class T>
inline const T Round(const T &a, const T &b) { return a/b+(a%b*2>b?1:0); }
#endif // __cplusplus

#define IsPtr(x) ((DWORD_PTR)x>(DWORD_PTR)SystemInfo.lpMinimumApplicationAddress && (DWORD_PTR)x<(DWORD_PTR)SystemInfo.lpMaximumApplicationAddress)

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8       0xBFBBEF

//#include <crtdbg.h>

#include "farrtl.hpp"

#ifdef  __cplusplus
#include "UnicodeString.hpp"
#include "format.hpp"
#include "global.hpp"
#include "local.hpp"
#include "plugin.hpp"
#include "farwinapi.hpp"
#include "cvtname.hpp"
#endif // __cplusplus

#ifdef _DEBUG
#define SELF_TEST(code) \
	namespace { \
		struct SelfTest { \
			SelfTest() { \
				code; \
			} \
		} _SelfTest; \
	}
#else
#define SELF_TEST(code)
#endif
