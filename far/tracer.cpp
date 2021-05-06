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
#include "log.hpp"

// Platform:
#include "platform.fs.hpp"
#include "platform.concurrency.hpp"

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
	std::vector<uintptr_t> Result;

	if (!imports.StackWalk64)
		return Result;

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
		// Cast to uintptr_t is ok here: although this function can be used
		// to capture a stack of 64-bit process from a 32-bit one,
		// we always use it with the current process only.
		Result.emplace_back(static_cast<uintptr_t>(StackFrame.AddrPC.Offset));
	}

	return Result;
}

// SYMBOL_INFO_PACKAGEW not defined in GCC headers :(
namespace
{
	template<typename header>
	struct package
	{
		header info;
		std::remove_all_extents_t<decltype(header::Name)> name[MAX_SYM_NAME + 1];
	};
}

static void get_symbols_impl(string_view const ModuleName, span<uintptr_t const> const BackTrace, function_ref<void(string&&, string&&, string&&)> const Consumer)
{
	const auto Process = GetCurrentProcess();

	const auto FormatAddress = [](uintptr_t const Value)
	{
		// It is unlikely that RVAs will be above 4 GiB,
		// so we can save some screen space here.
		const auto Width =
#ifdef _WIN64
			Value & 0xffffffff00000000? 16 : 8
#else
			8
#endif
			;

		return format(FSTR(L"0x{:0{}X}"sv), Value, Width);
	};

	std::variant
	<
		package<SYMBOL_INFOW>,
		package<SYMBOL_INFO>,
		package<IMAGEHLP_SYMBOL64>
	>
	SymbolData;

	string NameBuffer;

	const auto GetName = [&](uintptr_t const Address) -> string_view
	{
		const auto Get = [&](auto const& Getter, auto& Buffer, auto& To)
		{
			if (!Getter)
				return false;

			Buffer.info.SizeOfStruct = sizeof(Buffer.info);
			Buffer.info.MaxNameLen = MAX_SYM_NAME;

			if (!Getter(Process, Address, {}, &Buffer.info))
				return false;

			// Old dbghelp versions (e.g. XP) not always populate NameLen
			To = { Buffer.info.Name, Buffer.info.NameLen? Buffer.info.NameLen : std::char_traits<VALUE_TYPE(To)>::length(Buffer.info.Name) };
			return true;
		};

		if (string_view Name; Get(imports.SymFromAddrW, SymbolData.emplace<0>(), Name))
			return Name;

		if (std::string_view Name; Get(imports.SymFromAddr, SymbolData.emplace<1>(), Name))
			return NameBuffer = encoding::ansi::get_chars(Name);

		// This one is for Win2k, which doesn't have SymFromAddr.
		// However, I couldn't make it work with the out-of-the-box version.
		// Get a newer dbghelp.dll if you need traces there:
		// http://download.microsoft.com/download/A/6/A/A6AC035D-DA3F-4F0C-ADA4-37C8E5D34E3D/setup/WinSDKDebuggingTools/dbg_x86.msi
		{
			auto& Symbol = SymbolData.emplace<2>();
			Symbol.info.SizeOfStruct = sizeof(Symbol.info);
			Symbol.info.MaxNameLength = MAX_SYM_NAME;

			if (DWORD64 Displacement; imports.SymGetSymFromAddr64(Process, Address, &Displacement, &Symbol.info))
			{
				NameBuffer = encoding::ansi::get_chars(Symbol.info.Name);
				return NameBuffer;
			}
		}

		return L"<unknown> (get the pdb)"sv;
	};

	const auto GetLocation = [&](uintptr_t const Address)
	{
		const auto Get = [&](auto const& Getter, auto& Buffer)
		{
			Buffer.SizeOfStruct = sizeof(Buffer);
			DWORD Displacement;
			return Getter && Getter(Process, Address, &Displacement, &Buffer);
		};

		const auto Location = [](string_view const File, unsigned const Line)
		{
			return format(FSTR(L"{}({})"sv), File, Line);
		};

		if (IMAGEHLP_LINEW64 Line; Get(imports.SymGetLineFromAddrW64, Line))
			return Location(Line.FileName, Line.LineNumber);

		if (IMAGEHLP_LINE64 Line; Get(imports.SymGetLineFromAddr64, Line))
			return Location(encoding::ansi::get_chars(Line.FileName), Line.LineNumber);

		return L""s;
	};

	for (const auto Address: BackTrace)
	{
		IMAGEHLP_MODULEW64 Module{ static_cast<DWORD>(aligned_size(offsetof(IMAGEHLP_MODULEW64, LoadedImageName), 8)) }; // use the pre-07-Jun-2002 struct size, aligned to 8
		const auto HasModuleInfo = imports.SymGetModuleInfoW64 && imports.SymGetModuleInfoW64(Process, Address, &Module);

		Consumer(
			FormatAddress(Address - Module.BaseOfImage),
			Address? concat(HasModuleInfo? PointToName(Module.ImageName) : L"<unknown>"sv, L'!', GetName(Address)) : L""s,
			GetLocation(Address)
		);
	}
}

