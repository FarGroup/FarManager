#pragma once

/*
headers.hpp

Стандартные заголовки
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include <array>
#include <algorithm>
#include <bitset>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <queue>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cassert>
#include <cctype>
#include <cfloat>
#include <climits>
#include <cstdint>
#include <ctime>

#include <process.h>

#undef _W32API_OLD

#ifdef _MSC_VER
# if _MSC_VER < 1600
#  error Visual C++ 2010 (or higher) required
# endif
# include <sdkddkver.h>
# if _WIN32_WINNT < 0x0601
#  error Windows SDK v7.0 (or higher) required
# endif
#endif //_MSC_VER

#ifdef __GNUC__
# define GCC_VER_(gcc_major,gcc_minor,gcc_patch) (100*(gcc_major) + 10*(gcc_minor) + (gcc_patch))
# define _GCC_VER GCC_VER_(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

# if _GCC_VER < GCC_VER_(4,7,1)
#  error gcc 4.7.1 (or higher) required
# endif

# include <w32api.h>
# define _W32API_VER (100*(__W32API_MAJOR_VERSION) + (__W32API_MINOR_VERSION))
# if _W32API_VER < 314
#  error w32api-3.14 (or higher) required
# endif
# if _W32API_VER < 317
#  define _W32API_OLD 1
# endif
# undef WINVER
# undef _WIN32_WINNT
# undef _WIN32_IE
# define WINVER       0x0601
# define _WIN32_WINNT 0x0601
# define _WIN32_IE    0x0700
#endif // __GNUC__

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning( disable: 4458 5024 5025 5026 5027)
#endif

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h
#include <windows.h>
#undef WIN32_NO_STATUS
#include <winioctl.h>
#include <mmsystem.h>
#include <wininet.h>
#include <winspool.h>
#include <setupapi.h>
#include <aclapi.h>
#include <sddl.h>
#include <dbt.h>
#include <lm.h>
#define SECURITY_WIN32
#include <security.h>
#define PSAPI_VERSION 1
#include <psapi.h>
#include <shlobj.h>
#include <shellapi.h>

#ifdef _MSC_VER
# include <ntstatus.h>
# include <shobjidl.h>
# include <winternl.h>
# include <cfgmgr32.h>
# include <ntddscsi.h>
# include <virtdisk.h>
# include <RestartManager.h>
# include <lmdfs.h>
#endif // _MSC_VER

#ifdef __GNUC__
# define __NTDDK_H
  struct _ADAPTER_OBJECT;
  typedef struct _ADAPTER_OBJECT ADAPTER_OBJECT,*PADAPTER_OBJECT;
# ifdef _W32API_OLD
#  include <ntstatus.h>
#  include <cfgmgr32.h>
#  include <ntddscsi.h>
# else
#  include <ddk/ntstatus.h>
#  include <ddk/cfgmgr32.h>
#  include <ddk/ntddscsi.h>
#  include <ntdef.h>
# endif
#endif // __GNUC__

#include "SDK/sdk.common.h"

#ifdef _MSC_VER
# include "SDK/sdk.vc.h"
#endif // _MSC_VER

#ifdef __GNUC__
# include "SDK/sdk.gcc.h"
#endif // __GNUC__

#include "cpp.hpp"

#include "common.hpp"

#include "noncopyable.hpp"
#include "farrtl.hpp"
#include "format.hpp"
#include "local.hpp"
#include "farwinapi.hpp"
#include "cvtname.hpp"

#include "plugin.hpp"

#include "global.hpp"
