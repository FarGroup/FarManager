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
#include "macro.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "colors.hpp"
#include "palette.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "configdb.hpp"

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp,
               Plugin *Module,
               void *RawData,DWORD RawDataSize,
               DWORD RawDataFlags,DWORD RawType)
{
	return 0;
}

/* ************************************************************************
   $ 16.10.2000 SVS
   Простенький обработчик исключений.
*/

LPCWSTR GetFunctionName(int ExceptFunctionType)
{
	switch(ExceptFunctionType)
	{
		case EXCEPT_KERNEL: return L"wmain";
		case EXCEPT_GETGLOBALINFO: return L"GetGlobalInfo";
		case EXCEPT_SETSTARTUPINFO: return L"SetStartupInfo";
		case EXCEPT_GETVIRTUALFINDDATA: return L"GetVirtualFindData";
		case EXCEPT_OPEN: return L"Open";
		case EXCEPT_OPENFILEPLUGIN: return L"OpenFilePlugin";
		case EXCEPT_CLOSEPANEL: return L"ClosePanel";
		case EXCEPT_GETPLUGININFO: return L"GetPluginInfo";
		case EXCEPT_GETOPENPANELINFO: return L"GetOpenPanelInfo";
		case EXCEPT_GETFINDDATA: return L"GetFindData";
		case EXCEPT_FREEFINDDATA: return L"FreeFindData";
		case EXCEPT_FREEVIRTUALFINDDATA: return L"FreeVitrualFindData";
		case EXCEPT_SETDIRECTORY: return L"SetDirectory";
		case EXCEPT_GETFILES: return L"GetFiles";
		case EXCEPT_PUTFILES: return L"PutFiles";
		case EXCEPT_DELETEFILES: return L"DeleteFiles";
		case EXCEPT_MAKEDIRECTORY: return L"MakeDirectory";
		case EXCEPT_PROCESSHOSTFILE: return L"ProcessHostFile";
		case EXCEPT_SETFINDLIST: return L"SetFindList";
		case EXCEPT_CONFIGURE: return L"Configure";
		case EXCEPT_EXITFAR: return L"ExitFAR";
		case EXCEPT_PROCESSPANELINPUT: return L"ProcessPanelInput";
		case EXCEPT_PROCESSPANELEVENT: return L"ProcessPanelEvent";
		case EXCEPT_PROCESSEDITOREVENT: return L"ProcessEditorEvent";
		case EXCEPT_COMPARE: return L"Compare";
		case EXCEPT_PROCESSEDITORINPUT: return L"ProcessEditorInput";
		case EXCEPT_MINFARVERSION: return L"GetMinFarVersion";
		case EXCEPT_PROCESSVIEWEREVENT: return L"ProcessViewerEvent";
		case EXCEPT_PROCESSVIEWERINPUT: return L"ProcessViewerInput";
		case EXCEPT_PROCESSDIALOGEVENT: return L"ProcessDialogEvent";
		case EXCEPT_PROCESSSYNCHROEVENT: return L"ProcessSynchroEvent";
		case EXCEPT_ANALYSE: return L"Analyse";
		case EXCEPT_GETCUSTOMDATA: return L"GetCustomData";
		case EXCEPT_FREECUSTOMDATA: return L"FreeCustomData";
		case EXCEPT_CLOSEANALYSE: return L"CloseAnalyse";
#if defined(MANTIS_0000466)
		case EXCEPT_PROCESSMACRO: return L"ProcessMacro";
#endif
#if defined(MANTIS_0001687)
		case EXCEPT_PROCESSCONSOLEINPUT: return L"ProcessConsoleInput";
#endif
	}

	return L"";
};


static bool Is_STACK_OVERFLOW=false;
bool UseExternalHandler=false;