std::vector<uintptr_t> tracer_detail::tracer::get(string_view const Module, CONTEXT const& ContextRecord, HANDLE ThreadHandle)
{
	SCOPED_ACTION(with_symbols)(Module);

	return GetBackTrace(ContextRecord, ThreadHandle);
}

void tracer_detail::tracer::get_symbols(string_view const Module, span<uintptr_t const> const Trace, function_ref<void(string&& Line)> const Consumer)
{
	SCOPED_ACTION(with_symbols)(Module);

	get_symbols_impl(Module, Trace, [&](string&& Address, string&& Name, string&& Source)
	{
		if (!Name.empty())
			append(Address, L' ', Name);

		if (!Source.empty())
			append(Address, L" ("sv, Source, L')');

		Consumer(std::move(Address));
	});
}

void tracer_detail::tracer::get_symbol(string_view const Module, const void* Ptr, string& Address, string& Name, string& Source)
{
	SCOPED_ACTION(with_symbols)(Module);

	uintptr_t const Stack[]{ reinterpret_cast<uintptr_t>(Ptr) };

	get_symbols_impl(Module, Stack, [&](string&& StrAddress, string&& StrName, string&& StrSource)
	{
		Address = std::move(StrAddress);
		Name = std::move(StrName);
		Source = std::move(StrSource);
	});
}

void tracer_detail::tracer::sym_initialise(string_view Module)
{
	SCOPED_ACTION(std::lock_guard)(m_CS);

	if (m_SymInitialised)
	{
		++m_SymInitialised;
		return;
	}

	auto Path = os::fs::get_current_process_file_name();
	CutToParent(Path);

	if (!Module.empty())
	{
		CutToParent(Module);
		append(Path, L';', Module);
	}

	if (imports.SymSetOptions)
	{
		imports.SymSetOptions(
			SYMOPT_UNDNAME |
			SYMOPT_DEFERRED_LOADS |
			SYMOPT_LOAD_LINES |
			SYMOPT_FAIL_CRITICAL_ERRORS |
			SYMOPT_INCLUDE_32BIT_MODULES |
			SYMOPT_NO_PROMPTS
		);
	}

	if (
		(imports.SymInitializeW && imports.SymInitializeW(GetCurrentProcess(), EmptyToNull(Path), TRUE)) ||
		(imports.SymInitialize && imports.SymInitialize(GetCurrentProcess(), EmptyToNull(encoding::ansi::get_bytes(Path)), TRUE))
	)
		++m_SymInitialised;
}

void tracer_detail::tracer::sym_cleanup()
{
	SCOPED_ACTION(std::lock_guard)(m_CS);

	if (m_SymInitialised)
		--m_SymInitialised;

	if (!m_SymInitialised)
	{
		if (imports.SymCleanup)
			imports.SymCleanup(GetCurrentProcess());
	}
}

tracer_detail::tracer::with_symbols::with_symbols(string_view const Module)
{
	::tracer.sym_initialise(Module);
}

tracer_detail::tracer::with_symbols::~with_symbols()
{
	::tracer.sym_cleanup();
}

NIFTY_DEFINE(tracer_detail::tracer, tracer);
