#pragma once

/*
global.hpp

Описание глобальных переменных
Включать последним.
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

class global
{
public:
	global();
	~global();

	HANDLE MainThreadHandle() const {return m_MainThreadHandle;}
	inline bool IsMainThread() const {return GetCurrentThreadId() == m_MainThreadId;}
	const LARGE_INTEGER& FarUpTime() const {return m_FarUpTime;}

	bool IsPtr(const void* Address) const;
	bool IsUserAdmin() const;
	const OSVERSIONINFO& WinVer() const;
	const wchar_t* Version() const;
	const wchar_t* Copyright() const;

	// BUGBUG

	clock_t StartIdleTime;
	int WaitInMainLoop;
	int WaitInFastFind;
	string g_strFarModuleName;
	string g_strFarINI;
	string g_strFarPath;
	string strGlobalSearchString;
	string strInitTitle;
	bool GlobalSearchCase;
	bool GlobalSearchWholeWords; // значение "Whole words" для поиска
	bool GlobalSearchHex; // значение "Search for hex" для поиска
	bool GlobalSearchReverse;
	int ScreenSaverActive;
	int CloseFAR, CloseFARMenu, AllowCancelExit;
	int DisablePluginsOutput;
	BOOL IsProcessAssignMacroKey;
	BOOL IsRedrawFramesInProcess;
	size_t PluginPanelsCount;
	BOOL ProcessException;
	BOOL ProcessShowClock;
	const wchar_t *HelpFileMask;
	const wchar_t *HelpFormatLinkModule;
#if defined(SYSLOG)
	BOOL StartSysLog;
#endif

	static intptr_t CallNewDeleteVector;
	static intptr_t CallNewDeleteScalar;
	static intptr_t CallMallocFree;
	static size_t AllocatedMemoryBlocks;
	static size_t AllocatedMemorySize;
	static size_t TotalAllocationCalls;

	static int EnableSEH;

#ifdef DIRECT_RT
	bool DirectRT;
#endif
	class SaveScreen *GlobalSaveScrPtr;
	int CriticalInternalError;
	int KeepUserScreen;
	string g_strDirToSet; //RAVE!!!
	int Macro_DskShowPosType; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))
	DWORD ErrorMode;
#ifndef NO_WRAPPER
	string strRegRoot;
#endif // NO_WRAPPER

	// BUGBUG end

private:
	DWORD m_MainThreadId;
	LARGE_INTEGER m_FarUpTime;
	HANDLE m_MainThreadHandle;

public:
	FormatScreen FS;
	std::unique_ptr<class ImportedFunctions> ifn;
	std::unique_ptr<class console> Console;
	std::unique_ptr<class ScreenBuf> ScrBuf;
	std::unique_ptr<class TaskBarCore> TBC;
	std::unique_ptr<class consoleicons> ConsoleIcons;
	std::unique_ptr<class TPreRedrawFunc> PreRedraw;
	std::unique_ptr<class WindowHandler> Window;
	std::unique_ptr<class Options> Opt;
	std::unique_ptr<class Language> Lang;
	std::unique_ptr<class Language> OldLang;
	std::unique_ptr<class elevation> Elevation;
	std::unique_ptr<class TreeListCache> TreeCache;
	std::unique_ptr<class TreeListCache> tempTreeCache;
	std::unique_ptr<class PluginSynchro> PluginSynchroManager;
	std::unique_ptr<class codepages> CodePages;
	std::unique_ptr<class Database> Db;
	std::unique_ptr<class ControlObject> CtrlObject;
};

#define MSG(ID) Global->Lang->GetMsg(ID)

// VersionConstant: LOWBYTE - minor, HIBYTE - major
inline bool operator< (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) < VersionConstant;}
inline bool operator> (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) > VersionConstant;}
inline bool operator<= (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) <= VersionConstant;}
inline bool operator>= (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) >= VersionConstant;}

extern std::unique_ptr<global> Global;
