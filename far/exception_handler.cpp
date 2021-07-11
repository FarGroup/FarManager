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
#include "dialog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "message.hpp"
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
#include "platform.fs.hpp"
#include "platform.reg.hpp"
#include "platform.version.hpp"

// Common:
#include "common/scope_exit.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

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
	auto& exception_record() const noexcept { return m_ExceptionRecord; }
	auto& context_record() const noexcept { return m_ContextRecord; }
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

static std::atomic_size_t s_ExceptionHandlingInprogress;
bool exception_handling_in_progress()
{
	return s_ExceptionHandlingInprogress != 0;
}

void force_stderr_exception_ui(bool const Force)
{
	ForceStderrExceptionUI = Force;
}

void CreatePluginStartupInfo(PluginStartupInfo *PSI, FarStandardFunctions *FSF);

constexpr NTSTATUS
	EXCEPTION_MICROSOFT_CPLUSPLUS = 0xE06D7363, // EH_EXCEPTION_NUMBER
	EXCEPTION_TERMINATE           = 0xE074726D; // 'trm'

static void get_backtrace(string_view const Module, span<uintptr_t const> const Stack, span<uintptr_t const> const NestedStack, function_ref<void(string&&)> const Consumer)
{
	const auto Separator = [&](string&& Message)
	{
		Consumer(string(40, L'-'));
		Consumer(std::move(Message));
		Consumer(string(40, L'-'));
	};

	if (!NestedStack.empty())
	{
		tracer.get_symbols(Module, NestedStack, Consumer);
		Separator(L"Rethrow:"s);
	}

	tracer.get_symbols(Module, Stack, Consumer);

	Separator(L"Current stack:"s);
	tracer.get_symbols(Module, os::debug::current_stack(), Consumer);
}

static string get_report_location()
{
	auto [Date, Time] = get_time();
	std::replace(ALL_RANGE(Date), L'/', L'.');
	std::replace(ALL_RANGE(Time), L':', L'.');

	const auto SubDir = format(L"Far.{}_{}_{}"sv, Date, Time, GetCurrentProcessId());

	if (const auto CrashLogs = path::join(Global->Opt->LocalProfilePath, L"CrashLogs"); os::fs::is_directory(CrashLogs) || os::fs::create_directory(CrashLogs))
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
	MINIDUMP_EXCEPTION_INFORMATION Mei = { Context.thread_id(), &Pointers };

	return imports.MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), DumpFile.get().native_handle(), Type, &Mei, nullptr, nullptr) != FALSE;
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
			return;

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
	// TODO
#elif defined _M_ARM
	// TODO
#else
	// TODO
#endif
}

static string collect_information(
	exception_context const& Context,
	span<uintptr_t const> NestedStack,
	string_view const Module,
	span<std::pair<string_view, string_view> const> const BasicInfo
)
{
	string Strings;
	Strings.reserve(1024);

	const auto Eol = eol::system.str();

	for (const auto& [Label, Value]: BasicInfo)
	{
		format_to(Strings, FSTR(L"{} {}{}"sv), Label, Value, Eol);
	}

	append(Strings, Eol);

	read_registers(Strings, Context.context_record(), Eol);
	append(Strings, Eol);

	get_backtrace(Module, tracer.get(Module, Context.context_record(), Context.thread_handle()), NestedStack, [&](string_view const Line)
	{
		append(Strings, Line, Eol);
	});
	append(Strings, Eol);

	read_modules(Strings, Eol);

	return Strings;
}

static bool ExcDialog(string const& ReportLocation, bool const CanUnload)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	DialogBuilder Builder(lng::MExceptionDialogTitle);
	Builder.AddText(lng::MExceptionDialogMessage1);
	Builder.AddText(lng::MExceptionDialogMessage2);
	Builder.AddConstEditField(ReportLocation, 70);
	Builder.AddSeparator();

	if (CanUnload)
	{
		lng const MsgIDs[]{ lng::MExcTerminate, lng::MExcUnload, lng::MIgnore };
		Builder.AddButtons(MsgIDs, 0, std::size(MsgIDs) - 1);
	}
	else
	{
		lng const MsgIDs[]{ lng::MExcTerminate, lng::MIgnore };
		Builder.AddButtons(MsgIDs, 0, std::size(MsgIDs) - 1);
	}

	Builder.SetDialogMode(DMODE_WARNINGSTYLE | DMODE_NOPLUGINS);
	Builder.SetScrObjFlags(FSCROBJ_SPECIAL);

	switch (Builder.ShowDialogEx())
	{
	case 0: // Terminate
		UseTerminateHandler = true;
		return true;

	case 1: // Unload / Ignore
		return CanUnload;

	default:
		return false;
	}
}

