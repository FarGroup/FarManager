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
#include "platform.com.hpp"
#include "platform.debug.hpp"
#include "platform.fs.hpp"
#include "platform.process.hpp"
#include "platform.version.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

#if !IS_MICROSOFT_SDK()
#include <cxxabi.h>
#endif

//----------------------------------------------------------------------------

#define BUGREPORT_NAME   "bug_report.txt"
#define MINIDDUMP_NAME   "far.mdmp"
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

void CreatePluginStartupInfo(PluginStartupInfo *PSI, FarStandardFunctions *FSF);

static constexpr NTSTATUS
	EH_EXCEPTION_NUMBER           = 0xE06D7363, // 'msc'
	EH_SANITIZER                  = 0xE073616E, // 'san',
	EH_SANITIZER_ASAN             = EH_SANITIZER + 1,

	// Far-specific codes
	STATUS_FAR_ABORT              = make_far_ntstatus(0),
	STATUS_FAR_THREAD_RETHROW     = make_far_ntstatus(1);

static const auto Separator = L"----------------------------------------------------------------------"sv;

static void make_header(string_view const Message, function_ref<void(string_view)> const Consumer)
{
	Consumer({});

	Consumer(Separator);
	Consumer(Message);
	Consumer(Separator);
}

static void get_backtrace(string_view const Module, span<os::debug::stack_frame const> const Stack, span<os::debug::stack_frame const> const NestedStack, function_ref<void(string_view)> const Consumer)
{
	make_header(L"Exception stack"sv, Consumer);
	if (!NestedStack.empty())
	{
		tracer.get_symbols(Module, NestedStack, Consumer);
		make_header(L"Rethrow stack"sv, Consumer);
	}

	tracer.get_symbols(Module, Stack, Consumer);

	make_header(L"Exception handler stack"sv, Consumer);
	tracer.get_symbols(Module, os::debug::current_stack(), Consumer);
}

static string get_report_location()
{
	auto [Date, Time] = get_time();

	const auto not_digit = [](wchar_t const Char){ return !std::iswdigit(Char); };

	std::replace_if(ALL_RANGE(Date), not_digit, L'_');
	std::replace_if(ALL_RANGE(Time), not_digit, L'_');

	const auto SubDir = format(L"Far.{}_{}_{}"sv, Date, Time, GetCurrentProcessId());

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
	os::fs::file const File(FullPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS);
	if (!File)
		return false;

#define EOL "\r\n"

	// English text, ANSI will do fine.
	const auto Data =
		"Please send " BUGREPORT_NAME " and " MINIDDUMP_NAME " to the developers:" EOL
		EOL
		"  https://github.com/FarGroup/FarManager/issues" EOL
		"  https://bugs.farmanager.com" EOL
		"  https://forum.farmanager.com/viewforum.php?f=37" EOL
		"  https://forum.farmanager.com/viewforum.php?f=9" EOL
		EOL
		"------------------------------------------------------------" EOL
		"DO NOT SHARE " FULLDUMP_NAME " UNLESS EXPLICITLY ASKED TO DO SO." EOL
		"It could contain sensitive data." EOL
		"------------------------------------------------------------" EOL
		""sv;

#undef EOL

	return File.Write(Data.data(), Data.size() * sizeof(Data[0]));
}

static bool write_report(string_view const Data, string_view const FullPath)
{
	os::fs::file const File(FullPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS);
	if (!File)
		return false;
	return File.Write(Data.data(), Data.size() * sizeof(decltype(Data)::value_type));
}

