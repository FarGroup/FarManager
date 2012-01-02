#pragma once

/*
plugapi.hpp

API, доступное плагинам (диалоги, меню, ...)
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

#include "plugin.hpp"

//----------- PLUGIN API/FSF ---------------------------------------------------
//все эти функции, за исключение sprintf/sscanf имеют тип вызова __stdcall

wchar_t *WINAPI FarItoa(int value, wchar_t *string, int radix);
__int64 WINAPI FarAtoi64(const wchar_t *s);
wchar_t *WINAPI FarItoa64(__int64 value, wchar_t *string, int radix);
int WINAPI FarAtoi(const wchar_t *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *),void*);
void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));

void WINAPI DeleteBuffer(void* Buffer);

void __stdcall farUpperBuf(wchar_t *Buf, int Length);
void __stdcall farLowerBuf(wchar_t *Buf, int Length);
void __stdcall farStrUpper(wchar_t *s1);
void __stdcall farStrLower(wchar_t *s1);
wchar_t __stdcall farUpper(wchar_t Ch);
wchar_t __stdcall farLower(wchar_t Ch);
int __stdcall farStrCmpNI(const wchar_t *s1, const wchar_t *s2, int n);
int __stdcall farStrCmpI(const wchar_t *s1, const wchar_t *s2);
int __stdcall farIsLower(wchar_t Ch);
int __stdcall farIsUpper(wchar_t Ch);
int __stdcall farIsAlpha(wchar_t Ch);
int __stdcall farIsAlphaNum(wchar_t Ch);

size_t WINAPI farGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner,size_t Size);

size_t WINAPI farConvertPath(CONVERTPATHMODES Mode,const wchar_t *Src,wchar_t *Dest,size_t DestSize);

size_t WINAPI farGetReparsePointInfo(const wchar_t *Src,wchar_t *Dest,size_t DestSize);

size_t WINAPI farGetNumberOfLinks(const wchar_t* Name);

size_t WINAPI farGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize);

int WINAPI FarGetPluginDirList(INT_PTR PluginNumber,HANDLE hPlugin,
                               const wchar_t *Dir,struct PluginPanelItem **pPanelItem,
                               size_t *pItemsNumber);
void WINAPI FarFreePluginDirList(PluginPanelItem *PanelItem, size_t ItemsNumber);

int WINAPI FarMenuFn(INT_PTR PluginNumber,const GUID* Id,int X,int Y,int MaxHeight,
                     unsigned __int64 Flags,const wchar_t *Title,const wchar_t *Bottom,
                     const wchar_t *HelpTopic,const FarKey *BreakKeys,int *BreakCode,
                     const struct FarMenuItem *Item, size_t ItemsNumber);
const wchar_t* WINAPI FarGetMsgFn(INT_PTR PluginHandle,int MsgId);
int WINAPI FarMessageFn(INT_PTR PluginNumber,const GUID* Id,unsigned __int64 Flags,
                        const wchar_t *HelpTopic,const wchar_t * const *Items,size_t ItemsNumber,
                        int ButtonsNumber);
INT_PTR WINAPI FarPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,int Param1,void* Param2);
HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2);
void WINAPI FarRestoreScreen(HANDLE hScreen);

int WINAPI FarGetDirList(const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber);
void WINAPI FarFreeDirList(PluginPanelItem *PanelItem, size_t nItemsNumber);

int WINAPI FarViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2,int Y2,unsigned __int64 Flags, UINT CodePage);


int WINAPI FarEditor(const wchar_t* FileName, const wchar_t* Title,
	int X1, int Y1, int X2, int Y2, unsigned __int64 Flags, int StartLine, int StartChar, UINT CodePage);

void WINAPI FarText(int X,int Y,const FarColor* Color,const wchar_t *Str);

INT_PTR WINAPI FarEditorControl(int EditorID, EDITOR_CONTROL_COMMANDS Command, int Param1, void* Param2);

INT_PTR WINAPI FarViewerControl(int ViewerID, VIEWER_CONTROL_COMMANDS Command, int Param1, void* Param2);

/* Функция вывода помощи */
BOOL WINAPI FarShowHelp(const wchar_t *ModuleName,
                        const wchar_t *HelpTopic, FARHELPFLAGS Flags);

/* Обертка вокруг GetString для плагинов - с меньшей функциональностью.
   Сделано для того, чтобы не дублировать код GetString.*/

int WINAPI FarInputBox(INT_PTR PluginNumber,const GUID* Id,const wchar_t *Title,const wchar_t *Prompt,
                       const wchar_t *HistoryName,const wchar_t *SrcText,
                       wchar_t *DestText, size_t DestSize,
                       const wchar_t *HelpTopic,unsigned __int64 Flags);
/* Функция, которая будет действовать и в редакторе, и в панелях, и... */
INT_PTR WINAPI FarAdvControl(INT_PTR ModuleNumber, ADVANCED_CONTROL_COMMANDS Command, int Param1, void* Param2);
//  Функция расширенного диалога
HANDLE WINAPI FarDialogInit(INT_PTR PluginNumber, const GUID* Id, int X1, int Y1, int X2, int Y2,
                            const wchar_t *HelpTopic, const struct FarDialogItem *Item,
                           size_t ItemsNumber, DWORD Reserved, unsigned __int64 Flags,
                            FARWINDOWPROC Proc, void* Param);
int WINAPI FarDialogRun(HANDLE hDlg);
void WINAPI FarDialogFree(HANDLE hDlg);
//  Функция обработки диалога по умолчанию
INT_PTR WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2);
// Посылка сообщения диалогу
INT_PTR WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1, void* Param2);

INT_PTR WINAPI farPluginsControl(HANDLE hHandle, FAR_PLUGINS_CONTROL_COMMANDS Command, int Param1, void* Param2);

INT_PTR WINAPI farFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, int Param1, void* Param2);

INT_PTR WINAPI farRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, int Param1, void* Param2);

INT_PTR WINAPI farMacroControl(const GUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, int Param1, void* Param2);

INT_PTR WINAPI farSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, int Param1, void* Param2);

size_t WINAPI farGetCurrentDirectory(size_t Size,wchar_t* Buffer);
