/*
hook_wow64.c
*/
/*
Copyright © 2007 Far Group
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

#include <windows.h>

#if defined(__GNUC__) && __GNUC__ < 4
#warning "Consider using newer GCC version for this hook to work"
#else

#if ((!defined(_MSC_VER) || _MSC_VER < 1300) && !defined(__GNUC__)) || defined(_WIN64)
#error
#endif
/*
 * If compiled with VC7 and linking with ulink, please specify
 * -DLINK_WITH_ULINK
*/

#if defined(_MSC_VER)
#pragma optimize("gty", on)
#endif

//-----------------------------------------------------------------------------
static
#if defined(__GNUC__)
__thread
#endif
PVOID
#if !defined(__GNUC__)
__declspec(thread)
#endif
saveval;

typedef BOOL (WINAPI *DISABLE)(PVOID*);
typedef BOOL (WINAPI *REVERT)(PVOID);
typedef BOOL (WINAPI *ISWOW)(HANDLE,PBOOL);

typedef struct
{
	DISABLE disable;
	REVERT revert;
} WOW;

static BOOL WINAPI e_disable(PVOID* p) { (void)p; return FALSE; }
static BOOL WINAPI e_revert(PVOID p) { (void)p; return FALSE; }

volatile const WOW wow = { e_disable, e_revert };

//-----------------------------------------------------------------------------
void wow_restore(void)
{
	wow.revert(saveval);
}

PVOID wow_disable(void)
{
	PVOID p;
	wow.disable(&p);
	return p;
}

static void wow_disable_and_save(void)
{
	saveval = wow_disable();
}

//-----------------------------------------------------------------------------
static void init_hook(void);
static void WINAPI HookProc(PVOID h, DWORD dwReason, PVOID u)
{
	(void)h;
	(void)u;

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			init_hook();
		case DLL_THREAD_ATTACH:
			wow_disable_and_save();
		default:
			break;
	}
}

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_MSC_VER)
#if _MSC_VER < 1400 && !defined(LINK_WITH_ULINK)
#pragma message("VC8 or higher is strongly recommended")
#pragma data_seg(".CRT$XLY")
#else
#pragma const_seg(".CRT$XLY")
const
#endif
	PIMAGE_TLS_CALLBACK hook_wow64_tlscb = HookProc;
#pragma const_seg()
#else
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40404
const PIMAGE_TLS_CALLBACK hook_wow64_tlscb __attribute__((section(".CRT$XLY"))) = HookProc;
#else
ULONG __tls_index__ = 0;
char __tls_end__ __attribute__((section(".tls$zzz"))) = 0;
char __tls_start__ __attribute__((section(".tls"))) = 0;

PIMAGE_TLS_CALLBACK __crt_xl_start__ __attribute__((section(".CRT$XLA"))) = 0;
PIMAGE_TLS_CALLBACK hook_wow64_tlscb __attribute__((section(".CRT$XLY"))) = HookProc;
PIMAGE_TLS_CALLBACK __crt_xl_end__ __attribute__((section(".CRT$XLZ"))) = 0;

const IMAGE_TLS_DIRECTORY32 _tls_used __attribute__((section(".rdata$T"))) =
{
	(DWORD) &__tls_start__,
	(DWORD) &__tls_end__,
	(DWORD) &__tls_index__,
	(DWORD)(&__crt_xl_start__+1),
	(DWORD) 0,
	(DWORD) 0
};
#endif
#endif

#ifdef __cplusplus
}
#endif
// for ulink
#pragma comment(linker, "/include:_hook_wow64_tlscb")

