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
#include "farcolor.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "keys.hpp"
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

// Platform:
#include "platform.fs.hpp"
#include "platform.reg.hpp"
#include "platform.version.hpp"

// Common:
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

class exception_context
{
public:
	NONCOPYABLE(exception_context);

	explicit exception_context(EXCEPTION_POINTERS const& Pointers) noexcept:
		m_Pointers(Pointers),
		m_ThreadHandle(os::OpenCurrentThread()),
		m_ThreadId(GetCurrentThreadId())
	{
	}

	auto code() const noexcept { return m_Pointers.ExceptionRecord->ExceptionCode; }
	auto& pointers() const noexcept { return m_Pointers; }
	auto thread_handle() const noexcept { return m_ThreadHandle.native_handle(); }
	auto thread_id() const noexcept { return m_ThreadId; }

private:
	EXCEPTION_POINTERS m_Pointers;
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
	HandleCppExceptions = false;
	HandleSehExceptions = false;
}

void force_stderr_exception_ui(bool const Force)
{
	ForceStderrExceptionUI = Force;
}

void CreatePluginStartupInfo(PluginStartupInfo *PSI, FarStandardFunctions *FSF);

constexpr NTSTATUS
	EXCEPTION_MICROSOFT_CPLUSPLUS = 0xE06D7363, // EH_EXCEPTION_NUMBER
	EXCEPTION_TERMINATE           = 0xE074726D; // 'trm'

enum exception_dialog
{
	ed_doublebox,
	ed_first_line,
	ed_text_exception = ed_first_line,
	ed_edit_exception,
	ed_text_details,
	ed_edit_details,
	ed_text_errno,
	ed_edit_errno,
	ed_text_lasterror,
	ed_edit_lasterror,
	ed_text_ntstatus,
	ed_edit_ntstatus,
	ed_text_address,
	ed_edit_address,
	ed_text_function,
	ed_edit_function,
	ed_text_source,
	ed_edit_source,
	ed_text_module,
	ed_edit_module,
	ed_text_plugin_information,
	ed_edit_plugin_information,
	ed_text_far_version,
	ed_edit_far_version,
	ed_text_os_version,
	ed_edit_os_version,
	ed_text_kernel_version,
	ed_edit_kernel_version,
	ed_last_line = ed_edit_kernel_version,
	ed_separator_1,
	ed_button_copy,
	ed_button_stack,
	ed_button_minidump,
	ed_separator_2,
	ed_button_terminate,
	ed_button_unload,
	ed_button_ignore,

	ed_first_button = ed_button_copy,
	ed_last_button = ed_button_ignore,

	ed_items_count
};

static std::vector<DWORD64> current_stack()
{
	std::vector<DWORD64> Stack;
	Stack.reserve(128);

	for (DWORD i = 0; ;)
	{
		void* Pointers[128];
		const auto Size = imports.RtlCaptureStackBackTrace(i, static_cast<DWORD>(std::size(Pointers)), Pointers, {});
		if (!Size)
			break;

		std::transform(Pointers, Pointers + Size, std::back_inserter(Stack), [](void* Ptr)
		{
			return reinterpret_cast<uintptr_t>(Ptr);
		});

		i += Size;
	}

	return Stack;
}

static void get_backtrace(string_view const Module, const std::vector<DWORD64>& Stack, const std::vector<DWORD64>* NestedStack, function_ref<void(string&&)> const Consumer)
{
	const auto Separator = [&](string&& Message)
	{
		Consumer(string(40, L'-'));
		Consumer(std::move(Message));
		Consumer(string(40, L'-'));
	};

	if (NestedStack)
	{
		tracer::get_symbols(Module, *NestedStack, Consumer);
		Separator(L"Rethrow:"s);
	}

	tracer::get_symbols(Module, Stack, Consumer);

	Separator(L"Current stack:"s);
	tracer::get_symbols(Module, current_stack(), Consumer);
}

