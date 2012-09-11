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

extern clock_t StartIdleTime;

extern OSVERSIONINFO WinVer;

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern string g_strFarModuleName;
extern string g_strFarINI;
extern string g_strFarPath;

extern string strGlobalSearchString;
extern bool GlobalSearchCase;
extern bool GlobalSearchWholeWords; // значение "Whole words" для поиска
extern bool GlobalSearchHex; // значение "Search for hex" для поиска

extern bool GlobalSearchReverse;

extern int ScreenSaverActive;

extern int CloseFAR, CloseFARMenu, AllowCancelExit;

extern int DisablePluginsOutput;

extern BOOL IsProcessAssignMacroKey;
extern BOOL IsRedrawFramesInProcess;

extern size_t PluginPanelsCount;

extern const wchar_t* Version;
extern const wchar_t* Copyright;

extern int WidthNameForMessage;

extern BOOL ProcessException;

extern BOOL ProcessShowClock;

extern const wchar_t *HelpFileMask;
extern const wchar_t *HelpFormatLinkModule;

#if defined(SYSLOG)
extern BOOL StartSysLog;
extern long CallNewDelete;
extern long CallMallocFree;
#endif

class SaveScreen;
extern SaveScreen *GlobalSaveScrPtr;

extern int CriticalInternalError;

extern int KeepUserScreen;
extern string g_strDirToSet; //RAVE!!!

extern int Macro_DskShowPosType; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))

// Macro Const
#ifdef FAR_LUA
enum {
	constMsX          = 0,
	constMsY          = 1,
	constMsButton     = 2,
	constMsCtrlState  = 3,
	constMsEventFlags = 4,
	constMsLAST       = 5,
};
#else
extern const wchar_t constMsX[];
extern const wchar_t constMsY[];
extern const wchar_t constMsButton[];
extern const wchar_t constMsCtrlState[];
extern const wchar_t constMsEventFlags[];
extern const wchar_t constRCounter[];
extern const wchar_t constFarCfgErr[];
#endif

extern SYSTEM_INFO SystemInfo;
inline bool IsPtr(const void* Address)
{
	return reinterpret_cast<uintptr_t>(Address)>=reinterpret_cast<uintptr_t>(SystemInfo.lpMinimumApplicationAddress) && reinterpret_cast<uintptr_t>(Address)<=reinterpret_cast<uintptr_t>(SystemInfo.lpMaximumApplicationAddress);
}

extern FormatScreen FS;

extern DWORD ErrorMode;

extern LARGE_INTEGER FarUpTime;

extern HANDLE MainThreadHandle;
extern DWORD MainThreadId;

inline bool MainThread() {return GetCurrentThreadId() == MainThreadId;}

// VersionConstant: LOWBYTE - minor, HIBYTE - major
inline bool operator< (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) < VersionConstant;}
inline bool operator> (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) > VersionConstant;}
inline bool operator<= (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) <= VersionConstant;}
inline bool operator>= (const OSVERSIONINFO& OsVersionInfo, WORD VersionConstant) {return MAKEWORD(OsVersionInfo.dwMinorVersion, OsVersionInfo.dwMajorVersion) >= VersionConstant;}
