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

#include "common/compiler.hpp"

#include "disable_warnings_in_std_begin.hpp"
//----------------------------------------------------------------------------

#if !IS_MICROSOFT_SDK()
#include <w32api.h>

#if (100*(__W32API_MAJOR_VERSION) + (__W32API_MINOR_VERSION)) < 314
#error w32api-3.14 (or higher) required
#endif

#include <winsdkver.h>

#undef _WIN32_
#undef _WIN32_IE
#undef _WIN32_WINNT
#undef _WIN32_WINDOWS_
#undef NTDDI
#undef WINVER

#define _WIN32_          _WIN32_MAXVER
#define _WIN32_IE        _WIN32_IE_MAXVER
#define _WIN32_WINNT     _WIN32_WINNT_MAXVER
#define _WIN32_WINDOWS_  _WIN32_WINDOWS_MAXVER
#define NTDDI            NTDDI_MAXVER
#define WINVER           WINVER_MAXVER

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
#include <shobjidl.h>
#include <shellapi.h>
#include <userenv.h>
#include <dbghelp.h>
#include <dwmapi.h>
#include <restartmanager.h>
#include <commdlg.h>
#include <winternl.h>
#include <versionhelpers.h>
#include <virtdisk.h>
#include <ntstatus.h>
#include <cfgmgr32.h>
#include <ntddmmc.h>
#include <ntddscsi.h>
#include <lmdfs.h>
#include <dbgeng.h>
#include <mlang.h>

#define _NTSCSI_USER_MODE_

#if IS_MICROSOFT_SDK()
#include <scsi.h>
#else
#include <ntdef.h>
#include <ddk/scsi.h>
#endif

#include "platform.sdk.hpp"

//----------------------------------------------------------------------------
#include "disable_warnings_in_std_end.hpp"

#undef far
#undef near
#undef FAR
#undef NEAR
#define FAR
#define NEAR

#endif // PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