// Some parametes for _xfilter function
static int From=0;
static EXCEPTION_POINTERS *xp=nullptr;    // данные ситуации
static Plugin *Module=nullptr;     // модуль, приведший к исключению.
static DWORD Flags=0;                  // дополнительные флаги - пока только один
//        0x1 - спрашивать про выгрузку?

extern void CreatePluginStartupInfo(const Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF);

intptr_t WINAPI ExcDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
		{
			FarDialogItem di;
			SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,Param1,&di);

			if (di.Type==DI_EDIT)
			{
				FarColor Color=ColorIndexToColor(COL_WARNDIALOGTEXT);
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
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
					SendDlgMessage(hDlg,DM_SETFOCUS,11,0);
					return TRUE;
				}
				else if (Param1==11 && (key==KEY_RIGHT || key == KEY_NUMPAD6 || key==KEY_TAB))
				{
					SendDlgMessage(hDlg,DM_SETFOCUS,10,0);
					return TRUE;
				}
			}
		}
		break;
	default:
		break;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool ExcDialog(LPCWSTR ModuleName,LPCWSTR Exception,LPVOID Adress)
{
	string strAddr;
	strAddr.Format(L"0x%p",Adress);
	string strFunction=GetFunctionName(From);
#ifndef NO_WRAPPER
	if(Module && !Module->IsOemPlugin())
#endif // NO_WRAPPER
	{
		strFunction+=L"W";
	}

	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,8,0,nullptr,nullptr,0,MSG(MExcTrappedException)},
		{DI_TEXT,     5,2, 17,2,0,nullptr,nullptr,0,MSG(MExcException)},
		{DI_TEXT,    18,2, 70,2,0,nullptr,nullptr,0,Exception},
		{DI_TEXT,     5,3, 17,3,0,nullptr,nullptr,0,MSG(MExcAddress)},
		{DI_TEXT,    18,3, 70,3,0,nullptr,nullptr,0,strAddr},
		{DI_TEXT,     5,4, 17,4,0,nullptr,nullptr,0,MSG(MExcFunction)},
		{DI_TEXT,    18,4, 70,4,0,nullptr,nullptr,0,strFunction},
		{DI_TEXT,     5,5, 17,5,0,nullptr,nullptr,0,MSG(MExcModule)},
		{DI_EDIT,    18,5, 70,5,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,ModuleName},
		{DI_TEXT,    -1,6, 0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_CENTERGROUP,MSG((From == EXCEPT_KERNEL)?MExcTerminate:MExcUnload)},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MExcDebugger)},
	};
	MakeDialogItemsEx(EditDlgData,EditDlg);
	Dialog Dlg(EditDlg, ARRAYSIZE(EditDlg),ExcDlgProc);
	Dlg.SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg.SetPosition(-1,-1,76,10);
	Dlg.Process();
	return Dlg.GetExitCode()==11;
}