static void show_backtrace(string_view const Module, const std::vector<DWORD64>& Stack, const std::vector<DWORD64>* NestedStack, bool const UseDialog)
{
	if (UseDialog)
	{
		std::vector<string> Symbols;
		Symbols.reserve(Stack.size() + (NestedStack ? NestedStack->size() + 3 : 0));
		get_backtrace(Module, Stack, NestedStack, [&](string&& Line)
		{
			Symbols.emplace_back(std::move(Line));
		});

		Message(MSG_WARNING | MSG_LEFTALIGN,
			msg(lng::MExcTrappedException),
			std::move(Symbols),
			{ lng::MOk });
	}
	else
	{
		std::wcerr << L'\n';

		get_backtrace(Module, Stack, NestedStack, [&](string_view const Line)
		{
			std::wcerr << Line << L'\n';
		});
	}
}

static bool write_minidump(const exception_context& Context, string_view const Path)
{
	if (!imports.MiniDumpWriteDump)
		return false;

	const os::fs::file DumpFile(Path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS);
	if (!DumpFile)
		return false;

	auto PointersCopy = Context.pointers();
	MINIDUMP_EXCEPTION_INFORMATION Mei = { Context.thread_id(), &PointersCopy };

	return imports.MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), DumpFile.get().native_handle(), MiniDumpWithFullMemory, &Mei, nullptr, nullptr) != FALSE;
}

static string self_version()
{
	const auto Version = format(FSTR(L"{0} {1}"), version_to_string(build::version()), build::platform());
	const auto ScmRevision = build::scm_revision();
	return ScmRevision.empty()? Version : Version + format(FSTR(L" ({0:.7})"), ScmRevision);
}

static bool get_os_version(OSVERSIONINFOEX& Info)
{
	const auto InfoPtr = static_cast<OSVERSIONINFO*>(static_cast<void*>(&Info));

	if (imports.RtlGetVersion && imports.RtlGetVersion(InfoPtr) == STATUS_SUCCESS)
		return true;

WARNING_PUSH()
WARNING_DISABLE_MSC(4996) // 'GetVersionExW': was declared deprecated. So helpful. :(
WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
	return GetVersionEx(InfoPtr) != FALSE;
WARNING_POP()
}

static string os_version_from_api()
{
	OSVERSIONINFOEX Info{ sizeof(Info) };
	if (!get_os_version(Info))
		return L"Unknown"s;

	return format(FSTR(L"{0}.{1}.{2}.{3}.{4}.{5}.{6}.{7}"),
		Info.dwMajorVersion,
		Info.dwMinorVersion,
		Info.dwBuildNumber,
		Info.dwPlatformId,
		Info.wServicePackMajor,
		Info.wServicePackMinor,
		Info.wSuiteMask,
		Info.wProductType
	);
}

