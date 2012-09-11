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

/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "bootstrap/copyright.inc"

OSVERSIONINFO WinVer={sizeof(WinVer)};

// идет процесс назначения клавиши в макросе?
BOOL IsProcessAssignMacroKey=FALSE;

// идет процесс "вьювер/редактор" во время поиска файлов?
BOOL IsProcessVE_FindFile=FALSE;

// Идёт процесс перерисовки всех фреймов
BOOL IsRedrawFramesInProcess=FALSE;

size_t PluginPanelsCount = 0;

// идет процесс быстрого поиска в панелях?
int WaitInFastFind=FALSE;

// мы крутимся в основном цикле?
int WaitInMainLoop=FALSE;

clock_t StartIdleTime=0;

string g_strFarModuleName;
string g_strFarINI;
string g_strFarPath;

string strGlobalSearchString;
bool GlobalSearchCase=false;
bool GlobalSearchWholeWords=false; // значение "Whole words" для поиска
bool GlobalSearchHex=false;     // значение "Search for hex" для поиска
bool GlobalSearchReverse=false;

int ScreenSaverActive=FALSE;

int CloseFAR=FALSE,CloseFARMenu=FALSE, AllowCancelExit=TRUE;

int DisablePluginsOutput=FALSE;

int WidthNameForMessage=0;

BOOL ProcessException=FALSE;
BOOL ProcessShowClock=FALSE;

const wchar_t *HelpFileMask=L"*.hlf";
const wchar_t *HelpFormatLinkModule=L"<%s>%s";

#if defined(SYSLOG)
BOOL StartSysLog=0;
long CallNewDelete=0;
long CallMallocFree=0;
#endif

class SaveScreen;
SaveScreen *GlobalSaveScrPtr=nullptr;

int CriticalInternalError=FALSE;

int KeepUserScreen;
string g_strDirToSet;

int Macro_DskShowPosType=0; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))

#ifndef FAR_LUA
// Macro Const
const wchar_t constMsX[]=L"MsX";
const wchar_t constMsY[]=L"MsY";
const wchar_t constMsButton[]=L"MsButton";
const wchar_t constMsCtrlState[]=L"MsCtrlState";
const wchar_t constMsEventFlags[]=L"MsEventFlags";
const wchar_t constRCounter[]=L"RCounter";
const wchar_t constFarCfgErr[]=L"Far.Cfg.Err";
#endif

SYSTEM_INFO SystemInfo;

FormatScreen FS;

DWORD ErrorMode;

LARGE_INTEGER FarUpTime;

HANDLE MainThreadHandle;
DWORD MainThreadId;