static DWORD WINAPI _xfilter(LPVOID dummy=nullptr)
{
	ProcessException=TRUE;
	DWORD Result = EXCEPTION_EXECUTE_HANDLER;
	BOOL Res=FALSE;
//   if(From == EXCEPT_KERNEL)
//     CriticalInternalError=TRUE;

	if (!Is_STACK_OVERFLOW && Opt.ExceptUsed)
	{
		if (!Opt.strExceptEventSvc.IsEmpty())
		{
			HMODULE m = LoadLibrary(Opt.strExceptEventSvc);

			if (m)
			{
				typedef BOOL (WINAPI *ExceptionProc_t)(EXCEPTION_POINTERS *xp,
				                                       const PLUGINRECORD *Module,
				                                       const PluginStartupInfo *LocalStartupInfo,
				                                       LPDWORD Result);
				ExceptionProc_t p = (ExceptionProc_t)GetProcAddress(m,"ExceptionProc");

				if (p)
				{
					static PluginStartupInfo LocalStartupInfo;
					ClearStruct(LocalStartupInfo);
					static FarStandardFunctions LocalStandardFunctions;
					ClearStruct(LocalStandardFunctions);
					CreatePluginStartupInfo(nullptr, &LocalStartupInfo, &LocalStandardFunctions);
					LocalStartupInfo.ModuleName = Opt.strExceptEventSvc;
					static PLUGINRECORD PlugRec;

					if (Module)
					{
						ClearStruct(PlugRec);
						PlugRec.TypeRec=RTYPE_PLUGIN;
						PlugRec.SizeRec=sizeof(PLUGINRECORD);
						PlugRec.ModuleName=Module->GetModuleName();
						PlugRec.WorkFlags=Module->GetWorkFlags();
						PlugRec.CallFlags=Module->GetFuncFlags();
						PlugRec.FuncFlags=0;
						PlugRec.FuncFlags|=Module->HasGetGlobalInfo()?PICFF_GETGLOBALINFO:0;
						PlugRec.FuncFlags|=Module->HasSetStartupInfo()?PICFF_SETSTARTUPINFO:0;
						PlugRec.FuncFlags|=Module->HasOpenPanel()?PICFF_OPENPANEL:0;
						PlugRec.FuncFlags|=Module->HasAnalyse()?PICFF_ANALYSE:0;
						PlugRec.FuncFlags|=Module->HasClosePanel()?PICFF_CLOSEPANEL:0;
						PlugRec.FuncFlags|=Module->HasGetPluginInfo()?PICFF_GETPLUGININFO:0;
						PlugRec.FuncFlags|=Module->HasGetOpenPanelInfo()?PICFF_GETOPENPANELINFO:0;
						PlugRec.FuncFlags|=Module->HasGetFindData()?PICFF_GETFINDDATA:0;
						PlugRec.FuncFlags|=Module->HasFreeFindData()?PICFF_FREEFINDDATA:0;
						PlugRec.FuncFlags|=Module->HasGetVirtualFindData()?PICFF_GETVIRTUALFINDDATA:0;
						PlugRec.FuncFlags|=Module->HasFreeVirtualFindData()?PICFF_FREEVIRTUALFINDDATA:0;
						PlugRec.FuncFlags|=Module->HasSetDirectory()?PICFF_SETDIRECTORY:0;
						PlugRec.FuncFlags|=Module->HasGetFiles()?PICFF_GETFILES:0;
						PlugRec.FuncFlags|=Module->HasPutFiles()?PICFF_PUTFILES:0;
						PlugRec.FuncFlags|=Module->HasDeleteFiles()?PICFF_DELETEFILES:0;
						PlugRec.FuncFlags|=Module->HasMakeDirectory()?PICFF_MAKEDIRECTORY:0;
						PlugRec.FuncFlags|=Module->HasProcessHostFile()?PICFF_PROCESSHOSTFILE:0;
						PlugRec.FuncFlags|=Module->HasSetFindList()?PICFF_SETFINDLIST:0;
						PlugRec.FuncFlags|=Module->HasConfigure()?PICFF_CONFIGURE:0;
						PlugRec.FuncFlags|=Module->HasExitFAR()?PICFF_EXITFAR:0;
						PlugRec.FuncFlags|=Module->HasProcessPanelInput()?PICFF_PROCESSPANELINPUT:0;
						PlugRec.FuncFlags|=Module->HasProcessPanelEvent()?PICFF_PROCESSPANELEVENT:0;
						PlugRec.FuncFlags|=Module->HasProcessEditorEvent()?PICFF_PROCESSEDITOREVENT:0;
						PlugRec.FuncFlags|=Module->HasCompare()?PICFF_COMPARE:0;
						PlugRec.FuncFlags|=Module->HasProcessEditorInput()?PICFF_PROCESSEDITORINPUT:0;
						PlugRec.FuncFlags|=Module->HasMinFarVersion()?PICFF_MINFARVERSION:0;
						PlugRec.FuncFlags|=Module->HasProcessViewerEvent()?PICFF_PROCESSVIEWEREVENT:0;
						PlugRec.FuncFlags|=Module->HasProcessDialogEvent()?PICFF_PROCESSDIALOGEVENT:0;
						PlugRec.FuncFlags|=Module->HasProcessSynchroEvent()?PICFF_PROCESSSYNCHROEVENT:0;
#if defined(MANTIS_0000466)
						PlugRec.FuncFlags|=Module->HasProcessMacro()?PICFF_PROCESSMACRO:0;
#endif
#if defined(MANTIS_0001687)
						PlugRec.FuncFlags|=Module->HasProcessConsoleInput()?PICFF_PROCESSCONSOLEINPUT:0;
#endif
					}

					Res=p(xp,(Module?&PlugRec:nullptr),&LocalStartupInfo,&Result);
				}

				FreeLibrary(m);
			}
		}
	}

	if (Res)
	{
		if (From == EXCEPT_KERNEL)
		{
			CriticalInternalError=TRUE;
		}

		return Result;
	}

	struct __ECODE
	{
		NTSTATUS Code;     // код исключения
		LNGID IdMsg;    // ID сообщения из LNG-файла
		DWORD RetCode;  // Что вернем?
	} ECode[]=

	{
		{EXCEPTION_ACCESS_VIOLATION, MExcRAccess, EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_ARRAY_BOUNDS_EXCEEDED, MExcOutOfBounds, EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_INT_DIVIDE_BY_ZERO,MExcDivideByZero, EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_STACK_OVERFLOW,MExcStackOverflow, EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_BREAKPOINT,MExcBreakPoint, EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_FLT_DIVIDE_BY_ZERO,MExcFloatDivideByZero,EXCEPTION_EXECUTE_HANDLER}, // BUGBUG: Floating-point exceptions (VC) are disabled by default. See http://msdn2.microsoft.com/en-us/library/aa289157(vs.71).aspx#floapoint_topic8
		{EXCEPTION_FLT_OVERFLOW,MExcFloatOverflow,EXCEPTION_EXECUTE_HANDLER},           // BUGBUG:  ^^^
		{EXCEPTION_FLT_STACK_CHECK,MExcFloatStackOverflow,EXCEPTION_EXECUTE_HANDLER},   // BUGBUG:  ^^^
		{EXCEPTION_FLT_UNDERFLOW,MExcFloatUnderflow,EXCEPTION_EXECUTE_HANDLER},         // BUGBUG:  ^^^
		{EXCEPTION_ILLEGAL_INSTRUCTION,MExcBadInstruction,EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_PRIV_INSTRUCTION,MExcBadInstruction,EXCEPTION_EXECUTE_HANDLER},
		{EXCEPTION_DATATYPE_MISALIGNMENT, MExcDatatypeMisalignment, EXCEPTION_EXECUTE_HANDLER},
		// сюды добавляем.
	};
	// EXCEPTION_CONTINUE_EXECUTION  ??????
	DWORD rc;
	string strBuf1, strBuf2, strBuf3;
	LangString strBuf;
	string strFileName;
	BOOL ShowMessages=FALSE;
	// получим запись исключения
	EXCEPTION_RECORD *xr = xp->ExceptionRecord;

	// выведим дамп перед выдачей сообщений
	WriteEvent(FLOG_ALL,xp,Module,nullptr,0);

	// CONTEXT можно использовать для отображения или записи в лог
	//         содержимого регистров...
	// CONTEXT *xc = xp->ContextRecord;
	rc = Result;// EXCEPTION_EXECUTE_HANDLER;
	/*$ 23.01.2001 skv
	  Неизвестное исключение не стоит игнорировать.
	*/

	if (From == EXCEPT_KERNEL || !Module)
		strFileName=g_strFarModuleName;
	else
		strFileName = Module->GetModuleName();

	LPCWSTR Exception=nullptr;
	// просмотрим "знакомые" FAR`у исключения и обработаем...
	for (size_t I=0; I < ARRAYSIZE(ECode); ++I)
	{
		if (ECode[I].Code == static_cast<NTSTATUS>(xr->ExceptionCode))
		{
			Exception=MSG(ECode[I].IdMsg);
			rc=ECode[I].RetCode;

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

				strBuf2.Format(L"0x%p", xr->ExceptionInformation[1]+10);
				strBuf = MExcRAccess+Offset;
				strBuf << strBuf2;
				Exception=strBuf;
			}

			break;
		}
	}

	if (!Exception)
	{
		strBuf2.Format(L"%s (0x%X)", MSG(MExcUnknown), xr->ExceptionCode);
		Exception = strBuf2;
	}

	int MsgCode=0;
	if (FrameManager && !FrameManager->ManagerIsDown())
	{
		MsgCode=ExcDialog(strFileName,Exception,xr->ExceptionAddress);
		ShowMessages=TRUE;
	}

	if (ShowMessages && (Is_STACK_OVERFLOW || From == EXCEPT_KERNEL))
	{
		CriticalInternalError=TRUE;
	}

	if(MsgCode==1)
	{
		SetErrorMode(ErrorMode&~SEM_NOGPFAULTERRORBOX);
		rc=EXCEPTION_CONTINUE_SEARCH;
		UseExternalHandler=true;
	}
	else
	{
		rc = EXCEPTION_EXECUTE_HANDLER;
	}
	//return UnhandledExceptionFilter(xp);
	return rc;
}