// Mental OS - mental methods *facepalm*
static string os_version_from_registry()
{
	const auto Key = os::reg::key::open(os::reg::key::local_machine, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"sv, KEY_QUERY_VALUE);
	if (!Key)
		return {};

	string ReleaseId, CurrentBuild;
	unsigned UBR;
	if (!Key.get(L"ReleaseId"sv, ReleaseId) || !Key.get(L"CurrentBuild"sv, CurrentBuild) || !Key.get(L"UBR"sv, UBR))
		return {};

	return format(FSTR(L" (version {0}, OS build {1}.{2})"), ReleaseId, CurrentBuild, UBR);
}

static string os_version()
{
	return os_version_from_api() + os_version_from_registry();
}

static string kernel_version()
{
	os::version::file_version Version;
	if (!Version.read(L"ntoskrnl.exe"sv))
		return L"Unknown"s;

	if (const auto Str = Version.get_string(L"FileVersion"sv))
		return Str;

	const auto FixedInfo = Version.get_fixed_info();
	if (!FixedInfo)
		return L"Unknown"s;

	return format(FSTR(L"{0}.{1}.{2}.{3}"),
		HIWORD(FixedInfo->dwFileVersionMS),
		LOWORD(FixedInfo->dwFileVersionMS),
		HIWORD(FixedInfo->dwFileVersionLS),
		LOWORD(FixedInfo->dwFileVersionLS)
	);
}

struct dialog_data_type
{
	const exception_context* Context;
	const std::vector<DWORD64>* NestedStack;
	string_view Module;
};

static void read_registers(string& To, CONTEXT const& Context, string_view const Eol)
{
	const auto r = [&](string_view const Name, auto const Value)
	{
		format_to(To, FSTR(L"{0:3} = {1:0{2}X}{3}"), Name, Value, sizeof(Value) * 2, Eol);
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

static void copy_information(Dialog* const Dlg)
{
	string Strings;
	Strings.reserve(1024);

	const auto Eol = eol::system.str();
	FarDialogItem di;
	Dlg->SendMessage(DM_GETDLGITEMSHORT, 1, &di);
	const auto Width = di.X2 - di.X1 + 1;

	for (size_t i = ed_text_exception; i != ed_separator_1; ++i)
	{
		const auto Str = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, i, nullptr));
		append(Strings, format(FSTR(L"{0:{1}}{2}"), Str, i & 1 ? Width : 0, i & 1 ? L" "sv : Eol));
	}

	append(Strings, Eol);

	const auto& Data = *reinterpret_cast<dialog_data_type*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

	read_registers(Strings, *Data.Context->pointers().ContextRecord, Eol);
	append(Strings, Eol);

	get_backtrace(Data.Module, tracer::get(Data.Module, Data.Context->pointers(), Data.Context->thread_handle()), Data.NestedStack, [&](string_view const Line)
	{
		append(Strings, Line, Eol);
	});

	SetClipboardText(Strings);
}

static intptr_t ExcDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
		{
			FarDialogItem di;
			Dlg->SendMessage(DM_GETDLGITEMSHORT,Param1,&di);

			if (di.Type==DI_EDIT)
			{
				const auto& Color = colors::PaletteColorToFarColor(COL_WARNDIALOGTEXT);
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;

		case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD *>(Param2);
			if (record->EventType == KEY_EVENT)
			{
				switch (InputRecordToKey(record))
				{
				case KEY_LEFT:
				case KEY_NUMPAD4:
				case KEY_SHIFTTAB:
					if (Param1 == ed_first_button)
					{
						Dlg->SendMessage(DM_SETFOCUS, ed_last_button, nullptr);
						return TRUE;
					}
					break;

				case KEY_RIGHT:
				case KEY_NUMPAD6:
				case KEY_TAB:
					if (Param1 == ed_last_button)
					{
						Dlg->SendMessage(DM_SETFOCUS, ed_first_button, nullptr);
						return TRUE;
					}
					break;

				case KEY_CTRLC:
				case KEY_RCTRLC:
				case KEY_CTRLINS:
				case KEY_RCTRLINS:
				case KEY_CTRLNUMPAD0:
				case KEY_RCTRLNUMPAD0:
					copy_information(Dlg);
					break;
				}
			}
		}
		break;

		case DN_CLOSE:
		{
			const auto& Data = *reinterpret_cast<dialog_data_type*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

			switch (Param1)
			{
			case ed_button_copy:
				copy_information(Dlg);
				return FALSE;

			case ed_button_stack:
				show_backtrace(Data.Module, tracer::get(Data.Module, Data.Context->pointers(), Data.Context->thread_handle()), Data.NestedStack, true);
				return FALSE;

			case ed_button_minidump:
				{
					// TODO: subdirectory && timestamp
					auto Path = path::join(Global->Opt->LocalProfilePath, L"Far.mdmp"sv);

					if (write_minidump(*Data.Context, Path))
					{
						Message(0,
							msg(lng::MExcMinidump),
							{
								msg(lng::MExcMinidumpSuccess),
								std::move(Path)
							},
							{ lng::MOk });
					}
					else
					{
						const auto ErrorState = error_state::fetch();
						Message(MSG_WARNING, ErrorState,
							msg(lng::MError),
							{
								msg(lng::MEditCannotSave),
								std::move(Path)
							},
							{ lng::MOk });
					}

				}
				return FALSE;
			}
		}
		break;

	default:
		break;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

static size_t max_item_size(span<string_view const> const Items)
{
	return std::max_element(ALL_CONST_RANGE(Items), [](string_view const Str1, string_view const Str2)
	{
		return Str1.size() < Str2.size();
	})->size();
}

static bool ExcDialog(
	span<string_view const> const Labels,
	span<string_view const> const Values,
	exception_context const& Context,
	string_view const ModuleName,
	Plugin const* const PluginModule,
	std::vector<DWORD64> const* const NestedStack
)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	const auto SysArea = 5;
	const auto C1X = 5;
	const auto C1W = static_cast<int>(max_item_size(Labels));
	const auto C2X = C1X + C1W + 1;
	const auto DlgW = std::max(80, std::min(ScrX + 1, static_cast<int>(C2X + max_item_size(Values) + SysArea + 1)));
	const auto C2W = DlgW - C2X - SysArea - 1;

	const auto DY = static_cast<int>(Values.size() + 8);

	auto EditDlg = MakeDialogItems<ed_items_count>(
	{
		{ DI_DOUBLEBOX, {{3,   1 }, {DlgW-4,DY-2}}, DIF_NONE, msg(lng::MExcTrappedException), },

#define LABEL_AND_VALUE(n)\
		{ DI_TEXT,  {{C1X, n+2 }, {C1X+C1W, n+2 }}, DIF_NONE, Labels[n], },\
		{ DI_EDIT,  {{C2X, n+2 }, {C2X+C2W, n+2 }}, DIF_READONLY | DIF_SELECTONENTRY, Values[n], }

		LABEL_AND_VALUE(0),
		LABEL_AND_VALUE(1),
		LABEL_AND_VALUE(2),
		LABEL_AND_VALUE(3),
		LABEL_AND_VALUE(4),
		LABEL_AND_VALUE(5),
		LABEL_AND_VALUE(6),
		LABEL_AND_VALUE(7),
		LABEL_AND_VALUE(8),
		LABEL_AND_VALUE(9),
		LABEL_AND_VALUE(10),
		LABEL_AND_VALUE(11),
		LABEL_AND_VALUE(12),

#undef LABEL_AND_VALUE

		{ DI_TEXT,      {{-1,DY-6}, {0,     DY-6}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0, DY-5}, {0,     DY-5}}, DIF_CENTERGROUP | DIF_FOCUS, msg(lng::MExcCopy), },
		{ DI_BUTTON,    {{0, DY-5}, {0,     DY-5}}, DIF_CENTERGROUP, msg(lng::MExcStack), },
		{ DI_BUTTON,    {{0, DY-5}, {0,     DY-5}}, DIF_CENTERGROUP, msg(lng::MExcMinidump), },
		{ DI_TEXT,      {{-1,DY-4}, {0,     DY-4}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0, DY-3}, {0,     DY-3}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MExcTerminate), },
		{ DI_BUTTON,    {{0, DY-3}, {0,     DY-3}}, DIF_CENTERGROUP | (PluginModule? DIF_NONE : DIF_DISABLE | DIF_HIDDEN), msg(lng::MExcUnload), },
		{ DI_BUTTON,    {{0, DY-3}, {0,     DY-3}}, DIF_CENTERGROUP, msg(lng::MIgnore), },
	});

	dialog_data_type DlgData{ &Context, NestedStack, ModuleName };
	const auto Dlg = Dialog::create(EditDlg, ExcDlgProc, &DlgData);
	Dlg->SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg->SetFlags(FSCROBJ_SPECIAL);
	Dlg->SetPosition({ -1, -1, DlgW, static_cast<int>(DY) });
	Dlg->Process();

	switch (Dlg->GetExitCode())
	{
	case ed_button_terminate:
		UseTerminateHandler = true;
		[[fallthrough]];
	case ed_button_unload:
		return true;

	default:
		return false;
	}
}