//-----------------------------------------------------------------------------
static void
#if defined(__GNUC__)
// Unfortunatly GCC doesn't support this attribute on x86
//__attribute__((naked))
#else
__declspec(naked)
#endif
hook_ldr(void)
{
#if !defined(__GNUC__)
	__asm
	{
		call    wow_restore                                      // 5
		pop     edx             // real call                     // 1
		pop     eax             // real return                   // 1
		pop     ecx             // arg1                          // 1
		xchg    eax, [esp+8]    // real return <=> arg4          // 4
		xchg    eax, [esp+4]    // arg4 <=> arg3                 // 4
		xchg    eax, [esp]      // arg3 <=> arg2                 // 3
		push    eax             // arg2                          // 1
		push    ecx             // arg1                          // 1
		call    _l1                                              // 5
		push    eax     // answer                                // 1
		call    wow_disable                                      // 5
		pop     eax                                              // 1
		retn                                                     // 1
//-----
_l1:    push    240h                                             //+1 = 35
		jmp     edx
	}
#else
__asm__("call    _wow_restore \n\t"
        "popl    %edx         \n\t" // real call
        "popl    %eax         \n\t" // real return
        "popl    %ecx         \n\t" // arg1
        "xchg    8(%esp),%eax \n\t" // real return <=> arg4
        "xchg    4(%esp),%eax \n\t" // arg4 <=> arg3
        "xchg    (%esp),%eax  \n\t" // arg3 <=> arg2
        "pushl   %eax         \n\t" // arg2
        "pushl   %ecx         \n\t" // arg1
        "call    _l1          \n\t"
        "pushl   %eax         \n\t" // answer
        "call    _wow_disable \n\t"
        "popl    %eax         \n\t"
        "ret                  \n\t"
        "_l1:                 \n\t"
        "pushl   $0x240       \n\t"
        "jmp     *%edx");
#endif
#define HOOK_PUSH_OFFSET  35
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void init_hook(void)
{
	DWORD p;
	static const wchar_t k32_w[] = L"kernel32", ntd_w[] = L"ntdll";
	static const char dis_c[] = "Wow64DisableWow64FsRedirection",
	                  rev_c[] = "Wow64RevertWow64FsRedirection",
	                  wow_c[] = "IsWow64Process",
	                  ldr_c[] = "LdrLoadDll";
	WOW rwow;
	BOOL b=FALSE;
	ISWOW IsWow = NULL;
#pragma pack(1)
	struct
	{
		BYTE  cod;
		DWORD off;
	} data = { 0xE8, (DWORD)(SIZE_T)((LPCH)hook_ldr - sizeof(data)) };
#pragma pack()
	register union
	{
		HMODULE h;
		FARPROC f;
		LPVOID  p;
		DWORD   d;
	} ur;

	if ((ur.h = GetModuleHandleW(k32_w)) == NULL
	        || (IsWow = (ISWOW)GetProcAddress(ur.h, wow_c)) == NULL
	        || !(IsWow(GetCurrentProcess(), &b) && b)
	        || (rwow.disable = (DISABLE)GetProcAddress(ur.h, dis_c)) == NULL
	        || (rwow.revert = (REVERT)GetProcAddress(ur.h, rev_c)) == NULL
	        || (ur.h = GetModuleHandleW(ntd_w)) == NULL
	        || (ur.f = GetProcAddress(ur.h, ldr_c)) == NULL) return;

	if (*(LPBYTE)ur.p != 0x68     // push m32
	        && (*(LPDWORD)ur.p != 0x8B55FF8B || ((LPBYTE)ur.p)[4] != 0xEC)) return;

	// (Win2008-R2) mov edi, edi; push ebp; mov ebp, esp
	{
		DWORD   loff = *(LPDWORD)((LPBYTE)ur.p+1);
		LPBYTE  p_loff = (LPBYTE)&hook_ldr + HOOK_PUSH_OFFSET;

		if (loff != *(LPDWORD)p_loff)  // 0x240 in non vista, 0x244 in vista/2008
		{
			// don't use WriteProcessMemory here - BUG in 2003x64 32bit kernel32.dll :(
			if (!VirtualProtect(p_loff-1, 1+sizeof(loff), PAGE_EXECUTE_READWRITE, &p))
				return;

			if (*(LPBYTE)ur.p != 0x68)  // Win7r2 (not push .... => mov edi,edi)
			{
				((LPBYTE)p_loff)[-1] = 0x90;  // nop
				loff = 0xE5895590;  // nop; push ebp; mov ebp, esp
			}

			*(LPDWORD)p_loff = loff;
			VirtualProtect(p_loff-1, 1+sizeof(loff), p, (LPDWORD)&p_loff);
		}
	}
	data.off -= ur.d;

	if (!WriteProcessMemory(GetCurrentProcess(), ur.p, &data, sizeof(data), &data.off)
	        || data.off != sizeof(data)) return;

	// don't use WriteProcessMemory here - BUG in 2003x64 32bit kernel32.dll :(
	if (!VirtualProtect((void*)&wow, sizeof(wow), PAGE_EXECUTE_READWRITE, &data.off))
		return;

	*(WOW*)&wow = rwow;
	VirtualProtect((void*)&wow, sizeof(wow), data.off, &p);
}

#endif
