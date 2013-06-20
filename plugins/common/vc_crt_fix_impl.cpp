/*
vc_crt_fix_impl.cpp

;Workaround for Visual C++ CRT inncompability with old Windows versions
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

#include <windows.h>

// EncodePointer / DecodePointer (VC2010)
static PVOID WINAPI ReturnSamePointer(PVOID Ptr)
{
	return Ptr;
}

template<class T>
static PVOID WINAPI Wrapper(PVOID Ptr)
{
	typedef PVOID (WINAPI *PointerFunction)(PVOID);
	static PVOID FunctionAddress = GetProcAddress(GetModuleHandleW(L"kernel32"), T::get());
	static PointerFunction ProcessPointer = FunctionAddress? reinterpret_cast<PointerFunction>(FunctionAddress) : ReturnSamePointer;
	return ProcessPointer(Ptr);
}

#define STRING_OBJECT(name, string) struct name {static const char* get() {return string;}}
extern "C"
{
	PVOID WINAPI EncodePointerWrapper(PVOID Ptr) {STRING_OBJECT(EncodePointerName, "EncodePointer"); return Wrapper<EncodePointerName>(Ptr);}
	PVOID WINAPI DecodePointerWrapper(PVOID Ptr) {STRING_OBJECT(DecodePointerName, "DecodePointer"); return Wrapper<DecodePointerName>(Ptr);}
}


// GetModuleHandleExW (VC2012)
static BOOL WINAPI GetModuleHandleExWImpl(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
{
	BOOL Result = FALSE;
	if (Flags)
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}
	else
	{
		if ((*Module = GetModuleHandleW(ModuleName)))
		{
			Result = TRUE;
		}
	}
	return Result;
}

extern "C" BOOL WINAPI GetModuleHandleExWWrapper(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
{
	typedef BOOL (WINAPI *GetModuleHandleExWFunction)(DWORD, LPCWSTR, HMODULE*);
	static PVOID FunctionAddress = GetProcAddress(GetModuleHandleW(L"kernel32"), "GetModuleHandleExW");
	static GetModuleHandleExWFunction ProcessGetModuleHandleExW = FunctionAddress? reinterpret_cast<GetModuleHandleExWFunction>(FunctionAddress) : GetModuleHandleExWImpl;
	return ProcessGetModuleHandleExW(Flags, ModuleName, Module);
}
