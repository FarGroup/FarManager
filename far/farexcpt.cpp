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
#include "colors.hpp"
#include "colormix.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "language.hpp"
#include "message.hpp"
#include "imports.hpp"
#include "vmenu2.hpp"
#include "interf.hpp"
#include "strmix.hpp"

#ifdef _MSC_VER
# define WITH_MINIDUMP
# ifdef WITH_MINIDUMP
WARNING_PUSH()
WARNING_DISABLE_MSC(4091) // https://msdn.microsoft.com/en-us/library/eehkcz60.aspx 'typedef ': ignored on left of 'type' when no variable is declared
#  include <dbghelp.h>
WARNING_POP()
# endif
#endif

static const wchar_t* gFrom = nullptr;
static Plugin *PluginModule = nullptr;     // модуль, приведший к исключению.

void CreatePluginStartupInfo(const Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF);

#ifdef WITH_MINIDUMP
# define LAST_BUTTON 13
#else
# define LAST_BUTTON 12
#endif

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
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				if (Param1==10 && (key==KEY_LEFT || key == KEY_NUMPAD4 || key==KEY_SHIFTTAB))
				{
					Dlg->SendMessage(DM_SETFOCUS, LAST_BUTTON, nullptr);
					return TRUE;
				}
				else if (Param1==LAST_BUTTON && (key==KEY_RIGHT || key == KEY_NUMPAD6 || key==KEY_TAB))
				{
					Dlg->SendMessage(DM_SETFOCUS, 10, nullptr);
					return TRUE;
				}
			}
		}
		break;

		case DN_CLOSE:
		{
			if (Param1 == 11) // debug
			{
				// It's better to attach debugger ASAP, closing the dialog causes too much work in window manager
				attach_debugger();
				return FALSE;
			}
		}
		break;

	default:
		break;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

static bool LanguageLoaded()
{
	return Global && Global->Lang;
}

enum reply
{
	reply_handle,
	reply_debug,
	reply_ignore,
#ifdef WITH_MINIDUMP
	reply_minidump,
#endif
};

static reply ExcDialog(const string& ModuleName, LPCWSTR Exception, LPVOID Adress)
{
	// TODO: Far Dialog is not the best choice for exception reporting
	// replace with something trivial

	const auto strAddr = L"0x" + to_hex_wstring(reinterpret_cast<uintptr_t>(Adress));

	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,8,0,nullptr,nullptr,0,MSG(MExcTrappedException)},
		{DI_TEXT,     5,2, 17,2,0,nullptr,nullptr,0,MSG(MExcException)},
		{DI_TEXT,    18,2, 70,2,0,nullptr,nullptr,0,Exception},
		{DI_TEXT,     5,3, 17,3,0,nullptr,nullptr,0,MSG(MExcAddress)},
		{DI_TEXT,    18,3, 70,3,0,nullptr,nullptr,0,strAddr.data()},
		{DI_TEXT,     5,4, 17,4,0,nullptr,nullptr,0,MSG(MExcFunction)},
		{DI_TEXT,    18,4, 70,4,0,nullptr,nullptr,0,gFrom},
		{DI_TEXT,     5,5, 17,5,0,nullptr,nullptr,0,MSG(MExcModule)},
		{DI_EDIT,    18,5, 70,5,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,ModuleName.data()},
		{DI_TEXT,    -1,6, 0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_CENTERGROUP, MSG(PluginModule? MExcUnload : MExcTerminate)},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MExcDebugger)},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MIgnore)},
#ifdef WITH_MINIDUMP
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP, L"MiniDump"},
#endif
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	const auto Dlg = Dialog::create(EditDlg, ExcDlgProc);
	Dlg->SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg->SetPosition(-1,-1,76,10);
	Dlg->Process();

	switch (Dlg->GetExitCode())
	{
	case 10:
		return reply_handle;
	case 11:
		return reply_debug;
#ifdef WITH_MINIDUMP
	case 13:
		return reply_minidump;
#endif
	default: //case 12:
		return reply_ignore;
	}
}

