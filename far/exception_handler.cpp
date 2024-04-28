/*
exception_handler.cpp

*/
/*
Copyright © 2018 Far Group
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
#include "exception_handler.hpp"

// Internal:
#include "plugins.hpp"
#include "filepanels.hpp"
#include "manager.hpp"
#include "config.hpp"
#include "console.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "mix.hpp"
#include "imports.hpp"
#include "strmix.hpp"
#include "tracer.hpp"
#include "pathmix.hpp"
#include "global.hpp"
#include "farversion.hpp"
#include "clipboard.hpp"
#include "eol.hpp"
#include "log.hpp"
#include "datetime.hpp"
#include "execute.hpp"
#include "FarDlgBuilder.hpp"

// Platform:
#include "platform.hpp"
#include "platform.com.hpp"
#include "platform.debug.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.process.hpp"
#include "platform.version.hpp"

// Common:
#include "common/enum_substrings.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

#if !IS_MICROSOFT_SDK()
#include <cxxabi.h>
#endif

//----------------------------------------------------------------------------

#define BUGREPORT_NAME   "bug_report.txt"
#define MINIDUMP_NAME    "far.mdmp"
#define FULLDUMP_NAME    "far_full.mdmp"

class exception_context
{
public:
	NONCOPYABLE(exception_context);

	explicit exception_context(EXCEPTION_POINTERS const& Pointers) noexcept:
		m_ExceptionRecord(*Pointers.ExceptionRecord),
		m_ContextRecord(*Pointers.ContextRecord),
		m_ThreadHandle(os::OpenCurrentThread()),
		m_ThreadId(GetCurrentThreadId())
	{
	}

	auto code() const noexcept { return m_ExceptionRecord.ExceptionCode; }
	const auto& exception_record() const noexcept { return m_ExceptionRecord; }
	const auto& context_record() const noexcept { return m_ContextRecord; }
	auto thread_handle() const noexcept { return m_ThreadHandle.native_handle(); }
	auto thread_id() const noexcept { return m_ThreadId; }

private:
	EXCEPTION_RECORD m_ExceptionRecord;
	CONTEXT m_ContextRecord;
	os::handle m_ThreadHandle;
	DWORD m_ThreadId;
};

class seh_exception_context: public exception_context
{
public:
	NONMOVABLE(seh_exception_context);

	using exception_context::exception_context;

	~seh_exception_context()
	{
		ResumeThread(thread_handle());
	}
};

static bool HandleCppExceptions = true;
static bool HandleSehExceptions = true;
static bool ForceStderrExceptionUI = false;

static std::atomic_bool UseTerminateHandler = false;

void disable_exception_handling()
{
	if (!HandleCppExceptions && !HandleSehExceptions)
		return;

	HandleCppExceptions = false;
	HandleSehExceptions = false;

	LOGWARNING(L"Exception handling disabled"sv);
}

static std::atomic_bool s_ExceptionHandlingInprogress{};
bool exception_handling_in_progress()
{
	return s_ExceptionHandlingInprogress;
}

void force_stderr_exception_ui(bool const Force)
{
	ForceStderrExceptionUI = Force;
}

static constexpr NTSTATUS make_far_ntstatus(uint16_t const Number)
{
	// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/87fba13e-bf06-450e-83b1-9241dc81e781

	// These codes are used purely internally (so far), so we don't really need to do all this.
	// However, why not.

	const unsigned Severity = STATUS_SEVERITY_ERROR;

	const auto FarMagic =
		'F' << 16 |
		'A' << 8 |
		'R' << 0;

	const auto FarMagicCompressed =
		(FarMagic & 0x00FFF000) >> 12 ^
		(FarMagic & 0x00000FFF) >> 0;

	const auto FarFacility = FarMagicCompressed;

	return
		Severity    << 30 |
		1           << 29 |
		FarFacility << 16 |
		Number      << 0;
}

static constexpr NTSTATUS visual_cpp_exception(unsigned const Severity, unsigned const Error)
{
	return Severity | FACILITY_VISUALCPP << 16 | Error;
}

static constexpr NTSTATUS
	EH_EXCEPTION_NUMBER           = os::debug::EH_EXCEPTION_NUMBER,
	EH_DELAYLOAD_MODULE           = visual_cpp_exception(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND),
	EH_DELAYLOAD_PROCEDURE        = visual_cpp_exception(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND),
	EH_CLR_EXCEPTION              = 0xE0434352, // 'CCR'
	EH_SANITIZER                  = 0xE073616E, // 'san'
	EH_SANITIZER_ASAN             = EH_SANITIZER + 1,

	// Far-specific codes
	STATUS_FAR_ABORT              = make_far_ntstatus(0),
	STATUS_FAR_THREAD_RETHROW     = make_far_ntstatus(1);

static const auto DoubleSeparator = L"======================================================================"sv;
static const auto Separator       = L"----------------------------------------------------------------------"sv;
static const auto ColumnSeparator = L" | "sv;

static void make_header(string_view const Message, function_ref<void(string_view)> const Consumer)
{
	Consumer({});

	Consumer(DoubleSeparator);
	Consumer(Message);
	Consumer(DoubleSeparator);
}

static void make_subheader(string_view const Message, function_ref<void(string_view)> const Consumer)
{
	Consumer({});

	Consumer(Separator);
	Consumer(Message);
	Consumer(Separator);
}

static string get_report_location()
{
	const auto SubDir = unique_name();

	if (const auto CrashLogs = path::join(Global? Global->Opt->LocalProfilePath : L"."sv, L"CrashLogs"); os::fs::is_directory(CrashLogs) || os::fs::create_directory(CrashLogs))
	{
		if (const auto Path = path::join(CrashLogs, SubDir); os::fs::create_directory(Path))
		{
			return Path;
		}
	}

	if (string TempPath; os::fs::GetTempPath(TempPath))
	{
		if (const auto Path = path::join(TempPath, SubDir); os::fs::create_directory(Path))
		{
			return Path;
		}
	}

	if (os::fs::create_directory(SubDir))
		return SubDir;

	return L"."s;
}

static bool write_readme(string_view const FullPath)
{
	os::fs::file const File(FullPath, GENERIC_WRITE, os::fs::file_share_read, nullptr, CREATE_ALWAYS);
	if (!File)
	{
		const auto LastError = os::last_error();
		LOGERROR(L"Error opening {}: {}"sv, FullPath, LastError);
		return false;
	}

#define EOL "\r\n"

	// English text, ANSI will do fine.
	const auto Data =
		"Please send " BUGREPORT_NAME " and " MINIDUMP_NAME " to the developers:" EOL
		EOL
		"  https://github.com/FarGroup/FarManager/issues" EOL
		"  https://bugs.farmanager.com" EOL
		"  https://forum.farmanager.com/viewforum.php?f=37" EOL
		"  https://forum.farmanager.com/viewforum.php?f=9" EOL
		EOL
		"Please include the steps needed to reproduce the problem" EOL
		"and any other potentially useful information." EOL
		EOL
		"------------------------------------------------------------" EOL
		"DO NOT SHARE " FULLDUMP_NAME " UNLESS EXPLICITLY ASKED TO DO SO." EOL
		"It could contain sensitive data." EOL
		"------------------------------------------------------------" EOL
		""sv;

#undef EOL

	if (File.Write(Data.data(), Data.size() * sizeof(Data[0])))
		return true;

	const auto LastError = os::last_error();
	LOGERROR(L"Error writing to {}: {}"sv, FullPath, LastError);
	return false;
}

static bool write_report(string_view const Data, string_view const FullPath)
{
	os::fs::file const File(FullPath, GENERIC_WRITE, os::fs::file_share_read, nullptr, CREATE_ALWAYS);
	if (!File)
	{
		const auto LastError = os::last_error();
		LOGERROR(L"Error opening {}: {}"sv, FullPath, LastError);
		return false;
	}

	if (File.Write(Data.data(), Data.size() * sizeof(decltype(Data)::value_type)))
		return true;

	const auto LastError = os::last_error();
	LOGERROR(L"Error writing to {}: {}"sv, FullPath, LastError);
	return false;
}

static bool write_minidump(const exception_context& Context, string_view const FullPath, MINIDUMP_TYPE const Type)
{
#ifdef _DEBUG
	if (Type & MiniDumpWithFullMemory)
		return false;
#endif

	if (!imports.MiniDumpWriteDump)
		return false;

	const os::fs::file DumpFile(FullPath, GENERIC_WRITE, os::fs::file_share_read, nullptr, CREATE_ALWAYS);
	if (!DumpFile)
	{
		const auto LastError = os::last_error();
		LOGERROR(L"Error opening {}: {}"sv, FullPath, LastError);
		return false;
	}

	auto ExceptionRecord = Context.exception_record();
	auto ContextRecord = Context.context_record();
	EXCEPTION_POINTERS Pointers{ &ExceptionRecord, &ContextRecord };
	MINIDUMP_EXCEPTION_INFORMATION Mei{ Context.thread_id(), &Pointers };

	// https://docs.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump#remarks
	// MiniDumpWriteDump may not produce a valid stack trace for the calling thread.
	// You can call the function from a new worker thread and filter this worker thread from the dump.

	bool Result = false;
	os::thread([&]
	{
		struct writer_context
		{
			DWORD const ThreadId{ GetCurrentThreadId() };

			static BOOL WINAPI callback(void* const Param, MINIDUMP_CALLBACK_INPUT* const Input, MINIDUMP_CALLBACK_OUTPUT*)
			{
				const auto& Ctx = *static_cast<writer_context const*>(Param);

				if (Input->CallbackType == IncludeThreadCallback && Input->IncludeThread.ThreadId == Ctx.ThreadId)
					return FALSE;

				return TRUE;
			}
		}
		WriterContext;

		MINIDUMP_CALLBACK_INFORMATION Mci
		{
			&writer_context::callback,
			&WriterContext
		};

		auto DegradedType = Type;

		for (;;)
		{
			Result = imports.MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), DumpFile.get().native_handle(), DegradedType, &Mei, {}, &Mci) != FALSE;
			if (Result)
			{
				if (DegradedType != Type)
					LOGWARNING(L"MiniDumpWriteDump(): requested: 0x{:08X}, accepted: 0x{:08X}, rejected: 0x{:08X}"sv, Type, DegradedType, Type & ~DegradedType);

				break;
			}

			if (!any_of(static_cast<int>(os::last_error().Win32Error), ERROR_INVALID_PARAMETER, E_INVALIDARG))
				break;

			// 1. Each version of dbghelp.dll has its own set of supported flags and rejects everything else.
			// 2. It is not possible to reliably determine the version of dbghelp.dll.
			// As retarded as it gets.

			// The best we can do is try over and over, dropping the flags one by one from newest to oldest.
			DegradedType = static_cast<MINIDUMP_TYPE>(DegradedType & ~bit(std::bit_width(static_cast<uint32_t>(DegradedType)) - 1));
		}
	});

	return Result;
}

static void read_modules(std::span<HMODULE const> const Modules, string& To, string_view const Eol)
{
	string Name;
	os::version::file_version FileVersion;

	for (const auto& i: Modules)
	{
		To += str(static_cast<void const*>(i));

		if (!os::fs::get_module_file_name({}, i, Name))
		{
			append(To, ColumnSeparator, os::last_error().to_string(), Eol);
			continue;
		}

		append(To, ColumnSeparator, Name);

		if (!FileVersion.read(Name))
		{
			append(To, ColumnSeparator, os::last_error().Win32ErrorStr(), Eol);
			continue;
		}

		if (const auto Description = FileVersion.description(); !Description.empty())
			append(To, ColumnSeparator, Description);

		if (const auto Version = FileVersion.version(); !Version.empty())
			append(To, ColumnSeparator, Version);
		else
			append(To, ColumnSeparator, os::last_error().Win32ErrorStr());

		To += Eol;
	}
}

static void read_modules(string& To, string_view const Eol)
{
	HMODULE ModulesStatic[1024];
	std::vector<HMODULE> ModulesDynamic;

	auto Data = ModulesStatic;
	DWORD Size = sizeof(ModulesStatic);
	DWORD Needed = 0;

	for (;;)
	{
		if (!EnumProcessModules(GetCurrentProcess(), Data, Size, &Needed))
		{
			const auto LastError = os::last_error();
			far::format_to(To, L"{}"sv, LastError);
			LOGWARNING(L"EnumProcessModules(): {}"sv, LastError);
			return;
		}

		if (Needed <= Size)
			return read_modules({ Data, Needed / sizeof(HMODULE) }, To, Eol);

		ModulesDynamic.resize(Needed / sizeof(HMODULE));
		Data = ModulesDynamic.data();
		Size = Needed;
	}
}

static void read_env(string& To, string_view const Eol)
{
	const os::env::provider::strings EnvStrings;
	for (const auto& i: enum_substrings(EnvStrings.data()))
	{
		if (starts_with_icase(i, L"FAR"sv))
		{
			append(To, i, Eol);
		}
	}
}

static string self_version()
{
	const auto Version = far::format(L"{} {}"sv, version_to_string(build::version()), build::platform());
	const auto ScmRevision = build::scm_revision();
	return ScmRevision.empty()? Version : Version + far::format(L" ({:.7})"sv, ScmRevision);
}

static string timestamp(SYSTEMTIME const& SystemTime)
{
	const auto [Date, Time] = format_datetime(SystemTime);
	return concat(Date, L' ', Time);
}

static string timestamp(os::chrono::time_point const Point)
{
	const auto FileTime = os::chrono::nt_clock::to_filetime(Point);
	SYSTEMTIME SystemTime{};
	if (!FileTimeToSystemTime(&FileTime, &SystemTime))
	{
		LOGWARNING(L"FileTimeToSystemTime(): {}"sv, os::last_error());
		return far::format(L"{:16X}"sv, Point.time_since_epoch().count());
	}

	return timestamp(SystemTime);
}

static string pe_timestamp()
{
	const auto FarModule = GetModuleHandle({});
	const auto& FarDosHeader = view_as<IMAGE_DOS_HEADER>(FarModule, 0);
	const auto& FarNtHeaders = view_as<IMAGE_NT_HEADERS>(FarModule, FarDosHeader.e_lfanew);
	return timestamp(os::chrono::nt_clock::from_time_t(FarNtHeaders.FileHeader.TimeDateStamp));
}

static string file_timestamp()
{
	os::fs::find_data Data;
	const auto& ModuleName = Global? Global->g_strFarModuleName : os::fs::get_current_process_file_name();

	if (!os::fs::get_find_data(ModuleName, Data))
	{
		const auto LastError = os::last_error();
		LOGWARNING(L"get_find_data({}): {}"sv, ModuleName, LastError);
		return LastError.to_string();
	}

	return timestamp(Data.LastWriteTime);
}

static string system_timestamp()
{
	return timestamp(os::chrono::nt_clock::now());
}

static string local_timestamp()
{
	SYSTEMTIME LocalTime;
	if (!os::chrono::utc_to_local(os::chrono::nt_clock::now(), LocalTime))
		return {};

	return far::format(L"{} {}"sv, timestamp(LocalTime), MkStrFTime(L"%z, %Z"sv));
}

static void read_registers(string& To, CONTEXT const& Context, string_view const Eol)
{
	const auto r = [&](string_view const Name, auto const Value)
	{
		far::format_to(To, L"{:3} = {:0{}X}{}"sv, Name, Value, sizeof(Value) * 2, Eol);
	};

#if defined _M_X64
	r(L"RAX"sv, Context.Rax);
	r(L"RBX"sv, Context.Rbx);
	r(L"RCX"sv, Context.Rcx);
	r(L"RDX"sv, Context.Rdx);
	r(L"RSI"sv, Context.Rsi);
	r(L"RDI"sv, Context.Rdi);
	r(L"R8 "sv, Context.R8);
	r(L"R9 "sv, Context.R9);
	r(L"R10"sv, Context.R10);
	r(L"R11"sv, Context.R11);
	r(L"R12"sv, Context.R12);
	r(L"R13"sv, Context.R13);
	r(L"R14"sv, Context.R14);
	r(L"R15"sv, Context.R15);
	r(L"RIP"sv, Context.Rip);
	r(L"RSP"sv, Context.Rsp);
	r(L"RBP"sv, Context.Rbp);
	r(L"EFL"sv, Context.EFlags);
#elif defined _M_IX86
	r(L"EAX"sv, Context.Eax);
	r(L"EBX"sv, Context.Ebx);
	r(L"ECX"sv, Context.Ecx);
	r(L"EDX"sv, Context.Edx);
	r(L"ESI"sv, Context.Esi);
	r(L"EDI"sv, Context.Edi);
	r(L"EIP"sv, Context.Eip);
	r(L"ESP"sv, Context.Esp);
	r(L"EBP"sv, Context.Ebp);
	r(L"EFL"sv, Context.EFlags);
#elif defined _M_ARM64
	r(L"X0 "sv, Context.X0);
	r(L"X1 "sv, Context.X1);
	r(L"X2 "sv, Context.X2);
	r(L"X3 "sv, Context.X3);
	r(L"X4 "sv, Context.X4);
	r(L"X5 "sv, Context.X5);
	r(L"X6 "sv, Context.X6);
	r(L"X7 "sv, Context.X7);
	r(L"X8 "sv, Context.X8);
	r(L"X9 "sv, Context.X9);
	r(L"X10"sv, Context.X10);
	r(L"X11"sv, Context.X11);
	r(L"X12"sv, Context.X12);
	r(L"X13"sv, Context.X13);
	r(L"X14"sv, Context.X14);
	r(L"X15"sv, Context.X15);
	r(L"X16"sv, Context.X16);
	r(L"X17"sv, Context.X17);
	r(L"X18"sv, Context.X18);
	r(L"X19"sv, Context.X19);
	r(L"X20"sv, Context.X20);
	r(L"X21"sv, Context.X21);
	r(L"X22"sv, Context.X22);
	r(L"X23"sv, Context.X23);
	r(L"X24"sv, Context.X24);
	r(L"X25"sv, Context.X25);
	r(L"X26"sv, Context.X26);
	r(L"X27"sv, Context.X27);
	r(L"X28"sv, Context.X28);
	r(L"FP "sv, Context.Fp);
	r(L"SP "sv, Context.Sp);
	r(L"LR "sv, Context.Lr);
	r(L"PC "sv, Context.Pc);
	r(L"CPS"sv, Context.Cpsr);
#elif defined _M_ARM
	r(L"R0 "sv, Context.R0);
	r(L"R1 "sv, Context.R1);
	r(L"R2 "sv, Context.R2);
	r(L"R3 "sv, Context.R3);
	r(L"R4 "sv, Context.R4);
	r(L"R5 "sv, Context.R5);
	r(L"R6 "sv, Context.R6);
	r(L"R7 "sv, Context.R7);
	r(L"R8 "sv, Context.R8);
	r(L"R9 "sv, Context.R9);
	r(L"R10"sv, Context.R10);
	r(L"R11"sv, Context.R11);
	r(L"R12"sv, Context.R12);
	r(L"SP "sv, Context.Sp);
	r(L"LR "sv, Context.Lr);
	r(L"PC "sv, Context.Pc);
	r(L"CPS"sv, Context.Cpsr);
#else
	COMPILER_WARNING("Unknown platform")
#endif
}

WARNING_PUSH()
WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
class DebugOutputCallbacks final:
	public IDebugOutputCallbacks,
	public IDebugOutputCallbacksWide
{
public:
	static constexpr ULONG CallbackTypes =
		DEBUG_OUTPUT_NORMAL |
		DEBUG_OUTPUT_ERROR |
		DEBUG_OUTPUT_WARNING |
		DEBUG_OUTPUT_VERBOSE;

	explicit DebugOutputCallbacks(string* To):
		m_To(To)
	{}

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID InterfaceId, PVOID* Interface) override
	{
		if (none_of(InterfaceId, IID_IUnknown, IID_IDebugOutputCallbacks, IID_IDebugOutputCallbacksWide))
		{
			*Interface = {};
			return E_NOINTERFACE;
		}

		*Interface = this;
		AddRef();
		return S_OK;
	}

	// IUnknown
	STDMETHOD_(ULONG, AddRef)() override
	{
		return 1;
	}

	// IUnknown
	STDMETHOD_(ULONG, Release)() override
	{
		return 0;
	}

	// IDebugOutputCallbacks
	STDMETHOD(Output)(ULONG Mask, PCSTR Text) override
	{
		output_impl(Mask, encoding::ansi::get_chars(Text));
		return S_OK;
	}

	// IDebugOutputCallbacksWide
	STDMETHOD(Output)(ULONG Mask, PCWSTR Text) override
	{
		output_impl(Mask, Text);
		return S_OK;
	}

private:
	static auto output_type_to_level(ULONG const OutputType)
	{
		switch (OutputType)
		{
		default:
		case DEBUG_OUTPUT_VERBOSE: return logging::level::info;
		case DEBUG_OUTPUT_WARNING: return logging::level::warning;
		case DEBUG_OUTPUT_ERROR:   return logging::level::error;
		}
	}

	void output_impl(ULONG const Mask, string_view const Text) const
	{
		switch (Mask & CallbackTypes)
		{
		case DEBUG_OUTPUT_NORMAL:
			*m_To += Text;
			return;

		default:
			LOG(output_type_to_level(Mask), L"{}"sv, Text);
		}
	}

	string* m_To;
};
WARNING_POP()

class debug_client
{
public:
	explicit debug_client(string& To):
		m_To(&To),
		m_Callbacks(m_To)
	{
	}

	void disassembly(string_view const Module, std::span<os::debug::stack_frame const> const Stack, string_view const Eol)
	{
		if (Stack.empty())
			return;

		try
		{
			if (!m_DebugControl)
				initialize();

			const auto DisassembleFlags =
				DEBUG_DISASM_EFFECTIVE_ADDRESS |
				DEBUG_DISASM_MATCHING_SYMBOLS |
				DEBUG_DISASM_SOURCE_LINE_NUMBER |
				DEBUG_DISASM_SOURCE_FILE_NAME;

			const auto MaxFrames = 10;
			auto Frames = 0;

			for (const auto i: Stack)
			{
				if (os::debug::is_inline_frame(i.InlineContext))
					continue;

				tracer.get_symbols(Module, { &i, 1 }, [&](string_view const Line)
				{
					append(*m_To, Line, L':', Eol);
				});

				const auto PrevLines = 10;
				if (const auto Result = m_DebugControl->OutputDisassemblyLines(
					DEBUG_OUTCTL_THIS_CLIENT,
					PrevLines,
					PrevLines + 1,
					i.Address,
					DisassembleFlags,
					{},
					{},
					{},
					{}
				); FAILED(Result))
				{
					LOGWARNING(L"OutputDisassemblyLines(): {}"sv, os::format_error(Result));
				}

				if (++Frames == MaxFrames)
					break;

				*m_To += Eol;
			}
		}
		catch (os::com::exception const& e)
		{
			LOGWARNING(L"{}"sv, e);
		}

	}

private:
	void initialize()
	{
		if (m_DebugControl)
			return;

		if (!imports.DebugCreate)
			return;

		COM_INVOKE(imports.DebugCreate, (IID_IDebugClient, IID_PPV_ARGS_Helper(&ptr_setter(m_DebugClient))));

		COM_INVOKE(m_DebugClient->AttachProcess, ({}, GetCurrentProcessId(), DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND));

		COM_INVOKE(m_DebugClient->QueryInterface, (IID_IDebugControl, IID_PPV_ARGS_Helper(&ptr_setter(m_DebugControl))));

		if (const auto Result = m_DebugControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE); FAILED(Result))
			LOGWARNING(L"WaitForEvent(): {}"sv, os::format_error(Result));

		if (const auto Result = m_DebugClient->SetOutputMask(DebugOutputCallbacks::CallbackTypes); FAILED(Result))
			LOGWARNING(L"SetOutputMask(): {}"sv, os::format_error(Result));

		if (os::com::ptr<IDebugClient5> DebugClient5; SUCCEEDED(m_DebugClient->QueryInterface(IID_IDebugClient5, IID_PPV_ARGS_Helper(&ptr_setter(DebugClient5)))))
			COM_INVOKE(DebugClient5->SetOutputCallbacksWide, (&m_Callbacks));
		else
			COM_INVOKE(m_DebugClient->SetOutputCallbacks, (&m_Callbacks));
	}

	string* m_To;
	DebugOutputCallbacks m_Callbacks;
	os::com::ptr<IDebugClient> m_DebugClient;
	os::com::ptr<IDebugControl> m_DebugControl;
};

enum class handler_result
{
	execute_handler,
	continue_execution,
	continue_search,
};

static handler_result ExcDialog(bool const CanContinue, string const& ReportLocation, string const& PluginInformation)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	DialogBuilder Builder(lng::MExceptionDialogTitle);
	if (!PluginInformation.empty())
	{
		Builder.AddText(PluginInformation).Flags |= DIF_SHOWAMPERSAND;
		Builder.AddSeparator();
	}
	Builder.AddText(lng::MExceptionDialogMessage1);
	Builder.AddText(lng::MExceptionDialogMessage2);
	Builder.AddConstEditField(ReportLocation, 70);
	Builder.AddSeparator();

	std::vector<lng> Buttons;
	Buttons.reserve(4);
	std::optional<intptr_t> ContinueId, UnloadId;

	intptr_t const TerminateId = Buttons.size();
	Buttons.emplace_back(lng::MExcTerminate);

	if (!PluginInformation.empty())
	{
		UnloadId = Buttons.size();
		Buttons.emplace_back(lng::MExcUnload);
	}

	if (CanContinue)
	{
		ContinueId = Buttons.size();
		Buttons.emplace_back(lng::MExcContinue);
	}

	Buttons.emplace_back(lng::MIgnore);

	Builder.AddButtons(Buttons);

	Builder.SetDialogMode(DMODE_WARNINGSTYLE | DMODE_NOPLUGINS);
	Builder.SetScrObjFlags(FSCROBJ_SPECIAL);

	const auto Result = Builder.ShowDialogEx();

	if (Result == TerminateId)
	{
		UseTerminateHandler = true;
		return handler_result::execute_handler;
	}

	if (Result == UnloadId)
		return handler_result::execute_handler;

	if (Result == ContinueId)
		return handler_result::continue_execution;

	return handler_result::continue_search;
}

static void print_exception_message(string const& ReportLocation, string const& PluginInformation)
{
	const auto Eol = eol::std.str();

	std::array LngMsgs
	{
		L"Oops"sv,
		L"Something went wrong."sv,
		L"Please send the bug report to the developers."sv,
	};

	if (far_language::instance().is_loaded())
	{
		LngMsgs =
		{
			msg(lng::MExceptionDialogTitle),
			msg(lng::MExceptionDialogMessage1),
			msg(lng::MExceptionDialogMessage2),
		};
	}

	std::wcerr <<
		Eol <<
		Eol <<
		DoubleSeparator << Eol <<
		LngMsgs[0] << Eol <<
		Separator << Eol;

	if (!PluginInformation.empty())
	{
		std::wcerr <<
			PluginInformation << Eol <<
			Separator << Eol;
	}

	std::wcerr <<
		LngMsgs[1] << Eol <<
		LngMsgs[2] << Eol <<
		Separator << Eol <<
		ReportLocation << Eol <<
		Separator << Eol;
}

static handler_result ExcConsole(bool const CanContinue, string const& ReportLocation, string const& PluginInformation)
{
	string Message;
	string Keys;
	std::optional<intptr_t> ContinueId, UnloadId;

	intptr_t const TerminateId = Keys.size();
	Keys += L'T';
	Message += L"Terminate Far (T)\n"sv;

	if (!PluginInformation.empty())
	{
		UnloadId = Keys.size();
		Keys += L'U';
		Message += L"Unload plugin (U)\n"sv;
	}

	if (CanContinue)
	{
		ContinueId = Keys.size();
		Keys += L'C';
		Message += L"Continue (C)\n"sv;
	}

	Keys += L'I';
	Message += L"Ignore (I)\n"sv;

	Message += L"\nChoose one"sv;

	intptr_t const Result = ConsoleChoice(Message, Keys, TerminateId, [&]{ print_exception_message(ReportLocation, PluginInformation); });

	if (Result == TerminateId)
	{
		UseTerminateHandler = true;
		return handler_result::execute_handler;
	}

	if (Result == UnloadId)
		return handler_result::execute_handler;

	if (Result == ContinueId)
		return handler_result::continue_execution;

	return handler_result::continue_search;
}

static string get_locale()
{
	wchar_t NameBuffer[LOCALE_NAME_MAX_LENGTH];
	string_view Name;

	if (size_t const SizeWith0 = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SNAME, NameBuffer, static_cast<int>(std::size(NameBuffer))))
		Name = { NameBuffer, SizeWith0 - 1 };
	else
		Name = L"Unknown"sv;

	const auto LocaleId = GetUserDefaultLCID();
	const auto LanguageId = LANGIDFROMLCID(LocaleId);
	return far::format(L"{} | LCID={:08X} (Lang={:04X} (Primary={:03X} Sub={:02X}) Sort={:X} SortVersion={:X}) | ANSI={} OEM={}"sv,
		Name,
		LocaleId,
		LanguageId,
		PRIMARYLANGID(LanguageId),
		SUBLANGID(LanguageId),
		SORTIDFROMLCID(LocaleId),
		SORTVERSIONFROMLCID(LocaleId),
		encoding::codepage::ansi(),
		encoding::codepage::oem()
	);
}

static DWORD get_console_host_pid_from_nt()
{
	ULONG_PTR ConsoleHostProcess;
	if (const auto Status = imports.NtQueryInformationProcess(GetCurrentProcess(), ProcessConsoleHostProcess, &ConsoleHostProcess, sizeof(ConsoleHostProcess), {}); !NT_SUCCESS(Status))
		throw far_exception(error_state_ex{{ 0, Status }});

	return static_cast<DWORD>(ConsoleHostProcess & ~0b11);
}

static DWORD get_console_host_pid_from_window()
{
	// When you call GetWindowThreadProcessId(GetConsoleWindow()),
	// Windows lies and returns the ids of the hosted console process,
	// even though the console window is owned by the console host itself.
	// Amusingly, the HWND returned from ImmGetDefaultIMEWnd is not covered
	// by these shenanigans, allowing us to get real host ids.
	// Apparently this is also the only way to do it on WOW64,
	// since ProcessConsoleHostProcess doesn't work there.
	// Yes, it's horrible, but it's better than nothing.
	const auto ImeWnd = ImmGetDefaultIMEWnd(GetConsoleWindow());
	if (!ImeWnd)
		throw far_exception(error_state_ex{{ GetLastError(), 0 }});

	DWORD ProcessId;
	if (!GetWindowThreadProcessId(ImeWnd, &ProcessId))
		throw far_exception(error_state_ex{{ GetLastError(), 0 }});

	return ProcessId;
}

static std::variant<DWORD, string> get_console_host_pid()
{
	try
	{
		return get_console_host_pid_from_nt();
	}
	catch (far_exception const& e1)
	{
		try
		{
			return get_console_host_pid_from_window();
		}
		catch (far_exception const& e2)
		{
			return concat(e1.to_string(), L", "sv, e2.to_string());
		}
	}
}

static string get_console_host()
{
	const auto ConsoleHostProcessId = get_console_host_pid();
	if (ConsoleHostProcessId.index() == 1)
		return std::get<1>(ConsoleHostProcessId);

	const auto ConhostName = os::process::get_process_name(std::get<0>(ConsoleHostProcessId));
	if (ConhostName.empty())
		return {};

	const auto ConhostVersion = os::version::get_file_version(ConhostName);
	const auto ConhostLegacy = console.IsVtSupported()? L""sv : L" (legacy mode)"sv;

	return concat(ConhostName, L' ', ConhostVersion, ConhostLegacy);
}

template<typename process_basic_information_t>
static auto parent_process_id(process_basic_information_t const& Info)
{
	// GCC headers for once got it right
	if constexpr (requires {Info.InheritedFromUniqueProcessId; })
		return static_cast<DWORD>(Info.InheritedFromUniqueProcessId);
	// Windows SDK (at least up to 19041) defines it as "Reserved3".
	// Surprisingly, MSDN calls it InheritedFromUniqueProcessId, so it might get renamed one day.
	// For forward compatibility it's better to use the compiler rather than the preprocessor here.
	else if constexpr (requires { Info.Reserved3; })
		return static_cast<DWORD>(std::bit_cast<uintptr_t>(Info.Reserved3));
	else
		static_assert(!sizeof(Info));
}

static string get_parent_process()
{
	PROCESS_BASIC_INFORMATION ProcessInfo;
	if (const auto Status = imports.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &ProcessInfo, sizeof(ProcessInfo), {}); !NT_SUCCESS(Status))
		return os::format_ntstatus(Status);

	const auto ParentProcessId = parent_process_id(ProcessInfo);

	const auto ParentName = os::process::get_process_name(ParentProcessId);
	if (ParentName.empty())
		return {};

	const auto ParentVersion = os::version::get_file_version(ParentName);

	return concat(ParentName, L' ', ParentVersion);
}

static string get_uptime()
{
	os::chrono::time_point CreationTime;
	if (!os::chrono::get_process_creation_time(GetCurrentProcess(), CreationTime))
		return os::last_error().to_string();

	return ConvertDurationToHMS(os::chrono::nt_clock::now() - CreationTime);
}

static auto memory_status()
{
	const auto size_to_str = [](uint64_t const Size)
	{
		return FileSizeToStrInvariant(Size, 0, COLFLAGS_FLOATSIZE | COLFLAGS_SHOW_MULTIPLIER);
	};

	string MemoryStatus;

	if (MEMORYSTATUSEX ms{ sizeof(ms) }; GlobalMemoryStatusEx(&ms))
	{
		MemoryStatus = far::format(
			L"{} out of {} free ({}%)"sv,
			size_to_str(ms.ullAvailPageFile),
			size_to_str(ms.ullTotalPageFile),
			ToPercent(ms.ullAvailPageFile, ms.ullTotalPageFile)
		);
	}

	if (PROCESS_MEMORY_COUNTERS pmc{ sizeof(pmc) }; GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		far::format_to(MemoryStatus, L"{}{} used by the process"sv, MemoryStatus.empty()? L""sv : L"; "sv, size_to_str(pmc.PagefileUsage));
	}

	return MemoryStatus;
}

namespace detail
{
	struct PMD
	{
		int mdisp; // Offset of intended data within base
		int pdisp; // Displacement to virtual base pointer
		int vdisp; // Index within vbTable to offset of base
	};

	using PMFN = int; // Image relative offset of Member Function

	struct CatchableType
	{
		unsigned int properties;        // Catchable Type properties (Bit field)
		int          pType;             // Image relative offset of TypeDescriptor
		PMD          thisDisplacement;  // Pointer to instance of catch type within thrown object.
		int          sizeOrOffset;      // Size of simple-type object or offset into buffer of 'this' pointer for catch object
		PMFN         copyFunction;      // Copy constructor or CC-closure
	};

	struct CatchableTypeArray
	{
		int nCatchableTypes;
		int arrayOfCatchableTypes; // Image relative offset of Catchable Types
	};

	struct ThrowInfo
	{
		unsigned int attributes;           // Throw Info attributes (Bit field)
		PMFN         pmfnUnwind;           // Destructor to call when exception has been handled or aborted
		int          pForwardCompat;       // Image relative offset of Forward compatibility frame handler
		int          pCatchableTypeArray;  // Image relative offset of CatchableTypeArray
	};

	using PCImgDelayDescr = void*;

	struct DelayLoadProc
	{
		BOOL       fImportByName;
		union
		{
			LPCSTR szProcName;
			DWORD  dwOrdinal;
		};
	};

	struct DelayLoadInfo
	{
		DWORD           cb;          // size of structure
		PCImgDelayDescr pidd;        // raw form of data (everything is there)
		FARPROC*        ppfn;        // points to address of function to load
		LPCSTR          szDll;       // name of dll
		DelayLoadProc   dlp;         // name or ordinal of procedure
		HMODULE         hmodCur;     // the hInstance of the library we have loaded
		FARPROC         pfnCur;      // the actual function that will be called
		DWORD           dwLastError; // error received (if an error notification)
	};

	// https://github.com/microsoft/onefuzz/blob/main/src/agent/input-tester/src/test_result/asan.rs
	// All pointers are probably 64-bit in the original definition, but we don't care here since it's all within the same process.
	struct EXCEPTION_ASAN_ERROR
	{
		// The description string from asan, such as heap-use-after-free
		UINT64 uiRuntimeDescriptionLength;
		LPCWSTR pwRuntimeDescription;

		// A translation of the description string to something more user friendly, not localized
		UINT64 uiRuntimeShortMessageLength;
		LPCWSTR pwRuntimeShortMessage;

		// the full report from asan, not localized
		UINT64 uiRuntimeFullMessageLength;
		LPCWSTR pwRuntimeFullMessage;

		// azure payload, WIP
		UINT64 uiCustomDataLength;
		LPCWSTR pwCustomData;
	};

	struct EXCEPTION_SANITIZER_ERROR
	{
		DWORD cbSize;
		DWORD dwSanitizerKind;
		union
		{
			EXCEPTION_ASAN_ERROR Asan;
		}
		u;
	};

	struct THREAD_BASIC_INFORMATION
	{
		NTSTATUS  ExitStatus;
		PVOID     TebBaseAddress;
		CLIENT_ID ClientId;
		KAFFINITY AffinityMask;
		KPRIORITY Priority;
		KPRIORITY BasePriority;
	};
}

static bool is_cpp_exception(const EXCEPTION_RECORD& Record)
{
	return Record.ExceptionCode == static_cast<DWORD>(EH_EXCEPTION_NUMBER) && Record.NumberParameters >= 3;
}

static bool is_fake_cpp_exception(const EXCEPTION_RECORD& Record)
{
	return Record.ExceptionCode == static_cast<DWORD>(EH_EXCEPTION_NUMBER) && !Record.NumberParameters;
}

class [[nodiscard]] enum_catchable_objects: public enumerator<enum_catchable_objects, char const*>
{
	IMPLEMENTS_ENUMERATOR(enum_catchable_objects);

public:
	explicit enum_catchable_objects(EXCEPTION_RECORD const& Record)
	{
		if (!is_cpp_exception(Record))
			return;

		m_BaseAddress = Record.NumberParameters >= 4? Record.ExceptionInformation[3] : 0;
		const auto& ThrowInfoRef = view_as<detail::ThrowInfo>(Record.ExceptionInformation[2]);
		const auto& CatchableTypeArrayRef = view_as<detail::CatchableTypeArray>(m_BaseAddress + ThrowInfoRef.pCatchableTypeArray);
		m_CatchableTypesRVAs = { &CatchableTypeArrayRef.arrayOfCatchableTypes, static_cast<size_t>(CatchableTypeArrayRef.nCatchableTypes) };
	}

private:
	[[nodiscard, maybe_unused]]
	bool get(bool const Reset, char const*& Name) const
	{
		if (Reset)
			m_Index = 0;

		if (m_Index == m_CatchableTypesRVAs.size())
			return false;

		const auto& CatchableTypeRef = view_as<detail::CatchableType>(m_BaseAddress + m_CatchableTypesRVAs[m_Index++]);
		const auto& TypeInfoRef = view_as<std::type_info>(m_BaseAddress + CatchableTypeRef.pType);

		Name = TypeInfoRef.name();
		return true;
	}

	std::span<int const> m_CatchableTypesRVAs;
	size_t mutable m_Index{};
	uintptr_t m_BaseAddress{};
};

static string ExtractObjectType(EXCEPTION_RECORD const& xr)
{
	enum_catchable_objects const CatchableTypesEnumerator(xr);
	const auto Iterator = CatchableTypesEnumerator.cbegin();
	if (Iterator != CatchableTypesEnumerator.cend())
		return encoding::utf8::get_chars(*Iterator);

#if IS_MICROSOFT_SDK()
	return {};
#else
	const auto TypeInfo = abi::__cxa_current_exception_type();
	if (!TypeInfo)
		return {};

	const auto Name = TypeInfo->name();
	auto Status = -1;

	struct free_deleter
	{
		void operator()(void* Ptr) const
		{
			free(Ptr);
		}
	};

	std::unique_ptr<char, free_deleter> const DemangledName(abi::__cxa_demangle(Name, {}, {}, &Status));
	return encoding::utf8::get_chars(DemangledName.get());
#endif
}

static string_view exception_name(NTSTATUS const Code)
{
	switch (Code)
	{
#define CASE_STR(Code) case Code: return WIDE_SV(#Code);
	// TODO: Half of these probably can never occur and should be removed
	CASE_STR(STATUS_ACCESS_VIOLATION)
	CASE_STR(STATUS_DATATYPE_MISALIGNMENT)
	CASE_STR(STATUS_BREAKPOINT)
	CASE_STR(STATUS_SINGLE_STEP)
	CASE_STR(STATUS_ARRAY_BOUNDS_EXCEEDED)
	CASE_STR(STATUS_FLOAT_DENORMAL_OPERAND)
	CASE_STR(STATUS_FLOAT_DIVIDE_BY_ZERO)
	CASE_STR(STATUS_FLOAT_INEXACT_RESULT)
	CASE_STR(STATUS_FLOAT_INVALID_OPERATION)
	CASE_STR(STATUS_FLOAT_OVERFLOW)
	CASE_STR(STATUS_FLOAT_STACK_CHECK)
	CASE_STR(STATUS_FLOAT_UNDERFLOW)
	CASE_STR(STATUS_INTEGER_DIVIDE_BY_ZERO)
	CASE_STR(STATUS_INTEGER_OVERFLOW)
	CASE_STR(STATUS_PRIVILEGED_INSTRUCTION)
	CASE_STR(STATUS_IN_PAGE_ERROR)
	CASE_STR(STATUS_ILLEGAL_INSTRUCTION)
	CASE_STR(STATUS_NONCONTINUABLE_EXCEPTION)
	CASE_STR(STATUS_STACK_OVERFLOW)
	CASE_STR(STATUS_INVALID_DISPOSITION)
	CASE_STR(STATUS_GUARD_PAGE_VIOLATION)
	CASE_STR(STATUS_INVALID_HANDLE)
	CASE_STR(STATUS_POSSIBLE_DEADLOCK)
	CASE_STR(STATUS_CONTROL_C_EXIT)
	CASE_STR(STATUS_HEAP_CORRUPTION)
	CASE_STR(STATUS_NO_MEMORY)
	CASE_STR(STATUS_ASSERTION_FAILURE)
	CASE_STR(STATUS_INVALID_PARAMETER)
	CASE_STR(STATUS_INVALID_CRUNTIME_PARAMETER)
#undef CASE_STR

	case EH_EXCEPTION_NUMBER:           return L"C++ exception"sv;
	case EH_DELAYLOAD_MODULE:           return L"Delayload LoadLibrary error"sv;
	case EH_DELAYLOAD_PROCEDURE:        return L"Delayload GetProcAddress error"sv;
	case EH_CLR_EXCEPTION:              return L"CLR exception"sv;
	case EH_SANITIZER:                  return L"Sanitizer"sv;
	case STATUS_FAR_ABORT:              return L"std::abort"sv;
	default:                            return L"Unknown exception"sv;
	}
}

static string exception_name(EXCEPTION_RECORD const& ExceptionRecord, string_view const Type)
{
	const auto AppendType = [](string& Str, string_view const ExceptionType)
	{
		append(Str, L" ("sv, ExceptionType, ')');
	};

	const auto WithType = [&](string&& Str)
	{
		if (!Type.empty())
		{
			AppendType(Str, Type);
			return std::move(Str);
		}

		if (const auto TypeStr = ExtractObjectType(ExceptionRecord); !TypeStr.empty())
		{
			AppendType(Str, TypeStr);
		}
		return std::move(Str);
	};

	const auto Name = exception_name(ExceptionRecord.ExceptionCode);
	return WithType(far::format(L"0x{:0>8X} - {}"sv, ExceptionRecord.ExceptionCode, Name));
}

static string exception_details(string_view const Module, EXCEPTION_RECORD const& ExceptionRecord, string_view const Message, string& ExtraDetails)
{
	const auto default_details = [&]
	{
		return os::format_ntstatus(ExceptionRecord.ExceptionCode);
	};

	switch (const auto NtStatus = static_cast<NTSTATUS>(ExceptionRecord.ExceptionCode))
	{
	case STATUS_ACCESS_VIOLATION:
	case STATUS_IN_PAGE_ERROR:
		{
			if (!ExceptionRecord.NumberParameters)
				break;

			const auto Mode = [](ULONG_PTR Code)
			{
				switch (Code)
				{
				default:
				case EXCEPTION_READ_FAULT:    return L"read"sv;
				case EXCEPTION_WRITE_FAULT:   return L"written"sv;
				case EXCEPTION_EXECUTE_FAULT: return L"executed"sv;
				}
			}(ExceptionRecord.ExceptionInformation[0]);

			const auto Symbol = [&]
			{
				if (ExceptionRecord.NumberParameters < 2)
					return L"<unknown>"s;

				string SymbolName;
				tracer.get_symbols(Module, {{{ ExceptionRecord.ExceptionInformation[1], 0 }}}, [&](string&& Line)
				{
					SymbolName = std::move(Line);
				});

				if (SymbolName.empty())
					SymbolName = to_hex_wstring(ExceptionRecord.ExceptionInformation[1]);

				return SymbolName;
			}();

			auto Result = far::format(L"Memory at {} could not be {}"sv, Symbol, Mode);
			if (NtStatus == EXCEPTION_IN_PAGE_ERROR && ExceptionRecord.NumberParameters >= 3)
				append(Result, L": "sv, os::format_ntstatus(static_cast<NTSTATUS>(ExceptionRecord.ExceptionInformation[2])));

			return Result;
		}

	case STATUS_NO_MEMORY:
		{
			if (!ExceptionRecord.NumberParameters)
				break;

			return far::format(L"Unable to allocate {} bytes: {}"sv, ExceptionRecord.ExceptionInformation[0], default_details());
		}

	case EH_DELAYLOAD_MODULE:
	case EH_DELAYLOAD_PROCEDURE:
		{
			if (!ExceptionRecord.NumberParameters || !ExceptionRecord.ExceptionInformation[0])
				return {};

			const auto& Info = view_as<detail::DelayLoadInfo>(ExceptionRecord.ExceptionInformation[0]);
			return concat(
				encoding::ansi::get_chars(Info.szDll),
				L"::"sv,
				Info.dlp.fImportByName?
					encoding::ansi::get_chars(Info.dlp.szProcName) :
					str(Info.dlp.dwOrdinal),
				L": "sv,
				os::format_error(Info.dwLastError)
			);
		}

	case EH_EXCEPTION_NUMBER:
	case STATUS_FAR_ABORT:
		return string(Message);

	case STATUS_INVALID_CRUNTIME_PARAMETER:
		return Message.empty()?
			default_details() :
			far::format(L"{} Expression: {}"sv, default_details(), Message);

	case EH_CLR_EXCEPTION:
		{
			if (!ExceptionRecord.NumberParameters)
				return {};

			return os::format_error(ExceptionRecord.ExceptionInformation[0]);
		}

	case EH_SANITIZER:
		{
			if (!ExceptionRecord.NumberParameters || !ExceptionRecord.ExceptionInformation[0])
				return {};

			const auto& Record = view_as<detail::EXCEPTION_SANITIZER_ERROR>(ExceptionRecord.ExceptionInformation[0]);
			if (Record.cbSize < sizeof(Record))
				return L"Unrecognized sanitizer record size"s;

			switch (Record.dwSanitizerKind)
			{
			case static_cast<DWORD>(EH_SANITIZER_ASAN):
				if (Record.u.Asan.uiRuntimeFullMessageLength)
					ExtraDetails = { Record.u.Asan.pwRuntimeFullMessage, static_cast<size_t>(Record.u.Asan.uiRuntimeFullMessageLength) };
				else
					ExtraDetails = L"ASan report is missing, probably it is too large."s;

				return far::format(
					L"{} ({})"sv,
					string_view{ Record.u.Asan.pwRuntimeShortMessage, static_cast<size_t>(Record.u.Asan.uiRuntimeShortMessageLength) },
					string_view{ Record.u.Asan.pwRuntimeDescription, static_cast<size_t>(Record.u.Asan.uiRuntimeDescriptionLength) }
				);

			default:
				return L"Unrecognized sanitizer kind"s;
			}
		}
	}

	return default_details();
}

using thread_status = std::variant<NTSTATUS, os::error_state>;

static thread_status get_thread_status(HANDLE const Thread)
{
	if (!imports.NtQueryInformationThread)
		return STATUS_NOT_IMPLEMENTED;

	constexpr auto ThreadBasicInformation = static_cast<THREADINFOCLASS>(0);
	detail::THREAD_BASIC_INFORMATION BasicInformation;

	if (const auto Status = imports.NtQueryInformationThread(Thread, ThreadBasicInformation, &BasicInformation, sizeof(BasicInformation), {}); !NT_SUCCESS(Status))
		return Status;

	return os::error_state
	{
		os::get_last_error(BasicInformation.TebBaseAddress),
		os::get_last_nt_status(BasicInformation.TebBaseAddress)
	};
}

static string collect_information(
	exception_context const& Context,
	source_location const& Location,
	string_view const PluginInfo,
	string_view ModuleName,
	string_view const Type,
	string_view const Message,
	error_state_ex const& ErrorState,
	std::span<os::debug::stack_frame const> const NestedStack
)
{
	string Strings;
	Strings.reserve(1024);

	Strings.push_back(L'\uFEFF');

	const auto Eol = eol::system.str();

	const auto append_line = [&](string_view const Line = {})
	{
		append(Strings, Line, Eol);
	};

	const auto Exception = exception_name(Context.exception_record(), Type);

	string ExtraDetails;
	const auto Details = exception_details(ModuleName, Context.exception_record(), Message, ExtraDetails);

	string Address, Name, Source;
	tracer.get_symbol(ModuleName, Context.exception_record().ExceptionAddress, Address, Name, Source);

	Address.insert(0, L"0x"sv);

	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (!Source.empty())
		append(Address, L" ("sv, Source, L')');

	const auto Errno = ErrorState.ErrnoStr();
	const auto LastError = ErrorState.Win32ErrorStr();
	const auto LastNtStatus = ErrorState.NtErrorStr();
	const auto LocationStr = source_location_to_string(Location);
	const auto PluginInfoStr = PluginInfo.empty()? L"N/A"sv : PluginInfo;

	const auto Version = self_version();
	const auto Compiler = build::compiler();
	const auto PeTime = pe_timestamp();
	const auto FileTime = file_timestamp();
	const auto SystemTime = system_timestamp();
	const auto LocalTime = local_timestamp();
	const auto Uptime = get_uptime();
	const auto OsVersion = os::version::os_version();
	const auto Locale = get_locale();
	const auto ConsoleHost = get_console_host();
	const auto Parent = get_parent_process();
	const auto Command = GetCommandLine();
	const auto AccessLevel = os::security::is_admin()? L"Administrator"sv : L"User"sv;
	const auto MemoryStatus = memory_status();

	const auto
		LastErrorTitle = L"LastError:"sv,
		NtStatusTitle = L"NTSTATUS: "sv;

	struct info_block
	{
		string_view Name, Value;
	}
	const ExceptionInfo[]
	{
		{ L"Exception:"sv, Exception,     },
		{ L"Details:  "sv, Details,       },
		{ L"errno:    "sv, Errno,         },
		{ LastErrorTitle,  LastError,     },
		{ NtStatusTitle,   LastNtStatus,  },
		{ L"Address:  "sv, Address,       },
		{ L"Location: "sv, LocationStr,   },
		{ L"Module:   "sv, ModuleName,    },
		{ L"Plugin:   "sv, PluginInfoStr, },
	},
	SystemInfo[]
	{
		{ L"Far:      "sv, Version,       },
		{ L"Compiler: "sv, Compiler,      },
		{ L"PE time:  "sv, PeTime,        },
		{ L"File time:"sv, FileTime,      },
		{ L"Time:     "sv, SystemTime,    },
		{ L"Local:    "sv, LocalTime,     },
		{ L"Uptime:   "sv, Uptime,        },
		{ L"OS:       "sv, OsVersion,     },
		{ L"Locale:   "sv, Locale,        },
		{ L"Host:     "sv, ConsoleHost,   },
		{ L"Parent:   "sv, Parent,        },
		{ L"Command:  "sv, Command,       },
		{ L"Access:   "sv, AccessLevel,   },
		{ L"Memory:   "sv, MemoryStatus   },
	};

	const auto log_message = [](std::span<info_block const> const Info)
	{
		auto LogMessage = join(L"\n"sv, Info | std::views::transform([](info_block const& Param)
		{
			return far::format(L"{} {}"sv, Param.Name, Param.Value);
		}));

		LogMessage += L"\n\n"sv;

		return LogMessage;
	};

	LOGERROR(L"\n{}\n"sv, log_message(ExceptionInfo));

	const auto print_info_block = [&](std::span<info_block const> const Info)
	{
		for (const auto& [Label, Value] : Info)
		{
			far::format_to(Strings, L"{} {}{}"sv, Label, Value, Eol);
		}
	};

	make_header(L"Summary"sv, append_line);
	print_info_block(ExceptionInfo);

	make_header(L"System information"sv, append_line);
	print_info_block(SystemInfo);

	const auto
		StackTitle = L"Stack"sv,
		DisassemblyTitle = L"Disassembly"sv,
		RegistersTitle = L"Registers"sv;

	make_header(L"Exception"sv, append_line);

	if (!ExtraDetails.empty())
	{
		make_subheader(L"Details"sv, append_line);
		append_line(ExtraDetails);
	}

	make_subheader(StackTitle, append_line);
	if (!NestedStack.empty())
	{
		tracer.get_symbols(ModuleName, NestedStack, append_line);
		make_subheader(L"Rethrow stack"sv, append_line);
	}
	const auto Stack = tracer.stacktrace(ModuleName, Context.context_record(), Context.thread_handle());
	tracer.get_symbols(ModuleName, Stack, append_line);

	make_subheader(RegistersTitle, append_line);
	read_registers(Strings, Context.context_record(), Eol);

	// Read disassembly before modules - it will load dbgeng.dll and we might want to see it too just in case
	make_subheader(DisassemblyTitle, append_line);
	debug_client DebugClient(Strings);
	DebugClient.disassembly(ModuleName, NestedStack.empty()? Stack : NestedStack, Eol);

	make_header(L"Exception handler thread"sv, append_line);

	make_subheader(StackTitle, append_line);
	tracer.current_stacktrace(ModuleName, append_line);

	{
		os::process::enum_processes const Enum;
		const auto CurrentPid = GetCurrentProcessId();
		const auto CurrentThreadId = GetCurrentThreadId();
		const auto CurrentEntry = std::ranges::find(Enum, CurrentPid, &os::process::enum_process_entry::Pid);
		if (CurrentEntry != Enum.cend())
		{
			for (const auto& i: CurrentEntry->Threads)
			{
				const auto Tid = std::bit_cast<uintptr_t>(i.ClientId.UniqueThread);
				if (Tid == CurrentThreadId)
					continue;

				auto ThreadTitle = far::format(L"Thread {0} / 0x{0:X}"sv, Tid);

				os::handle const Thread(OpenThread(THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, false, Tid));
				if (!Thread)
				{
					make_header(ThreadTitle, append_line);
					append_line(far::format(L"Error opening thread {}: {}"sv, Tid, os::last_error()));
					continue;
				}

				SuspendThread(Thread.native_handle());
				SCOPE_EXIT{ ResumeThread(Thread.native_handle()); };

				if (const auto ThreadName = os::debug::get_thread_name(Thread.native_handle()); !ThreadName.empty())
					append(ThreadTitle, L" ("sv, ThreadName, L')');

				make_header(ThreadTitle, append_line);

				std::visit(overload
				{
					[&](NTSTATUS const Status)
					{
						append_line(far::format(L"Error getting thread status: {}"sv, os::format_ntstatus(Status)));
					},
					[&](os::error_state const& State)
					{
						append_line(concat(LastErrorTitle, ' ', State.Win32ErrorStr()));
						append_line(concat(NtStatusTitle, ' ', State.NtErrorStr()));
					}
				}, get_thread_status(Thread.native_handle()));

				CONTEXT ThreadContext{};
				ThreadContext.ContextFlags = CONTEXT_ALL;
				if (!GetThreadContext(Thread.native_handle(), &ThreadContext))
				{
					append_line(far::format(L"Error getting thread context: {}"sv, os::last_error()));
					continue;
				}

				make_subheader(StackTitle, append_line);
				const auto ThreadStack = tracer.stacktrace(ModuleName, ThreadContext, Thread.native_handle());
				tracer.get_symbols(ModuleName, ThreadStack, append_line);

				make_subheader(RegistersTitle, append_line);
				read_registers(Strings, ThreadContext, Eol);

				make_subheader(DisassemblyTitle, append_line);
				DebugClient.disassembly(ModuleName, ThreadStack, Eol);
			}
		}
	}

	make_header(L"Modules"sv, append_line);
	read_modules(Strings, Eol);

	make_header(L"Environment"sv, append_line);
	read_env(Strings, Eol);

	return Strings;
}

static handler_result handle_generic_exception(
	exception_context const& Context,
	source_location const& Location,
	Plugin const* const PluginModule,
	string_view const Type,
	string_view const Message,
	error_state_ex const& ErrorState,
	std::span<os::debug::stack_frame const> const NestedStack = {}
)
{
	static bool ExceptionHandlingIgnored = false;
	if (ExceptionHandlingIgnored)
		return handler_result::continue_search;

	if (s_ExceptionHandlingInprogress)
		return handler_result::execute_handler;

	s_ExceptionHandlingInprogress = true;
	SCOPE_EXIT{ s_ExceptionHandlingInprogress = false; };

	const auto ModuleName = PluginModule?
		PluginModule->ModuleName() :
		Global?
			Global->g_strFarModuleName :
			os::fs::get_current_process_file_name();

	SCOPED_ACTION(tracer_detail::tracer::with_symbols)(PluginModule? ModuleName : L""sv);

	const auto ReportLocation = get_report_location();

	LOGERROR(L"Unhandled exception, see {} for details"sv, ReportLocation);

	const auto PluginInfo = PluginModule?
	far::format(L"{} {} ({}, {})"sv,
		PluginModule->Title(),
		version_to_string(PluginModule->version()),
		PluginModule->Description(),
		PluginModule->Author()
	) :
	L""s;

	constexpr auto MinidumpFlags = static_cast<MINIDUMP_TYPE>(
		MiniDumpNormal |
		MiniDumpWithHandleData |
		MiniDumpWithUnloadedModules |
		MiniDumpWithIndirectlyReferencedMemory |
		MiniDumpWithProcessThreadData |
		MiniDumpWithThreadInfo
	);

	constexpr auto FulldumpFlags = static_cast<MINIDUMP_TYPE>(
		MinidumpFlags |
		MiniDumpWithFullMemory |
		MiniDumpIgnoreInaccessibleMemory
	);

	const auto MinidumpNormal = write_minidump(Context, path::join(ReportLocation, WIDE_SV(MINIDUMP_NAME)), MinidumpFlags);
	const auto MinidumpFull = write_minidump(Context, path::join(ReportLocation, WIDE_SV(FULLDUMP_NAME)), FulldumpFlags);
	const auto BugReport = collect_information(Context, Location, PluginInfo, ModuleName, Type, Message, ErrorState, NestedStack);
	const auto ReportOnDisk = write_report(BugReport, path::join(ReportLocation, WIDE_SV(BUGREPORT_NAME)));
	const auto ReportInClipboard = !ReportOnDisk && SetClipboardText(BugReport);
	const auto ReadmeOnDisk = write_readme(path::join(ReportLocation, L"README.txt"sv));
	const auto AnythingOnDisk = ReportOnDisk || MinidumpNormal || MinidumpFull || ReadmeOnDisk;

	if (AnythingOnDisk && os::is_interactive_user_session())
		OpenFolderInShell(ReportLocation);

	const auto UseDialog = !ForceStderrExceptionUI && Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown();
	const auto CanContinue = !(Context.exception_record().ExceptionFlags & EXCEPTION_NONCONTINUABLE);

	const auto Result = AnythingOnDisk || ReportInClipboard?
		(UseDialog? ExcDialog : ExcConsole)(
			CanContinue,
			AnythingOnDisk?
				ReportLocation :
				msg(lng::MExceptionDialogClipboard),
			PluginInfo
		) :
		// Should never happen - neither the filesystem nor clipboard are writable, so just dump it to the screen:
		ExcConsole(CanContinue, BugReport, PluginInfo);

	switch (Result)
	{
	case handler_result::continue_execution:
		break;

	case handler_result::execute_handler:
		if (!PluginModule && Global)
			Global->CriticalInternalError = true;
		break;

	case handler_result::continue_search:
		ExceptionHandlingIgnored = true;
		break;
	}

	return Result;
}

void restore_system_exception_handler()
{
	disable_exception_handling();
	os::unset_error_mode(SEM_NOGPFAULTERRORBOX);
}

class far_wrapper_exception final: public far_exception, public std::nested_exception
{
public:
	explicit far_wrapper_exception(source_location const& Location):
		far_exception(L"exception_ptr"sv, true, Location),
		m_ThreadHandle(std::make_shared<os::handle>(os::OpenCurrentThread())),
		m_Stack(tracer.exception_stacktrace({}))
	{
	}

	std::span<os::debug::stack_frame const> get_stack() const noexcept { return m_Stack; }

private:
	std::shared_ptr<os::handle> m_ThreadHandle;
	std::vector<os::debug::stack_frame> m_Stack;
};

static_assert(std::derived_from<far_wrapper_exception, std::nested_exception>);

std::exception_ptr wrap_current_exception(source_location const& Location)
{
	try
	{
		throw far_wrapper_exception(Location);
	}
	catch (...)
	{
		return std::current_exception();
	}
}

void rethrow_if(std::exception_ptr& Ptr)
{
	if (Ptr)
		std::rethrow_exception(std::exchange(Ptr, {}));
}

static std::pair<string, string> extract_nested_exceptions(EXCEPTION_RECORD const& Record, const std::exception& Exception, bool Top = true)
{
	std::pair<string, string> Result;
	auto& [ObjectType, What] = Result;

	ObjectType = ExtractObjectType(Record);

	// far_exception.what() returns additional information (function, file and line).
	// We don't need it on top level because it's extracted separately
	if (const auto FarException = Top? dynamic_cast<const detail::far_base_exception*>(&Exception) : nullptr)
	{
		What = FarException->message();
		if (ObjectType.empty())
			ObjectType = WIDE_SV_LITERAL(detail::far_base_exception);
	}
	else
	{
		What = encoding::utf8_or_ansi::get_chars(Exception.what());
		if (ObjectType.empty())
			ObjectType = WIDE_SV_LITERAL(std::exception);
	}

	try
	{
		std::rethrow_if_nested(Exception);
	}
	catch (std::exception const& e)
	{
		const auto& [NestedObjectType, NestedWhat] = extract_nested_exceptions(*os::debug::exception_information().ExceptionRecord, e, false);
		ObjectType = concat(NestedObjectType, L" -> "sv, ObjectType);
		What = concat(NestedWhat, L" -> "sv, What);
	}
	catch (...)
	{
		auto NestedObjectType = ExtractObjectType(*os::debug::exception_information().ExceptionRecord);
		if (NestedObjectType.empty())
			NestedObjectType = L"Unknown"sv;

		ObjectType = concat(NestedObjectType, L" -> "sv, ObjectType);
		What = concat(L"?"sv, L" -> "sv, What);
	}

	return Result;
}

static bool handle_std_exception(
	exception_context const& Context,
	const std::exception& e,
	const Plugin* const Module,
	source_location const& Location
)
{
	error_state_ex const LastError{ os::last_error(), {}, errno };
	const auto& [Type, What] = extract_nested_exceptions(Context.exception_record(), e);

	if (const auto FarException = dynamic_cast<const detail::far_base_exception*>(&e))
	{
		const auto NestedStack = [&]
		{
			const auto Wrapper = dynamic_cast<const far_wrapper_exception*>(&e);
			return Wrapper? Wrapper->get_stack() : std::span<os::debug::stack_frame const>{};
		}();

		return handle_generic_exception(Context, FarException->location(), Module, Type, What, *FarException, NestedStack) == handler_result::execute_handler;
	}

	return handle_generic_exception(Context, Location, Module, Type, What, LastError) == handler_result::execute_handler;
}

bool handle_std_exception(const std::exception& e, const Plugin* const Module, source_location const& Location)
{
	return handle_std_exception(exception_context(os::debug::exception_information()), e, Module, Location);
}

class seh_exception::seh_exception_impl
{
public:
	explicit seh_exception_impl(EXCEPTION_POINTERS const& Pointers):
		Context(Pointers),
		ErrorState(os::last_error())
	{}

	seh_exception_context Context;
	os::error_state ErrorState;
};

seh_exception::seh_exception():
	os::event(os::event::type::manual, os::event::state::nonsignaled)
{}

seh_exception::~seh_exception()
{
	if (m_Impl)
		raise();
}

void seh_exception::set(EXCEPTION_POINTERS const& Pointers)
{
	m_Impl = std::make_unique<seh_exception_impl>(Pointers);
	event::set();
}

void seh_exception::raise()
{
	assert(m_Impl);

	ULONG_PTR const Arguments[]
	{
		std::bit_cast<ULONG_PTR>(this)
	};

	RaiseException(STATUS_FAR_THREAD_RETHROW, 0, static_cast<DWORD>(std::size(Arguments)), Arguments);
}

void seh_exception::dismiss()
{
	m_Impl.reset();
	reset();
}

seh_exception::seh_exception_impl const& seh_exception::get() const
{
	return *m_Impl;
}

static handler_result handle_seh_exception(
	exception_context const& Context,
	Plugin const* const PluginModule,
	source_location const& Location
)
{
	error_state_ex const LastError{ os::last_error(), {}, errno };
	const auto& Record = Context.exception_record();

	if (Record.ExceptionCode == static_cast<DWORD>(STATUS_FAR_THREAD_RETHROW) && Record.NumberParameters == 1)
	{
		const auto& OriginalExceptionData = std::bit_cast<seh_exception const*>(Record.ExceptionInformation[0])->get();
		// We don't need to care about the rethrow stack here: SEH is synchronous, so it will be a part of the handler stack
		return handle_generic_exception(OriginalExceptionData.Context, Location, PluginModule, {}, {}, OriginalExceptionData.ErrorState);
	}

	for (const auto& i : enum_catchable_objects(Record))
	{
		if (std::strstr(i, "std::exception"))
			return handle_std_exception(Context, view_as<std::exception>(Record.ExceptionInformation[1]), PluginModule, Location)?
				handler_result::execute_handler :
				handler_result::continue_search;
	}

	return handle_generic_exception(Context, Location, PluginModule, {}, {}, LastError);
}

bool handle_unknown_exception(const Plugin* const Module, source_location const& Location)
{
	return handle_seh_exception(exception_context(os::debug::exception_information()), Module, Location) == handler_result::execute_handler;
}

bool use_terminate_handler()
{
	return UseTerminateHandler;
}

static void abort_handler_impl()
{
	if (!HandleCppExceptions)
	{
		restore_system_exception_handler();
		return;
	}

	static auto InsideHandler = false;
	if (InsideHandler)
	{
		restore_system_exception_handler();
		os::process::terminate(STATUS_FATAL_APP_EXIT);
	}

	InsideHandler = true;
	SCOPE_EXIT{ InsideHandler = false; };

	constexpr auto Location = source_location::current();

	// If it's a SEH or a C++ exception implemented in terms of SEH (and not a fake for GCC) it's better to handle it as SEH
	if (const auto Info = os::debug::exception_information(); Info.ContextRecord && Info.ExceptionRecord && !is_fake_cpp_exception(*Info.ExceptionRecord))
	{
		if (handle_seh_exception(exception_context(Info), {}, Location) == handler_result::execute_handler)
			os::process::terminate_by_user();
	}

	// It's a C++ exception, implemented in some other way (GCC)
	if (const auto CurrentException = std::current_exception())
	{
		try
		{
			std::rethrow_exception(CurrentException);
		}
		catch (std::exception const& e)
		{
			if (handle_std_exception(e, {}, Location))
				os::process::terminate_by_user();
		}
		catch (...)
		{
			if (handle_unknown_exception({}, Location))
				os::process::terminate_by_user();
		}
	}

	// No exception in flight, must be a direct call
	exception_context const Context{ os::debug::fake_exception_information(STATUS_FAR_ABORT) };
	error_state_ex const LastError{ os::last_error(), {}, errno };

	if (handle_generic_exception(Context, Location, {}, {}, L"Abnormal termination"sv, LastError) == handler_result::execute_handler)
		os::process::terminate_by_user();

	restore_system_exception_handler();
}

static LONG WINAPI unhandled_exception_filter_impl(EXCEPTION_POINTERS* const Pointers)
{
	const auto Result = detail::seh_filter(Pointers, {});
	if (Result == EXCEPTION_EXECUTE_HANDLER)
		os::process::terminate_by_user(Pointers->ExceptionRecord->ExceptionCode);

	return Result;
}

unhandled_exception_filter::unhandled_exception_filter():
	m_PreviousFilter(SetUnhandledExceptionFilter(unhandled_exception_filter_impl))
{
}

unhandled_exception_filter::~unhandled_exception_filter()
{
	SetUnhandledExceptionFilter(m_PreviousFilter);
}


#if !IS_MICROSOFT_SDK()
// For GCC. For some reason the default one works in Debug, but not in Release.
#ifndef _DEBUG
extern "C"
{
	void __cxa_pure_virtual();

	void __cxa_pure_virtual()
	{
		std::abort();
	}
}
#endif
#endif

static void signal_handler_impl(int const Signal)
{
	switch (Signal)
	{
	case SIGABRT:
		// terminate() defaults to abort(), so this also covers various C++ runtime failures.
		return abort_handler_impl();

	default:
		return;
	}
}

signal_handler::signal_handler():
	m_PreviousHandler(std::signal(SIGABRT, signal_handler_impl))
{
}

signal_handler::~signal_handler()
{
	if (m_PreviousHandler != SIG_ERR)
		std::signal(SIGABRT, m_PreviousHandler);
}

#if IS_MICROSOFT_SDK()
#ifndef _DEBUG // 🤦
extern "C" void _invalid_parameter(wchar_t const*, wchar_t const*, wchar_t const*, unsigned int, uintptr_t);
#endif
#else
static void _invalid_parameter(wchar_t const*, wchar_t const*, wchar_t const*, unsigned int, uintptr_t)
{
	os::process::terminate(STATUS_INVALID_CRUNTIME_PARAMETER);
}
#endif

static void invalid_parameter_handler_impl(const wchar_t* const Expression, const wchar_t* const Function, const wchar_t* const File, unsigned int const Line, uintptr_t const Reserved)
{
	if (!HandleCppExceptions)
	{
		restore_system_exception_handler();
		std::abort();
	}

	static auto InsideHandler = false;
	if (InsideHandler)
	{
		restore_system_exception_handler();
		os::process::terminate(STATUS_INVALID_CRUNTIME_PARAMETER);
	}

	InsideHandler = true;
	SCOPE_EXIT{ InsideHandler = false; };

	exception_context const Context{ os::debug::fake_exception_information(STATUS_INVALID_CRUNTIME_PARAMETER, true) };
	error_state_ex const LastError{ os::last_error(), {}, errno };
	constexpr auto Location = source_location::current();

	switch (handle_generic_exception(
		Context,
		Function && File?
			source_location(encoding::utf8::get_bytes(Function).c_str(), encoding::utf8::get_bytes(File).c_str(), Line) :
			Location,
		{},
		{},
		NullToEmpty(Expression),
		LastError
	))
	{
	case handler_result::execute_handler:
		os::process::terminate_by_user();

	case handler_result::continue_execution:
		return;

	case handler_result::continue_search:
		restore_system_exception_handler();
		_set_invalid_parameter_handler({});
		_invalid_parameter(Expression, Function, File, Line, Reserved);
		break;
	}
}

invalid_parameter_handler::invalid_parameter_handler():
	m_PreviousHandler(_set_invalid_parameter_handler(invalid_parameter_handler_impl))
{
}

invalid_parameter_handler::~invalid_parameter_handler()
{
	_set_invalid_parameter_handler(m_PreviousHandler);
}

static LONG NTAPI vectored_exception_handler_impl(EXCEPTION_POINTERS* const Pointers)
{
	if (static_cast<NTSTATUS>(Pointers->ExceptionRecord->ExceptionCode) == STATUS_HEAP_CORRUPTION)
	{
		// VEH handlers shouldn't do this in general, but it's not like we can make things much worse at this point anyways.
		if (detail::seh_filter(Pointers, {}) == EXCEPTION_EXECUTE_HANDLER)
			os::process::terminate_by_user(Pointers->ExceptionRecord->ExceptionCode);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

vectored_exception_handler::vectored_exception_handler():
	m_Handler(imports.AddVectoredExceptionHandler? imports.AddVectoredExceptionHandler(false, vectored_exception_handler_impl) : nullptr)
{
}

vectored_exception_handler::~vectored_exception_handler()
{
	if (m_Handler && imports.RemoveVectoredExceptionHandler)
		imports.RemoveVectoredExceptionHandler(m_Handler);
}

namespace detail
{
	void cpp_try(
		function_ref<void()> const Callable,
		function_ref<void(source_location const&)> const UnknownHandler,
		function_ref<void(std::exception const&, source_location const&)> const StdHandler,
		source_location const& Location
	)
	{
		if (!HandleCppExceptions)
		{
			return Callable();
		}

		if (StdHandler)
		{
			try
			{
				return Callable();
			}
			catch (std::exception const& e)
			{
				return StdHandler(e, Location);
			}
			catch (...)
			{
				return UnknownHandler(Location);
			}
		}

		try
		{
			return Callable();
		}
		catch (...)
		{
			return UnknownHandler(Location);
		}
	}

	static thread_local bool StackOverflowHappened;

	void seh_try(function_ref<void()> const Callable, function_ref<DWORD(EXCEPTION_POINTERS*)> const Filter, function_ref<void(DWORD)> const Handler)
	{
#if COMPILER(GCC) || (COMPILER(CLANG) && !defined _WIN64 && defined __GNUC__)
		// GCC doesn't support these currently
		// Clang x86 crashes with "Assertion failed: STI.isTargetWindowsMSVC() && "funclets only supported in MSVC env""
		return Callable();
#else
#if COMPILER(CLANG)
		// Workaround for clang "filter expression type should be an integral value" error
		const auto FilterWrapper = [&](EXCEPTION_POINTERS* const Pointers){ return Filter(Pointers); };
#define Filter FilterWrapper
#endif

		__try
		{
			Callable();
		}
		__except (set_fp_exceptions(false), Filter(GetExceptionInformation()))
		{
			if (StackOverflowHappened)
			{
				if (!_resetstkoflw())
					os::process::terminate(GetExceptionCode());

				StackOverflowHappened = false;
			}

			Handler(GetExceptionCode());
		}

#if COMPILER(CLANG)
#undef Filter
#endif
#endif
	}

	int seh_filter(EXCEPTION_POINTERS const* const Info, Plugin const* const Module, source_location const& Location)
	{
		if (!HandleSehExceptions)
		{
			restore_system_exception_handler();
			return EXCEPTION_CONTINUE_SEARCH;
		}

		set_fp_exceptions(false);
		const exception_context Context(*Info);
		auto Result = handler_result::continue_search;

		if (static_cast<NTSTATUS>(Info->ExceptionRecord->ExceptionCode) == EXCEPTION_STACK_OVERFLOW)
		{
			{
				os::thread([&]
				{
					os::debug::set_thread_name(L"Stack overflow handler");
					Result = handle_seh_exception(Context, Module, Location);
				});
			}

			StackOverflowHappened = true;
		}
		else
		{
			Result = handle_seh_exception(Context, Module, Location);
		}

		switch (Result)
		{
		case handler_result::continue_execution:
			return EXCEPTION_CONTINUE_EXECUTION;

		case handler_result::execute_handler:
			return EXCEPTION_EXECUTE_HANDLER;

		case handler_result::continue_search:
			restore_system_exception_handler();
			return EXCEPTION_CONTINUE_SEARCH;

		default:
			std::unreachable();
		}
	}

	int seh_thread_filter(seh_exception& Exception, EXCEPTION_POINTERS const* const Info)
	{
		if (!HandleSehExceptions)
		{
			restore_system_exception_handler();
			return EXCEPTION_CONTINUE_SEARCH;
		}

		Exception.set(*Info);

		// The thread is about to quit, but we still need it to get the stack trace and write a minidump.
		// It will be released once the corresponding exception context is destroyed.
		// The thread has to be suspended right here in the filter to ensure a successful stack capture.
		SuspendThread(GetCurrentThread());

		return EXCEPTION_EXECUTE_HANDLER;
	}

	void seh_thread_handler(DWORD)
	{
	}

	void set_fp_exceptions(bool const Enable)
	{
		_clearfp();
		_controlfp(Enable? 0 : _MCW_EM, _MCW_EM);
	}
}
