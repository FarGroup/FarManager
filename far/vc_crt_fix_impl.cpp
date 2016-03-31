/*
vc_crt_fix_impl.cpp

Workaround for Visual C++ CRT incompatibility with old Windows versions
*/
/*
Copyright © 2011 Far Group
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

#ifndef FAR_USE_INTERNALS
#define FAR_USE_INTERNALS
#endif // END FAR_USE_INTERNALS
#ifdef FAR_USE_INTERNALS
#include "disable_warnings_in_std_begin.hpp"
//----------------------------------------------------------------------------
#endif // END FAR_USE_INTERNALS
#include <windows.h>
#ifdef FAR_USE_INTERNALS
//----------------------------------------------------------------------------
#include "disable_warnings_in_std_end.hpp"
#endif // END FAR_USE_INTERNALS

template<typename T>
T GetFunctionPointer(const wchar_t* ModuleName, const char* FunctionName, T Replacement)
{
	const auto Address = GetProcAddress(GetModuleHandleW(ModuleName), FunctionName);
	return Address? reinterpret_cast<T>(Address) : Replacement;
}

#define CREATE_FUNCTION_POINTER(ModuleName, FunctionName)\
static const auto Function = GetFunctionPointer(ModuleName, #FunctionName, &implementation::FunctionName)

static const wchar_t kernel32[] = L"kernel32";

// EncodePointer (VC2010)
extern "C" PVOID WINAPI EncodePointerWrapper(PVOID Ptr)
{
	struct implementation
	{
		static PVOID WINAPI EncodePointer(PVOID Ptr) { return Ptr; }
	};

	CREATE_FUNCTION_POINTER(kernel32, EncodePointer);
	return Function(Ptr);
}

// DecodePointer(VC2010)
extern "C" PVOID WINAPI DecodePointerWrapper(PVOID Ptr)
{
	struct implementation
	{
		static PVOID WINAPI DecodePointer(PVOID Ptr) { return Ptr; }
	};

	CREATE_FUNCTION_POINTER(kernel32, DecodePointer);
	return Function(Ptr);
}

// GetModuleHandleExW (VC2012)
extern "C" BOOL WINAPI GetModuleHandleExWWrapper(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
{
	struct implementation
	{
		static BOOL WINAPI GetModuleHandleExW(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
		{
			BOOL Result = FALSE;
			if (Flags)
			{
				SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			}
			else
			{
				*Module = GetModuleHandleW(ModuleName);
				if (*Module)
				{
					Result = TRUE;
				}
			}
			return Result;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, GetModuleHandleExW);
	return Function(Flags, ModuleName, Module);
}

// InitializeSListHead (VC2015)
extern "C" void WINAPI InitializeSListHeadWrapper(PSLIST_HEADER ListHead)
{
	struct implementation
	{
		static void WINAPI InitializeSListHead(PSLIST_HEADER ListHead)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, InitializeSListHead);
	return Function(ListHead);
}

// InterlockedFlushSList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedFlushSListWrapper(PSLIST_HEADER ListHead)
{
	struct implementation
	{
		static PSLIST_ENTRY WINAPI InterlockedFlushSList(PSLIST_HEADER ListHead)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, InterlockedFlushSList);
	return Function(ListHead);
}

// InterlockedPopEntrySList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedPopEntrySListWrapper(PSLIST_HEADER ListHead)
{
	struct implementation
	{
		static PSLIST_ENTRY WINAPI InterlockedPopEntrySList(PSLIST_HEADER ListHead)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, InterlockedPopEntrySList);
	return Function(ListHead);
}

// InterlockedPushEntrySList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedPushEntrySListWrapper(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
{
	struct implementation
	{
		static PSLIST_ENTRY WINAPI InterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, InterlockedPushEntrySList);
	return Function(ListHead, ListEntry);
}

// InterlockedPushListSList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedPushListSListExWrapper(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count)
{
	struct implementation
	{
		static PSLIST_ENTRY WINAPI InterlockedPushListSListEx(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, InterlockedPushListSListEx);
	return Function(ListHead, List, ListEnd, Count);
}

// RtlFirstEntrySList (VC2015)
extern "C" PSLIST_ENTRY WINAPI RtlFirstEntrySListWrapper(PSLIST_HEADER ListHead)
{
	struct implementation
	{
		static PSLIST_ENTRY WINAPI RtlFirstEntrySList(PSLIST_HEADER ListHead)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, RtlFirstEntrySList);
	return Function(ListHead);
}

// QueryDepthSList (VC2015)
extern "C" USHORT WINAPI QueryDepthSListWrapper(PSLIST_HEADER ListHead)
{
	struct implementation
	{
		static USHORT WINAPI QueryDepthSList(PSLIST_HEADER ListHead)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return 0;
		}
	};

	CREATE_FUNCTION_POINTER(kernel32, QueryDepthSList);
	return Function(ListHead);
}

// RtlGenRandom (VC2015)
// RtlGenRandom is used in rand_s implementation in ucrt.
// As long as we don't use the rand_s in the code (which is easy: it's non-standard, requires _CRT_RAND_S to be defined first and not recommended in general)
// this function is never called so it's ok to provide this dummy implementation only to have the _SystemFunction036@8 symbol in binary to make their loader happy.
extern "C" BOOLEAN WINAPI SystemFunction036(PVOID Buffer, ULONG Size)
{
	return TRUE;
}