DWORD WINAPI xfilter(int From,EXCEPTION_POINTERS *xp, Plugin *Module,DWORD Flags)
{
	DWORD Result=EXCEPTION_CONTINUE_SEARCH;
	if(Opt.ExceptRules && !UseExternalHandler)
	{
		// dummy parametrs setting
		::From=From;
		::xp=xp;
		::Module=Module;
		::Flags=Flags;

		if (xp->ExceptionRecord->ExceptionCode == static_cast<DWORD>(STATUS_STACK_OVERFLOW)) // restore stack & call_xfilter ;
		{
			Is_STACK_OVERFLOW=true;
#ifdef _M_IA64
			// TODO: Bad way to restore IA64 stacks (CreateThread)
			// Can you do smartly? See REMINDER file, section IA64Stacks
			static HANDLE hThread = nullptr;

			if (!(hThread = CreateThread(nullptr, 0, _xfilter, nullptr, 0, nullptr)))
			{
				TerminateProcess(GetCurrentProcess(), 1);
			}

			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
#else
			static struct
			{
				BYTE      stack_space[32768];
				intptr_t ret_addr;
				intptr_t args[4];
			} _stack;
			_stack.ret_addr = 0;
#ifndef _WIN64
#ifdef _M_ARM
			// BUGBUG
#else
			//_stack.args[0] = (intptr_t)From;
			//_stack.args[1] = (intptr_t)xp;
			//_stack.args[2] = (intptr_t)Module;
			//_stack.args[3] = Flags;
			xp->ContextRecord->Esp = (DWORD)(intptr_t)(&_stack.ret_addr);
			xp->ContextRecord->Eip = (DWORD)(intptr_t)(&_xfilter);
#endif
#else
			//xp->ContextRecord->Rcx = (intptr_t)From;
			//xp->ContextRecord->Rdx = (intptr_t)xp;
			//xp->ContextRecord->R8  = (intptr_t)Module;
			//xp->ContextRecord->R9  = Flags;
			xp->ContextRecord->Rsp = (intptr_t)(&_stack.ret_addr);
			xp->ContextRecord->Rip = (intptr_t)(&_xfilter);
#endif
#endif
			Result=EXCEPTION_CONTINUE_EXECUTION;
		}
		else
		{
			Result=_xfilter();
		}
	}
	return Result;
}