static bool ExcConsole(
	span<string_view const> const Labels,
	span<string_view const> const Values,
	exception_context const& Context,
	string_view const ModuleName,
	Plugin const* const PluginModule,
	std::vector<DWORD64> const* const NestedStack
)
{
	std::wcerr << L'\n';

	for (const auto& [m, v]: zip(Labels, Values))
	{
		const auto Label = fit_to_left(string(m), max_item_size(Labels));
		std::wcerr << Label << L' ' << v << L'\n';
	}

	{
		string Registers(L"\n"sv);
		read_registers(Registers, *Context.pointers().ContextRecord, L"\n"sv);
		std::wcerr << Registers;
	}

	show_backtrace(ModuleName, tracer::get(ModuleName, Context.pointers(), Context.thread_handle()), NestedStack, false);
	std::wcerr << std::endl;

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
	string_view const PluginInformation,
	Plugin const* const PluginModule,
	std::vector<DWORD64> const* const NestedStack
)
{
	SCOPED_ACTION(tracer::with_symbols)(PluginModule? ModuleName : L""sv);

	string Address, Name, Source;
	tracer::get_symbol(ModuleName, Context.pointers().ExceptionRecord->ExceptionAddress, Address, Name, Source);

	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	const auto Errors = ErrorState.format_errors();
	const auto Version = self_version();
	const auto OsVersion = os_version();
	const auto KernelVersion = kernel_version();

	std::array Labels
	{
		L"Exception:"sv,
		L"Details:  "sv,
		L"errno:    "sv,
		L"LastError:"sv,
		L"NTSTATUS: "sv,
		L"Address:  "sv,
		L"Function: "sv,
		L"Source:   "sv,
		L"File:     "sv,
		L"Plugin:   "sv,
		L"Far:      "sv,
		L"OS:       "sv,
		L"Kernel:   "sv,
	};

	if (far_language::instance().is_loaded())
	{
		Labels =
		{
			msg(lng::MExcException),
			msg(lng::MExcDetails),
			L"errno:"sv,
			L"LastError:"sv,
			L"NTSTATUS:"sv,
			msg(lng::MExcAddress),
			msg(lng::MExcFunction),
			msg(lng::MExcSource),
			msg(lng::MExcFileName),
			msg(lng::MExcPlugin),
			msg(lng::MExcFarVersion),
			msg(lng::MExcOSVersion),
			msg(lng::MExcKernelVersion),
		};
	}

	const string_view Values[]
	{
		Exception,
		Details,
		Errors[0],
		Errors[1],
		Errors[2],
		Address,
		Function,
		Source,
		ModuleName,
		PluginInformation,
		Version,
		OsVersion,
		KernelVersion,
	};

	static_assert(std::size(Labels) == std::size(Values));
	static_assert(std::size(Labels) == (ed_last_line - ed_first_line) / 2 + 1);

	return (UseDialog? ExcDialog : ExcConsole)(Labels, span(Values), Context, ModuleName, PluginModule, NestedStack); // This goofy explicit span() is a workaround for GCC
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

static bool handle_generic_exception(
	exception_context const& Context,
	std::string_view const Function,
	string_view const Location,
	Plugin const* const PluginModule,
	string_view const Type,
	string_view const Message,
	error_state const& ErrorState = error_state::fetch(),
	std::vector<DWORD64> const* const NestedStack = nullptr
)
{
	static bool ExceptionHandlingIgnored = false;
	if (ExceptionHandlingIgnored)
		return false;


	if (Global)
		Global->ProcessException = true;

	if (ProcessExternally(Context.pointers(), PluginModule))
	{
		if (!PluginModule && Global)
			Global->CriticalInternalError = true;

		return true;
	}

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

	string strFileName;

	const auto xr = Context.pointers().ExceptionRecord;

	if (!PluginModule)
	{
		if (Global)
		{
			strFileName=Global->g_strFarModuleName;
		}
		else
		{
			// BUGBUG check result
			(void)os::fs::GetModuleFileName(nullptr, nullptr, strFileName);
		}
	}
	else
	{
		strFileName = PluginModule->ModuleName();
	}

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

		if (const auto TypeStr = ExtractObjectType(*xr); !TypeStr.empty())
		{
			AppendType(Str, TypeStr);
		}
		return std::move(Str);
	};

	const auto Exception = [&](NTSTATUS Code)
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(KnownExceptions, i) { return i.second == Code; });
		const auto Name = ItemIterator != std::cend(KnownExceptions)? ItemIterator->first : L"Unknown exception"sv;
		return WithType(format(FSTR(L"0x{0:0>8X} - {1}"), static_cast<DWORD>(Code), Name));
	}(Context.code());

	string Details;

	switch(static_cast<NTSTATUS>(Context.code()))
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_IN_PAGE_ERROR:
		{
			const auto AccessedAddress = to_hex_wstring(xr->ExceptionInformation[1]);
			const auto Mode = [](ULONG_PTR Code)
			{
				switch (Code)
				{
				default:
				case EXCEPTION_READ_FAULT:    return L"read"sv;
				case EXCEPTION_WRITE_FAULT:   return L"written"sv;
				case EXCEPTION_EXECUTE_FAULT: return L"executed"sv;
				}
			}(xr->ExceptionInformation[0]);

			Details = format(FSTR(L"Memory at {0} could not be {1}"), AccessedAddress, Mode);
		}
		break;

	case EXCEPTION_MICROSOFT_CPLUSPLUS:
	case EXCEPTION_TERMINATE:
		Details = Message;
		break;

	default:
		Details = os::GetErrorString(true, Context.code());
		break;
	}

	const auto PluginInformation = PluginModule? format(FSTR(L"{0} {1} ({2}, {3})"),
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
	far_wrapper_exception(const char* const Function, string_view const File, int const Line):
		far_exception(L"exception_ptr"sv, Function, File, Line),
		m_ThreadHandle(std::make_shared<os::handle>(os::OpenCurrentThread())),
		m_Stack(tracer::get({}, exception_information(), m_ThreadHandle->native_handle()))
	{
	}

	const auto& get_stack() const noexcept { return m_Stack; }

private:
	std::shared_ptr<os::handle> m_ThreadHandle;
	std::vector<DWORD64> m_Stack;
};

