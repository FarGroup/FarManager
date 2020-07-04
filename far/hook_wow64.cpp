// validator: no-self-include
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

#if !defined(_WIN64)

// Internal:

// Platform:

// Common:
#include "common/compiler.hpp"
#include "common/preprocessor.hpp"

// External:

//----------------------------------------------------------------------------

static thread_local void* SavedState;

static const volatile struct wow
{
	static BOOL WINAPI e_disable(PVOID*) noexcept { return FALSE; }
	static BOOL WINAPI e_revert(PVOID) noexcept { return FALSE; }

	decltype(&Wow64DisableWow64FsRedirection) disable;
	decltype(&Wow64RevertWow64FsRedirection) revert;
}
Wow
{
	&wow::e_disable,
	&wow::e_revert,
};

static_assert(std::is_trivial_v<wow>);


#if COMPILER(GCC) || COMPILER(CLANG)
extern "C" void wow_restore() __asm__("wow_restore");
extern "C" void* wow_disable() __asm__("wow_disable");
#endif


extern "C" void wow_restore()
{
	Wow.revert(SavedState);
}


extern "C" void* wow_disable()
{
	void* p;
	Wow.disable(&p);
	return p;
}

#if COMPILER(GCC) || COMPILER(CLANG)
	#define NAKED __attribute__((naked))
#else
	#define NAKED __declspec(naked)
#endif

#if COMPILER(GCC)
	#define ASM_BLOCK_BEGIN __asm__(
	#define ASM_BLOCK_END );
	#define ASM(...) #__VA_ARGS__"\n"
#else
	#define ASM_BLOCK_BEGIN __asm {
	#define ASM_BLOCK_END }
	#define ASM(...) __VA_ARGS__
#endif

static void NAKED hook_ldr() noexcept
{
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
}

constexpr auto HookPushOffset = 35;

#undef ASM_BLOCK_BEGIN
#undef ASM_BLOCK_END
#undef ASM

#undef NAKED

template<auto Function>
static auto GetProcAddressImpl(HMODULE const Module, const char* const Name) noexcept
{
	return reinterpret_cast<decltype(Function)>(reinterpret_cast<void*>(GetProcAddress(Module, Name)));
}

#define GETPROCADDRESS(Module, Name) GetProcAddressImpl<&Name>(Module, #Name)

class remove_protection
{
public:
	NONCOPYABLE(remove_protection);

	remove_protection(void* const Address, DWORD const Size):
		m_Address(Address),
		m_Size(Size)
	{
		if (!VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &m_Protection))
			m_Address = {};
	}

	~remove_protection()
	{
		if (m_Address)
			VirtualProtect(m_Address, m_Size, m_Protection, &m_Protection);
	}

	explicit operator bool() const
	{
		return m_Address != nullptr;
	}

private:
	void* m_Address;
	DWORD m_Size;
	DWORD m_Protection;
};

static void init_hook() noexcept
{
	const auto Kernel32 = GetModuleHandle(L"kernel32");
	if (!Kernel32)
		return;

	BOOL IsWowValue;
	const auto IsWow = GETPROCADDRESS(Kernel32, IsWow64Process);
	if (!IsWow || !IsWow(GetCurrentProcess(), &IsWowValue) || !IsWowValue)
		return;

	const auto Disable = GETPROCADDRESS(Kernel32, Wow64DisableWow64FsRedirection);
	if (!Disable)
		return;

	const auto Revert = GETPROCADDRESS(Kernel32, Wow64RevertWow64FsRedirection);
	if (!Revert)
		return;

	const auto Ntdll = GetModuleHandle(L"ntdll");
	if (!Ntdll)
		return;

	const auto LdrLoadDll = GetProcAddress(Ntdll, "LdrLoadDll");
	if (!LdrLoadDll)
		return;

	const auto FunctionData = reinterpret_cast<BYTE*>(LdrLoadDll);

	if (!
		(
			// push m32
			FunctionData[0] == 0x68
		||
		(
			// (Win2008-R2) mov edi, edi; push ebp; mov ebp, esp
			FunctionData[0] == 0x8b &&
			FunctionData[1] == 0xff &&
			FunctionData[2] == 0x55 &&
			FunctionData[3] == 0x8b &&
			FunctionData[4] == 0xEC
		))
	)
		return;

	auto loff = *reinterpret_cast<DWORD const*>(FunctionData + 1);
	const auto p_loff = reinterpret_cast<BYTE*>(&hook_ldr) + HookPushOffset;

	if (loff != *reinterpret_cast<DWORD const*>(p_loff))  // 0x240 in non vista, 0x244 in vista/2008
	{
		if (SCOPED_ACTION(remove_protection){ p_loff - 1, sizeof(loff) + 1 })
		{
			if (FunctionData[0] != 0x68)   // Win7r2 (not push .... => mov edi,edi)
			{
				p_loff[-1] = 0x90;  // nop
				loff = 0xE5895590;  // nop; push ebp; mov ebp, esp
			}

			*reinterpret_cast<DWORD*>(p_loff) = loff;
		}
		else
			return;
	}

	PACK_PUSH(1)
	const struct
	{
		BYTE  cod;
		DWORD off;
	}
	Data
	{
		0xE8,
		reinterpret_cast<DWORD>(hook_ldr) - sizeof(Data) - reinterpret_cast<DWORD>(LdrLoadDll)
	};
	PACK_POP()

	if (SCOPED_ACTION(remove_protection){ FunctionData, sizeof(Data) })
	{
		std::memcpy(FunctionData, &Data, sizeof(Data));
	}
	else
		return;

	if (SCOPED_ACTION(remove_protection){ const_cast<wow*>(&Wow), sizeof(Wow) })
	{
		auto& MutableWow = const_cast<volatile wow&>(Wow);
		MutableWow.disable = Disable;
		MutableWow.revert = Revert;
	}
}


static void WINAPI HookProc(void*, DWORD dwReason, void*) noexcept
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		init_hook();
		[[fallthrough]];
	case DLL_THREAD_ATTACH:
		SavedState = wow_disable();
		break;

	default:
		break;
	}
}

WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wmissing-variable-declarations")

#if COMPILER(CL) || COMPILER(INTEL)
#pragma const_seg(".CRT$XLY")
#endif
extern "C" const PIMAGE_TLS_CALLBACK hook_wow64_tlscb
#if COMPILER(GCC) || COMPILER(CLANG)
__attribute__((section(".CRT$XLY")))
#endif
= HookProc;
#if COMPILER(CL) || COMPILER(INTEL)
#pragma const_seg()
#endif

WARNING_POP()

// for ulink
#pragma comment(linker, "/include:_hook_wow64_tlscb")

#endif
