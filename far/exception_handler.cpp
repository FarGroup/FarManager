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

#include "exception_handler.hpp"

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

#include "platform.fs.hpp"

#include "common/zip_view.hpp"

#include "format.hpp"

void CreatePluginStartupInfo(PluginStartupInfo *PSI, FarStandardFunctions *FSF);

#define EXCEPTION_MICROSOFT_CPLUSPLUS ((NTSTATUS)0xE06D7363)

enum exception_dialog
{
	ed_doublebox,
	ed_text_exception,
	ed_edit_exception,
	ed_text_details,
	ed_edit_details,
	ed_text_lasterror,
	ed_edit_lasterror,
	ed_text_ntstatus,
	ed_edit_ntstatus,
	ed_text_address,
	ed_edit_address,
	ed_text_source,
	ed_edit_source,
	ed_text_function,
	ed_edit_function,
	ed_text_module,
	ed_edit_module,
	ed_separator,
	ed_button_terminate,
	ed_button_stack,
	ed_button_minidump,
	ed_button_ignore,

	ed_first_button = ed_button_terminate,
	ed_last_button = ed_button_ignore
};

static void ShowStackTrace(const std::vector<const void*>& Stack, const std::vector<const void*>* NestedStack)
{
	std::vector<string> Symbols;
	Symbols.reserve(Stack.size() + (NestedStack? NestedStack->size() + 3 : 0));

	const auto Consumer = [&Symbols](string&& Address, string&& Name, string&& Source)
	{
		if (!Name.empty())
			append(Address, L' ', Name);

		if (!Source.empty())
			append(Address, L" ("sv, Source, L')');

		Symbols.emplace_back(std::move(Address));
	};

	tracer::get_symbols(Stack, Consumer);

	if (NestedStack)
	{
		Symbols.emplace_back(40, L'-');
		Symbols.emplace_back(L"Nested stack:"s);
		Symbols.emplace_back(40, L'-');

		tracer::get_symbols(*NestedStack, Consumer);
	}

	if (Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		Message(MSG_WARNING | MSG_LEFTALIGN,
			msg(lng::MExcTrappedException),
			std::move(Symbols),
			{ lng::MOk });
	}
	else
	{
		std::wcerr << L'\n';

		for (const auto& Str: Symbols)
		{
			std::wcerr << Str << L'\n';
		}
	}
}

static bool write_minidump(const detail::exception_context& Context)
{
	if (!imports.MiniDumpWriteDump)
		return false;

	// TODO: subdirectory && timestamp
	const os::fs::file DumpFile(path::join(Global->Opt->LocalProfilePath, L"Far.mdmp"sv), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS);
	if (!DumpFile)
		return false;

	MINIDUMP_EXCEPTION_INFORMATION Mei = { Context.thread_id(), Context.pointers() };
	return imports.MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), DumpFile.get().native_handle(), MiniDumpWithFullMemory, &Mei, nullptr, nullptr) != FALSE;
}

using dialog_data_type = std::pair<const detail::exception_context*, const std::vector<const void*>*>;

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
				const auto key = InputRecordToKey(record);

				if (Param1 == ed_first_button && (key == KEY_LEFT || key == KEY_NUMPAD4 || key == KEY_SHIFTTAB))
				{
					Dlg->SendMessage(DM_SETFOCUS, ed_last_button, nullptr);
					return TRUE;
				}

				if (Param1 == ed_last_button && (key == KEY_RIGHT || key == KEY_NUMPAD6 || key == KEY_TAB))
				{
					Dlg->SendMessage(DM_SETFOCUS, ed_first_button, nullptr);
					return TRUE;
				}
			}
		}
		break;

		case DN_CLOSE:
		{
			const auto& DialogData = *reinterpret_cast<dialog_data_type*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

			switch (Param1)
			{
			case ed_button_stack:
				ShowStackTrace(tracer::get(*DialogData.first->pointers(), DialogData.first->thread_handle()), DialogData.second);
				return FALSE;

			case ed_button_minidump:
				write_minidump(*DialogData.first);
				return FALSE;
			}
		}
		break;

	default:
		break;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

enum reply
{
	reply_handle,
	reply_minidump,
	reply_ignore,
};