static bool ExcConsole(string const& ReportLocation, bool)
{
	const auto Eol = eol::std.str();

	std::array Msgs
	{
		L"Oops"sv,
		L"Something went wrong."sv,
		L"Please send the bug report to the developers."sv,
	};

	if (far_language::instance().is_loaded())
	{
		Msgs =
		{
			msg(lng::MExceptionDialogTitle),
			msg(lng::MExceptionDialogMessage1),
			msg(lng::MExceptionDialogMessage2),
		};
	}

	std::wcerr << Eol << Eol;

	for (const auto& i: Msgs)
		std::wcerr << i << Eol;

	std::wcerr << ReportLocation << Eol;

	if (!ConsoleYesNo(L"Terminate the process"sv, true))
		return false;

	UseTerminateHandler = true;
	return true;
}

static bool ShowExceptionUI(
	bool const UseDialog,
	exception_context const& Context,
	string_view const Exception,
	string_view const Details,
	error_state const& ErrorState,
	string_view const Function,
	string_view const Location,
	string_view const ModuleName,
	string_view const PluginInfo,
	Plugin const* const PluginModule,
	span<uintptr_t const> const NestedStack
)
{
	SCOPED_ACTION(tracer_detail::tracer::with_symbols)(PluginModule? ModuleName : L""sv);

	string Address, Name, Source;
	tracer.get_symbol(ModuleName, Context.exception_record().ExceptionAddress, Address, Name, Source);

	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	const auto Errors = ErrorState.format_errors();
	const auto Version = self_version();
	const auto OsVersion = os::version::os_version();
	const auto KernelVersion = kernel_version();

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
		{ L"Far:      "sv, Version,       },
		{ L"OS:       "sv, OsVersion,     },
		{ L"Kernel:   "sv, KernelVersion, },
	};

	const auto log_message = [&]
	{
		auto Message = join(select(BasicInfo, [](auto const& Pair)
		{
			return format(FSTR(L"{} {}"sv), Pair.first, Pair.second);
		}), L"\n"sv);

		Message += L"\n\n"sv;

		get_backtrace(ModuleName, tracer.get(ModuleName, Context.context_record(), Context.thread_handle()), NestedStack, [&](string_view const Line)
		{
			append(Message, Line, L'\n');
		});

		return Message;
	};

	LOG(PluginModule? logging::level::error : logging::level::fatal, L"\n{}\n"sv, log_message());

	const auto CanUnload = PluginModule != nullptr;
	const auto ReportLocation = get_report_location();
	const auto BugReport = collect_information(Context, NestedStack, ModuleName, BasicInfo);
	const auto ReportOnDisk = write_report(BugReport, path::join(ReportLocation, L"bug_report.txt"sv));
	const auto ReportInClipboard = !ReportOnDisk && SetClipboardText(BugReport);
	const auto MinidumpNormal = write_minidump(Context, path::join(ReportLocation, L"far.mdmp"sv), MiniDumpNormal);
	const auto MinidumpFull = write_minidump(Context, path::join(ReportLocation, L"far_full.mdmp"sv), MiniDumpWithFullMemory);
	const auto AnythingOnDisk = ReportOnDisk || MinidumpNormal || MinidumpFull;

	if (AnythingOnDisk)
		OpenFolderInShell(ReportLocation);

	if (AnythingOnDisk || ReportInClipboard)
		return (UseDialog? ExcDialog : ExcConsole)(
			AnythingOnDisk? ReportLocation : msg(lng::MExceptionDialogClipboard),
			CanUnload
		);

	// Should never happen - neither the filesystem nor clipboard are writable, so just dump it to the screen:
	return ExcConsole(BugReport, CanUnload);
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
}

