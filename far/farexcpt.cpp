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

#include "headers.hpp"
#pragma hdrstop

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

void CreatePluginStartupInfo(const Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF);

#define EXCEPTION_MICROSOFT_CPLUSPLUS ((NTSTATUS)0xE06D7363)

enum exception_dialog
{
	ed_doublebox,
	ed_text_exception,
	ed_edit_exception,
	ed_text_address,
	ed_edit_address,
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

static void ShowStackTrace(const std::vector<string>& Symbols)
{
	if (Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		Message(MSG_WARNING | MSG_LEFTALIGN,
			msg(lng::MExcTrappedException),
			Symbols,
			{ lng::MOk });
	}
	else
	{
		for (const auto& Str: Symbols)
		{
			std::wcerr << Str << L'\n';
		}
		std::wcerr.flush();
	}
}

static bool write_minidump(const exception_context& Context)
{
	if (!Imports().MiniDumpWriteDump)
		return false;

	// TODO: subdirectory && timestamp
	const os::fs::file DumpFile(Global->Opt->LocalProfilePath + L"\\Far.mdmp", GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS);
	if (!DumpFile)
		return false;

	MINIDUMP_EXCEPTION_INFORMATION Mei = { Context.GetThreadId(), Context.GetPointers() };
	return Imports().MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), DumpFile.handle().native_handle(), MiniDumpWithFullMemory, &Mei, nullptr, nullptr) != FALSE;
}

intptr_t ExcDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
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
			const auto& Context = *reinterpret_cast<exception_context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

			switch (Param1)
			{
			case ed_button_stack:
				ShowStackTrace(tracer::get(Context));
				return FALSE;

			case ed_button_minidump:
				write_minidump(Context);
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

static reply ExcDialog(const string& ModuleName, LPCWSTR Exception, const exception_context& Context, const wchar_t* Function, const Plugin* PluginModule)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	const auto strAddr = tracer::get_one(Context.GetPointers()->ExceptionRecord->ExceptionAddress);

	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,76,8,0,nullptr,nullptr,0,msg(lng::MExcTrappedException).data()},
		{DI_TEXT,     5,2, 17,2,0,nullptr,nullptr,0,msg(lng::MExcException).data()},
		{DI_EDIT,    18,2, 75,2,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,Exception},
		{DI_TEXT,     5,3, 17,3,0,nullptr,nullptr,0,msg(lng::MExcAddress).data()},
		{DI_EDIT,    18,3, 75,3,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,strAddr.data()},
		{DI_TEXT,     5,4, 17,4,0,nullptr,nullptr,0,msg(lng::MExcFunction).data()},
		{DI_EDIT,    18,4, 75,4,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,Function},
		{DI_TEXT,     5,5, 17,5,0,nullptr,nullptr,0,msg(lng::MExcModule).data()},
		{DI_EDIT,    18,5, 75,5,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,ModuleName.data()},
		{DI_TEXT,    -1,6, 0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_CENTERGROUP, msg(PluginModule? lng::MExcUnload : lng::MExcTerminate).data()},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MExcStack).data()},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MExcMinidump).data()},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MIgnore).data()},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	const auto Dlg = Dialog::create(EditDlg, ExcDlgProc, const_cast<exception_context*>(&Context));
	Dlg->SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg->SetPosition(-1, -1, 80, 10);
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