static bool write_minidump(const exception_context& Context, string_view const FullPath, MINIDUMP_TYPE const Type)
{
	if (!imports.MiniDumpWriteDump)
		return false;

	const os::fs::file DumpFile(FullPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS);
	if (!DumpFile)
		return false;

	auto ExceptionRecord = Context.exception_record();
	auto ContextRecord = Context.context_record();
	EXCEPTION_POINTERS Pointers{ &ExceptionRecord, &ContextRecord };
	MINIDUMP_EXCEPTION_INFORMATION Mei{ Context.thread_id(), &Pointers };

	// https://docs.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump#remarks
	// MiniDumpWriteDump may not produce a valid stack trace for the calling thread.
	// You can call the function from a new worker thread and filter this worker thread from the dump.

	bool Result = false;
	os::thread(os::thread::mode::join, [&]
	{
		struct writer_context
		{
			DWORD const ThreadId{ GetCurrentThreadId() };
		}
		WriterContext;

		MINIDUMP_CALLBACK_INFORMATION Mci
		{
			[](void* const Param, MINIDUMP_CALLBACK_INPUT* const Input, MINIDUMP_CALLBACK_OUTPUT*)
			{
				const auto& Ctx = *static_cast<writer_context const*>(Param);

				if (Input->CallbackType == IncludeThreadCallback && Input->IncludeThread.ThreadId == Ctx.ThreadId)
					return FALSE;

				return TRUE;
			},
			&WriterContext
		};

		Result = imports.MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), DumpFile.get().native_handle(), Type, &Mei, {}, &Mci) != FALSE;
	});

	return Result;
}