static reply ExcDialog(
	string const& ModuleName,
	string const& Exception,
	string const& Details,
	detail::exception_context const& Context,
	string_view const Function,
	string_view const Location,
	Plugin const* const PluginModule,
	error_state const* const ErrorState,
	std::vector<const void*> const* const NestedStack
)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	SCOPED_ACTION(auto)(tracer::with_symbols());

	string Address, Name, Source;
	tracer::get_symbol(Context.pointers()->ExceptionRecord->ExceptionAddress, Address, Name, Source);

	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	const string FunctionName(Function);

	const auto Errors = FormatSystemErrors(ErrorState);

	const string* Messages[]
	{
		&Exception,
		&Details,
		&Errors[0],
		&Errors[1],
		&Address,
		&Source,
		&FunctionName,
		&ModuleName,
	};

	const auto MaxSize = (*std::max_element(ALL_CONST_RANGE(Messages), [](string const* const Str1, string const* const Str2) { return Str1->size() < Str2->size(); }))->size();
	const auto SysArea = 5;
	const auto Col1X = 5;
	const auto Col1W = 12;
	const auto Col2X = Col1X + Col1W + 1;
	const auto DialogWidth = std::max(80, std::min(ScrX + 1, static_cast<int>(Col2X + MaxSize + SysArea + 1)));
	const auto Col2W = DialogWidth - Col2X - SysArea - 1;

	FarDialogItem EditDlgData[]
	{
		{DI_DOUBLEBOX,3, 1, DialogWidth-4,12, 0, nullptr, nullptr, 0, msg(lng::MExcTrappedException).c_str()},
		{DI_TEXT, Col1X, 2, Col1X + Col1W, 2, 0, nullptr, nullptr, 0, msg(lng::MExcException).c_str()},
		{DI_EDIT, Col2X, 2, Col2X + Col2W, 2, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, Exception.c_str()},
		{DI_TEXT, Col1X, 3, Col1X + Col1W, 3, 0, nullptr, nullptr, 0, msg(lng::MExcDetails).c_str()},
		{DI_EDIT, Col2X, 3, Col2X + Col2W, 3, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, Details.c_str()},
		{DI_TEXT, Col1X, 4, Col1X + Col1W, 4, 0, nullptr, nullptr, 0, L"LastError:"},
		{DI_EDIT, Col2X, 4, Col2X + Col2W, 4, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, Errors[0].c_str()},
		{DI_TEXT, Col1X, 5, Col1X + Col1W, 5, 0, nullptr, nullptr, 0, L"NTSTATUS:"},
		{DI_EDIT, Col2X, 5, Col2X + Col2W, 5, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, Errors[1].c_str()},
		{DI_TEXT, Col1X, 6, Col1X + Col1W, 6, 0, nullptr, nullptr, 0, msg(lng::MExcAddress).c_str()},
		{DI_EDIT, Col2X, 6, Col2X + Col2W, 6, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, Address.c_str()},
		{DI_TEXT, Col1X, 7, Col1X + Col1W, 7, 0, nullptr, nullptr, 0, msg(lng::MExcSource).c_str()},
		{DI_EDIT, Col2X, 7, Col2X + Col2W, 7, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, Source.c_str()},
		{DI_TEXT, Col1X, 8, Col1X + Col1W, 8, 0, nullptr, nullptr, 0, msg(lng::MExcFunction).c_str()},
		{DI_EDIT, Col2X, 8, Col2X + Col2W, 8, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, FunctionName.c_str()},
		{DI_TEXT, Col1X, 9, Col1X + Col1W, 9, 0, nullptr, nullptr, 0, msg(lng::MExcModule).c_str()},
		{DI_EDIT, Col2X, 9, Col2X + Col2W, 9, 0, nullptr, nullptr, DIF_READONLY | DIF_SELECTONENTRY, ModuleName.c_str()},
		{DI_TEXT,    -1,10,             0,10, 0, nullptr, nullptr, DIF_SEPARATOR, L""},
		{DI_BUTTON,   0,11,             0,11, 0, nullptr, nullptr, DIF_DEFAULTBUTTON | DIF_FOCUS | DIF_CENTERGROUP, msg(PluginModule? lng::MExcUnload : lng::MExcTerminate).c_str()},
		{DI_BUTTON,   0,11,             0,11, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MExcStack).c_str()},
		{DI_BUTTON,   0,11,             0,11, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MExcMinidump).c_str()},
		{DI_BUTTON,   0,11,             0,11, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MIgnore).c_str()},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	auto DlgData = dialog_data_type(&Context, NestedStack);
	const auto Dlg = Dialog::create(EditDlg, ExcDlgProc, &DlgData);
	Dlg->SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg->SetPosition({ -1, -1, DialogWidth, 14 });
	Dlg->Process();

	switch (Dlg->GetExitCode())
	{
	case ed_button_terminate:
		return reply_handle;
	case ed_button_minidump:
		return reply_minidump;
	default:
		return reply_ignore;
	}
}