static reply ExcConsole(const string& ModuleName, LPCWSTR Exception, const exception_context& Context, const wchar_t* Function, const Plugin* Module)
{
	const auto strAddr = tracer::get_one(Context.GetPointers()->ExceptionRecord->ExceptionAddress);

	string Msg[4];
	if (far_language::instance().is_loaded())
	{
		Msg[0] = msg(lng::MExcException);
		Msg[1] = msg(lng::MExcAddress);
		Msg[2] = msg(lng::MExcFunction);
		Msg[3] = msg(lng::MExcModule);
	}
	else
	{
		Msg[0] = L"Exception:";
		Msg[1] = L"Address:  ";
		Msg[2] = L"Function: ";
		Msg[3] = L"Module:   ";
	}

	string Dump =
		Msg[0] + L' ' + Exception + L'\n' +
		Msg[1] + L' ' + strAddr + L'\n' +
		Msg[2] + L' ' + Function + L'\n' +
		Msg[3] + L' ' + ModuleName + L'\n';

	std::wcerr << Dump << std::endl;

	ShowStackTrace(tracer::get(Context));

	for (;;)
	{
		std::wcin.clear();
		std::wcout << L"\nTerminate process? [Y/N]: ";
		string Input;
		std::wcin >> Input;
		if (Input.size() == 1)
		{
			if (upper(Input.front()) == L'Y')
				return reply_handle;
			else if (upper(Input.front()) == L'N')
				return reply_ignore;
		}
	}
}

template<char c0, char c1, char c2, char c3>
struct MakeFourCC
{
	enum { value = MAKELONG(MAKEWORD(c0, c1), MAKEWORD(c2, c3)) };
};

enum FARRECORDTYPE
{
	RTYPE_PLUGIN = MakeFourCC<'C', 'P', 'L', 'G'>::value, // информация о текущем плагине
};

static string ExtractObjectName(const EXCEPTION_RECORD* xr)
{
	// https://blogs.msdn.microsoft.com/oldnewthing/20100730-00/?p=13273
	const auto BaseAddress = xr->NumberParameters == 4? xr->ExceptionInformation[3] : 0;
	const auto Ptr1 = reinterpret_cast<const DWORD*>(xr->ExceptionInformation[2]);
	const auto Ptr2 = reinterpret_cast<const DWORD*>(Ptr1[3] + BaseAddress);
	const auto Ptr3 = reinterpret_cast<const DWORD*>(Ptr2[1] + BaseAddress);
	const auto Ptr4 = reinterpret_cast<const char*>(Ptr3[1] + BaseAddress);
	auto DataPtr = Ptr4 + sizeof(void*) * 2 + 1;
	// TODO: try to undecorate?
	return encoding::ansi::get_chars(DataPtr);
}