static void ExcDump(const string& ModuleName,LPCWSTR Exception,LPVOID Adress)
{
	const auto strAddr = L"0x" + to_hex_wstring(reinterpret_cast<uintptr_t>(Adress));

	string Msg[4];
	if (LanguageLoaded())
	{
		Msg[0] = MSG(MExcException);
		Msg[1] = MSG(MExcAddress);
		Msg[2] = MSG(MExcFunction);
		Msg[3] = MSG(MExcModule);
	}
	else
	{
		Msg[0] = L"Exception:";
		Msg[1] = L"Address:  ";
		Msg[2] = L"Function: ";
		Msg[3] = L"Module:   ";
	}

	string Dump =
		Msg[0] + L" " + Exception + L"\n" +
		Msg[1] + L" " + strAddr + L"\n" +
		Msg[2] + L" " + gFrom + L"\n" +
		Msg[3] + L" " + ModuleName + L"\n";

	std::wcerr << Dump << std::endl;
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

#ifdef WITH_MINIDUMP
static bool write_minidump(EXCEPTION_POINTERS *ex_pointers)
{
#if 0
	if (::IsDebuggerPresent())
		return false;
#endif
	bool written = false;
	auto hm = ::LoadLibraryA("dbghelp.dll");
	if (hm) {
		typedef BOOL(WINAPI *MINIDUMP_WRITE_DUMP)(
			IN HANDLE         hProcess,
			IN DWORD          ProcessId,
			IN HANDLE         hFile,
			IN MINIDUMP_TYPE  DumpType,
			IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
			OPTIONAL IN PVOID UserStreamParam,
			OPTIONAL IN PVOID CallbackParam
		);
		union {
			void *pv;
			MINIDUMP_WRITE_DUMP pf;
		} u_MiniDumpWriteDump;

		u_MiniDumpWriteDump.pv = ::GetProcAddress(hm, "MiniDumpWriteDump");
		if (u_MiniDumpWriteDump.pv) {
			string dump_file = os::env::get_variable(L"FARPROFILE") + L"\\Far.mdmp";
			auto hDump_File = ::CreateFile(dump_file.data(),
				GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
			);
			if (INVALID_HANDLE_VALUE != hDump_File) {
				MINIDUMP_EXCEPTION_INFORMATION m;
				m.ThreadId = ::GetCurrentThreadId();
				m.ExceptionPointers = ex_pointers;
				m.ClientPointers = FALSE;

				written = FALSE != (*u_MiniDumpWriteDump.pf) (
					::GetCurrentProcess(), ::GetCurrentProcessId(), hDump_File, MiniDumpWithFullMemory,
					ex_pointers ? &m : nullptr, nullptr, nullptr
				);
				::CloseHandle(hDump_File);
			}
		}
		::FreeLibrary(hm);
	}
	return written;
}
#endif

static bool ProcessSEHExceptionImpl(EXCEPTION_POINTERS *xp)
{
	if (Global)
		Global->ProcessException=TRUE;
	BOOL Res=FALSE;

	if (Global && Global->Opt->ExceptUsed && !Global->Opt->strExceptEventSvc.empty())
	{
		os::rtdl::module m(Global->Opt->strExceptEventSvc.data());

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

			os::rtdl::function_pointer<BOOL(WINAPI*)(EXCEPTION_POINTERS* xp, const PLUGINRECORD* Module, const PluginStartupInfo* LocalStartupInfo, LPDWORD Result)> p(m, "ExceptionProc");

			if (p)
			{
				static PluginStartupInfo LocalStartupInfo;
				ClearStruct(LocalStartupInfo);
				static FarStandardFunctions LocalStandardFunctions;
				ClearStruct(LocalStandardFunctions);
				CreatePluginStartupInfo(nullptr, &LocalStartupInfo, &LocalStandardFunctions);
				LocalStartupInfo.ModuleName = Global->Opt->strExceptEventSvc.data();
				static PLUGINRECORD PlugRec;

				if (PluginModule)
				{
					ClearStruct(PlugRec);
					PlugRec.TypeRec=RTYPE_PLUGIN;
					PlugRec.SizeRec=sizeof(PLUGINRECORD);
					PlugRec.ModuleName = PluginModule->GetModuleName().data();
				}

				DWORD dummy;
				Res = p(xp, (PluginModule? &PlugRec : nullptr), &LocalStartupInfo, &dummy);
			}
		}
	}

	if (Res)
	{
		if (!PluginModule)
		{
			if (Global)
				Global->CriticalInternalError=TRUE;
		}

		return true;
	}

	static const struct
	{
		NTSTATUS Code;     // код исключения
		const wchar_t* DefaultMsg; // Lng-files may not be loaded yet
		LNGID IdMsg;    // ID сообщения из LNG-файла
	}
	ECode[]=
	{
		#define CODEANDTEXT(x) x, L###x
		{CODEANDTEXT(EXCEPTION_ACCESS_VIOLATION), MExcRAccess},
		{CODEANDTEXT(EXCEPTION_ARRAY_BOUNDS_EXCEEDED), MExcOutOfBounds},
		{CODEANDTEXT(EXCEPTION_INT_DIVIDE_BY_ZERO),MExcDivideByZero},
		{CODEANDTEXT(EXCEPTION_STACK_OVERFLOW),MExcStackOverflow},
		{CODEANDTEXT(EXCEPTION_BREAKPOINT),MExcBreakPoint},
		{CODEANDTEXT(EXCEPTION_FLT_DIVIDE_BY_ZERO),MExcFloatDivideByZero}, // BUGBUG: Floating-point exceptions (VC) are disabled by default. See http://msdn2.microsoft.com/en-us/library/aa289157(vs.71).aspx#floapoint_topic8
		{CODEANDTEXT(EXCEPTION_FLT_OVERFLOW),MExcFloatOverflow},           // BUGBUG:  ^^^
		{CODEANDTEXT(EXCEPTION_FLT_STACK_CHECK),MExcFloatStackOverflow},   // BUGBUG:  ^^^
		{CODEANDTEXT(EXCEPTION_FLT_UNDERFLOW),MExcFloatUnderflow},         // BUGBUG:  ^^^
		{CODEANDTEXT(EXCEPTION_ILLEGAL_INSTRUCTION),MExcBadInstruction},
		{CODEANDTEXT(EXCEPTION_PRIV_INSTRUCTION),MExcBadInstruction},
		{CODEANDTEXT(EXCEPTION_DATATYPE_MISALIGNMENT), MExcDatatypeMisalignment},
		// сюды добавляем.

		#undef CODEANDTEXT
	};

	string strBuf1, strBuf2;
	string strBuf;
	string strFileName;
	BOOL ShowMessages=FALSE;
	// получим запись исключения
	EXCEPTION_RECORD *xr = xp->ExceptionRecord;

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
		return i.Code == static_cast<NTSTATUS>(xr->ExceptionCode);
	});

	if (ItemIterator != std::cend(ECode))
	{
		Exception=LanguageLoaded()? MSG(ItemIterator->IdMsg) : ItemIterator->DefaultMsg;

		if (xr->ExceptionCode == static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION))
		{
			int Offset = 0;
			// вот только не надо здесь неочевидных оптимизаций вида
			// if ( xr->ExceptionInformation[0] == 8 ) Offset = 2 else Offset = xr->ExceptionInformation[0],
			// а то M$ порадует нас как-нибудь xr->ExceptionInformation[0] == 4 и все будет в полной жопе.

			switch (xr->ExceptionInformation[0])
			{
				case 0:
					Offset = 0;
					break;
				case 1:
					Offset = 1;
					break;
				case 8:
					Offset = 2;
					break;
			}

			strBuf2 = L"0x" + to_hex_wstring(xr->ExceptionInformation[1]);

			if (LanguageLoaded())
			{
				strBuf = string_format(MExcRAccess + Offset, strBuf2);
				Exception=strBuf.data();
			}
			else
			{
				const wchar_t* AVs[] = {L"read from ", L"write to ", L"execute at "};
				strBuf1 = Exception;
				strBuf1.append(L" (").append(AVs[Offset]).append(strBuf2).append(L")");
				Exception=strBuf1.data();
			}
		}
	}

	if (!Exception)
	{
		const wchar_t* Template = LanguageLoaded()? MSG(MExcUnknown) : L"Unknown exception";
		strBuf2.append(Template).append(L" (0x").append(to_hex_wstring(xr->ExceptionCode)).append(L")");
		Exception = strBuf2.data();
	}

	reply MsgCode = reply_handle;

	if (Global && Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		MsgCode=ExcDialog(strFileName,Exception,xr->ExceptionAddress);
		ShowMessages=TRUE;
	}
	else
	{
		ExcDump(strFileName, Exception, xr->ExceptionAddress);
	}

	if (ShowMessages && !PluginModule)
	{
		Global->CriticalInternalError=TRUE;
	}

	switch (MsgCode)
	{
	case reply_handle: // terminate
		return true;

#ifdef WITH_MINIDUMP
	case reply_minidump: // write %FARPROFILE%\Far.mdmp and terminate
		write_minidump(xp);
		return true;
#endif

	case reply_debug:
		attach_debugger();
		return false;

	case reply_ignore:
	default:
		return false;
	}
}

