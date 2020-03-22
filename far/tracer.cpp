﻿/*
tracer.cpp
*/
/*
Copyright © 2016 Far Group
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

// Self:
#include "tracer.hpp"

// Internal:
#include "imports.hpp"
#include "encoding.hpp"
#include "pathmix.hpp"
#include "exception.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------


// StackWalk64() may modify context record passed to it, so we will use a copy.
static auto GetBackTrace(CONTEXT ContextRecord, HANDLE ThreadHandle)
{
	std::vector<const void*> Result;

#if defined _M_X64 || defined _M_IX86
	STACKFRAME64 StackFrame{};
	const DWORD MachineType =
#ifdef _WIN64
		IMAGE_FILE_MACHINE_AMD64;
	StackFrame.AddrPC.Offset = ContextRecord.Rip;
	StackFrame.AddrFrame.Offset = ContextRecord.Rbp;
	StackFrame.AddrStack.Offset = ContextRecord.Rsp;
#else
		IMAGE_FILE_MACHINE_I386;
	StackFrame.AddrPC.Offset = ContextRecord.Eip;
	StackFrame.AddrFrame.Offset = ContextRecord.Ebp;
	StackFrame.AddrStack.Offset = ContextRecord.Esp;
#endif
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Mode = AddrModeFlat;

	while (imports.StackWalk64(MachineType, GetCurrentProcess(), ThreadHandle, &StackFrame, &ContextRecord, nullptr, imports.SymFunctionTableAccess64, imports.SymGetModuleBase64, nullptr))
	{
		Result.emplace_back(reinterpret_cast<const void*>(StackFrame.AddrPC.Offset));
	}
#endif

	return Result;
}

static void GetSymbols(const std::vector<const void*>& BackTrace, function_ref<void(string&&, string&&, string&&)> const Consumer)
{
	SCOPED_ACTION(tracer::with_symbols);

	const auto Process = GetCurrentProcess();
	const auto MaxNameLen = MAX_SYM_NAME;
	const auto BufferSize = sizeof(SYMBOL_INFO) + MaxNameLen + 1;

	imports.SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);

	IMAGEHLP_MODULEW64 Module{ static_cast<DWORD>(aligned_size(offsetof(IMAGEHLP_MODULEW64, LoadedImageName), 8)) }; // use the pre-07-Jun-2002 struct size, aligned to 8

	const auto MaxAddressSize = format(FSTR(L"{0:0X}"), reinterpret_cast<uintptr_t>(*std::max_element(ALL_CONST_RANGE(BackTrace)))).size();

	block_ptr<SYMBOL_INFO, BufferSize> Symbol(BufferSize);
	Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen = MaxNameLen;

	block_ptr<SYMBOL_INFOW, BufferSize> SymbolW(BufferSize);
	SymbolW->SizeOfStruct = sizeof(SYMBOL_INFOW);
	SymbolW->MaxNameLen = MaxNameLen;

	const auto GetName = [&](DWORD_PTR const Address) -> string
	{
		if (imports.SymFromAddrW && imports.SymFromAddrW(Process, Address, nullptr, SymbolW.data()))
			return SymbolW->Name;

		if (imports.SymFromAddr && imports.SymFromAddr(Process, Address, nullptr, Symbol.data()))
			return encoding::ansi::get_chars(Symbol->Name);

		return L"<unknown> (get the pdb)"s;
	};

	const auto GetLocation = [&](DWORD_PTR const Address) -> string
	{
		DWORD Displacement;

		IMAGEHLP_LINEW64 LineW{ sizeof(LineW) };
		if (imports.SymGetLineFromAddrW64 && imports.SymGetLineFromAddrW64(Process, Address, &Displacement, &LineW))
			return format(FSTR(L"{0}:{1}"), LineW.FileName, LineW.LineNumber);

		IMAGEHLP_LINE64 Line{ sizeof(Line) };
		if (imports.SymGetLineFromAddr64 && imports.SymGetLineFromAddr64(Process, Address, &Displacement, &Line))
			return format(FSTR(L"{0}:{1}"), encoding::ansi::get_chars(Line.FileName), Line.LineNumber);

		return {};
	};

	for (const auto i: BackTrace)
	{
		const auto Address = reinterpret_cast<DWORD_PTR>(i);
		string sFunction, sLocation;
		if (Address)
		{
			sFunction = concat(
				imports.SymGetModuleInfoW64(Process, Address, &Module)?
					PointToName(Module.ImageName) :
					L"<unknown>"sv,
				L'!',
				GetName(Address));

			sLocation = GetLocation(Address);
		}

		Consumer(format(FSTR(L"0x{0:0{1}X}"), Address, MaxAddressSize), std::move(sFunction), std::move(sLocation));
	}
}

#if IS_MICROSOFT_SDK()
extern "C" void** __current_exception();
extern "C" void** __current_exception_context();
#else
static void** __current_exception()
{
	static EXCEPTION_RECORD DummyRecord{};
	static void* DummyRecordPtr = &DummyRecord;
	return &DummyRecordPtr;
}

static void** __current_exception_context()
{
	static CONTEXT DummyContext{};
	static void* DummyContextPtr = &DummyContext;
	return &DummyContextPtr;
}
#endif

EXCEPTION_POINTERS tracer::get_pointers()
{
	return
	{
		static_cast<EXCEPTION_RECORD*>(*__current_exception()),
		static_cast<CONTEXT*>(*__current_exception_context())
	};
}

std::vector<const void*> tracer::get(const EXCEPTION_POINTERS& Pointers, HANDLE ThreadHandle)
{
	SCOPED_ACTION(tracer::with_symbols);

	return GetBackTrace(*Pointers.ContextRecord, ThreadHandle);
}

void tracer::get_symbols(const std::vector<const void*>& Trace, function_ref<void(string&& Address, string&& Name, string&& Source)> const Consumer)
{
	GetSymbols(Trace, Consumer);
}

void tracer::get_symbol(const void* Ptr, string& Address, string& Name, string& Source)
{
	GetSymbols({Ptr}, [&](string&& StrAddress, string&& StrName, string&& StrSource)
	{
		Address = std::move(StrAddress);
		Name = std::move(StrName);
		Source = std::move(StrSource);
	});
}

static int s_SymInitialised = 0;

void tracer::sym_initialise()
{
	if (s_SymInitialised)
	{
		++s_SymInitialised;
		return;
	}

	string Path;
	if (!os::fs::GetModuleFileName(nullptr, nullptr, Path))
		throw MAKE_FAR_FATAL_EXCEPTION(L"GetModuleFileName failed"sv);

	CutToParent(Path);

	if (
		(imports.SymInitializeW && imports.SymInitializeW(GetCurrentProcess(), EmptyToNull(Path), TRUE)) ||
		(imports.SymInitialize && imports.SymInitialize(GetCurrentProcess(), EmptyToNull(encoding::ansi::get_bytes(Path)), TRUE))
	)
		++s_SymInitialised;
}

void tracer::sym_cleanup()
{
	if (s_SymInitialised)
		--s_SymInitialised;

	if (!s_SymInitialised)
		imports.SymCleanup(GetCurrentProcess());
}