static reply ExcConsole(
	string const& ModuleName,
	string const& Exception,
	string const& Details,
	detail::exception_context const& Context,
	string_view const Function,
	string_view const Location,
	Plugin const* const Module,
	error_state const* const ErrorState,
	std::vector<const void*> const* const NestedStack
)
{
	SCOPED_ACTION(auto)(tracer::with_symbols());

	string Address, Name, Source;
	tracer::get_symbol(Context.pointers()->ExceptionRecord->ExceptionAddress, Address, Name, Source);

	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	std::array<string_view, 8> Msg;
	if (far_language::instance().is_loaded())
	{
		Msg =
		{
			msg(lng::MExcException),
			msg(lng::MExcDetails),
			L"LastError:"sv,
			L"NTSTATUS:"sv,
			msg(lng::MExcAddress),
			msg(lng::MExcSource),
			msg(lng::MExcFunction),
			msg(lng::MExcModule),
		};
	}
	else
	{
		Msg =
		{
			L"Exception:"sv,
			L"Details:  "sv,
			L"LastError:"sv,
			L"NTSTATUS: "sv,
			L"Address:  "sv,
			L"Source:   "sv,
			L"Function: "sv,
			L"Module:   "sv,
		};
	}

	const auto ColumnWidth = std::max_element(ALL_CONST_RANGE(Msg), [](string_view const Str1, string_view const Str2){ return Str1.size() < Str2.size(); })->size();

	const auto Errors = FormatSystemErrors(ErrorState);

	const string_view Values[] =
	{
		Exception,
		Details,
		Errors[0],
		Errors[1],
		Address,
		Source,
		Function,
		ModuleName,
	};

	static_assert(std::size(Msg) == std::size(Values));

	for (const auto& [m, v] : zip(Msg, Values))
	{
		const auto Label = fit_to_left(string(m), ColumnWidth);
		std::wcerr << Label << L' ' << v << L'\n';
	}

	ShowStackTrace(tracer::get(*Context.pointers(), Context.thread_handle()), NestedStack);
	std::wcerr << std::endl;

	return ConsoleYesNo(L"Terminate process"sv)? reply_handle : reply_ignore;
}

template<size_t... I>
static constexpr uint32_t any_cc(char const* const Str, std::index_sequence<I...>)
{
	return (... | (Str[I] << 8 * I)) | (Str[sizeof...(I)] * 0);
}

static constexpr uint32_t fourcc(char const* const Str)
{
	return any_cc(Str, std::make_index_sequence<4>{});
}

enum FARRECORDTYPE
{
	RTYPE_PLUGIN = fourcc("CPLG"), // информация о текущем плагине
};

struct catchable_type
{
	DWORD properties;                // Catchable Type properties (Bit field)
	DWORD pType;                     // Image relative offset of TypeDescriptor
};

struct catchable_type_array
{
	DWORD nCatchableTypes;
	DWORD arrayOfCatchableTypes;     // Image relative offset of Catchable Types
};

struct throw_info
{
	DWORD attributes;                // Throw Info attributes (Bit field)
	DWORD pmfnUnwind;                // Destructor to call when exception has been handled or aborted
	DWORD pForwardCompat;            // Image relative offset of Forward compatibility frame handler
	DWORD pCatchableTypeArray;       // Image relative offset of CatchableTypeArray
};

