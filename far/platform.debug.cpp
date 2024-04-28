/*
platform.debug.cpp

*/
/*
Copyright © 2022 Far Group
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
#include "platform.debug.hpp"

// Internal:
#include "encoding.hpp"
#include "imports.hpp"
#include "log.hpp"
#include "map_file.hpp"
#include "pathmix.hpp"
#include "string_utils.hpp"
#include "strmix.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.memory.hpp"

// Common:
#include "common.hpp"
#include "common/function_ref.hpp"
#include "common/enum_tokens.hpp"

// External:

#include <crtdbg.h>

//----------------------------------------------------------------------------

namespace os::debug
{
	bool is_debugger_present()
	{
		return IsDebuggerPresent() != FALSE;
	}

	void breakpoint()
	{
		DebugBreak();
	}

	void breakpoint_if_debugging()
	{
		if (is_debugger_present())
			breakpoint();
	}

	void print(const wchar_t* const Str)
	{
		OutputDebugString(Str);
	}

	void print(string const& Str)
	{
		print(Str.c_str());
	}

	void set_thread_name(const wchar_t* const Name)
	{
		if (imports.SetThreadDescription)
			imports.SetThreadDescription(GetCurrentThread(), Name);
	}

	void set_thread_name(string const& Name)
	{
		set_thread_name(Name.c_str());
	}

	string get_thread_name(HANDLE const ThreadHandle)
	{
		if (!imports.GetThreadDescription)
			return {};

		memory::local::ptr<wchar_t> Name;
		if (FAILED(imports.GetThreadDescription(ThreadHandle, &ptr_setter(Name))))
			return {};

		return Name.get();
	}

	static void** dummy_noncontinuable_exception(NTSTATUS const Code)
	{
		static EXCEPTION_RECORD DummyRecord{};

		DummyRecord.ExceptionCode = static_cast<DWORD>(Code);
		DummyRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;

		static void* DummyRecordPtr = &DummyRecord;
		return &DummyRecordPtr;
	}

	static void** dummy_continuable_exception(NTSTATUS const Code)
	{
		static EXCEPTION_RECORD DummyRecord{};

		DummyRecord.ExceptionCode = static_cast<DWORD>(Code);

		static void* DummyRecordPtr = &DummyRecord;
		return &DummyRecordPtr;
	}

	static void** dummy_current_exception_context()
	{
		static CONTEXT DummyContext{};
		static void* DummyContextPtr = &DummyContext;
		return &DummyContextPtr;
	}

#if IS_MICROSOFT_SDK()
	extern "C" void** __current_exception();
	extern "C" void** __current_exception_context();
#else
	static void** __current_exception()
	{
		return dummy_noncontinuable_exception(EH_EXCEPTION_NUMBER);
	}

	static void** __current_exception_context()
	{
		return dummy_current_exception_context();
	}
#endif

	EXCEPTION_POINTERS exception_information()
	{
		if (!std::current_exception())
			return {};

		return
		{
			static_cast<EXCEPTION_RECORD*>(*__current_exception()),
			static_cast<CONTEXT*>(*__current_exception_context())
		};
	}

	EXCEPTION_POINTERS fake_exception_information(unsigned const Code, bool const Continuable)
	{
		return
		{
			static_cast<EXCEPTION_RECORD*>(*(Continuable? dummy_continuable_exception : dummy_noncontinuable_exception)(Code)),
			static_cast<CONTEXT*>(*dummy_current_exception_context())
		};
	}

	std::vector<stack_frame> current_stacktrace(size_t const FramesToSkip, size_t const FramesToCapture)
	{
		if (!imports.RtlCaptureStackBackTrace)
			return {};

		std::vector<stack_frame> Stack;
		Stack.reserve(128);

		// http://web.archive.org/web/20140815000000*/http://msdn.microsoft.com/en-us/library/windows/hardware/ff552119(v=vs.85).aspx
		// In Windows XP and Windows Server 2003, the sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
		static const auto Limit = IsWindowsVistaOrGreater()? std::numeric_limits<size_t>::max() : 62;

		const auto Skip = FramesToSkip + 1; // 1 for this frame
		const auto Capture = std::min(FramesToCapture, Limit - Skip);

		for (size_t i = 0; i != FramesToCapture;)
		{
			void* Pointers[128];

			DWORD DummyHash;
			const auto Size = imports.RtlCaptureStackBackTrace(
				static_cast<DWORD>(Skip + i),
				static_cast<DWORD>(std::min(std::size(Pointers), Capture - i)),
				Pointers,
				&DummyHash // MSDN says it's optional, but it's not true on Win2k
			);

			if (!Size)
				break;

			std::ranges::transform(Pointers, Pointers + Size, std::back_inserter(Stack), [](void* Ptr)
			{
				return stack_frame{ std::bit_cast<uintptr_t>(Ptr), INLINE_FRAME_CONTEXT_INIT };
			});

			i += Size;
		}

		return Stack;
	}

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

	static auto address(DWORD64 const Offset)
	{
		return ADDRESS64{ Offset, 0, AddrModeFlat };
	};

	template<typename T>
	static void stack_walk(auto const& Data, function_ref<bool(T&)> const& Walker, function_ref<void(uintptr_t, DWORD)> const& Handler)
	{
		T StackFrame{};
		StackFrame.AddrPC = address(Data.PC);
		StackFrame.AddrFrame = address(Data.Frame);
		StackFrame.AddrStack = address(Data.Stack);

		if constexpr (std::same_as<T, STACKFRAME_EX>)
		{
			StackFrame.StackFrameSize = sizeof(StackFrame);
		}

		while (Walker(StackFrame))
		{
			// Cast to uintptr_t is ok here: although this function can be used
			// to capture a stack of 64-bit process from a 32-bit one,
			// we always use it with the current process only.

			DWORD InlineFrameContext;
			if constexpr (std::same_as<T, STACKFRAME_EX>)
				InlineFrameContext = StackFrame.InlineFrameContext;
			else
				InlineFrameContext = 0;

			Handler(static_cast<uintptr_t>(StackFrame.AddrPC.Offset), InlineFrameContext);
		}
	}

	// StackWalk64() may modify context record passed to it, so we will use a copy.
	std::vector<stack_frame> stacktrace(CONTEXT ContextRecord, HANDLE ThreadHandle)
	{
		std::vector<stack_frame> Result;

		if (!imports.StackWalkEx && !imports.StackWalk64)
			return Result;

		const auto Process = GetCurrentProcess();
		const auto Data = platform_specific_data(ContextRecord);

		if (Data.MachineType == IMAGE_FILE_MACHINE_UNKNOWN || (!Data.PC && !Data.Frame && !Data.Stack))
			return Result;

		const auto handler = [&](uintptr_t const Address, DWORD const InlineFrameContext)
		{
			Result.push_back({ Address, InlineFrameContext });
		};

		if (imports.StackWalkEx)
		{
			stack_walk<STACKFRAME_EX>(
				Data,
				[&](STACKFRAME_EX& StackFrame)
				{
					return imports.StackWalkEx(
						Data.MachineType,
						Process,
						ThreadHandle,
						&StackFrame,
						&ContextRecord,
						{},
						imports.SymFunctionTableAccess64,
						imports.SymGetModuleBase64,
						{},
						SYM_STKWALK_DEFAULT
					);
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
					return imports.StackWalk64(
						Data.MachineType,
						Process,
						ThreadHandle,
						&StackFrame,
						&ContextRecord,
						{},
						imports.SymFunctionTableAccess64,
						imports.SymGetModuleBase64,
						{}
					);
				},
				handler
			);
		}

		return Result;
	}

	std::vector<stack_frame> exception_stacktrace()
	{
		if (!std::current_exception())
			return {};

		const auto ExceptionInformation = exception_information();
		return stacktrace(*ExceptionInformation.ContextRecord, GetCurrentThread());
	}

	bool is_inline_frame(DWORD const InlineContext)
	{
		INLINE_FRAME_CONTEXT const frameContext{ InlineContext };

		if (frameContext.ContextValue == INLINE_FRAME_CONTEXT_IGNORE)
			return false;

		return (frameContext.FrameType & STACK_FRAME_TYPE_INLINE) != 0;
	}

	void crt_report_to_ui()
	{
#ifdef _DEBUG
		// _OUT_TO_STDERR is the default for console apps, but it is less convenient for debugging.
		// Use -service to set it back to _OUT_TO_STDERR (e.g. for macro tests on CI).
		_set_error_mode(_OUT_TO_MSGBOX);

		const auto ReportToUI = [](int const ReportType)
		{
			_CrtSetReportMode(ReportType, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
		};

		ReportToUI(_CRT_WARN);
		ReportToUI(_CRT_ERROR);
		ReportToUI(_CRT_ASSERT);
#endif
	}

	void crt_report_to_stderr()
	{
#ifdef _DEBUG
		_set_error_mode(_OUT_TO_STDERR);

		const auto ReportToStdErr = [](int const ReportType)
		{
			(void)_CrtSetReportFile(ReportType, _CRTDBG_FILE_STDERR);
			_CrtSetReportMode(ReportType, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
		};

		ReportToStdErr(_CRT_WARN);
		ReportToStdErr(_CRT_ERROR);
		ReportToStdErr(_CRT_ASSERT);
#endif
	}
}

namespace os::debug::symbols
{
	static std::optional<bool> initialize(HANDLE const Process, string const& Path)
	{
		if (imports.SymInitializeW)
			return imports.SymInitializeW(Process, EmptyToNull(Path), TRUE) != FALSE;

		if (imports.SymInitialize)
			return imports.SymInitialize(Process, EmptyToNull(encoding::ansi::get_bytes(Path)), TRUE) != FALSE;

		return {};
	}

	static auto event_level(DWORD const EventSeverity)
	{
		switch (EventSeverity)
		{
		default:
		case sevInfo:    return logging::level::info;
		case sevProblem: return logging::level::warning;
		case sevAttn:    return logging::level::error;
		case sevFatal:   return logging::level::fatal;
		}
	}

	enum context_encoding
	{
		ansi,
		unicode
	};

	static BOOL CALLBACK callback([[maybe_unused]] HANDLE const Process, ULONG const ActionCode, ULONG64 const CallbackData, ULONG64 const UserContext)
	{
		switch (ActionCode)
		{
		case CBA_EVENT:
			{
				const auto& Event = view_as<IMAGEHLP_CBA_EVENT>(static_cast<uintptr_t>(CallbackData));
				const auto Level = event_level(Event.severity);

				string Buffer;
				string_view Message;

				if (UserContext == context_encoding::unicode)
				{
					Message = static_cast<wchar_t const*>(static_cast<void const*>(Event.desc));
				}
				else
				{
					Buffer = encoding::ansi::get_chars(Event.desc);
					Message = Buffer;
				}

				while (IsEol(Message.back()))
					Message.remove_suffix(1);

				LOG(Level, L"{}"sv, Message);
				return true;
			}

		default:
			return false;
		}
	}

	static std::optional<bool> register_callback(HANDLE const Process)
	{
		if (imports.SymRegisterCallbackW64)
			return imports.SymRegisterCallbackW64(Process, callback, context_encoding::unicode) != FALSE;

		if (imports.SymRegisterCallback64)
			return imports.SymRegisterCallback64(Process, callback, context_encoding::ansi) != FALSE;

		return {};
	}

	static void append_to_search_path(string& Path, string_view const Str)
	{
		append(Path, Path.empty()? L""sv : L";"sv, Str);
	};

	static void update_symbols_search_path(HANDLE const Process, string_view const NewPath)
	{
		string ExistingPath;

		if (imports.SymGetSearchPathW)
		{
			// This stupid function doesn't fill the buffer if it's not large enough.
			// It also doesn't provide any way to detect that.
			wchar_t Buffer[MAX_PATH * 4];
			if (imports.SymGetSearchPathW(Process, Buffer, static_cast<DWORD>(std::size(Buffer))))
				ExistingPath = Buffer;
			else
				LOGWARNING(L"SymGetSearchPathW(): {}"sv, os::last_error());
		}
		else if (imports.SymGetSearchPath)
		{
			char Buffer[MAX_PATH * 4];
			if (imports.SymGetSearchPath(Process, Buffer, static_cast<DWORD>(std::size(Buffer))))
				ExistingPath = encoding::ansi::get_chars(Buffer);
			else
				LOGWARNING(L"SymGetSearchPath(): {}"sv, os::last_error());
		}

		string_view Path;

		if (ExistingPath.empty())
		{
			Path = NewPath;
		}
		else
		{
			const auto contains = [&](string_view const NewPathElement)
			{
				const auto Enumerator = enum_tokens(ExistingPath, L";"sv);
				return std::ranges::find_if(Enumerator, [&](string_view const i){ return equal_icase(i, NewPathElement); }) != Enumerator.cend();
			};

			bool PathChanged = false;

			for (const auto& i: enum_tokens(NewPath, L";"))
			{
				if (contains(i))
					continue;

				append_to_search_path(ExistingPath, i);
				PathChanged = true;
			}

			if (!PathChanged)
				return;

			Path = ExistingPath;
		}

		if (imports.SymSetSearchPathW)
		{
			if (!imports.SymSetSearchPathW(Process, null_terminated(Path).c_str()))
				LOGWARNING(L"SymSetSearchPathW({}): {}"sv, Path, os::last_error());
		}
		else if (imports.SymSetSearchPath)
		{
			if (!imports.SymSetSearchPath(Process, encoding::ansi::get_bytes(Path).c_str()))
				LOGWARNING(L"SymSetSearchPath({}): {}"sv, Path, os::last_error());
		}
	}

	bool initialize(string_view Module)
	{
		string Path;

		if (const auto FarPath = fs::get_current_process_file_name(); !FarPath.empty() && FarPath != Module)
		{
			string_view FarPathView = FarPath;
			CutToParent(FarPathView);
			append_to_search_path(Path, FarPathView);
		}

		if (!Module.empty())
		{
			CutToParent(Module);
			append_to_search_path(Path, Module);
		}

		for (const auto& Var: { L"_NT_SYMBOL_PATH"sv, L"_NT_ALTERNATE_SYMBOL_PATH"sv })
		{
			if (const auto EnvSymbolPath = env::get(Var); !EnvSymbolPath.empty())
				append_to_search_path(Path, EnvSymbolPath);
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

		const auto Process = GetCurrentProcess();

		const auto Result = initialize(Process, Path);
		if (Result == false) // optional
		{
			if (const auto LastError = last_error(); LastError.Win32Error == ERROR_INVALID_PARAMETER)
			{
				// Symbols are already initialized by something else; try to at least propagate our search paths
				update_symbols_search_path(Process, Path);
			}
			else
				LOGWARNING(L"SymInitialize({}): {}"sv, Path, LastError);
		}

		if (register_callback(Process) == false) // optional
			LOGWARNING(L"SymRegisterCallback(): {}"sv, last_error());

		return Result.value_or(false);
	}

	void clean()
	{
		if (imports.SymCleanup)
		{
			if (!imports.SymCleanup(GetCurrentProcess()))
			{
				LOGWARNING(L"SymCleanup(): {}"sv, last_error());
			}
		}
	}

	namespace
	{
		template<typename header>
		struct package
		{
			header info;
			static constexpr auto max_name_size = MAX_SYM_NAME;

			using char_type = std::ranges::range_value_t<decltype(info.Name)>;
			char_type name[max_name_size + 1];

			struct symbol
			{
				std::basic_string_view<char_type> Name;
				size_t Displacement;
			};
		};

		struct symbol_storage
		{
			std::variant
			<
				package<SYMBOL_INFOW>,
				package<SYMBOL_INFO>,
				package<IMAGEHLP_SYMBOL64>
			>
			SymbolInfo;

			string
				SymbolName,
				FileName;
		};
	}

	static symbol frame_get_symbol(HANDLE const Process, uintptr_t const Address, symbol_storage& Storage)
	{
		const auto Get = [&]<typename T>(auto const& Getter, package<T>& Buffer) -> typename package<T>::symbol
		{
			Buffer.info.SizeOfStruct = sizeof(Buffer.info);

			constexpr auto IsOldApi = std::same_as<T, IMAGEHLP_SYMBOL64>;
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
			return { { Buffer.info.Name, NameSize? NameSize : std::char_traits<typename package<T>::char_type>::length(Buffer.info.Name) }, static_cast<size_t>(Displacement) };
		};

		if (imports.SymFromAddrW)
		{
			const auto Symbol = Get(imports.SymFromAddrW, Storage.SymbolInfo.emplace<0>());
			if (Symbol.Name.empty())
				return {};

			return { Symbol.Name, Symbol.Displacement };
		}

		if (imports.SymFromAddr)
		{
			const auto Symbol = Get(imports.SymFromAddr, Storage.SymbolInfo.emplace<1>());
			if (Symbol.Name.empty())
				return {};

			return { Storage.SymbolName = encoding::ansi::get_chars(Symbol.Name), Symbol.Displacement };
		}

		if (imports.SymGetSymFromAddr64)
		{
			const auto Symbol = Get(imports.SymGetSymFromAddr64, Storage.SymbolInfo.emplace<2>());
			if (Symbol.Name.empty())
				return {};

			return { Storage.SymbolName = encoding::ansi::get_chars(Symbol.Name), Symbol.Displacement };
		}

		return {};
	}

	static location frame_get_location(HANDLE const Process, uintptr_t const Address, symbol_storage& Storage)
	{
		DWORD Displacement;

		const auto Get = [&](auto const& Getter, auto& Buffer)
		{
			Buffer.SizeOfStruct = sizeof(Buffer);
			return Getter(Process, Address, &Displacement, &Buffer);
		};

		if (imports.SymGetLineFromAddrW64)
		{
			IMAGEHLP_LINEW64 Line;
			if (!Get(imports.SymGetLineFromAddrW64, Line))
				return {};

			return { Line.FileName, Line.LineNumber, Displacement };
		}

		if (imports.SymGetLineFromAddr64)
		{
			IMAGEHLP_LINE64 Line;
			if (!Get(imports.SymGetLineFromAddr64, Line))
				return {};

			return { Storage.FileName = encoding::ansi::get_chars(Line.FileName), Line.LineNumber, Displacement };
		}

		return {};
	}

	static symbol inline_frame_get_symbol(HANDLE const Process, stack_frame const& Frame, symbol_storage& Storage)
	{
		auto& Buffer = Storage.SymbolInfo.emplace<0>();
		Buffer.info.SizeOfStruct = sizeof(Buffer.info);
		Buffer.info.MaxNameLen = Buffer.max_name_size;
		DWORD64 Displacement;

		// Both W and A APIs were added together in dbghelp 6.2 (8), we don't need to fallback to A.
		if (!imports.SymFromInlineContextW(Process, Frame.Address, Frame.InlineContext, &Displacement, &Buffer.info))
			return {};

		return { { Buffer.info.Name, Buffer.info.NameLen }, static_cast<size_t>(Displacement) };
	}

	static location inline_frame_get_location(HANDLE const Process, stack_frame const& Frame, uintptr_t const BaseAddress)
	{
		DWORD Displacement;
		IMAGEHLP_LINEW64 Buffer{ sizeof(Buffer) };
		// Both W and A APIs were added together in dbghelp 6.2 (8), we don't need to fallback to A.
		if (!imports.SymGetLineFromInlineContextW(Process, Frame.Address, Frame.InlineContext, BaseAddress, &Displacement, &Buffer))
			return {};

		return { Buffer.FileName, Buffer.LineNumber, Displacement };
	}

	static HMODULE module_from_address(uintptr_t const Address)
	{
		if (HMODULE Module; imports.GetModuleHandleExW && imports.GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, std::bit_cast<LPCWSTR>(Address), &Module))
			return Module;

		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQuery(std::bit_cast<void*>(Address), &mbi, sizeof(mbi)))
			return std::bit_cast<HMODULE>(mbi.AllocationBase);

		return {};
	}

	static void handle_frame(
		HANDLE const Process,
		string_view const ModuleName,
		stack_frame const& Frame,
		bool const IsInlineFrame,
		symbol_storage& Storage,
		std::unordered_map<uintptr_t, map_file>& MapFiles,
		function_ref<void(uintptr_t, string_view, bool, symbol, location)> const Consumer
	)
	{
		std::optional<IMAGEHLP_MODULEW64> Module(std::in_place);

		// use the pre-07-Jun-2002 struct size, aligned to 8
		Module->SizeOfStruct = static_cast<DWORD>(aligned_size(offsetof(IMAGEHLP_MODULEW64, LoadedImageName), 8));

		if (!imports.SymGetModuleInfoW64 || !imports.SymGetModuleInfoW64(Process, Frame.Address, &*Module))
		{
			if (const auto ModuleFromAddress = module_from_address(Frame.Address))
			{
				Module->BaseOfImage = std::bit_cast<uintptr_t>(ModuleFromAddress);

				if (string ModuleFromAddressFileName; fs::get_module_file_name({}, ModuleFromAddress, ModuleFromAddressFileName))
					xwcsncpy(Module->ImageName, ModuleFromAddressFileName.data(), std::size(Module->ImageName));
			}
			else
				Module.reset();
		}

		const auto BaseAddress = [&]() -> uintptr_t
		{
			if (Module)
				return static_cast<uintptr_t>(Module->BaseOfImage);

			const auto ModuleNamePtr = EmptyToNull(null_terminated(ModuleName).c_str());

			if (const auto ModuleBaseAddress = std::bit_cast<uintptr_t>(GetModuleHandle(ModuleNamePtr)); ModuleBaseAddress < Frame.Address)
				return ModuleBaseAddress;

			if (!ModuleNamePtr)
				return 0;

			if (const auto ModuleBaseAddress = std::bit_cast<uintptr_t>(GetModuleHandle({})); ModuleBaseAddress < Frame.Address)
				return ModuleBaseAddress;

			return 0;
		}();

		const auto ImageName = Module && *Module->ImageName? Module->ImageName : ModuleName;

		symbol Symbol;
		location Location;

		if (Frame.Address)
		{
			if (IsInlineFrame)
			{
				Symbol = inline_frame_get_symbol(Process, Frame, Storage);
				Location = inline_frame_get_location(Process, Frame, BaseAddress);
			}
			else
			{
				Symbol = frame_get_symbol(Process, Frame.Address, Storage);
				Location = frame_get_location(Process, Frame.Address, Storage);
			}

			if (Symbol.Name.empty())
			{
				auto& MapFile = MapFiles.try_emplace(BaseAddress, ImageName).first->second;
				const auto Info = MapFile.get(Frame.Address - BaseAddress);
				Symbol.Name = Info.Symbol;
				Symbol.Displacement = Info.Displacement;

				if (Location.FileName.empty())
				{
					Location.FileName = Info.File;
					Location.Line.reset();
				}
			}
		}

		const auto Fixup = IsInlineFrame && !is_inline_frame(Frame.InlineContext)? 1 : 0;

		Consumer(
			Frame.Address? Frame.Address + Fixup - BaseAddress : 0,
			ImageName,
			IsInlineFrame,
			Symbol,
			Location
		);
	}

	void get(
		string_view const ModuleName,
		std::span<stack_frame const> const BackTrace,
		std::unordered_map<uintptr_t, map_file>& MapFiles,
		function_ref<void(uintptr_t, string_view, bool, symbol, location)> const Consumer
	)
	{
		const auto Process = GetCurrentProcess();
		symbol_storage Storage;

		for (const auto& i: BackTrace)
		{
			if (i.InlineContext)
			{
				// If InlineContext is populated, the frames are from StackWalkEx and any inline frames are already included.
				handle_frame(Process, ModuleName, i, is_inline_frame(i.InlineContext), Storage, MapFiles, Consumer);
				continue;
			}

			// StackWalk64 and RtlCaptureStackBackTrace do not include inline frames, we have to ask for them manually.
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
							handle_frame(Process, ModuleName, Frame, true, Storage, MapFiles, Consumer);
							++Frame.InlineContext;
						}
					}
				}
			}

			handle_frame(Process, ModuleName, i, false, Storage, MapFiles, Consumer);
		}
	}
}
