/*
global.cpp

Глобальные переменные
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

#include "headers.hpp"
#pragma hdrstop

/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "copyright.inc"

/* $ 07.12.2000 SVS
   + Версия берется из файла farversion.inc
*/
#include "farversion.inc"

OSVERSIONINFO WinVer={0};

// языковой файл загружен?
bool LanguageLoaded=false;

// флаг на запрет юзание Ctrl-Alt-Shift
BOOL NotUseCAS=FALSE;

// идет процесс назначения клавиши в макросе?
BOOL IsProcessAssignMacroKey=FALSE;

// идет процесс "вьювер/редактор" во время поиска файлов?
BOOL IsProcessVE_FindFile=FALSE;

// Идёт процесс перерисовки всех фреймов
BOOL IsRedrawFramesInProcess=FALSE;

// идет процесс быстрого поиска в панелях?
int WaitInFastFind=FALSE;

// мы крутимся в основном цикле?
int WaitInMainLoop=FALSE;

clock_t StartIdleTime=0;

clock_t StartExecTime=0;

string g_strFarPath;

string strLastFarTitle;
int  TitleModified=FALSE;
wchar_t RegColorsHighlight[]=L"Colors\\Highlight";


string strGlobalSearchString;
int GlobalSearchCase=FALSE;
int GlobalSearchWholeWords=FALSE; // значение "Whole words" для поиска
int GlobalSearchHex=FALSE;     // значение "Search for hex" для поиска
int GlobalSearchReverse=FALSE;

int ScreenSaverActive=FALSE;

int CloseFAR=FALSE,CloseFARMenu=FALSE;

int CmpNameSearchMode=FALSE;
int DisablePluginsOutput=FALSE;
int CmdMode=FALSE;

int WidthNameForMessage=0;

const wchar_t DOS_EOL_fmt[]  = L"\r\n";
const wchar_t UNIX_EOL_fmt[] = L"\n";
const wchar_t MAC_EOL_fmt[]  = L"\r";
const wchar_t WIN_EOL_fmt[]  = L"\r\r\n";

BOOL ProcessException=FALSE;
BOOL ProcessShowClock=FALSE;

const wchar_t *FarTitleAddons=L" - Far";

const wchar_t *HelpFileMask=L"*.hlf";
const wchar_t *HelpFormatLinkModule=L"<%s>%s";

#if defined(SYSLOG)
BOOL StartSysLog=0;
long CallNewDelete=0;
long CallMallocFree=0;
#endif

class SaveScreen;
SaveScreen *GlobalSaveScrPtr=NULL;

int CriticalInternalError=FALSE;

#if defined(DETECT_ALT_ENTER)
int PrevFarAltEnterMode=-1;
#endif

int _localLastError=0;

const wchar_t *ReservedFilenameSymbols=L"<>|";

int KeepUserScreen;
string g_strDirToSet;

int Macro_DskShowPosType=0; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))

const wchar_t *FavoriteCodePagesKey=L"CodePages\\Favorites";

// Macro Const
const wchar_t constMsX[]=L"MsX";
const wchar_t constMsY[]=L"MsY";
const wchar_t constMsButton[]=L"MsButton";
const wchar_t constMsCtrlState[]=L"MsCtrlState";

DWORD RedrawTimeout=200;

SYSTEM_INFO SystemInfo;

string g_strMacroCONVFMT;

FormatScreen FS;