bool ProcessSEHException(Plugin *Module, const wchar_t* From, EXCEPTION_POINTERS *xp)
{
	// dummy parametrs setting
	::gFrom=From;
	::PluginModule = Module;

	return ProcessSEHExceptionImpl(xp);
}

void attach_debugger()
{
	SetErrorMode(Global->ErrorMode&~SEM_NOGPFAULTERRORBOX);
	DebugBreak();
	SetErrorMode(Global->ErrorMode);
}

bool ProcessStdException(const std::exception& e, const Plugin* Module, const wchar_t* function)
{
	if (Global)
		Global->ProcessException = TRUE;
	switch (Message(MSG_WARNING, 3, MSG(MExcTrappedException), wide(e.what()).data(), MSG(Module? MExcUnload : MExcTerminate), MSG(MExcDebugger), MSG(MIgnore)))
	{
	case 0:
		return true;
	case 1:
		attach_debugger();
		return false;
	case 2:
	default:
		return false;
	}
}

static DWORD WINAPI ProcessSEHExceptionWrapper(EXCEPTION_POINTERS* xp)
{
	ProcessSEHException(nullptr, nullptr, xp);
	return 0;
}

LONG WINAPI VectoredExceptionHandler(EXCEPTION_POINTERS *xp)
{
	// restore stack & call ProcessSEHExceptionWrapper
	if (xp->ExceptionRecord->ExceptionCode == (DWORD)STATUS_STACK_OVERFLOW)
	{
#if 1 // it is much better way than hack stack and modify original context
//#ifdef _M_IA64
		// TODO: Bad way to restore IA64 stacks (CreateThread)
		// Can you do smartly? See REMINDER file, section IA64Stacks
		static HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ProcessSEHExceptionWrapper, xp, 0, nullptr);
		if (hThread)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		TerminateProcess(GetCurrentProcess(), 1);
#else
		static struct
		{
			BYTE stack_space[32768];
			DWORD_PTR ret_addr;
			DWORD_PTR args[4];
		}
		stack;
		stack.ret_addr = 0;
#ifndef _WIN64
		stack.args[0] = reinterpret_cast<DWORD_PTR>(xp);
		//stack.args[1] = ...
		//stack.args[2] = ...
		//stack.args[3] = ...
		xp->ContextRecord->Esp = reinterpret_cast<DWORD_PTR>(&stack.ret_addr);
		xp->ContextRecord->Eip = reinterpret_cast<DWORD_PTR>(&ProcessSEHExceptionWrapper);
#else
		xp->ContextRecord->Rcx = reinterpret_cast<DWORD_PTR>(xp);
		//xp->ContextRecord->Rdx = ...
		//xp->ContextRecord->R8  = ...
		//xp->ContextRecord->R9  = ...
		xp->ContextRecord->Rsp = reinterpret_cast<DWORD_PTR>(&stack.ret_addr);
		xp->ContextRecord->Rip = reinterpret_cast<DWORD_PTR>(&ProcessSEHExceptionWrapper);
#endif
		return EXCEPTION_CONTINUE_EXECUTION;
#endif
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

inline void SETranslator(UINT Code, EXCEPTION_POINTERS* ExceptionInfo)
{
	throw SException(Code, ExceptionInfo);
}

void EnableSeTranslation()
{
#ifdef _MSC_VER
	_set_se_translator(SETranslator);
#endif
}

void EnableVectoredExceptionHandling()
{
	static bool VEH_installed = false;
	if (!VEH_installed)
	{
		Imports().AddVectoredExceptionHandler(TRUE, &VectoredExceptionHandler);
		VEH_installed = true;
	}
}

#if defined(FAR_ALPHA_VERSION)
WARNING_PUSH()

WARNING_DISABLE_MSC(4717) // https://msdn.microsoft.com/en-us/library/97c54274.aspx 'function' : recursive on all control paths, function will cause runtime stack overflow
WARNING_DISABLE_CLANG("-Winfinite-recursion")

static void Test_EXCEPTION_STACK_OVERFLOW(char* target)
{
	char Buffer[2048]; /* чтобы быстрее рвануло */
	strcpy(Buffer, "zzzz");
	Test_EXCEPTION_STACK_OVERFLOW(Buffer);

	// "side effect" to prevent deletion of this function call due to C4718.
	Sleep(0);
}
WARNING_POP()

static int ExceptionTestHook(Manager::Key key)
{
	// сей код для проверки исключатор, просьба не трогать :-)
	if (
		key.FarKey() == KEY_CTRLALTAPPS ||
		key.FarKey() == KEY_RCTRLRALTAPPS ||
		key.FarKey() == KEY_CTRLRALTAPPS ||
		key.FarKey() == KEY_RCTRLALTAPPS
		)
	{
		static const struct ECODE
		{
			NTSTATUS Code;
			const wchar_t *Name;
		}
		ECode[] =
		{
			{ 0, L"C++ std::exception", },
			{ EXCEPTION_ACCESS_VIOLATION, L"Access Violation (Read)" },
			{ EXCEPTION_ACCESS_VIOLATION, L"Access Violation (Write)" },
			{ EXCEPTION_INT_DIVIDE_BY_ZERO, L"Divide by zero" },
			{ EXCEPTION_ILLEGAL_INSTRUCTION, L"Illegal instruction" },
			{ EXCEPTION_STACK_OVERFLOW, L"Stack Overflow" },
			{ EXCEPTION_FLT_DIVIDE_BY_ZERO, L"Floating-point divide by zero" },
			{ EXCEPTION_BREAKPOINT, L"Breakpoint" },
#ifdef _M_IA64
			{ EXCEPTION_DATATYPE_MISALIGNMENT, L"Alignment fault (IA64 specific)", },
#endif
			/*
			{EXCEPTION_FLT_OVERFLOW,"EXCEPTION_FLT_OVERFLOW"},
			{EXCEPTION_SINGLE_STEP,"EXCEPTION_SINGLE_STEP",},
			{EXCEPTION_ARRAY_BOUNDS_EXCEEDED,"EXCEPTION_ARRAY_BOUNDS_EXCEEDED",},
			{EXCEPTION_FLT_DENORMAL_OPERAND,"EXCEPTION_FLT_DENORMAL_OPERAND",},
			{EXCEPTION_FLT_INEXACT_RESULT,"EXCEPTION_FLT_INEXACT_RESULT",},
			{EXCEPTION_FLT_INVALID_OPERATION,"EXCEPTION_FLT_INVALID_OPERATION",},
			{EXCEPTION_FLT_STACK_CHECK,"EXCEPTION_FLT_STACK_CHECK",},
			{EXCEPTION_FLT_UNDERFLOW,"EXCEPTION_FLT_UNDERFLOW",},
			{EXCEPTION_INT_OVERFLOW,"EXCEPTION_INT_OVERFLOW",0},
			{EXCEPTION_PRIV_INSTRUCTION,"EXCEPTION_PRIV_INSTRUCTION",0},
			{EXCEPTION_IN_PAGE_ERROR,"EXCEPTION_IN_PAGE_ERROR",0},
			{EXCEPTION_NONCONTINUABLE_EXCEPTION,"EXCEPTION_NONCONTINUABLE_EXCEPTION",0},
			{EXCEPTION_INVALID_DISPOSITION,"EXCEPTION_INVALID_DISPOSITION",0},
			{EXCEPTION_GUARD_PAGE,"EXCEPTION_GUARD_PAGE",0},
			{EXCEPTION_INVALID_HANDLE,"EXCEPTION_INVALID_HANDLE",0},
			*/
		};
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

		std::for_each(CONST_RANGE(ECode, i)
		{
			ModalMenu->AddItem(i.Name);
		});

		int ExitCode = ModalMenu->Run();

		switch (ExitCode)
		{
		case -1:
			return TRUE;
		case 0:
			throw std::runtime_error("test error");
		case 1:
			zero_const.i = *zero_const.iptr;
			break;
		case 2:
			*zero_const.iptr = 0;
			break;
		case 3:
			zero_const.i = 1 / zero_const.i;
			break;
		case 4:
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
		case 5:
			Test_EXCEPTION_STACK_OVERFLOW(nullptr);
			break;
		case 6:
			zero_const.d = 1.0 / zero_const.d;
			break;
		case 7:
			attach_debugger();
			break;
#ifdef _M_IA64
		case 8:
		{
			BYTE temp[10] = {};
			double* val;
			val = (double*)(&temp[3]);
			printf("%lf\n", *val);
		}
#endif
		}

		Message(MSG_WARNING, 1, L"Test Exceptions failed", L"", ECode[ExitCode].Name, L"", MSG(MOk));
		return TRUE;
	}
	return FALSE;
}
#endif

void RegisterTestExceptionsHook()
{
#ifdef FAR_ALPHA_VERSION
	Global->WindowManager->AddGlobalKeyHandler(ExceptionTestHook);
#endif
}