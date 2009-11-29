#pragma once

/*
global.hpp

Описание глобальных переменных
Включать последним.
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
extern clock_t StartExecTime;

#if defined(_INC_WINDOWS) || defined(_WINDOWS_) || defined(_WINDOWS_H)
extern DWORD InitialConsoleMode;
extern OSVERSIONINFOW WinVer;
#endif

class FileEditor;
extern FileEditor *CurrentEditor;

extern CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo;
extern SHORT ScrX,ScrY;
extern SHORT PrevScrX,PrevScrY;
extern HANDLE hConOut,hConInp;

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern string g_strFarPath;

extern string strGlobalSearchString;
extern int GlobalSearchCase;
extern int GlobalSearchWholeWords; // значение "Whole words" для поиска
extern int GlobalSearchHex; // значение "Search for hex" для поиска

extern int GlobalSearchReverse;

extern int ScreenSaverActive;

extern string strLastFarTitle;
extern int TitleModified;
extern int CloseFAR, CloseFARMenu;

extern int CmpNameSearchMode;
extern int DisablePluginsOutput;
extern int CmdMode;

extern const DWORD FAR_VERSION;

extern wchar_t RegColorsHighlight[];

extern bool LanguageLoaded;

extern BOOL NotUseCAS;
extern BOOL IsProcessAssignMacroKey;
extern BOOL IsProcessVE_FindFile;
extern BOOL IsRedrawFramesInProcess;

extern const char *Copyright;

extern int WidthNameForMessage;

extern const wchar_t DOS_EOL_fmt[];
extern const wchar_t UNIX_EOL_fmt[];
extern const wchar_t MAC_EOL_fmt[];
extern const wchar_t WIN_EOL_fmt[];

extern const char DOS_EOL_fmtA[];
extern const char UNIX_EOL_fmtA[];
extern const char MAC_EOL_fmtA[];
extern const char WIN_EOL_fmtA[];

extern BOOL ProcessException;

extern BOOL ProcessShowClock;

extern const wchar_t *FarTitleAddons;

extern const wchar_t FAR_VerticalBlock[];
extern const wchar_t FAR_VerticalBlock_Unicode[];

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

extern int UsedInternalClipboard;

#if defined(DETECT_ALT_ENTER)
extern int PrevFarAltEnterMode;
#endif

extern WCHAR BoxSymbols[];

extern int _localLastError;

extern const wchar_t *ReservedFilenameSymbols;

extern int KeepUserScreen;
extern string g_strDirToSet; //RAVE!!!

extern int Macro_DskShowPosType; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))

extern const wchar_t *FavoriteCodePagesKey;

// Macro Const
extern const wchar_t constMsX[];
extern const wchar_t constMsY[];
extern const wchar_t constMsButton[];
extern const wchar_t constMsCtrlState[];

extern DWORD RedrawTimeout;

extern SYSTEM_INFO SystemInfo;

extern string g_strMacroCONVFMT;

extern FormatScreen FS;
