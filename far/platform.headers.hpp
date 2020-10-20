#ifndef PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
#define PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
#pragma once

/*
platform.headers.hpp

Platform headers
*/
/*
Copyright © 2020 Far Group
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

#include "disable_warnings_in_std_begin.hpp"
//----------------------------------------------------------------------------

#ifdef __GNUC__
#undef _W32API_OLD
#include <w32api.h>
#define _W32API_VER (100*(__W32API_MAJOR_VERSION) + (__W32API_MINOR_VERSION))
#if _W32API_VER < 314
#error w32api-3.14 (or higher) required
#endif
#if _W32API_VER < 317
#define _W32API_OLD 1
#endif
#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE
#define WINVER       0x0603
#define _WIN32_WINNT 0x0603
#define _WIN32_IE    0x0700
#endif // __GNUC__

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#ifndef NOMINMAX
#define NOMINMAX
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
#include <userenv.h>
#include <dbghelp.h>
#include <dwmapi.h>
#include <restartmanager.h>
#include <commdlg.h>

#define _NTSCSI_USER_MODE_

#ifdef _MSC_VER
#include <ntstatus.h>
#include <shobjidl.h>
#include <winternl.h>
#include <cfgmgr32.h>
#include <ntddmmc.h>
#include <ntddscsi.h>
#include <virtdisk.h>
#include <lmdfs.h>
#include <scsi.h>
#endif // _MSC_VER

#ifdef __GNUC__
# define __NTDDK_H
struct _ADAPTER_OBJECT;
typedef struct _ADAPTER_OBJECT ADAPTER_OBJECT,*PADAPTER_OBJECT;
#ifdef _W32API_OLD
#include <ntstatus.h>
#include <cfgmgr32.h>
#include <ntddmmc.h>
#include <ntddscsi.h>
#else
#include <ddk/ntstatus.h>
#include <ddk/cfgmgr32.h>
#include <ddk/ntddmmc.h>
#include <ddk/ntddscsi.h>
#include <ntdef.h>
#endif

// Workaround for MinGW, see a66e40
// Their loony headers are unversioned,
// so the only way to make it compatible
// with both old and new is this madness:
#include <netfw.h>
#ifndef __INetFwProduct_FWD_DEFINED__
#define _LBA
#define _MSF
#endif
#include <ddk/scsi.h>
#ifndef __INetFwProduct_FWD_DEFINED__
#undef _MSF
#undef _LBA
#endif
#endif // __GNUC__

#include "platform.sdk.hpp"

//----------------------------------------------------------------------------
#include "disable_warnings_in_std_end.hpp"

#undef far
#undef near

#endif // PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
