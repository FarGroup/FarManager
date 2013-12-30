/*
global.cpp

Глобальные переменные
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

#include "imports.hpp"
#include "console.hpp"
#include "scrbuf.hpp"
#include "TaskBar.hpp"
#include "format.hpp"
#include "TPreRedrawFunc.hpp"
#include "window.hpp"
#include "config.hpp"
#include "language.hpp"
#include "elevation.hpp"
#include "treelist.hpp"
#include "interf.hpp"
#include "PluginSynchro.hpp"
#include "codepage.hpp"
#include "configdb.hpp"
#include "ctrlobj.hpp"
#include "edit.hpp"
#include "notification.hpp"
#include "manager.hpp"

thread DWORD global::m_LastError = ERROR_SUCCESS;
thread NTSTATUS global::m_LastStatus = STATUS_SUCCESS;

global::global():
	m_MainThreadId(GetCurrentThreadId()),
	ifn(nullptr),
	Console(nullptr),
	ScrBuf(nullptr),
	TBC(nullptr),
	ConsoleIcons(nullptr),
	//FS(nullptr),
	PreRedraw(nullptr),
	Notifier(nullptr),
	Window(nullptr),
	Opt(nullptr),
	Lang(nullptr),
	OldLang(nullptr),
	Elevation(nullptr),
	TreeCache(nullptr),
	tempTreeCache(nullptr),
	PluginSynchroManager(nullptr),
	CodePages(nullptr),
	Sets(nullptr),
	Db(nullptr),
	CtrlObject(nullptr)
{
	Global = this;

	QueryPerformanceCounter(&m_FarUpTime);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &m_MainThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

	// BUGBUG

	// идет процесс назначения клавиши в макросе?
	IsProcessAssignMacroKey=FALSE;
	// Идёт процесс перерисовки всех фреймов
	IsRedrawFramesInProcess=FALSE;
	PluginPanelsCount = 0;
	// идет процесс быстрого поиска в панелях?
	WaitInFastFind=FALSE;
	// мы крутимся в основном цикле?
	WaitInMainLoop=FALSE;
	StartIdleTime=0;
	GlobalSearchCase=false;
	GlobalSearchWholeWords=false; // значение "Whole words" для поиска
	GlobalSearchHex=false;     // значение "Search for hex" для поиска
	GlobalSearchReverse=false;
	ScreenSaverActive=FALSE;
	CloseFAR=FALSE;
	CloseFARMenu=FALSE;
	AllowCancelExit=TRUE;
	DisablePluginsOutput=FALSE;
	ProcessException=FALSE;
	ProcessShowClock=FALSE;
	HelpFileMask=L"*.hlf";
#if defined(SYSLOG)
	StartSysLog=0;
#endif
#ifdef DIRECT_RT
	DirectRT = false;
#endif
	GlobalSaveScrPtr=nullptr;
	CriticalInternalError=FALSE;
	KeepUserScreen = 0;
	Macro_DskShowPosType=0; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))
	ErrorMode = SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;

	// BUGBUG end

	ifn = new ImportedFunctions;
	Console = console::CreateInstance(true);
	ScrBuf = new ScreenBuf;
	TBC = new TaskBarCore;
	ConsoleIcons = new consoleicons;
	//FS = new FormatScreen;
	PreRedraw = new TPreRedrawFunc;
	Notifier = new notifier;
	Window = new WindowHandler;
	FrameManager = new Manager;
	Opt = new Options;
	Lang = new Language;
	OldLang = new Language;
	Elevation = new elevation;
	TreeCache = new TreeListCache;
	tempTreeCache = new TreeListCache;
	PluginSynchroManager = new PluginSynchro;
	CodePages = new codepages;
	Sets = new sets;
}

global::~global()
{
	delete CtrlObject;
	CtrlObject = nullptr;
	delete Db;
	Db = nullptr;
	delete Sets;
	Sets = nullptr;
	delete CodePages;
	CodePages = nullptr;
	delete PluginSynchroManager;
	PluginSynchroManager = nullptr;
	delete tempTreeCache;
	tempTreeCache = nullptr;
	delete TreeCache;
	TreeCache = nullptr;
	delete Elevation;
	Elevation = nullptr;
	delete OldLang;
	OldLang = nullptr;
	delete Lang;
	Lang = nullptr;
	delete Opt;
	Opt = nullptr;
	delete FrameManager;
	FrameManager = nullptr;
	delete Window;
	Window = nullptr;
	delete Notifier;
	Notifier = nullptr;
	delete PreRedraw;
	PreRedraw = nullptr;
	//delete FS;
	//FS = nullptr;
	delete ConsoleIcons;
	ConsoleIcons = nullptr;
	delete TBC;
	TBC = nullptr;
	delete ScrBuf;
	ScrBuf = nullptr;
	delete Console;
	Console = nullptr;
	delete ifn;
	ifn = nullptr;

	CloseHandle(m_MainThreadHandle);

	Global = nullptr;
}

bool global::IsUserAdmin() const
{
	static bool Checked = false;
	static bool Result = false;

	if(!Checked)
	{
		SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;
		try
		{
			api::sid_object AdministratorsGroup(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
			BOOL IsMember = FALSE;
			if(CheckTokenMembership(nullptr, AdministratorsGroup.get(), &IsMember) && IsMember)
			{
				Result = true;
			}
		}
		catch(const FarRecoverableException&)
		{
			// TODO: Log
		}
		Checked = true;
	}
	return Result;
}

const OSVERSIONINFO& global::WinVer() const
{
	static OSVERSIONINFO Info;
	static bool Checked = false;
	if(!Checked)
	{
		Info.dwOSVersionInfoSize = sizeof(Info);
		GetVersionEx(&Info);
		Checked = true;
	}
	return Info;
}

bool global::IsPtr(const void* Address) const
{
	static SYSTEM_INFO info;
	static bool Checked = false;
	if(!Checked)
	{
		GetSystemInfo(&info);
		Checked = true;
	}
	return reinterpret_cast<uintptr_t>(Address) >= reinterpret_cast<uintptr_t>(info.lpMinimumApplicationAddress) &&
		reinterpret_cast<uintptr_t>(Address) <= reinterpret_cast<uintptr_t>(info.lpMaximumApplicationAddress);
}

void global::CatchError()
{
	m_LastError = GetLastError();
	m_LastStatus = Global->ifn->RtlGetLastNtStatus();
}

#include "bootstrap/copyright.inc"