static bool is_cpp_exception(const EXCEPTION_RECORD& Record)
{
	return Record.ExceptionCode == static_cast<DWORD>(EXCEPTION_MICROSOFT_CPLUSPLUS) && Record.NumberParameters;
}

static bool is_fake_cpp_exception(const EXCEPTION_RECORD& Record)
{
	return Record.ExceptionCode == static_cast<DWORD>(EXCEPTION_MICROSOFT_CPLUSPLUS) && !Record.NumberParameters;
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
		const auto& ThrowInfoRef = *reinterpret_cast<detail::ThrowInfo const*>(Record.ExceptionInformation[2]);
		const auto& CatchableTypeArrayRef = *reinterpret_cast<detail::CatchableTypeArray const*>(m_BaseAddress + ThrowInfoRef.pCatchableTypeArray);
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

		const auto& CatchableTypeRef = *reinterpret_cast<detail::CatchableType const*>(m_BaseAddress + m_CatchableTypesRVAs[m_Index++]);
		const auto& TypeInfoRef = *reinterpret_cast<std::type_info const*>(m_BaseAddress + CatchableTypeRef.pType);

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
	if (Iterator == CatchableTypesEnumerator.cend())
		return {};

	return encoding::utf8::get_chars(*Iterator);
}

static bool ProcessExternally(EXCEPTION_POINTERS const& Pointers, Plugin const* const PluginModule)
{
	if (!Global || !Global->Opt->ExceptUsed || Global->Opt->strExceptEventSvc.empty())
		return false;

	const os::rtdl::module Module(Global->Opt->strExceptEventSvc);
	if (!Module)
		return false;

	struct PLUGINRECORD       // информация о плагине
	{
		DWORD TypeRec;          // Тип записи = RTYPE_PLUGIN
		DWORD SizeRec;          // Размер
		DWORD Reserved1[4];
		// DWORD SysID; UUID
		const wchar_t *ModuleName;
		DWORD Reserved2[2];    // резерв :-)
		DWORD SizeModuleName;
	};

	os::rtdl::function_pointer<BOOL(WINAPI*)(EXCEPTION_POINTERS*, const PLUGINRECORD*, const PluginStartupInfo*, DWORD*)> Function(Module, "ExceptionProc");
	if (!Function)
		return false;

	static PluginStartupInfo LocalStartupInfo;
	LocalStartupInfo = {};
	static FarStandardFunctions LocalStandardFunctions;
	LocalStandardFunctions = {};
	CreatePluginStartupInfo(&LocalStartupInfo, &LocalStandardFunctions);
	LocalStartupInfo.ModuleName = Global->Opt->strExceptEventSvc.c_str();
	static PLUGINRECORD PlugRec;

	if (PluginModule)
	{
		PlugRec = {};
		PlugRec.TypeRec = RTYPE_PLUGIN;
		PlugRec.SizeRec = sizeof(PlugRec);
		PlugRec.ModuleName = PluginModule->ModuleName().c_str();
	}

	DWORD dummy;
	auto PointersCopy = Pointers;

	return Function(&PointersCopy, PluginModule ? &PlugRec : nullptr, &LocalStartupInfo, &dummy) != FALSE;
}