static string ExtractObjectType(const EXCEPTION_RECORD* xr)
{
	const auto BaseAddress = xr->NumberParameters == 4? xr->ExceptionInformation[3] : 0;
	const auto& ThrowInfo = *reinterpret_cast<const throw_info*>(xr->ExceptionInformation[2]);
	const auto& CatchableTypeArray = *reinterpret_cast<const catchable_type_array*>(BaseAddress + ThrowInfo.pCatchableTypeArray);
	const auto& CatchableType = *reinterpret_cast<const catchable_type*>(BaseAddress + CatchableTypeArray.arrayOfCatchableTypes);
	const auto& TypeInfo = *reinterpret_cast<const std::type_info*>(BaseAddress + CatchableType.pType);

	return encoding::ansi::get_chars(TypeInfo.name());
}

static bool ProcessExternally(EXCEPTION_POINTERS* Pointers, Plugin const* const PluginModule)
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
		// DWORD SysID; GUID
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
		PlugRec.SizeRec = sizeof(PLUGINRECORD);
		PlugRec.ModuleName = PluginModule->ModuleName().c_str();
	}

	DWORD dummy;
	return Function(Pointers, PluginModule ? &PlugRec : nullptr, &LocalStartupInfo, &dummy) != FALSE;
}

static bool ProcessGenericException(
	detail::exception_context const& Context,
	string_view const Function,
	string_view const Location,
	Plugin const* const PluginModule,
	string_view const Message,
	error_state const* const ErrorState = nullptr,
	std::vector<const void*> const* const NestedStack = nullptr
)
{
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

		{L"C++ exception"sv, EXCEPTION_MICROSOFT_CPLUSPLUS},
	};

	string strFileName;

	const auto xr = Context.pointers()->ExceptionRecord;

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

	const auto Exception = [&](NTSTATUS Code)
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(KnownExceptions, i) { return i.second == Code; });
		const auto Name = ItemIterator != std::cend(KnownExceptions)? ItemIterator->first : L"Unknown exception"sv;
		return format(L"0x{0:0>8X} - {1}"sv, static_cast<DWORD>(Code), Name);
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
				case 0: return L"read"sv;
				case 1: return L"written"sv;
				case 8: return L"executed"sv;
				}
			}(xr->ExceptionInformation[0]);

			Details = format(L"Memory at {0} could not be {1}"sv, AccessedAddress, Mode);
		}
		break;

	case EXCEPTION_MICROSOFT_CPLUSPLUS:
		Details = xr->NumberParameters? ExtractObjectType(xr) : L""s;
		if (!Message.empty())
			append(Details, Details.empty()? L""sv : L", "sv, L"what(): "sv, Message);
		break;

	default:
		Details = os::GetErrorString(true, Context.code());
		break;
	}

	reply MsgCode;

	if (Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		MsgCode = ExcDialog(strFileName, Exception, Details, Context, Function, Location, PluginModule, ErrorState, NestedStack);
	}
	else
	{
		MsgCode = ExcConsole(strFileName, Exception, Details, Context, Function, Location, PluginModule, ErrorState, NestedStack);
	}

	switch (MsgCode)
	{
	case reply_handle: // terminate / unload
		if (!PluginModule && Global)
			Global->CriticalInternalError = true;
		return true;

	case reply_ignore:
	default:
		return false;
	}
}

void RestoreGPFaultUI()
{
	os::unset_error_mode(SEM_NOGPFAULTERRORBOX);
}

static string extract_nested_messages(const std::exception& Exception, bool Top = true)
{
	string Result;

	// far_exception.what() returns additional information (function, file and line).
	// We don't need it on top level because it's extracted separately
	if (const auto FarException = Top? dynamic_cast<const detail::far_base_exception*>(&Exception) : nullptr)
		Result = FarException->get_message();
	else
		Result = encoding::utf8::get_chars(Exception.what());
	
	try
	{
		std::rethrow_if_nested(Exception);
	}
	catch (const std::exception& e)
	{
		append(Result, L" -> "sv, extract_nested_messages(e, false));
	}
	catch (...)
	{
		append(Result, L" -> Unknown exception"sv);

		const auto Pointers = tracer::get_pointers();
		if (IsCppException(Pointers.ExceptionRecord))
			append(Result, L" ("sv, ExtractObjectType(Pointers.ExceptionRecord), L')');
	}

	return Result;
}

