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
#include "map_file.hpp"

// Platform:
#include "platform.env.hpp"
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

template<typename T, typename data>
static void stack_walk(data const& Data, function_ref<bool(T&)> const& Walker, function_ref<void(uintptr_t, DWORD)> const& Handler)
{
	const auto address = [](DWORD64 const Offset)
	{
		return ADDRESS64{ Offset, 0, AddrModeFlat };
	};

	T StackFrame{};
	StackFrame.AddrPC = address(Data.PC);
	StackFrame.AddrFrame = address(Data.Frame);
	StackFrame.AddrStack = address(Data.Stack);

	if constexpr (std::is_same_v<T, STACKFRAME_EX>)
	{
		StackFrame.StackFrameSize = sizeof(StackFrame);
	}

	while (Walker(StackFrame))
	{
		// Cast to uintptr_t is ok here: although this function can be used
		// to capture a stack of 64-bit process from a 32-bit one,
		// we always use it with the current process only.

		DWORD InlineFrameContext;
		if constexpr (std::is_same_v<T, STACKFRAME_EX>)
			InlineFrameContext = StackFrame.InlineFrameContext;
		else
			InlineFrameContext = 0;

		Handler(static_cast<uintptr_t>(StackFrame.AddrPC.Offset), InlineFrameContext);
	}
}

// StackWalk64() may modify context record passed to it, so we will use a copy.
static auto GetBackTrace(CONTEXT ContextRecord, HANDLE ThreadHandle)
{
	std::vector<os::debug::stack_frame> Result;

	if (!imports.StackWalkEx && !imports.StackWalk64)
		return Result;

	const auto Process = GetCurrentProcess();
	const auto Data = platform_specific_data(ContextRecord);

	if (Data.MachineType == IMAGE_FILE_MACHINE_UNKNOWN || (!Data.PC && !Data.Frame && !Data.Stack))
		return Result;

	const auto handler = [&](uintptr_t const Address, DWORD const InlineFrameContext)
	{
		Result.emplace_back(Address, InlineFrameContext);
	};

	if (imports.StackWalkEx)
	{
		stack_walk<STACKFRAME_EX>(
			Data,
			[&](STACKFRAME_EX& StackFrame)
			{
				return imports.StackWalkEx(Data.MachineType, Process, ThreadHandle, &StackFrame, &ContextRecord, {}, imports.SymFunctionTableAccess64, imports.SymGetModuleBase64, {}, SYM_STKWALK_DEFAULT);
			},
			handler
		);
	}
	else
	{
		stack_walk<STACKFRAME64>(
			Data,
			[&](STACKFRAME64& StackFrame)
			{
				return imports.StackWalk64(Data.MachineType, Process, ThreadHandle, &StackFrame, &ContextRecord, {}, imports.SymFunctionTableAccess64, imports.SymGetModuleBase64, {});
			},
			handler
		);
	}

	return Result;
}

static constexpr auto BitsPerHexChar = 4;

template<typename T>
static constexpr auto width_in_hex_chars = std::numeric_limits<T>::digits / BitsPerHexChar;

static string FormatAddress(uintptr_t const Value)
{
	// It is unlikely that RVAs will be above 4 GiB,
	// so we can save some screen space here.
	const auto Width = Value > std::numeric_limits<uint32_t>::max()?
		width_in_hex_chars<decltype(Value)> :
		width_in_hex_chars<uint32_t>;

	return format(FSTR(L"{:0{}X}"sv), Value, Width);
}

// SYMBOL_INFO_PACKAGEW not defined in GCC headers :(
namespace
{
	template<typename header>
	struct package
	{
		header info;
		static constexpr auto max_name_size = MAX_SYM_NAME;

		using char_type = VALUE_TYPE(header::Name);
		char_type name[max_name_size + 1];

		using result_type = std::pair<std::basic_string_view<char_type>, size_t>;
	};
}

static bool is_inline_frame(DWORD const InlineFrameContext)
{
	INLINE_FRAME_CONTEXT const frameContext{ InlineFrameContext };

	if (frameContext.ContextValue == INLINE_FRAME_CONTEXT_IGNORE)
		return false;

	return (frameContext.FrameType & STACK_FRAME_TYPE_INLINE) != 0;
}