std::exception_ptr wrap_current_exception(const char* const Function, string_view const File, int const Line)
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

class seh_exception: public error_state_ex, public std::exception
{
public:
	explicit seh_exception(EXCEPTION_POINTERS& Pointers):
		error_state_ex(fetch()),
		m_Context(std::make_shared<seh_exception_context>(Pointers))
	{
	}

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
		auto NestedStack = tracer::get({}, SehException->context().pointers(), SehException->context().thread_handle());
		return handle_generic_exception(Context, Function, {}, Module, {}, {}, *SehException, &NestedStack);
	}

	const auto& [Type, What] = extract_nested_exceptions(*Context.pointers().ExceptionRecord, e);

	if (const auto FarException = dynamic_cast<const detail::far_base_exception*>(&e))
	{
		const auto NestedStack = [&]
		{
			const auto Wrapper = dynamic_cast<const far_wrapper_exception*>(&e);
			return Wrapper ? &Wrapper->get_stack() : nullptr;
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
	for (const auto& i : enum_catchable_objects(*Context.pointers().ExceptionRecord))
	{
		if (strstr(i, "std::exception"))
			return handle_std_exception(Context, *reinterpret_cast<std::exception const*>(Context.pointers().ExceptionRecord->ExceptionInformation[1]), Function, PluginModule);
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
	if (exception_context const Context(exception_information());
		Context.pointers().ExceptionRecord &&
		Context.pointers().ContextRecord &&
		!is_fake_cpp_exception(*Context.pointers().ExceptionRecord)
	)
	{
		if (handle_seh_exception(Context, __FUNCTION__, {}))
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
			if (handle_std_exception(e, __FUNCTION__, {}))
				std::abort();
		}
		catch (...)
		{
			if (handle_unknown_exception(__FUNCTION__, {}))
				std::abort();
		}
	}

	// No exception in flight, must be a direct call
	exception_context const Context
	({
		static_cast<EXCEPTION_RECORD*>(*dummy_current_exception(EXCEPTION_TERMINATE)),
		static_cast<CONTEXT*>(*dummy_current_exception_context())
	});

	if (handle_generic_exception(Context, __FUNCTION__, {}, {}, {}, L"Abnormal termination"sv))
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
	if (handle_seh_exception(exception_context(*Pointers), __FUNCTION__, {}))
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

	void seh_try(function_ref<void()> const Callable, function_ref<DWORD(EXCEPTION_POINTERS*)> const Filter, function_ref<void()> const Handler)
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

			Handler();
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
					os::thread(os::thread::mode::join, [&]{ Result = handle_seh_exception(Context, Function, Module); });
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

		Ptr = std::make_exception_ptr(seh_exception(*Info));
		return EXCEPTION_EXECUTE_HANDLER;
	}

	void seh_thread_handler()
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