static string exception_name(EXCEPTION_RECORD const& ExceptionRecord, string_view const Type)
{
	static const std::pair<string_view, NTSTATUS> KnownExceptions[]
	{
#define TEXTANDCODE(x) L###x##sv, x
		{TEXTANDCODE(EXCEPTION_ACCESS_VIOLATION)},
		{TEXTANDCODE(EXCEPTION_DATATYPE_MISALIGNMENT)},
		{TEXTANDCODE(EXCEPTION_BREAKPOINT)},
		{TEXTANDCODE(EXCEPTION_SINGLE_STEP)},
		{TEXTANDCODE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)},
		{TEXTANDCODE(EXCEPTION_FLT_DENORMAL_OPERAND)},
		{TEXTANDCODE(EXCEPTION_FLT_DIVIDE_BY_ZERO)},
		{TEXTANDCODE(EXCEPTION_FLT_INEXACT_RESULT)},
		{TEXTANDCODE(EXCEPTION_FLT_INVALID_OPERATION)},
		{TEXTANDCODE(EXCEPTION_FLT_OVERFLOW)},
		{TEXTANDCODE(EXCEPTION_FLT_STACK_CHECK)},
		{TEXTANDCODE(EXCEPTION_FLT_UNDERFLOW)},
		{TEXTANDCODE(EXCEPTION_INT_DIVIDE_BY_ZERO)},
		{TEXTANDCODE(EXCEPTION_INT_OVERFLOW)},
		{TEXTANDCODE(EXCEPTION_PRIV_INSTRUCTION)},
		{TEXTANDCODE(EXCEPTION_IN_PAGE_ERROR)},
		{TEXTANDCODE(EXCEPTION_ILLEGAL_INSTRUCTION)},
		{TEXTANDCODE(EXCEPTION_NONCONTINUABLE_EXCEPTION)},
		{TEXTANDCODE(EXCEPTION_STACK_OVERFLOW)},
		{TEXTANDCODE(EXCEPTION_INVALID_DISPOSITION)},
		{TEXTANDCODE(EXCEPTION_GUARD_PAGE)},
		{TEXTANDCODE(EXCEPTION_INVALID_HANDLE)},
		{TEXTANDCODE(EXCEPTION_POSSIBLE_DEADLOCK)},
		{TEXTANDCODE(CONTROL_C_EXIT)},
#undef TEXTANDCODE

		{L"C++ exception"sv,  EXCEPTION_MICROSOFT_CPLUSPLUS},
		{L"std::terminate"sv, EXCEPTION_TERMINATE},
	};

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

	const auto ItemIterator = std::find_if(CONST_RANGE(KnownExceptions, i) { return static_cast<DWORD>(i.second) == ExceptionRecord.ExceptionCode; });
	const auto Name = ItemIterator != std::cend(KnownExceptions) ? ItemIterator->first : L"Unknown exception"sv;
	return WithType(format(FSTR(L"0x{:0>8X} - {}"sv), ExceptionRecord.ExceptionCode, Name));
}