static void get_symbols_impl(
	string_view const ModuleName,
	span<os::debug::stack_frame const> const BackTrace,
	std::unordered_map<uintptr_t, map_file>& MapFiles,
	function_ref<void(string&&, bool, string&&, string&&)> const Consumer
)
{
	const auto Process = GetCurrentProcess();

	std::variant
	<
		package<SYMBOL_INFOW>,
		package<SYMBOL_INFO>,
		package<IMAGEHLP_SYMBOL64>
	>
	SymbolData;

	string NameBuffer;

	const auto frame_get_name = [&](uintptr_t const Address) -> std::pair<string_view, size_t>
	{
		const auto Get = [&](auto const& Getter, auto& Buffer) -> package<decltype(Buffer.info)>::result_type
		{
			if (!Getter)
				return {};

			Buffer.info.SizeOfStruct = sizeof(Buffer.info);

			constexpr auto IsOldApi = std::is_same_v<decltype(Buffer.info), IMAGEHLP_SYMBOL64>;
			if constexpr (IsOldApi)
			{
				// This one is for Win2k, which doesn't have SymFromAddr.
				// However, I couldn't make it work with the out-of-the-box version.
				// Get a newer dbghelp.dll if you need traces there:
				// http://download.microsoft.com/download/A/6/A/A6AC035D-DA3F-4F0C-ADA4-37C8E5D34E3D/setup/WinSDKDebuggingTools/dbg_x86.msi

				Buffer.info.MaxNameLength = Buffer.max_name_size;
			}
			else
			{
				Buffer.info.MaxNameLen = Buffer.max_name_size;
			}

			DWORD64 Displacement;
			if (!Getter(Process, Address, &Displacement, &Buffer.info))
				return {};

			size_t NameSize{};
			if constexpr (!IsOldApi)
			{
				NameSize = Buffer.info.NameLen;
			}

			// Old dbghelp versions (e.g. XP) not always populate NameLen
			return { { Buffer.info.Name, NameSize? NameSize : std::char_traits<VALUE_TYPE(Buffer.info.Name)>::length(Buffer.info.Name) }, static_cast<size_t>(Displacement) };
		};

		if (const auto Name = Get(imports.SymFromAddrW, SymbolData.emplace<0>()); !Name.first.empty())
			return Name;

		if (const auto Name = Get(imports.SymFromAddr, SymbolData.emplace<1>()); !Name.first.empty())
			return { NameBuffer = encoding::ansi::get_chars(Name.first), Name.second };

		if (const auto Name = Get(imports.SymGetSymFromAddr64, SymbolData.emplace<2>()); !Name.first.empty())
			return { NameBuffer = encoding::ansi::get_chars(Name.first), Name.second };

		return {};
	};

	const auto location = [](string_view const File, unsigned const Line, unsigned const Displacement)
	{
		return format(FSTR(L"{}({})"sv), File, Line);
	};

	const auto frame_get_location = [&](uintptr_t const Address)
	{
		DWORD Displacement;

		const auto Get = [&](auto const& Getter, auto& Buffer)
		{
			Buffer.SizeOfStruct = sizeof(Buffer);
			return Getter && Getter(Process, Address, &Displacement, &Buffer);
		};

		if (IMAGEHLP_LINEW64 Line; Get(imports.SymGetLineFromAddrW64, Line))
			return location(Line.FileName, Line.LineNumber, Displacement);

		if (IMAGEHLP_LINE64 Line; Get(imports.SymGetLineFromAddr64, Line))
			return location(encoding::ansi::get_chars(Line.FileName), Line.LineNumber, Displacement);

		return L""s;
	};

	const auto inline_frame_get_name = [&](os::debug::stack_frame const& Frame) -> std::pair<string_view, size_t>
	{
		auto& Buffer = SymbolData.emplace<0>();
		Buffer.info.SizeOfStruct = sizeof(Buffer.info);
		Buffer.info.MaxNameLen = Buffer.max_name_size;
		DWORD64 Displacement;

		// Both W and A APIs were added together in dbghelp 6.2 (8), we don't need to fallback to A.
		if (!imports.SymFromInlineContextW(Process, Frame.Address, Frame.InlineContext, &Displacement, &Buffer.info))
			return {};

		return { { Buffer.info.Name, Buffer.info.NameLen }, Displacement };
	};

	const auto inline_frame_get_location = [&](os::debug::stack_frame const& Frame, uintptr_t const BaseAddress)
	{
		DWORD Displacement;
		IMAGEHLP_LINEW64 Buffer{ sizeof(Buffer) };
		// Both W and A APIs were added together in dbghelp 6.2 (8), we don't need to fallback to A.
		if (!imports.SymGetLineFromInlineContextW(Process, Frame.Address, Frame.InlineContext, BaseAddress, &Displacement, &Buffer))
			return L""s;

		return location(Buffer.FileName, Buffer.LineNumber, Displacement);
	};

	const auto handle_frame = [&](os::debug::stack_frame const& Frame, bool const IsInlineFrame, int Fixup = 0)
	{
		std::optional<IMAGEHLP_MODULEW64> Module(std::in_place);
		// use the pre-07-Jun-2002 struct size, aligned to 8
		Module->SizeOfStruct = static_cast<DWORD>(aligned_size(offsetof(IMAGEHLP_MODULEW64, LoadedImageName), 8));
		if (!imports.SymGetModuleInfoW64 || !imports.SymGetModuleInfoW64(Process, Frame.Address, &*Module))
			Module.reset();

		const auto BaseAddress = Module? Module->BaseOfImage : 0;
		const auto ImageName = Module? PointToName(Module->ImageName) : L""sv;

		string_view SymbolName;
		size_t Displacement;
		string Location;

		if (Frame.Address)
		{
			if (IsInlineFrame)
			{
				std::tie(SymbolName, Displacement) = inline_frame_get_name(Frame);
				Location = inline_frame_get_location(Frame, BaseAddress);
			}
			else
			{
				std::tie(SymbolName, Displacement) = frame_get_name(Frame.Address);
				Location = frame_get_location(Frame.Address);
			}

			if (SymbolName.empty() && Module)
			{
				auto& MapFile = MapFiles.try_emplace(
					Module->BaseOfImage,
					*Module->ImageName?
						Module->ImageName :
						ModuleName
				).first->second;

				const auto Info = MapFile.get(Frame.Address - BaseAddress);
				SymbolName = Info.Symbol;
				Displacement = Info.Displacement;

				if (Location.empty())
					Location = Info.File;
			}
		}

		Consumer(
			FormatAddress(Frame.Address? Frame.Address + Fixup - BaseAddress : 0),
			IsInlineFrame,
			Frame.Address?
				format(FSTR(L"{}!{}{}"sv),
					!ImageName.empty()?
						PointToName(ImageName):
						L"<unknown>"sv,
					!SymbolName.empty()?
						SymbolName :
						L"<unknown> (get the pdb)"sv,
					!SymbolName.empty()?
						format(FSTR(L"+0x{:X}"sv), Displacement) :
						L""s
				) :
				L""s,
			std::move(Location)
		);
	};

	for (const auto& i: BackTrace)
	{
		if (i.InlineContext)
		{
			handle_frame(i, is_inline_frame(i.InlineContext));
			continue;
		}

		if (imports.SymAddrIncludeInlineTrace)
		{
			auto Frame = i;
			if (Frame.Address != 0)
				--Frame.Address;

			if (const auto InlineFramesCount = imports.SymAddrIncludeInlineTrace(Process, Frame.Address))
			{
				ULONG FrameIndex{};
				if (imports.SymQueryInlineTrace(Process, Frame.Address, INLINE_FRAME_CONTEXT_INIT, Frame.Address, Frame.Address, &Frame.InlineContext, &FrameIndex))
				{
					for (DWORD n = FrameIndex; n != InlineFramesCount; ++n)
					{
						handle_frame(Frame, true, 1);
						++Frame.InlineContext;
					}
				}
			}
		}

		handle_frame(i, false);
	}
}

