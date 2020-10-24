/*
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "tracer.hpp"

// Internal:
#include "imports.hpp"
#include "encoding.hpp"
#include "pathmix.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static auto platform_specific_data(CONTEXT const& ContextRecord)
{
	const struct
	{
		DWORD MachineType;
		DWORD64 PC, Frame, Stack;
	}
	Data
	{
#if defined _M_X64
		IMAGE_FILE_MACHINE_AMD64,
		ContextRecord.Rip,
		ContextRecord.Rbp,
		ContextRecord.Rsp
#elif defined _M_IX86
		IMAGE_FILE_MACHINE_I386,
		ContextRecord.Eip,
		ContextRecord.Ebp,
		ContextRecord.Esp
#elif defined _M_ARM64
		IMAGE_FILE_MACHINE_ARM64,
		ContextRecord.Pc,
		ContextRecord.Fp,
		ContextRecord.Sp
#elif defined _M_ARM
		IMAGE_FILE_MACHINE_ARM,
		ContextRecord.Pc,
		ContextRecord.R11,
		ContextRecord.Sp
#else
		IMAGE_FILE_MACHINE_UNKNOWN
#endif
	};

	return Data;
}

// StackWalk64() may modify context record passed to it, so we will use a copy.
static auto GetBackTrace(CONTEXT ContextRecord, HANDLE ThreadHandle)
{
	std::vector<DWORD64> Result;

	const auto Data = platform_specific_data(ContextRecord);

	if (Data.MachineType == IMAGE_FILE_MACHINE_UNKNOWN || (!Data.PC && !Data.Frame && !Data.Stack))
		return Result;

	const auto address = [](DWORD64 const Offset)
	{
		return ADDRESS64{ Offset, 0, AddrModeFlat };
	};

	STACKFRAME64 StackFrame{};
	StackFrame.AddrPC    = address(Data.PC);
	StackFrame.AddrFrame = address(Data.Frame);
	StackFrame.AddrStack = address(Data.Stack);

	while (imports.StackWalk64(Data.MachineType, GetCurrentProcess(), ThreadHandle, &StackFrame, &ContextRecord, nullptr, imports.SymFunctionTableAccess64, imports.SymGetModuleBase64, nullptr))
	{
		Result.emplace_back(StackFrame.AddrPC.Offset);
	}

	return Result;
}

static void GetSymbols(string_view const ModuleName, span<DWORD64 const> const BackTrace, function_ref<void(string&&, string&&, string&&)> const Consumer)
{
	SCOPED_ACTION(tracer::with_symbols)(ModuleName);

	const auto Process = GetCurrentProcess();
	const auto MaxNameLen = MAX_SYM_NAME;
	const auto BufferSize = sizeof(SYMBOL_INFO) + MaxNameLen + 1;

	imports.SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);

	block_ptr<SYMBOL_INFOW, BufferSize> SymbolW(BufferSize);
	SymbolW->SizeOfStruct = sizeof(SYMBOL_INFOW);
	SymbolW->MaxNameLen = MaxNameLen;

	block_ptr<SYMBOL_INFO, BufferSize> Symbol(BufferSize);
	Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen = MaxNameLen;

	const auto FormatAddress = [](DWORD64 const Value)
	{
		// It is unlikely that RVAs will be above 4 GiB,
		// so we can save some screen space here.
		return format(FSTR(L"0x{0:0{1}X}"), Value, (Value & 0xffffffff00000000)? 16 : 8);
	};

	const auto GetName = [&](DWORD64 const Address)
	{
		if (imports.SymFromAddrW && imports.SymFromAddrW(Process, Address, nullptr, SymbolW.data()))
			return string(SymbolW->Name);

		if (imports.SymFromAddr && imports.SymFromAddr(Process, Address, nullptr, Symbol.data()))
			return encoding::ansi::get_chars(Symbol->Name);

		return L"<unknown> (get the pdb)"s;
	};

	const auto GetLocation = [&](DWORD64 const Address)
	{
		const auto Location = [](string_view const File, unsigned const Line)
		{
			return format(FSTR(L"{0}:{1}"), File, Line);
		};

		DWORD Displacement;

		IMAGEHLP_LINEW64 LineW{ sizeof(LineW) };
		if (imports.SymGetLineFromAddrW64 && imports.SymGetLineFromAddrW64(Process, Address, &Displacement, &LineW))
			return Location(LineW.FileName, LineW.LineNumber);

		IMAGEHLP_LINE64 Line{ sizeof(Line) };
		if (imports.SymGetLineFromAddr64 && imports.SymGetLineFromAddr64(Process, Address, &Displacement, &Line))
			return Location(encoding::ansi::get_chars(Line.FileName), Line.LineNumber);

		return L""s;
	};

	for (const auto Address: BackTrace)
	{
		IMAGEHLP_MODULEW64 Module{ static_cast<DWORD>(aligned_size(offsetof(IMAGEHLP_MODULEW64, LoadedImageName), 8)) }; // use the pre-07-Jun-2002 struct size, aligned to 8
		const auto HasModuleInfo = imports.SymGetModuleInfoW64(Process, Address, &Module);

		Consumer(
			FormatAddress(Address - Module.BaseOfImage),
			Address? concat(HasModuleInfo? PointToName(Module.ImageName) : L"<unknown>"sv, L'!', GetName(Address)) : L""s,
			GetLocation(Address)
		);
	}
}

std::vector<DWORD64> tracer::get(string_view const Module, const EXCEPTION_POINTERS& Pointers, HANDLE ThreadHandle)
{
	SCOPED_ACTION(tracer::with_symbols)(Module);

	return GetBackTrace(*Pointers.ContextRecord, ThreadHandle);
}

void tracer::get_symbols(string_view const Module, span<DWORD64 const> const Trace, function_ref<void(string&& Line)> const Consumer)
{
	GetSymbols(Module, Trace, [&](string&& Address, string&& Name, string&& Source)
	{
		if (!Name.empty())
			append(Address, L' ', Name);

		if (!Source.empty())
			append(Address, L" ("sv, Source, L')');

		Consumer(std::move(Address));
	});
}

void tracer::get_symbol(string_view const Module, const void* Ptr, string& Address, string& Name, string& Source)
{
	DWORD64 const Stack[]{ reinterpret_cast<DWORD_PTR>(Ptr) };
	GetSymbols(Module, Stack, [&](string&& StrAddress, string&& StrName, string&& StrSource)
	{
		Address = std::move(StrAddress);
		Name = std::move(StrName);
		Source = std::move(StrSource);
	});
}

static int s_SymInitialised = 0;

void tracer::sym_initialise(string_view Module)
{
	if (s_SymInitialised)
	{
		++s_SymInitialised;
		return;
	}

	string Path;
	(void)os::fs::GetModuleFileName(nullptr, nullptr, Path);
	CutToParent(Path);

	if (!Module.empty())
	{
		CutToParent(Module);
		append(Path, L';', Module);
	}

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