bool ProcessStdException(const std::exception& e, string_view const Function, const Plugin* const Module)
{
	if (const auto SehException = dynamic_cast<const seh_exception*>(&e))
	{
		return ProcessGenericException(SehException->context(), Function, {}, Module, {});
	}

	detail::exception_context const Context(EXCEPTION_MICROSOFT_CPLUSPLUS, tracer::get_pointers(), os::OpenCurrentThread(), GetCurrentThreadId());

	if (const auto FarException = dynamic_cast<const detail::far_base_exception*>(&e))
	{
		const auto Message = extract_nested_messages(e);

		const std::vector<const void*>* NestedStack = nullptr;

		if (const auto Wrapper = dynamic_cast<const far_wrapper_exception*>(&e))
		{
			const auto& Stack = Wrapper->get_stack();
			NestedStack = &Stack;
		}

		return ProcessGenericException(Context, FarException->get_function(), FarException->get_location(), Module, Message, &FarException->get_error_state(), NestedStack);
	}

	return ProcessGenericException(Context, Function, {}, Module, encoding::utf8::get_chars(e.what()));
}

bool ProcessUnknownException(string_view const Function, const Plugin* const Module)
{
	// C++ exception to EXCEPTION_POINTERS translation relies on Microsoft implementation.
	// It won't work in gcc etc.
	// Set ExceptionCode manually so ProcessGenericException will at least report it as C++ exception
	const detail::exception_context Context(EXCEPTION_MICROSOFT_CPLUSPLUS, tracer::get_pointers(), os::OpenCurrentThread(), GetCurrentThreadId());
	return ProcessGenericException(Context, Function, {}, Module, {});
}

static LONG WINAPI FarUnhandledExceptionFilter(EXCEPTION_POINTERS* const Pointers)
{
	detail::SetFloatingPointExceptions(false);
	const detail::exception_context Context(Pointers->ExceptionRecord->ExceptionCode, *Pointers, os::OpenCurrentThread(), GetCurrentThreadId());
	if (ProcessGenericException(Context, L"FarUnhandledExceptionFilter"sv, {}, {}, {}))
	{
		std::terminate();
	}
	RestoreGPFaultUI();
	return EXCEPTION_CONTINUE_SEARCH;
}

unhandled_exception_filter::unhandled_exception_filter()
{
	SetUnhandledExceptionFilter(FarUnhandledExceptionFilter);
}

unhandled_exception_filter::~unhandled_exception_filter()
{
	dismiss();
}

void unhandled_exception_filter::dismiss()
{
	SetUnhandledExceptionFilter(nullptr);
}

bool IsCppException(const EXCEPTION_RECORD* Record)
{
	return Record->ExceptionCode == static_cast<DWORD>(EXCEPTION_MICROSOFT_CPLUSPLUS);
}

namespace detail
{
	static thread_local bool StackOverflowHappened;

	int SehFilter(int const Code, const EXCEPTION_POINTERS* const Info, string_view const Function, const Plugin* const Module)
	{
		const exception_context Context(Code, *Info, os::OpenCurrentThread(), GetCurrentThreadId());
		if (Code == EXCEPTION_STACK_OVERFLOW)
		{
			bool Result = false;
			{
				os::thread(&os::thread::join, [&]{ Result = ProcessGenericException(Context, Function, {}, Module, {}); });
			}

			StackOverflowHappened = true;

			if (Result)
			{
				return EXCEPTION_EXECUTE_HANDLER;
			}
		}
		else
		{
			if (ProcessGenericException(Context, Function, {}, Module, {}))
				return EXCEPTION_EXECUTE_HANDLER;
		}

		unhandled_exception_filter::dismiss();
		RestoreGPFaultUI();
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void ResetStackOverflowIfNeeded()
	{
		if (StackOverflowHappened)
		{
			if (!_resetstkoflw())
				std::terminate();

			StackOverflowHappened = false;
		}
	}

	void SetFloatingPointExceptions(bool const Enable)
	{
		_clearfp();
		_controlfp(Enable? 0 : _MCW_EM, _MCW_EM);
	}

	std::exception_ptr MakeSehExceptionPtr(DWORD const Code, EXCEPTION_POINTERS* const Info, bool const ResumeThread)
	{
		return std::make_exception_ptr(seh_exception(Code, *Info, os::OpenCurrentThread(), GetCurrentThreadId(), ResumeThread));
	}
}

#undef EXCEPTION_MICROSOFT_CPLUSPLUS