static string exception_details(string_view const Module, EXCEPTION_RECORD const& ExceptionRecord, string_view const Message)
{
	switch (static_cast<NTSTATUS>(ExceptionRecord.ExceptionCode))
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_IN_PAGE_ERROR:
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
		tracer.get_symbols(Module, { ExceptionRecord.ExceptionInformation[1] }, [&](string&& Line)
		{
			Symbol = std::move(Line);
		});

		if (Symbol.empty())
			Symbol = to_hex_wstring(ExceptionRecord.ExceptionInformation[1]);

		return format(FSTR(L"Memory at {} could not be {}"sv), Symbol, Mode);
	}

	case EXCEPTION_MICROSOFT_CPLUSPLUS:
	case EXCEPTION_TERMINATE:
		return string(Message);

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
	span<uintptr_t const> const NestedStack = {}
)
{
	static bool ExceptionHandlingIgnored = false;
	if (ExceptionHandlingIgnored)
		return false;

	if (exception_handling_in_progress())
		return true;

	++s_ExceptionHandlingInprogress;
	SCOPE_EXIT{ --s_ExceptionHandlingInprogress; };

	{
		auto ExceptionRecord = Context.exception_record();
		auto ContextRecord = Context.context_record();
		EXCEPTION_POINTERS const Pointers{ &ExceptionRecord, &ContextRecord };

		if (ProcessExternally(Pointers, PluginModule))
		{
			if (!PluginModule && Global)
				Global->CriticalInternalError = true;

			return true;
		}
	}

	string strFileName;

	if (!PluginModule)
	{
		strFileName = Global?
			Global->g_strFarModuleName :
			os::fs::get_current_process_file_name();
	}
	else
	{
		strFileName = PluginModule->ModuleName();
	}

	const auto Exception = exception_name(Context.exception_record(), Type);
	const auto Details = exception_details(strFileName, Context.exception_record(), Message);

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
	return dummy_current_exception(EXCEPTION_MICROSOFT_CPLUSPLUS);
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

class far_wrapper_exception: public far_exception
{
public:
	far_wrapper_exception(std::string_view const Function, std::string_view const File, int const Line):
		far_exception(true, L"exception_ptr"sv, Function, File, Line),
		m_ThreadHandle(std::make_shared<os::handle>(os::OpenCurrentThread())),
		m_Stack(tracer.get({}, *exception_information().ContextRecord, m_ThreadHandle->native_handle()))
	{
	}

	span<uintptr_t const> get_stack() const noexcept { return m_Stack; }

private:
	std::shared_ptr<os::handle> m_ThreadHandle;
	std::vector<uintptr_t> m_Stack;
};

std::exception_ptr wrap_current_exception(std::string_view const Function, std::string_view const File, int const Line)
{
	try
	{
		std::throw_with_nested(far_wrapper_exception(Function, File, Line));
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
	catch (const std::exception& e)
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

class seh_exception: public far_exception
{
public:
	template<typename... args>
	explicit seh_exception(EXCEPTION_POINTERS& Pointers, args&&... Args):
		far_exception(FWD(Args)...),
		m_Context(std::make_shared<seh_exception_context>(Pointers))
	{}

	const auto& context() const noexcept { return *m_Context; }

private:
	// Q: Why do you need a shared_ptr here?
	// A: The exception must be copyable
	std::shared_ptr<seh_exception_context> m_Context;
};


static bool handle_std_exception(
	exception_context const& Context,
	const std::exception& e,
	std::string_view const Function,
	const Plugin* const Module
)
{
	if (const auto SehException = dynamic_cast<const seh_exception*>(&e))
	{
		return handle_generic_exception(
			Context,
			Function,
			SehException->location(),
			Module,
			{},
			SehException->message(),
			*SehException,
			tracer.get({}, SehException->context().context_record(), SehException->context().thread_handle())
		);
	}

	const auto& [Type, What] = extract_nested_exceptions(Context.exception_record(), e);

	if (const auto FarException = dynamic_cast<const detail::far_base_exception*>(&e))
	{
		const auto NestedStack = [&]
		{
			const auto Wrapper = dynamic_cast<const far_wrapper_exception*>(&e);
			return Wrapper? Wrapper->get_stack() : span<uintptr_t const>{};
		}();

		return handle_generic_exception(Context, FarException->function(), FarException->location(), Module, Type, What, *FarException, NestedStack);
	}

	return handle_generic_exception(Context, Function, {}, Module, Type, What);
}

bool handle_std_exception(const std::exception& e, std::string_view const Function, const Plugin* const Module)
{
	return handle_std_exception(exception_context(exception_information()), e, Function, Module);
}

static bool handle_seh_exception(
	exception_context const& Context,
	std::string_view const Function,
	Plugin const* const PluginModule
)
{
	for (const auto& i : enum_catchable_objects(Context.exception_record()))
	{
		if (strstr(i, "std::exception"))
			return handle_std_exception(Context, *reinterpret_cast<std::exception const*>(Context.exception_record().ExceptionInformation[1]), Function, PluginModule);
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

static void seh_terminate_handler_impl()
{
	if (!HandleCppExceptions)
		std::abort();

	static auto InsideTerminateHandler = false;
	if (InsideTerminateHandler)
		std::abort();

	InsideTerminateHandler = true;

	// If it's a SEH or a C++ exception implemented in terms of SEH (and not a fake for GCC) it's better to handle it as SEH
	if (const auto Info = exception_information(); Info.ContextRecord && Info.ExceptionRecord && !is_fake_cpp_exception(*Info.ExceptionRecord))
	{
		if (handle_seh_exception(exception_context(Info), CURRENT_FUNCTION_NAME, {}))
			std::abort();
	}

	// It's a C++ exception, implemented in some other way (GCC)
	if (const auto CurrentException = std::current_exception())
	{
		try
		{
			std::rethrow_exception(CurrentException);
		}
		catch(std::exception const& e)
		{
			if (handle_std_exception(e, CURRENT_FUNCTION_NAME, {}))
				std::abort();
		}
		catch (...)
		{
			if (handle_unknown_exception(CURRENT_FUNCTION_NAME, {}))
				std::abort();
		}
	}

	// No exception in flight, must be a direct call
	exception_context const Context
	({
		static_cast<EXCEPTION_RECORD*>(*dummy_current_exception(EXCEPTION_TERMINATE)),
		static_cast<CONTEXT*>(*dummy_current_exception_context())
	});

	if (handle_generic_exception(Context, CURRENT_FUNCTION_NAME, {}, {}, {}, L"Abnormal termination"sv))
		std::abort();

	restore_system_exception_handler();
}

seh_terminate_handler::seh_terminate_handler():
	m_PreviousHandler(std::set_terminate(seh_terminate_handler_impl))
{
}

seh_terminate_handler::~seh_terminate_handler()
{
	std::set_terminate(m_PreviousHandler);
}

static LONG WINAPI unhandled_exception_filter_impl(EXCEPTION_POINTERS* const Pointers)
{
	if (!HandleSehExceptions)
	{
		restore_system_exception_handler();
		return EXCEPTION_CONTINUE_SEARCH;
	}

	detail::set_fp_exceptions(false);
	if (handle_seh_exception(exception_context(*Pointers), CURRENT_FUNCTION_NAME, {}))
	{
		std::_Exit(EXIT_FAILURE);
	}
	restore_system_exception_handler();
	return EXCEPTION_CONTINUE_SEARCH;
}

unhandled_exception_filter::unhandled_exception_filter():
	m_PreviousFilter(SetUnhandledExceptionFilter(unhandled_exception_filter_impl))
{
}

unhandled_exception_filter::~unhandled_exception_filter()
{
	SetUnhandledExceptionFilter(m_PreviousFilter);
}

[[noreturn]]
static void purecall_handler_impl()
{
	// VC invokes abort if the user handler isn't set,
	// so we call terminate here, which we already intercept.
	// GCC just invokes terminate directly, no further actions needed.
	std::terminate();
}

static _purecall_handler set_purecall_handler(_purecall_handler const Handler)
{
	return
#if IS_MICROSOFT_SDK()
		_set_purecall_handler(Handler)
#else
		nullptr
#endif
		;
}

purecall_handler::purecall_handler():
	m_PreviousHandler(set_purecall_handler(purecall_handler_impl))
{
}

purecall_handler::~purecall_handler()
{
	set_purecall_handler(m_PreviousHandler);
}

static void invalid_parameter_handler_impl(const wchar_t* const Expression, const wchar_t* const Function, const wchar_t* const File, unsigned int const Line, uintptr_t const Reserved)
{
	exception_context const Context
	({
		static_cast<EXCEPTION_RECORD*>(*dummy_current_exception(EXCEPTION_TERMINATE)),
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
		std::abort();

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
					std::_Exit(EXIT_FAILURE);

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
		if (HandleSehExceptions)
		{
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
		}

		restore_system_exception_handler();
		return EXCEPTION_CONTINUE_SEARCH;
	}

	int seh_thread_filter(std::exception_ptr& Ptr, EXCEPTION_POINTERS* const Info)
	{
		// SEH transport between threads is currenly implemented in terms of C++ exceptions, so it requires both
		if (!(HandleSehExceptions && HandleCppExceptions))
		{
			restore_system_exception_handler();
			return EXCEPTION_CONTINUE_SEARCH;
		}

		Ptr = std::make_exception_ptr(
			MAKE_EXCEPTION(
				seh_exception,
				*Info,
				true,
				concat(
					exception_name(*Info->ExceptionRecord, {}),
					L" - "sv,
					exception_details({}, *Info->ExceptionRecord, {})
				)
			)
		);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	void seh_thread_handler(DWORD)
	{
		// The thread is about to quit, but we still need it to get a stack trace / write a minidump.
		// It will be released once the corresponding exception context is destroyed.
		// The caller MUST detach it if ExceptionPtr is not empty.
		SuspendThread(GetCurrentThread());
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
	static_assert(fourcc("CPLG"sv) == 0x474C5043);
	static_assert(fourcc("avc1"sv) == 0x31637661);

	REQUIRE(true);
}
#endif