tracer_detail::tracer::tracer():
	m_MapFiles(std::make_unique<std::unordered_map<uintptr_t, map_file>>())
{
}

tracer_detail::tracer::~tracer() = default;

std::vector<os::debug::stack_frame> tracer_detail::tracer::get(string_view const Module, CONTEXT const& ContextRecord, HANDLE ThreadHandle)
{
	SCOPED_ACTION(with_symbols)(Module);

	return GetBackTrace(ContextRecord, ThreadHandle);
}

void tracer_detail::tracer::get_symbols(string_view const Module, span<os::debug::stack_frame const> const Trace, function_ref<void(string&& Line)> const Consumer) const
{
	SCOPED_ACTION(with_symbols)(Module);

	get_symbols_impl(Module, Trace, *m_MapFiles, [&](string&& Address, bool const InlineFrame, string&& Name, string&& Source)
	{
		Address += InlineFrame? L" I"sv : L"  "sv;

		if (!Name.empty())
			append(Address, L' ', Name);

		if (!Source.empty())
			append(Address, L" ("sv, Source, L')');

		Consumer(std::move(Address));
	});
}

void tracer_detail::tracer::get_symbol(string_view const Module, const void* Ptr, string& Address, string& Name, string& Source) const
{
	SCOPED_ACTION(with_symbols)(Module);

	os::debug::stack_frame const Stack[]{ { reinterpret_cast<uintptr_t>(Ptr), INLINE_FRAME_CONTEXT_INIT } };

	get_symbols_impl(Module, Stack, *m_MapFiles, [&](string&& StrAddress, bool, string&& StrName, string&& StrSource)
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

	string Path = os::env::get(L"_NT_SYMBOL_PATH"sv);

	const auto append_to_path = [&](string_view const Str)
	{
		append(Path, Path.empty()? L""sv : L";"sv, Str);
	};

	if (const auto AltSymbolPath = os::env::get(L"_NT_ALTERNATE_SYMBOL_PATH"sv); !AltSymbolPath.empty())
	{
		append_to_path(AltSymbolPath);
	}

	if (const auto FarPath = os::fs::get_current_process_file_name(); !FarPath.empty())
	{
		string_view FarPathView = FarPath;
		CutToParent(FarPathView);
		append_to_path(FarPathView);
	}

	if (!Module.empty())
	{
		CutToParent(Module);
		append_to_path(Module);
	}

	if (imports.SymSetOptions)
	{
		imports.SymSetOptions(
			SYMOPT_UNDNAME |
			SYMOPT_DEFERRED_LOADS |
			SYMOPT_LOAD_LINES |
			SYMOPT_FAIL_CRITICAL_ERRORS |
			SYMOPT_INCLUDE_32BIT_MODULES |
			SYMOPT_NO_PROMPTS |
			SYMOPT_DEBUG
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
