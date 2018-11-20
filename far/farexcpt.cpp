/*
farexcpt.cpp

Все про исключения
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "farexcpt.hpp"

#include "plugins.hpp"
#include "filepanels.hpp"
#include "manager.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "farcolor.hpp"
#include "colormix.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "message.hpp"
#include "imports.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "interf.hpp"
#include "strmix.hpp"
#include "tracer.hpp"
#include "string_utils.hpp"
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

static void ShowStackTrace(std::vector<string>&& Symbols, const std::vector<string>* NestedStack)
{
	if (Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		if (NestedStack && !NestedStack->empty())
		{
			Symbols.reserve(Symbols.size() + 3 + NestedStack->size());
			Symbols.emplace_back(40, L'-');
			Symbols.emplace_back(L"Nested stack:"s);
			Symbols.emplace_back(40, L'-');
			Symbols.insert(Symbols.end(), ALL_CONST_RANGE(*NestedStack));
		}

		Message(MSG_WARNING | MSG_LEFTALIGN,
			msg(lng::MExcTrappedException),
			std::move(Symbols),
			{ lng::MOk });
	}
	else
	{
		for (const auto& Str: Symbols)
		{
			std::wcerr << Str << L'\n';
		}
	}
}

static bool write_minidump(const exception_context& Context)
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

using dialog_data_type = std::pair<const exception_context*, const std::vector<string>*>;

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
				ShowStackTrace(tracer::get(*DialogData.first), DialogData.second);
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
	exception_context const& Context,
	string_view const Function,
	string_view const Location,
	Plugin const* const PluginModule,
	std::vector<string> const* const NestedStack
)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	string Address, Name, Source;
	SCOPED_ACTION(auto)(tracer::with_symbols());
	tracer::get_one(Context.pointers()->ExceptionRecord->ExceptionAddress, Address, Name, Source);
	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	const null_terminated FunctionName(Function);

	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,76,10,0,nullptr,nullptr,0,msg(lng::MExcTrappedException).c_str()},
		{DI_TEXT,     5,2, 17,2,0,nullptr,nullptr,0,msg(lng::MExcException).c_str()},
		{DI_EDIT,    18,2, 75,2,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,Exception.c_str()},
		{DI_TEXT,     5,3, 17,3,0,nullptr,nullptr,0,msg(lng::MExcDetails).c_str()},
		{DI_EDIT,    18,3, 75,3,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,Details.c_str()},
		{DI_TEXT,     5,4, 17,4,0,nullptr,nullptr,0,msg(lng::MExcAddress).c_str()},
		{DI_EDIT,    18,4, 75,4,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,Address.c_str()},
		{DI_TEXT,     5,5, 17,5,0,nullptr,nullptr,0,msg(lng::MExcSource).c_str()},
		{DI_EDIT,    18,5, 75,5,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,Source.c_str()},
		{DI_TEXT,     5,6, 17,6,0,nullptr,nullptr,0,msg(lng::MExcFunction).c_str()},
		{DI_EDIT,    18,6, 75,6,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY, FunctionName.c_str()},
		{DI_TEXT,     5,7, 17,7,0,nullptr,nullptr,0,msg(lng::MExcModule).c_str()},
		{DI_EDIT,    18,7, 75,7,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,ModuleName.c_str()},
		{DI_TEXT,    -1,8,  0,8,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,9,  0,9,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_CENTERGROUP, msg(PluginModule? lng::MExcUnload : lng::MExcTerminate).c_str()},
		{DI_BUTTON,   0,9,  0,9,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MExcStack).c_str()},
		{DI_BUTTON,   0,9,  0,9,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MExcMinidump).c_str()},
		{DI_BUTTON,   0,9,  0,9,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MIgnore).c_str()},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	auto DlgData = dialog_data_type(&Context, NestedStack);
	const auto Dlg = Dialog::create(EditDlg, ExcDlgProc, &DlgData);
	Dlg->SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg->SetPosition({ -1, -1, 80, 12 });
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
	exception_context const& Context,
	string_view const Function,
	string_view const Location,
	Plugin const* const Module,
	std::vector<string> const* const NestedStack
)
{
	string Address, Name, Source;
	tracer::get_one(Context.pointers()->ExceptionRecord->ExceptionAddress, Address, Name, Source);
	if (!Name.empty())
		append(Address, L" - "sv, Name);

	if (Source.empty())
		Source = Location;

	std::array<string_view, 6> Msg;
	if (far_language::instance().is_loaded())
	{
		Msg =
		{
			msg(lng::MExcException),
			msg(lng::MExcDetails),
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
			L"Address:  "sv,
			L"Source:   "sv,
			L"Function: "sv,
			L"Module:   "sv,
		};
	}

	const string_view Values[] =
	{
		Exception,
		Details,
		Address,
		Source,
		Function,
		ModuleName,
	};

	static_assert(std::size(Msg) == std::size(Values));

	for (const auto& i : zip(Msg, Values))
	{
		std::wcerr << std::get<0>(i) << L' ' << std::get<1>(i) << L'\n';
	}

	ShowStackTrace(tracer::get(Context), NestedStack);

	std::wcerr << std::endl;

	for (;;)
	{
		std::wcout << L"Terminate process (Y/N)? "sv << std::flush;

		wchar_t Input;
		std::wcin.get(Input).ignore(std::numeric_limits<std::streamsize>::max(), L'\n');

		switch (upper(Input))
		{
		case L'Y':
			return reply_handle;

		case L'N':
			return reply_ignore;

		default:
			break;
		}
	}
}

template<char c0, char c1, char c2, char c3>
constexpr uint32_t fourcc = MAKELONG(MAKEWORD(c0, c1), MAKEWORD(c2, c3));

enum FARRECORDTYPE
{
	RTYPE_PLUGIN = fourcc<'C', 'P', 'L', 'G'>, // информация о текущем плагине
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

static bool ProcessGenericException(
	exception_context const& Context,
	string_view const Function,
	string_view const Location,
	Plugin const* const PluginModule,
	string_view const Message,
	std::vector<string> const* const NestedStack = nullptr
)
{
	if (Global)
		Global->ProcessException = true;

	auto Result = false;

	if (Global && Global->Opt->ExceptUsed && !Global->Opt->strExceptEventSvc.empty())
	{
		os::rtdl::module m(Global->Opt->strExceptEventSvc);

		if (m)
		{
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

			os::rtdl::function_pointer<BOOL(WINAPI*)(EXCEPTION_POINTERS*, const PLUGINRECORD*, const PluginStartupInfo*, DWORD*)> p(m, "ExceptionProc");

			if (p)
			{
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
					PlugRec.TypeRec=RTYPE_PLUGIN;
					PlugRec.SizeRec=sizeof(PLUGINRECORD);
					PlugRec.ModuleName = PluginModule->ModuleName().c_str();
				}

				DWORD dummy;
				Result = p(Context.pointers(), PluginModule? &PlugRec : nullptr, &LocalStartupInfo, &dummy) != FALSE;
			}
		}
	}

	if (Result)
	{
		if (!PluginModule)
		{
			if (Global)
				Global->CriticalInternalError = true;
		}

		return true;
	}

	static const std::pair<string_view, NTSTATUS> KnownExceptions[] =
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
	auto ShowMessages = false;

	const auto xr = Context.pointers()->ExceptionRecord;

	if (!PluginModule)
	{
		if (Global)
		{
			strFileName=Global->g_strFarModuleName;
		}
		else
		{
			os::fs::GetModuleFileName(nullptr, nullptr, strFileName);
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
		MsgCode = ExcDialog(strFileName, Exception, Details, Context, Function, Location, PluginModule, NestedStack);
		ShowMessages = true;
	}
	else
	{
		MsgCode = ExcConsole(strFileName, Exception, Details, Context, Function, Location, PluginModule, NestedStack);
	}

	if (ShowMessages && !PluginModule)
	{
		Global->CriticalInternalError = true;
	}

	switch (MsgCode)
	{
	case reply_handle: // terminate / unload
		return true;

	case reply_ignore:
	default:
		return false;
	}
}

void SetFloatingPointExceptions(bool Enable)
{
	_clearfp();
	_controlfp(Enable? 0 : _MCW_EM, _MCW_EM);
}

void RestoreGPFaultUI()
{
	os::unset_error_mode(SEM_NOGPFAULTERRORBOX);
}

extern "C" void** __current_exception();
extern "C" void** __current_exception_context();

static string extract_nested_messages(const std::exception& Exception, bool Top = true)
{
	string Result;

	// far_exception.what() returns additional information (function, file and line).
	// We don't need it on top level because it's extracted separately
	if (const auto FarException = Top? dynamic_cast<const far_base_exception*>(&Exception) : nullptr)
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

#ifdef _MSC_VER
		const auto Record = static_cast<EXCEPTION_RECORD*>(*__current_exception());
		if (IsCppException(Record))
			append(Result, L" ("sv, ExtractObjectType(Record), L')');
#endif
	}

	return Result;
}

bool ProcessStdException(const std::exception& e, string_view const Function, const Plugin* const Module)
{
	if (const auto SehException = dynamic_cast<const seh_exception*>(&e))
	{
		return ProcessGenericException(SehException->context(), Function, {}, Module, {});
	}

	auto Context = tracer::get_exception_context(&e);
	if (!Context)
	{
		// C++ exception to EXCEPTION_POINTERS translation relies on Microsoft implementation.
		// It won't work in gcc etc.
		// Set ExceptionCode manually so ProcessGenericException will at least report it as std::exception and display what()
		Context = std::make_unique<exception_context>(EXCEPTION_MICROSOFT_CPLUSPLUS);
	}

	if (const auto FarException = dynamic_cast<const far_base_exception*>(&e))
	{
		const auto Message = extract_nested_messages(e);
		return ProcessGenericException(*Context, FarException->get_function(), FarException->get_location(), Module, Message, &FarException->get_stack());
	}

	return ProcessGenericException(*Context, Function, {}, Module, encoding::utf8::get_chars(e.what()));
}

bool ProcessUnknownException(string_view const Function, const Plugin* const Module)
{
	const EXCEPTION_POINTERS* PointersPtr = nullptr;

#ifdef _MSC_VER
	EXCEPTION_POINTERS Pointers
	{
		static_cast<EXCEPTION_RECORD*>(*__current_exception()),
		static_cast<CONTEXT*>(*__current_exception_context())
	};
	PointersPtr = &Pointers;
#endif

	// C++ exception to EXCEPTION_POINTERS translation relies on Microsoft implementation.
	// It won't work in gcc etc.
	// Set ExceptionCode manually so ProcessGenericException will at least report it as C++ exception
	const exception_context Context(EXCEPTION_MICROSOFT_CPLUSPLUS, PointersPtr);
	return ProcessGenericException(Context, Function, {}, Module, {});
}

static LONG WINAPI FarUnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
	SetFloatingPointExceptions(false);
	const exception_context Context(ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo);
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

#if defined(FAR_ALPHA_VERSION)
WARNING_PUSH()

WARNING_DISABLE_MSC(4717) // https://msdn.microsoft.com/en-us/library/97c54274.aspx 'function' : recursive on all control paths, function will cause runtime stack overflow
WARNING_DISABLE_CLANG("-Winfinite-recursion")

static void Test_EXCEPTION_STACK_OVERFLOW(volatile char* target)
{
	volatile char Buffer[10240];
	// "side effect" to prevent deletion of this function call due to C4718.
	*Buffer = 0;
	Test_EXCEPTION_STACK_OVERFLOW(Buffer);
}
WARNING_POP()

static bool ExceptionTestHook(Manager::Key key)
{
	// сей код для проверки исключатор, просьба не трогать :-)
	if (
		key() == KEY_CTRLALTAPPS ||
		key() == KEY_RCTRLRALTAPPS ||
		key() == KEY_CTRLRALTAPPS ||
		key() == KEY_RCTRLALTAPPS
		)
	{
		enum class exception_types
		{
			cpp_std,
			cpp_std_bad_alloc,
			cpp_unknown,
			access_violation_read,
			access_violation_write,
			access_violation_execute,
			divide_by_zero,
			illegal_instruction,
			stack_overflow,
			fp_divide_by_zero,
			fp_overflow,
			fp_underflow,
			fp_inexact_result,
			breakpoint,
			alignment_fault,

			count
		};

		static const string_view Names[]
		{
			L"C++ std::exception"sv,
			L"C++ std::bad_alloc"sv,
			L"C++ unknown exception"sv,
			L"Access Violation (Read)"sv,
			L"Access Violation (Write)"sv,
			L"Access Violation (Execute)"sv,
			L"Divide by zero"sv,
			L"Illegal instruction"sv,
			L"Stack Overflow"sv,
			L"Floating-point divide by zero"sv,
			L"Floating-point overflow"sv,
			L"Floating-point underflow"sv,
			L"Floating-point inexact result"sv,
			L"Breakpoint"sv,
			L"Alignment fault"sv,

			/*
			L"EXCEPTION_SINGLE_STEP"sv,
			L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED"sv,
			L"EXCEPTION_FLT_DENORMAL_OPERAND"sv,
			L"EXCEPTION_FLT_INVALID_OPERATION"sv,
			L"EXCEPTION_FLT_STACK_CHECK"sv,
			L"EXCEPTION_INT_OVERFLOW"sv,
			L"EXCEPTION_PRIV_INSTRUCTION"sv,
			L"EXCEPTION_IN_PAGE_ERROR"sv,
			L"EXCEPTION_NONCONTINUABLE_EXCEPTION"sv,
			L"EXCEPTION_INVALID_DISPOSITION"sv,
			L"EXCEPTION_GUARD_PAGE"sv,
			L"EXCEPTION_INVALID_HANDLE"sv,
			*/
		};

		static_assert(std::size(Names) == static_cast<size_t>(exception_types::count));

		const auto ModalMenu = VMenu2::create(L"Test Exceptions"s, {}, ScrY - 4);
		ModalMenu->SetMenuFlags(VMENU_WRAPMODE);
		ModalMenu->SetPosition({ -1, -1, 0, 0 });

		std::for_each(CONST_RANGE(Names, i)
		{
			ModalMenu->AddItem(string(i));
		});

		const auto ExitCode = ModalMenu->Run();
		if (ExitCode == -1)
			return true;

		switch (static_cast<exception_types>(ExitCode))
		{
		case exception_types::cpp_std:
			throw MAKE_FAR_EXCEPTION(L"Test error"sv);

		case exception_types::cpp_std_bad_alloc:
			{
				// Less than the physical limit to leave some space for a service block, if any
				const auto Ptr = std::make_unique<char[]>(std::numeric_limits<size_t>::max() - 1024 * 1024);
			}
			break;

		case exception_types::cpp_unknown:
			throw 42u;

		case exception_types::access_violation_read:
			{
				volatile const int* InvalidAddress = nullptr;
				volatile const auto Result = *InvalidAddress;
				(void)Result;
			}
			break;

		case exception_types::access_violation_write:
			{
				volatile int* InvalidAddress = nullptr;
				*InvalidAddress = 42;
			}
			break;

		case exception_types::access_violation_execute:
			{
				using func_t = void(*)();
				volatile const func_t InvalidAddress = nullptr;
				InvalidAddress();
			}
			break;

		case exception_types::divide_by_zero:
			{
				volatile const auto InvalidDenominator = 0;
				volatile const auto Result = 42 / InvalidDenominator;
				(void)Result;
			}
			break;

		case exception_types::illegal_instruction:
			{
#if COMPILER == C_GCC || COMPILER == C_CLANG
				const auto& __ud2 = []{ asm("ud2"); };
#endif
				__ud2();
			}
			break;

		case exception_types::stack_overflow:
			Test_EXCEPTION_STACK_OVERFLOW(nullptr);
			break;

		case exception_types::fp_divide_by_zero:
			{
				SetFloatingPointExceptions(true);
				volatile const auto InvalidDenominator = 0.0;
				volatile const auto Result = 42.0 / InvalidDenominator;
				(void)Result;
			}
			break;

		case exception_types::fp_overflow:
			{
				SetFloatingPointExceptions(true);
				volatile const auto Max = std::numeric_limits<double>::max();
				volatile const auto Result = Max * 2;
				(void)Result;
			}
			break;

		case exception_types::fp_underflow:
			{
				SetFloatingPointExceptions(true);
				volatile const auto Min = std::numeric_limits<double>::min();
				volatile const auto Result = Min / 2;
				(void)Result;
			}
			break;

		case exception_types::fp_inexact_result:
			{
				SetFloatingPointExceptions(true);
				volatile const auto Max = std::numeric_limits<double>::max();
				volatile const auto Result = Max + 1;
				(void)Result;
			}
			break;

		case exception_types::breakpoint:
			DebugBreak();
			break;

		case exception_types::alignment_fault:
			{
				volatile const struct data
				{
					char Data[3 + sizeof(double)];
				}
				Data{};
				volatile const auto Result = *reinterpret_cast<volatile const double*>(Data.Data + 3);
				(void)Result;
			}
			break;

		case exception_types::count:
			// makes no sense, just to make compiler happy
			break;
		}

		Message(MSG_WARNING,
			L"Test Exceptions failed"sv,
			{
				string(Names[ExitCode]),
			},
			{ lng::MOk });
		return true;
	}
	return false;
}
#endif

void RegisterTestExceptionsHook()
{
#ifdef FAR_ALPHA_VERSION
	Global->WindowManager->AddGlobalKeyHandler(ExceptionTestHook);
#endif
}

bool IsCppException(const EXCEPTION_RECORD* Record)
{
	return Record->ExceptionCode == static_cast<DWORD>(EXCEPTION_MICROSOFT_CPLUSPLUS);
}

static thread_local bool StackOverflowHappened;

void ResetStackOverflowIfNeeded()
{
	if (StackOverflowHappened)
	{
		if (!_resetstkoflw())
			std::terminate();

		StackOverflowHappened = false;
	}
}

int SehFilter(int const Code, const EXCEPTION_POINTERS* Info, string_view const Function, const Plugin* Module)
{
	const exception_context Context(Code, Info);
	if (Code == EXCEPTION_STACK_OVERFLOW)
	{
		bool Result = false;
		{
			os::thread(&os::thread::join, [&] { Result = ProcessGenericException(Context, Function, {}, Module, {}); });
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

#undef EXCEPTION_MICROSOFT_CPLUSPLUS