static bool ProcessGenericException(const exception_context& Context, const wchar_t* Function, const Plugin* PluginModule, const char* Message)
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
				CreatePluginStartupInfo(nullptr, &LocalStartupInfo, &LocalStandardFunctions);
				LocalStartupInfo.ModuleName = Global->Opt->strExceptEventSvc.data();
				static PLUGINRECORD PlugRec;

				if (PluginModule)
				{
					PlugRec = {};
					PlugRec.TypeRec=RTYPE_PLUGIN;
					PlugRec.SizeRec=sizeof(PLUGINRECORD);
					PlugRec.ModuleName = PluginModule->GetModuleName().data();
				}

				DWORD dummy;
				Result = p(Context.GetPointers(), PluginModule? &PlugRec : nullptr, &LocalStartupInfo, &dummy) != FALSE;
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

	static const struct
	{
		const wchar_t* DefaultMsg; // Lng-files may not be loaded yet
		NTSTATUS Code;     // код исключения
		lng IdMsg;    // ID сообщения из LNG-файла
	}
	ECode[]=
	{
		#define TEXTANDCODE(x) L###x, x
		{TEXTANDCODE(EXCEPTION_ACCESS_VIOLATION), lng::MExcRAccess},
		{TEXTANDCODE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED), lng::MExcOutOfBounds},
		{TEXTANDCODE(EXCEPTION_INT_DIVIDE_BY_ZERO), lng::MExcDivideByZero},
		{TEXTANDCODE(EXCEPTION_STACK_OVERFLOW), lng::MExcStackOverflow},
		{TEXTANDCODE(EXCEPTION_BREAKPOINT), lng::MExcBreakPoint},
		{TEXTANDCODE(EXCEPTION_FLT_DIVIDE_BY_ZERO), lng::MExcFloatDivideByZero}, // BUGBUG: Floating-point exceptions (VC) are disabled by default. See http://msdn2.microsoft.com/en-us/library/aa289157(vs.71).aspx#floapoint_topic8
		{TEXTANDCODE(EXCEPTION_FLT_OVERFLOW), lng::MExcFloatOverflow},           // BUGBUG:  ^^^
		{TEXTANDCODE(EXCEPTION_FLT_STACK_CHECK), lng::MExcFloatStackOverflow},   // BUGBUG:  ^^^
		{TEXTANDCODE(EXCEPTION_FLT_UNDERFLOW), lng::MExcFloatUnderflow},         // BUGBUG:  ^^^
		{TEXTANDCODE(EXCEPTION_ILLEGAL_INSTRUCTION), lng::MExcBadInstruction},
		{TEXTANDCODE(EXCEPTION_PRIV_INSTRUCTION), lng::MExcBadInstruction},
		{TEXTANDCODE(EXCEPTION_DATATYPE_MISALIGNMENT), lng::MExcDatatypeMisalignment},
		{TEXTANDCODE(EXCEPTION_MICROSOFT_CPLUSPLUS), lng::MExcCplusPlus},
		// сюды добавляем.

		#undef TEXTANDCODE
	};

	string strBuf;
	string strFileName;
	auto ShowMessages = false;
	// получим запись исключения
	const auto xr = Context.GetPointers()->ExceptionRecord;

	if (!PluginModule)
	{
		if (Global)
		{
			strFileName=Global->g_strFarModuleName;
		}
		else
		{
			os::GetModuleFileName(nullptr, strFileName);
		}
	}
	else
	{
		strFileName = PluginModule->GetModuleName();
	}

	LPCWSTR Exception=nullptr;
	// просмотрим "знакомые" FAR`у исключения и обработаем...
	const auto ItemIterator = std::find_if(CONST_RANGE(ECode, i)
	{
		return i.Code == static_cast<NTSTATUS>(Context.GetCode());
	});

	if (ItemIterator != std::cend(ECode))
	{
		Exception = far_language::instance().is_loaded()? msg(ItemIterator->IdMsg).data() : ItemIterator->DefaultMsg;

		if (Context.GetCode() == static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION))
		{
			int Offset = 0;
			// вот только не надо здесь неочевидных оптимизаций вида
			// if ( xr->ExceptionInformation[0] == 8 ) Offset = 2 else Offset = xr->ExceptionInformation[0],
			// а то M$ порадует нас как-нибудь xr->ExceptionInformation[0] == 4 и все будет в полной жопе.

			switch (xr->ExceptionInformation[0])
			{
				case 0: // read
					Offset = 0;
					break;
				case 1: // write
					Offset = 1;
					break;
				case 8: // execute
					Offset = 2;
					break;
			}

			strBuf = L"0x" + to_hex_wstring(xr->ExceptionInformation[1]);

			if (far_language::instance().is_loaded())
			{
				strBuf = format(lng::MExcRAccess + Offset, strBuf);
				Exception=strBuf.data();
			}
			else
			{
				static const string_view AVs[] = {L"read from "_sv, L"write to "_sv, L"execute at "_sv };
				strBuf = concat(Exception, L" ("_sv, AVs[Offset], strBuf, L')');
				Exception=strBuf.data();
			}
		}
		else if (Context.GetCode() == static_cast<DWORD>(EXCEPTION_MICROSOFT_CPLUSPLUS))
		{
			strBuf = concat(Exception, L" ("_sv);
			const auto ObjectName = xr->NumberParameters? ExtractObjectName(xr) : L"";
			strBuf += ObjectName;
			if (Message)
			{
				if (!ObjectName.empty())
					append(strBuf, L" -> "_sv);

				append(strBuf, L"std::exception: "_sv, encoding::utf8::get_chars(Message));
			}
			strBuf += L')';
			Exception = strBuf.data();
		}
	}

	if (!Exception)
	{
		const auto Template = far_language::instance().is_loaded()? msg(lng::MExcUnknown) : L"Unknown exception";
		append(strBuf, Template, L" (0x"_sv, to_hex_wstring(Context.GetCode()), L')');
		Exception = strBuf.data();
	}

	reply MsgCode;

	if (Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		MsgCode = ExcDialog(strFileName, Exception, Context, Function, PluginModule);
		ShowMessages = true;
	}
	else
	{
		MsgCode = ExcConsole(strFileName, Exception, Context, Function, PluginModule);
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

void RestoreGPFaultUI()
{
	SetErrorMode(Global? Global->ErrorMode & ~SEM_NOGPFAULTERRORBOX : 0);
}

bool ProcessStdException(const std::exception& e, const wchar_t* Function, Plugin* Module)
{
	if (const auto SehException = dynamic_cast<const seh_exception*>(&e))
	{
		return ProcessGenericException(SehException->GetContext(), Function, Module, e.what());
	}

	std::unique_ptr<exception_context> Context;
	auto ContextPtr = tracer::get_exception_context(&e);
	if (!ContextPtr)
	{
		// std::exception to EXCEPTION_POINTERS translation relies on Microsoft C++ exception implementation.
		// It won't work in gcc etc.
		// Set ExceptionCode manually so ProcessGenericException will at least report it as std::exception and display what()
		Context = std::make_unique<exception_context>(EXCEPTION_MICROSOFT_CPLUSPLUS);
		ContextPtr = Context.get();
	}

	return ProcessGenericException(*ContextPtr, Function, Module, e.what());
}

bool ProcessUnknownException(const wchar_t* Function, Plugin* Module)
{
	exception_context Context;
	return ProcessGenericException(Context, Function, Module, nullptr);
}

static LONG WINAPI FarUnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
	exception_context Context(ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo);
	if (ProcessGenericException(Context, L"FarUnhandledExceptionFilter", nullptr, nullptr))
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

static void Test_EXCEPTION_STACK_OVERFLOW(char* target)
{
	char Buffer[2048]; /* чтобы быстрее рвануло */

	Test_EXCEPTION_STACK_OVERFLOW(Buffer);

	// "side effect" to prevent deletion of this function call due to C4718.
	Sleep(0);
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
			cpp_unknown,
			access_violation_read,
			access_violation_write,
			access_violation_execute,
			divide_by_zero,
			illegal_instruction,
			stack_overflow,
			fp_divide_by_zero,
			breakpoint,
#ifdef _M_IA64
			alignment_fault,
#endif

			count
		};

		static const wchar_t* Names[] =
		{
			L"C++ std::exception",
			L"C++ unknown exception",
			L"Access Violation (Read)",
			L"Access Violation (Write)",
			L"Access Violation (Execute)",
			L"Divide by zero",
			L"Illegal instruction",
			L"Stack Overflow",
			L"Floating-point divide by zero",
			L"Breakpoint",
#ifdef _M_IA64
			L"Alignment fault (IA64 specific)",
#endif
			/*
			L"EXCEPTION_FLT_OVERFLOW",
			L"EXCEPTION_SINGLE_STEP",
			L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED",
			L"EXCEPTION_FLT_DENORMAL_OPERAND",
			L"EXCEPTION_FLT_INEXACT_RESULT",
			L"EXCEPTION_FLT_INVALID_OPERATION",
			L"EXCEPTION_FLT_STACK_CHECK",
			L"EXCEPTION_FLT_UNDERFLOW",
			L"EXCEPTION_INT_OVERFLOW",
			L"EXCEPTION_PRIV_INSTRUCTION",
			L"EXCEPTION_IN_PAGE_ERROR",
			L"EXCEPTION_NONCONTINUABLE_EXCEPTION",
			L"EXCEPTION_INVALID_DISPOSITION",
			L"EXCEPTION_GUARD_PAGE",
			L"EXCEPTION_INVALID_HANDLE",
			*/
		};

		static_assert(std::size(Names) == static_cast<size_t>(exception_types::count));

		static union
		{
			int     i;
			int     *iptr;
			double  d;
		} zero_const; //, refers;
		zero_const.i = 0L;
		const auto ModalMenu = VMenu2::create(L"Test Exceptions", nullptr, 0, ScrY - 4);
		ModalMenu->SetMenuFlags(VMENU_WRAPMODE);
		ModalMenu->SetPosition(-1, -1, 0, 0);

		std::for_each(CONST_RANGE(Names, i)
		{
			ModalMenu->AddItem(i);
		});

		int ExitCode = ModalMenu->Run();
		if (ExitCode == -1)
			return true;

		switch (static_cast<exception_types>(ExitCode))
		{
		case exception_types::cpp_std:
			throw MAKE_FAR_EXCEPTION(L"Test error");

		case exception_types::cpp_unknown:
			throw 42;

		case exception_types::access_violation_read:
			zero_const.i = *zero_const.iptr;
			break;

		case exception_types::access_violation_write:
			*zero_const.iptr = 0;
			break;

		case exception_types::access_violation_execute:
			using func_t = void(*)();
			func_t{}();
			break;

		case exception_types::divide_by_zero:
			zero_const.i = 1 / zero_const.i;
			break;

		case exception_types::illegal_instruction:
#if COMPILER == C_CL || COMPILER == C_INTEL
#ifdef _M_IA64
			const int REG_IA64_IntR0 = 1024;
			__setReg(REG_IA64_IntR0, 666);
#else
			__ud2();
#endif
#elif COMPILER == C_GCC || COMPILER == C_CLANG
			asm("ud2");
#endif
			break;

		case exception_types::stack_overflow:
			Test_EXCEPTION_STACK_OVERFLOW(nullptr);
			break;

		case exception_types::fp_divide_by_zero:
			zero_const.d = 1.0 / zero_const.d;
			break;

		case exception_types::breakpoint:
			DebugBreak();
			break;

#ifdef _M_IA64
		case exception_types::alignment_fault:
			{
				BYTE temp[10] = {};
				double* val;
				val = (double*)(&temp[3]);
				printf("%lf\n", *val);
				break;
			}
#endif
		case exception_types::count:
			// makes no sense, just to make compiler happy
			break;
		}

		Message(MSG_WARNING,
			L"Test Exceptions failed",
			{
				L"",
				Names[ExitCode],
				L"",
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

bool IsCppException(const EXCEPTION_POINTERS* e)
{
	return e->ExceptionRecord->ExceptionCode == static_cast<DWORD>(EXCEPTION_MICROSOFT_CPLUSPLUS);
}

thread_local bool StackOverflowHappened;

void ResetStackOverflowIfNeeded()
{
	if (StackOverflowHappened)
	{
		if (!_resetstkoflw())
		{
			std::terminate();
		}
	}
}

int SehFilter(int Code, EXCEPTION_POINTERS* Info, const wchar_t* Function, Plugin* Module)
{
	exception_context Context(Code, Info);
	if (Code == EXCEPTION_STACK_OVERFLOW)
	{
		bool Result = false;
		{
			os::thread(&os::thread::join, [&]{ Result = ProcessGenericException(Context, nullptr, nullptr, nullptr); });
		}

		StackOverflowHappened = true;

		if (Result)
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}
	else
	{
		if (ProcessGenericException(Context, Function, Module, nullptr))
			return EXCEPTION_EXECUTE_HANDLER;
	}

	unhandled_exception_filter::dismiss();
	RestoreGPFaultUI();
	return EXCEPTION_CONTINUE_SEARCH;
}
