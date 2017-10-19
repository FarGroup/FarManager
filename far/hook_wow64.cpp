/*
hook_wow64.cpp
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

#if (defined(_MSC_VER) || defined(__GNUC__)) && !defined(_WIN64)
#include "disable_warnings_in_std_begin.hpp"
#include <windows.h>
#include "disable_warnings_in_std_end.hpp"

#include "common/compiler.hpp"

WARNING_DISABLE_GCC("-Wcast-qual")

//-----------------------------------------------------------------------------
static thread_local PVOID saveval;

using DISABLE = decltype(&Wow64DisableWow64FsRedirection);
using REVERT = decltype(&Wow64RevertWow64FsRedirection);

static const volatile struct WOW
{
	static BOOL WINAPI e_disable(PVOID*) { return FALSE; }
	static BOOL WINAPI e_revert(PVOID) { return FALSE; }

	DISABLE disable { e_disable };
	REVERT revert { e_revert };
}
wow;

//-----------------------------------------------------------------------------

#if COMPILER == C_GCC || COMPILER == C_CLANG
extern "C" void wow_restore() __asm__("wow_restore");
extern "C" PVOID wow_disable() __asm__("wow_disable");
#endif

extern "C" void wow_restore()
{
	wow.revert(saveval);
}

extern "C" PVOID wow_disable()
{
	PVOID p;
	wow.disable(&p);
	return p;
}

//-----------------------------------------------------------------------------
static void
#if COMPILER == C_GCC || COMPILER == C_CLANG
// Unfortunately GCC doesn't support this attribute on x86
//__attribute__((naked))
#else
__declspec(naked)
#endif
hook_ldr()
{
#ifdef __GNUC__
#define ASM_BLOCK_BEGIN __asm__(
#define ASM_BLOCK_END );
#define ASM(...) #__VA_ARGS__"\n"
#else
#define ASM_BLOCK_BEGIN __asm {
#define ASM_BLOCK_END }
#define ASM(...) __VA_ARGS__
#endif

	ASM_BLOCK_BEGIN
		ASM(call    wow_restore )                                  // 5
		ASM(pop     edx         ) // real call                     // 1
		ASM(pop     eax         ) // real return                   // 1
		ASM(pop     ecx         ) // arg1                          // 1
		ASM(xchg    eax, [esp+8]) // real return <=> arg4          // 4
		ASM(xchg    eax, [esp+4]) // arg4 <=> arg3                 // 4
		ASM(xchg    eax, [esp]  ) // arg3 <=> arg2                 // 3
		ASM(push    eax         ) // arg2                          // 1
		ASM(push    ecx         ) // arg1                          // 1
		ASM(call    _l1         )                                  // 5
		ASM(push    eax         ) // answer                        // 1
		ASM(call    wow_disable )                                  // 5
		ASM(pop     eax         )                                  // 1
		ASM(ret                 )                                  // 1
		ASM(_l1:                )
		ASM(push    0x240       )                                  //+1 = 35
		ASM(jmp     edx         )
	ASM_BLOCK_END

#define HOOK_PUSH_OFFSET  35

#undef ASM_BLOCK_BEGIN
#undef ASM_BLOCK_END
#undef ASM
}

//-----------------------------------------------------------------------------
static void init_hook()
{
	DWORD p;
	static const wchar_t
		k32_w[] = L"kernel32",
		ntd_w[] = L"ntdll";

	static const char
		dis_c[] = "Wow64DisableWow64FsRedirection",
		rev_c[] = "Wow64RevertWow64FsRedirection",
		wow_c[] = "IsWow64Process",
		ldr_c[] = "LdrLoadDll";

	WOW rwow;
	BOOL b=FALSE;

	using ISWOW = decltype(&IsWow64Process);

	ISWOW IsWow = nullptr;

PACK_PUSH(1)
	struct
	{
		BYTE  cod { 0xE8 };
		DWORD off { static_cast<DWORD>(reinterpret_cast<uintptr_t>(reinterpret_cast<char*>(hook_ldr) - sizeof(*this))) };
	}
	data;
PACK_POP()

union
	{
		HMODULE h;
		FARPROC f;
		LPVOID  p;
		DWORD   d;
	} ur;

	if ((ur.h = GetModuleHandleW(k32_w)) == nullptr
	        || (IsWow = reinterpret_cast<ISWOW>(reinterpret_cast<void*>(GetProcAddress(ur.h, wow_c)))) == nullptr
	        || !(IsWow(GetCurrentProcess(), &b) && b)
	        || (rwow.disable = reinterpret_cast<DISABLE>(reinterpret_cast<void*>(GetProcAddress(ur.h, dis_c)))) == nullptr
	        || (rwow.revert = reinterpret_cast<REVERT>(reinterpret_cast<void*>(GetProcAddress(ur.h, rev_c)))) == nullptr
	        || (ur.h = GetModuleHandleW(ntd_w)) == nullptr
	        || (ur.f = GetProcAddress(ur.h, ldr_c)) == nullptr) return;

	if (*static_cast<LPBYTE>(ur.p) != 0x68     // push m32
		&& (*static_cast<LPDWORD>(ur.p) != 0x8B55FF8B || static_cast<LPBYTE>(ur.p)[4] != 0xEC))
			return;

	// (Win2008-R2) mov edi, edi; push ebp; mov ebp, esp
	{
		auto loff = *reinterpret_cast<LPDWORD>(static_cast<LPBYTE>(ur.p) + 1);
		auto p_loff = reinterpret_cast<LPBYTE>(&hook_ldr) + HOOK_PUSH_OFFSET;

		if (loff != *reinterpret_cast<LPDWORD>(p_loff))  // 0x240 in non vista, 0x244 in vista/2008
		{
			// don't use WriteProcessMemory here - BUG in 2003x64 32bit kernel32.dll :(
			if (!VirtualProtect(p_loff-1, 1+sizeof(loff), PAGE_EXECUTE_READWRITE, &p))
				return;

			if (*static_cast<LPBYTE>(ur.p) != 0x68)  // Win7r2 (not push .... => mov edi,edi)
			{
				static_cast<LPBYTE>(p_loff)[-1] = 0x90;  // nop
				loff = 0xE5895590;  // nop; push ebp; mov ebp, esp
			}

			*reinterpret_cast<LPDWORD>(p_loff) = loff;
			VirtualProtect(p_loff-1, 1+sizeof(loff), p, reinterpret_cast<LPDWORD>(&p_loff));
		}
	}
	data.off -= ur.d;

	if (!WriteProcessMemory(GetCurrentProcess(), ur.p, &data, sizeof(data), &data.off)
	        || data.off != sizeof(data)) return;

	// don't use WriteProcessMemory here - BUG in 2003x64 32bit kernel32.dll :(
	if (!VirtualProtect(const_cast<WOW*>(&wow), sizeof(wow), PAGE_EXECUTE_READWRITE, &data.off))
		return;

	const_cast<WOW&>(wow) = rwow;
	VirtualProtect(const_cast<WOW*>(&wow), sizeof(wow), data.off, &p);
}

//-----------------------------------------------------------------------------
static void WINAPI HookProc(PVOID, DWORD dwReason, PVOID)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		init_hook();
	case DLL_THREAD_ATTACH:
		saveval = wow_disable();
	default:
		break;
	}
}

#if COMPILER == C_CL || COMPILER == C_INTEL
#pragma const_seg(".CRT$XLY")
#endif
extern "C" const PIMAGE_TLS_CALLBACK hook_wow64_tlscb
#if COMPILER == C_GCC || COMPILER == C_CLANG
__attribute__((section(".CRT$XLY")))
#endif
= HookProc;
#if COMPILER == C_CL || COMPILER == C_INTEL
#pragma const_seg()
#endif

// for ulink
#pragma comment(linker, "/include:_hook_wow64_tlscb")

#endif