static void read_modules(span<HMODULE const> const Modules, string& To, string_view const Eol)
{
	string Name;
	for (const auto& i: Modules)
	{
		if (os::fs::get_module_file_name({}, i, Name))
			append(To, Name, L' ', os::version::get_file_version(Name), Eol);
		else
			append(To, str(static_cast<void const*>(i)), Eol);
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
			const auto LastError = last_error();
			format_to(To, FSTR(L"{}"sv), LastError);
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

static string self_version()
{
	const auto Version = format(FSTR(L"{} {}"sv), version_to_string(build::version()), build::platform());
	const auto ScmRevision = build::scm_revision();
	return ScmRevision.empty()? Version : Version + format(FSTR(L" ({:.7})"sv), ScmRevision);
}

static string kernel_version()
{
	return os::version::get_file_version(L"ntoskrnl.exe"sv);
}

struct dialog_data_type
{
	const exception_context* Context;
	span<uintptr_t const> NestedStack;
	string_view Module;
	span<string_view const> Labels, Values;
};

static void read_registers(string& To, CONTEXT const& Context, string_view const Eol)
{
	const auto r = [&](string_view const Name, auto const Value)
	{
		format_to(To, FSTR(L"{:3} = {:0{}X}{}"sv), Name, Value, sizeof(Value) * 2, Eol);
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
		output_impl(Mask, encoding::utf8::get_chars(Text));
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

static void read_disassembly(string& To, string_view const Module, span<os::debug::stack_frame const> const Stack, string_view const Eol)
{
	if (Stack.empty())
		return;

	if (!imports.DebugCreate)
		return;

	try
	{
		DebugOutputCallbacks Callbacks(&To);

		os::com::ptr<IDebugClient> DebugClient;
		COM_INVOKE(imports.DebugCreate, (IID_IDebugClient, IID_PPV_ARGS_Helper(&ptr_setter(DebugClient))));

		COM_INVOKE(DebugClient->AttachProcess, ({}, GetCurrentProcessId(), DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND));

		os::com::ptr<IDebugControl> DebugControl;
		COM_INVOKE(DebugClient->QueryInterface, (IID_IDebugControl, IID_PPV_ARGS_Helper(&ptr_setter(DebugControl))));

		if (const auto Result = DebugControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE); FAILED(Result))
			LOGWARNING(L"WaitForEvent(): {}"sv, os::format_error(Result));

		if (const auto Result = DebugClient->SetOutputMask(DebugOutputCallbacks::CallbackTypes); FAILED(Result))
			LOGWARNING(L"SetOutputMask(): {}"sv, os::format_error(Result));

		if (os::com::ptr<IDebugClient5> DebugClient5; SUCCEEDED(DebugClient->QueryInterface(IID_IDebugClient5, IID_PPV_ARGS_Helper(&ptr_setter(DebugClient5)))))
			COM_INVOKE(DebugClient5->SetOutputCallbacksWide, (&Callbacks));
		else
			COM_INVOKE(DebugClient->SetOutputCallbacks, (&Callbacks));

		const auto DisassembleFlags =
			DEBUG_DISASM_EFFECTIVE_ADDRESS |
			DEBUG_DISASM_MATCHING_SYMBOLS |
			DEBUG_DISASM_SOURCE_LINE_NUMBER |
			DEBUG_DISASM_SOURCE_FILE_NAME;

		const auto MaxFrames = 10;
		auto Frames = 0;
		uintptr_t LastFrameAddress = 0;

		for (const auto i: Stack)
		{
			if (i.Address == LastFrameAddress)
				continue;

			LastFrameAddress = i.Address;

			tracer.get_symbols(Module, {&i, 1}, [&](string_view const Line)
			{
				append(To, Line, L':', Eol);
			});

			const auto PrevLines = 10;
			if (const auto Result = DebugControl->OutputDisassemblyLines(
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

			To += Eol;
		}
	}
	catch (os::com::exception const& e)
	{
		LOGWARNING(L"{}"sv, e);
	}
}

static string collect_information(
	exception_context const& Context,
	span<os::debug::stack_frame const> NestedStack,
	string_view const Module,
	span<std::pair<string_view, string_view> const> const BasicInfo,
	string_view const ExtraDetails
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

	for (const auto& [Label, Value]: BasicInfo)
	{
		format_to(Strings, FSTR(L"{} {}{}"sv), Label, Value, Eol);
	}

	if (!ExtraDetails.empty())
	{
		make_header(L"Details"sv, append_line);
		append_line(ExtraDetails);
	}

	const auto Stack = tracer.get(Module, Context.context_record(), Context.thread_handle());
	get_backtrace(Module, Stack, NestedStack, append_line);

	{
		os::process::enum_processes const Enum;
		const auto CurrentPid = GetCurrentProcessId();
		const auto CurrentThreadId = GetCurrentThreadId();
		const auto CurrentEntry = std::find_if(ALL_CONST_RANGE(Enum), [&](os::process::enum_process_entry const& Entry){ return Entry.Pid == CurrentPid; });
		if (CurrentEntry != Enum.cend())
		{
			for (const auto& i: CurrentEntry->Threads)
			{
				const auto Tid = reinterpret_cast<uintptr_t>(i.ClientId.UniqueThread);
				if (Tid == CurrentThreadId)
					continue;

				os::handle const Thread(OpenThread(THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, false, Tid));
				if (!Thread)
					continue;

				SuspendThread(Thread.native_handle());
				SCOPE_EXIT{ ResumeThread(Thread.native_handle()); };

				auto ThreadTitle = format(FSTR(L"Thread {0} / 0x{0:X}"sv), Tid);
				if (const auto ThreadName = os::debug::get_thread_name(Thread.native_handle()); !ThreadName.empty())
					append(ThreadTitle, L" ("sv, ThreadName, L')');
				append(ThreadTitle, L" stack"sv);

				make_header(ThreadTitle, append_line);

				CONTEXT ThreadContext{};
				ThreadContext.ContextFlags = CONTEXT_ALL;
				if (!GetThreadContext(Thread.native_handle(), &ThreadContext))
					continue;

				tracer.get_symbols(Module, tracer.get(Module, ThreadContext, Thread.native_handle()), append_line);
			}
		}
	}

	// Read disassembly before modules - it will load dbgeng.dll and we might want to see it too just in case
	make_header(L"Disassembly"sv, append_line);
	read_disassembly(Strings, Module, NestedStack.empty()? Stack : NestedStack, Eol);

	make_header(L"Modules"sv, append_line);
	read_modules(Strings, Eol);

	make_header(L"Registers"sv, append_line);
	read_registers(Strings, Context.context_record(), Eol);

	return Strings;
}

static bool ExcDialog(string const& ReportLocation, string const& PluginInformation)
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

	if (!PluginInformation.empty())
	{
		Builder.AddButtons({ lng::MExcTerminate, lng::MExcUnload, lng::MIgnore });
	}
	else
	{
		Builder.AddButtons({ lng::MExcTerminate, lng::MIgnore });
	}

	Builder.SetDialogMode(DMODE_WARNINGSTYLE | DMODE_NOPLUGINS);
	Builder.SetScrObjFlags(FSCROBJ_SPECIAL);

	switch (Builder.ShowDialogEx())
	{
	case 0: // Terminate
		UseTerminateHandler = true;
		return true;

	case 1: // Unload / Ignore
		return !PluginInformation.empty();

	default:
		return false;
	}
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

static bool ExcConsole(string const& ReportLocation, string const& PluginInformation)
{
	if (!ConsoleYesNo(L"Terminate the process"sv, true, [&]{ print_exception_message(ReportLocation, PluginInformation); }))
		return false;

	UseTerminateHandler = true;
	return true;
}

static string get_console_host()
{
	if (!imports.NtQueryInformationProcess)
		return {};

	ULONG_PTR ConsoleHostProcess;
	const auto Status = imports.NtQueryInformationProcess(GetCurrentProcess(), ProcessConsoleHostProcess, &ConsoleHostProcess, sizeof(ConsoleHostProcess), {});
	if (!NT_SUCCESS(Status))
		return {};

	const auto ConsoleHostProcessId = ConsoleHostProcess & ~3;

	const auto ConhostName = os::process::get_process_name(ConsoleHostProcessId);
	if (ConhostName.empty())
		return {};

	const auto ConhostVersion = os::version::get_file_version(ConhostName);
	const auto ConhostLegacy = console.IsVtSupported()? L""sv : L" (legacy mode)"sv;

	return concat(ConhostName, L' ', ConhostVersion, ConhostLegacy);
}

namespace detail
{
	// GCC headers for once got it right
	IS_DETECTED(has_InheritedFromUniqueProcessId, T::InheritedFromUniqueProcessId);

	// Windows SDK (at least up to 19041) defines it as "Reserved3".
	// Surprisingly, MSDN calls it InheritedFromUniqueProcessId, so it might get renamed one day.
	// For forward compatibility it's better to use the compiler rather than the preprocessor here.
	IS_DETECTED(has_Reserved3, T::Reserved3);
}

template<typename process_basic_information_t>
static auto parent_process_id(process_basic_information_t const& Info)
{
	if constexpr (detail::has_InheritedFromUniqueProcessId<process_basic_information_t>)
		return static_cast<DWORD>(Info.InheritedFromUniqueProcessId);
	else if constexpr (detail::has_Reserved3<process_basic_information_t>)
		return static_cast<DWORD>(reinterpret_cast<uintptr_t>(Info.Reserved3));
	else
		static_assert(!sizeof(Info));
}

static string get_parent_process()
{
	if (!imports.NtQueryInformationProcess)
		return {};

	PROCESS_BASIC_INFORMATION ProcessInfo;
	if (!NT_SUCCESS(imports.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &ProcessInfo, sizeof(ProcessInfo), {})))
		return {};

	const auto ParentProcessId = parent_process_id(ProcessInfo);

	const auto ParentName = os::process::get_process_name(ParentProcessId);
	if (ParentName.empty())
		return {};

	const auto ParentVersion = os::version::get_file_version(ParentName);

	return concat(ParentName, L' ', ParentVersion);
}

static bool ShowExceptionUI(
	bool const UseDialog,
	exception_context const& Context,
	string_view const Exception,
	string_view const Details,
	string_view const ExtraDetails,
	error_state const& ErrorState,
	string_view const Function,
	string_view const Location,
	string_view const ModuleName,
	string const& PluginInfo,
	Plugin const* const PluginModule,
	span<os::debug::stack_frame const> const NestedStack
)
{
	SCOPED_ACTION(tracer_detail::tracer::with_symbols)(PluginModule? ModuleName : L""sv);

	string Address, Name, Source;
	tracer.get_symbol(ModuleName, Context.exception_record().ExceptionAddress, Address, Name, Source);

	if (!Name.empty())
		Address = concat(L"0x"sv, Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	const auto Errors = ErrorState.format_errors();
	const auto Version = self_version();
	const auto Compiler = build::compiler();
	const auto OsVersion = os::version::os_version();
	const auto KernelVersion = kernel_version();
	const auto ConsoleHost = get_console_host();
	const auto Parent = get_parent_process();

	std::pair<string_view, string_view> const BasicInfo[]
	{
		{ L"Exception:"sv, Exception,     },
		{ L"Details:  "sv, Details,       },
		{ L"errno:    "sv, Errors[0],     },
		{ L"LastError:"sv, Errors[1],     },
		{ L"NTSTATUS: "sv, Errors[2],     },
		{ L"Address:  "sv, Address,       },
		{ L"Function: "sv, Function,      },
		{ L"Source:   "sv, Source,        },
		{ L"File:     "sv, ModuleName,    },
		{ L"Plugin:   "sv, PluginInfo,    },
		{},
		{ L"Far:      "sv, Version,       },
		{ L"Compiler: "sv, Compiler,      },
		{ L"OS:       "sv, OsVersion,     },
		{ L"Kernel:   "sv, KernelVersion, },
		{ L"Host:     "sv, ConsoleHost,   },
		{ L"Parent:   "sv, Parent,        },
	};

	const auto log_message = [&]
	{
		auto Message = join(L"\n"sv, select(BasicInfo, [](auto const& Pair)
		{
			return format(FSTR(L"{} {}"sv), Pair.first, Pair.second);
		}));

		Message += L"\n\n"sv;

		get_backtrace(ModuleName, tracer.get(ModuleName, Context.context_record(), Context.thread_handle()), NestedStack, [&](string_view const Line)
		{
			append(Message, Line, L'\n');
		});

		return Message;
	};

	LOG(PluginModule? logging::level::error : logging::level::fatal, L"\n{}\n"sv, log_message());

	const auto ReportLocation = get_report_location();
	const auto MinidumpNormal = write_minidump(Context, path::join(ReportLocation, WIDE_SV(MINIDDUMP_NAME)), MiniDumpNormal);
	const auto MinidumpFull = write_minidump(Context, path::join(ReportLocation, WIDE_SV(FULLDUMP_NAME)), MiniDumpWithFullMemory);
	const auto BugReport = collect_information(Context, NestedStack, ModuleName, BasicInfo, ExtraDetails);
	const auto ReportOnDisk = write_report(BugReport, path::join(ReportLocation, WIDE_SV(BUGREPORT_NAME)));
	const auto ReportInClipboard = !ReportOnDisk && SetClipboardText(BugReport);
	const auto ReadmeOnDisk = write_readme(path::join(ReportLocation, L"README.txt"sv));
	const auto AnythingOnDisk = ReportOnDisk || MinidumpNormal || MinidumpFull || ReadmeOnDisk;

	if (AnythingOnDisk && os::is_interactive_user_session())
		OpenFolderInShell(ReportLocation);

	if (AnythingOnDisk || ReportInClipboard)
		return (UseDialog? ExcDialog : ExcConsole)(
			AnythingOnDisk? ReportLocation : msg(lng::MExceptionDialogClipboard),
			PluginInfo
		);

	// Should never happen - neither the filesystem nor clipboard are writable, so just dump it to the screen:
	return ExcConsole(BugReport, PluginInfo);
}


template<size_t... I>
static constexpr uint32_t any_cc(std::string_view const Str, std::index_sequence<I...> Sequence)
{
	if (Str.size() != Sequence.size())
		throw;

	return (... | (Str[I] << 8 * I));
}

static constexpr uint32_t fourcc(std::string_view const Str)
{
	return any_cc(Str, std::make_index_sequence<4>{});
}

enum FARRECORDTYPE
{
	RTYPE_PLUGIN = fourcc("CPLG"sv), // информация о текущем плагине
};

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
}

static bool is_cpp_exception(const EXCEPTION_RECORD& Record)
{
	return Record.ExceptionCode == static_cast<DWORD>(EH_EXCEPTION_NUMBER) && Record.NumberParameters;
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

		m_BaseAddress = Record.NumberParameters == 4? Record.ExceptionInformation[3] : 0;
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

	span<int const> m_CatchableTypesRVAs;
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
	CASE_STR(STATUS_ASSERTION_FAILURE)
#undef CASE_STR

	case EH_EXCEPTION_NUMBER:           return L"C++ exception"sv;
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
	return WithType(format(FSTR(L"0x{:0>8X} - {}"sv), ExceptionRecord.ExceptionCode, Name));
}

static string exception_details(string_view const Module, EXCEPTION_RECORD const& ExceptionRecord, string_view const Message, string& ExtraDetails)
{
	switch (const auto NtStatus = static_cast<NTSTATUS>(ExceptionRecord.ExceptionCode))
	{
	case STATUS_ACCESS_VIOLATION:
	case STATUS_IN_PAGE_ERROR:
	{
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

		string Symbol;
		tracer.get_symbols(Module, { { ExceptionRecord.ExceptionInformation[1], 0 } }, [&](string&& Line)
		{
			Symbol = std::move(Line);
		});

		if (Symbol.empty())
			Symbol = to_hex_wstring(ExceptionRecord.ExceptionInformation[1]);

		auto Result = format(FSTR(L"Memory at {} could not be {}"sv), Symbol, Mode);
		if (NtStatus == EXCEPTION_IN_PAGE_ERROR)
			append(Result, L": "sv, os::format_ntstatus(static_cast<NTSTATUS>(ExceptionRecord.ExceptionInformation[2])));

		return Result;
	}

	case EH_EXCEPTION_NUMBER:
	case STATUS_FAR_ABORT:
		return string(Message);

	case EH_SANITIZER:
		if (ExceptionRecord.NumberParameters && ExceptionRecord.ExceptionInformation[0])
		{
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

				return { Record.u.Asan.pwRuntimeShortMessage, static_cast<size_t>(Record.u.Asan.uiRuntimeShortMessageLength) };

			default:
				return L"Unrecognized sanitizer kind"s;
			}
		}
		[[fallthrough]];

	default:
		return os::format_ntstatus(ExceptionRecord.ExceptionCode);
	}
}

static bool handle_generic_exception(
	exception_context const& Context,
	std::string_view const Function,
	string_view const Location,
	Plugin const* const PluginModule,
	string_view const Type,
	string_view const Message,
	error_state const& ErrorState = last_error(),
	span<os::debug::stack_frame const> const NestedStack = {}
)
{
	static bool ExceptionHandlingIgnored = false;
	if (ExceptionHandlingIgnored)
		return false;

	if (s_ExceptionHandlingInprogress)
		return true;

	s_ExceptionHandlingInprogress = true;
	SCOPE_EXIT{ s_ExceptionHandlingInprogress = false; };

	const auto strFileName = PluginModule?
		PluginModule->ModuleName() :
		Global?
			Global->g_strFarModuleName :
			os::fs::get_current_process_file_name();

	const auto Exception = exception_name(Context.exception_record(), Type);
	string ExtraDetails;
	const auto Details = exception_details(strFileName, Context.exception_record(), Message, ExtraDetails);

	const auto PluginInformation = PluginModule? format(FSTR(L"{} {} ({}, {})"sv),
		PluginModule->Title(),
		version_to_string(PluginModule->version()),
		PluginModule->Description(),
		PluginModule->Author()
	) : L""s;

	if (!ShowExceptionUI(
		!ForceStderrExceptionUI && Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown(),
		Context,
		Exception,
		Details,
		ExtraDetails,
		ErrorState,
		encoding::utf8::get_chars(Function),
		Location,
		strFileName,
		PluginInformation,
		PluginModule,
		NestedStack
	))
	{
		ExceptionHandlingIgnored = true;
		return false;
	}

	if (!PluginModule && Global)
		Global->CriticalInternalError = true;

	return true;
}

void restore_system_exception_handler()
{
	disable_exception_handling();
	os::unset_error_mode(SEM_NOGPFAULTERRORBOX);
}

static void** dummy_current_exception(NTSTATUS const Code)
{
	static EXCEPTION_RECORD DummyRecord{ static_cast<DWORD>(Code) };
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
	return dummy_current_exception(EH_EXCEPTION_NUMBER);
}

static void** __current_exception_context()
{
	return dummy_current_exception_context();
}
#endif

static EXCEPTION_POINTERS exception_information()
{
	if (!std::current_exception())
		return {};

	return
	{
		static_cast<EXCEPTION_RECORD*>(*__current_exception()),
		static_cast<CONTEXT*>(*__current_exception_context())
	};
}

class far_wrapper_exception final: public far_exception, public std::nested_exception
{
public:
	far_wrapper_exception(std::string_view const Function, std::string_view const File, int const Line):
		far_exception(true, L"exception_ptr"sv, Function, File, Line),
		m_ThreadHandle(std::make_shared<os::handle>(os::OpenCurrentThread())),
		m_Stack(tracer.get({}, *exception_information().ContextRecord, m_ThreadHandle->native_handle()))
	{
	}

	span<os::debug::stack_frame const> get_stack() const noexcept { return m_Stack; }

private:
	std::shared_ptr<os::handle> m_ThreadHandle;
	std::vector<os::debug::stack_frame> m_Stack;
};

static_assert(std::is_base_of_v<std::nested_exception, far_wrapper_exception>);

std::exception_ptr wrap_current_exception(std::string_view const Function, std::string_view const File, int const Line)
{
	try
	{
		throw far_wrapper_exception(Function, File, Line);
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
			ObjectType = WSTRVIEW(detail::far_base_exception);
	}
	else
	{
		What = encoding::utf8::get_chars(Exception.what());
		if (ObjectType.empty())
			ObjectType = WSTRVIEW(std::exception);
	}

	try
	{
		std::rethrow_if_nested(Exception);
	}
	catch (std::exception const& e)
	{
		const auto& [NestedObjectType, NestedWhat] = extract_nested_exceptions(*exception_information().ExceptionRecord, e, false);
		ObjectType = concat(NestedObjectType, L" -> "sv, ObjectType);
		What = concat(NestedWhat, L" -> "sv, What);
	}
	catch (...)
	{
		auto NestedObjectType = ExtractObjectType(*exception_information().ExceptionRecord);
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
	std::string_view const Function,
	const Plugin* const Module
)
{
	const auto& [Type, What] = extract_nested_exceptions(Context.exception_record(), e);

	if (const auto FarException = dynamic_cast<const detail::far_base_exception*>(&e))
	{
		const auto NestedStack = [&]
		{
			const auto Wrapper = dynamic_cast<const far_wrapper_exception*>(&e);
			return Wrapper? Wrapper->get_stack() : span<os::debug::stack_frame const>{};
		}();

		return handle_generic_exception(Context, FarException->function(), FarException->location(), Module, Type, What, *FarException, NestedStack);
	}

	return handle_generic_exception(Context, Function, {}, Module, Type, What);
}

bool handle_std_exception(const std::exception& e, std::string_view const Function, const Plugin* const Module)
{
	return handle_std_exception(exception_context(exception_information()), e, Function, Module);
}

class seh_exception::seh_exception_impl
{
public:
	explicit seh_exception_impl(EXCEPTION_POINTERS const& Pointers):
		Context(Pointers),
		ErrorState(last_error())
	{}

	seh_exception_context Context;
	error_state ErrorState;
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
		reinterpret_cast<ULONG_PTR>(this)
	};

	RaiseException(STATUS_FAR_THREAD_RETHROW, 0, static_cast<DWORD>(std::size(Arguments)), Arguments);

	dismiss();
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

static bool handle_seh_exception(
	exception_context const& Context,
	std::string_view const Function,
	Plugin const* const PluginModule
)
{
	const auto& Record = Context.exception_record();

	if (Record.ExceptionCode == static_cast<DWORD>(STATUS_FAR_THREAD_RETHROW) && Record.NumberParameters == 1)
	{
		const auto& OriginalExceptionData = reinterpret_cast<seh_exception const*>(Record.ExceptionInformation[0])->get();
		// We don't need to care about the rethrow stack here: SEH is synchronous, so it will be a part of the handler stack
		return handle_generic_exception(OriginalExceptionData.Context, Function, {}, PluginModule, {}, {}, OriginalExceptionData.ErrorState);
	}

	for (const auto& i : enum_catchable_objects(Record))
	{
		if (strstr(i, "std::exception"))
			return handle_std_exception(Context, view_as<std::exception>(Record.ExceptionInformation[1]), Function, PluginModule);
	}

	return handle_generic_exception(Context, Function, {}, PluginModule, {}, {});
}

bool handle_unknown_exception(std::string_view const Function, const Plugin* const Module)
{
	return handle_seh_exception(exception_context(exception_information()), Function, Module);
}

bool use_terminate_handler()
{
	return UseTerminateHandler;
}

static void seh_abort_handler_impl()
{
	static auto InsideHandler = false;
	if (!HandleCppExceptions || InsideHandler)
	{
		restore_system_exception_handler();
		std::abort();
	}

	InsideHandler = true;

	// If it's a SEH or a C++ exception implemented in terms of SEH (and not a fake for GCC) it's better to handle it as SEH
	if (const auto Info = exception_information(); Info.ContextRecord && Info.ExceptionRecord && !is_fake_cpp_exception(*Info.ExceptionRecord))
	{
		if (handle_seh_exception(exception_context(Info), CURRENT_FUNCTION_NAME, {}))
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
			if (handle_std_exception(e, CURRENT_FUNCTION_NAME, {}))
				os::process::terminate_by_user();
		}
		catch (...)
		{
			if (handle_unknown_exception(CURRENT_FUNCTION_NAME, {}))
				os::process::terminate_by_user();
		}
	}

	// No exception in flight, must be a direct call
	exception_context const Context
	({
		static_cast<EXCEPTION_RECORD*>(*dummy_current_exception(STATUS_FAR_ABORT)),
		static_cast<CONTEXT*>(*dummy_current_exception_context())
	});

	if (handle_generic_exception(Context, CURRENT_FUNCTION_NAME, {}, {}, {}, L"Abnormal termination"sv))
		os::process::terminate_by_user();

	restore_system_exception_handler();
}

static LONG WINAPI unhandled_exception_filter_impl(EXCEPTION_POINTERS* const Pointers)
{
	const auto Result = detail::seh_filter(Pointers, CURRENT_FUNCTION_NAME, {});
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
		return seh_abort_handler_impl();

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

static void invalid_parameter_handler_impl(const wchar_t* const Expression, const wchar_t* const Function, const wchar_t* const File, unsigned int const Line, uintptr_t const Reserved)
{
	static auto InsideHandler = false;
	if (!HandleCppExceptions || InsideHandler)
	{
		restore_system_exception_handler();
		std::abort();
	}

	InsideHandler = true;

	exception_context const Context
	({
		static_cast<EXCEPTION_RECORD*>(*dummy_current_exception(STATUS_FAR_ABORT)),
		static_cast<CONTEXT*>(*dummy_current_exception_context())
	});

	if (handle_generic_exception(
		Context,
		Function? encoding::utf8::get_bytes(Function) : CURRENT_FUNCTION_NAME,
		format(FSTR(L"{}({})"sv), File? File : WIDE(CURRENT_FILE_NAME), File? Line : __LINE__),
		{},
		{},
		Expression? Expression : L"Invalid parameter"sv
	))
		os::process::terminate_by_user();

	restore_system_exception_handler();
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
		if (detail::seh_filter(Pointers, CURRENT_FUNCTION_NAME, {}) == EXCEPTION_EXECUTE_HANDLER)
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
	void cpp_try(function_ref<void()> const Callable, function_ref<void()> const UnknownHandler, function_ref<void(std::exception const&)> const StdHandler)
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
				return StdHandler(e);
			}
			catch (...)
			{
				return UnknownHandler();
			}
		}

		try
		{
			return Callable();
		}
		catch (...)
		{
			return UnknownHandler();
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

	int seh_filter(EXCEPTION_POINTERS const* const Info, std::string_view const Function, Plugin const* const Module)
	{
		if (!HandleSehExceptions)
		{
			restore_system_exception_handler();
			return EXCEPTION_CONTINUE_SEARCH;
		}

		set_fp_exceptions(false);
		const exception_context Context(*Info);

		if (static_cast<NTSTATUS>(Info->ExceptionRecord->ExceptionCode) == EXCEPTION_STACK_OVERFLOW)
		{
			bool Result = false;
			{
				os::thread(os::thread::mode::join, [&]
				{
					os::debug::set_thread_name(L"Stack overflow handler");
					Result = handle_seh_exception(Context, Function, Module);
				});
			}

			StackOverflowHappened = true;

			if (Result)
			{
				return EXCEPTION_EXECUTE_HANDLER;
			}
		}
		else
		{
			if (handle_seh_exception(Context, Function, Module))
				return EXCEPTION_EXECUTE_HANDLER;
		}

		restore_system_exception_handler();
		return EXCEPTION_CONTINUE_SEARCH;
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

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("fourcc")
{
	STATIC_REQUIRE(fourcc("CPLG"sv) == 0x474C5043);
	STATIC_REQUIRE(fourcc("avc1"sv) == 0x31637661);
}
#endif
