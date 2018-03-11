/*
vc_crt_fix_ulink.cpp

Workaround for Visual C++ CRT incompatibility with old Windows versions (ulink version)
*/
/*
Copyright © 2010 Far Group
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
#include <windows.h>
#include "disable_warnings_in_std_end.hpp"
#include <delayimp.h>

//----------------------------------------------------------------------------
static LPVOID WINAPI no_recode_pointer(LPVOID p)
{
    return p;
}

//----------------------------------------------------------------------------
static void WINAPI sim_InitializeSListHead(PSLIST_HEADER ListHead)
{
    ((LPDWORD)ListHead)[1] = ((LPDWORD)ListHead)[0] = 0;
}

//----------------------------------------------------------------------------
static BOOL WINAPI sim_GetModuleHandleExW(DWORD flg, LPCWSTR name, HMODULE* pm)
{
    // GET_MODULE_HANDLE_EX_FLAG_PIN not implemented (and unneeded)
    HMODULE   hm;
    wchar_t   buf[MAX_PATH];

    *pm = NULL; // prepare to any return's
    if(flg & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) {
      MEMORY_BASIC_INFORMATION  mbi;
      if(!VirtualQuery(name, &mbi, sizeof(mbi))) return FALSE;
      hm = (HMODULE)mbi.AllocationBase;
      if(flg & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT) goto done;
      if(!GetModuleFileNameW(hm, buf, ARRAYSIZE(buf))) return FALSE;
      name = buf;
    } else if(flg & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT) {
      hm = GetModuleHandleW(name);
      goto done;
    }
    hm = LoadLibraryW(name);
done:
    return (*pm = hm) != NULL;
}

//----------------------------------------------------------------------------
static FARPROC WINAPI delayFailureHook(/*dliNotification*/unsigned dliNotify,
                                       PDelayLoadInfo pdli)
{
    if(   dliNotify == /*dliFailGetProcAddress*/dliFailGetProc
       && pdli && pdli->cb == sizeof(*pdli)
       && pdli->hmodCur == GetModuleHandleA("kernel32")
       && pdli->dlp.fImportByName && pdli->dlp.szProcName)
    {
#if _MSC_FULL_VER >= 191326128  // VS2017.6
#pragma warning(disable: 4191)  // unsafe conversion from...to
#endif
      if(   !lstrcmpA(pdli->dlp.szProcName, "EncodePointer")
         || !lstrcmpA(pdli->dlp.szProcName, "DecodePointer"))
      {
        return (FARPROC)no_recode_pointer;
      }
      if(!lstrcmpA(pdli->dlp.szProcName, "InitializeSListHead"))
        return (FARPROC)sim_InitializeSListHead;
      if(!lstrcmpA(pdli->dlp.szProcName, "GetModuleHandleExW"))
        return (FARPROC)sim_GetModuleHandleExW;
    }
    return nullptr;
}

//----------------------------------------------------------------------------
#if _MSC_FULL_VER >= 190024215 // VS2015sp3
const
#endif
PfnDliHook __pfnDliFailureHook2 = (PfnDliHook)delayFailureHook;

//----------------------------------------------------------------------------

// TODO: Add GetModuleHandleExW
