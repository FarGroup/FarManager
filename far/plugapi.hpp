#ifndef PLUGAPI_HPP_2389ECC5_6302_4627_9495_F76642AA9B56
#define PLUGAPI_HPP_2389ECC5_6302_4627_9495_F76642AA9B56
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

// Internal:
#include "plugin.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

//----------- PLUGIN API/FSF ---------------------------------------------------
//все эти функции, за исключение sprintf/sscanf имеют тип вызова __stdcall

namespace pluginapi
{
	int      WINAPIV apiSprintf(wchar_t* Dest, const wchar_t* Format, ...) noexcept;
	int      WINAPIV apiSnprintf(wchar_t* Dest, size_t Count, const wchar_t* Format, ...) noexcept;
	int      WINAPIV apiSscanf(const wchar_t* Src, const wchar_t* Format, ...) noexcept;
	wchar_t* WINAPI apiItoa(int value, wchar_t *Str, int radix) noexcept;
	long long  WINAPI apiAtoi64(const wchar_t *Str) noexcept;
	wchar_t* WINAPI apiItoa64(long long value, wchar_t *Str, int radix) noexcept;
	int      WINAPI apiAtoi(const wchar_t *Str) noexcept;
	void     WINAPI apiQsort(void *base, size_t nelem, size_t width, int (WINAPI *fcmp)(const void *, const void *, void *), void *user) noexcept;
	void*    WINAPI apiBsearch(const void *key, const void *base, size_t nelem, size_t width, int (WINAPI *fcmp)(const void *, const void *, void *), void *user) noexcept;
	wchar_t* WINAPI apiQuoteSpace(wchar_t *Str) noexcept;
	wchar_t* WINAPI apiInsertQuote(wchar_t *Str) noexcept;
	void     WINAPI apiUnquote(wchar_t *Str) noexcept;
	wchar_t* WINAPI apiRemoveLeadingSpaces(wchar_t *Str) noexcept;
	wchar_t* WINAPI apiRemoveTrailingSpaces(wchar_t *Str) noexcept;
	wchar_t* WINAPI apiRemoveExternalSpaces(wchar_t *Str) noexcept;
	wchar_t* WINAPI apiQuoteSpaceOnly(wchar_t *Str) noexcept;
	void     WINAPI apiUpperBuf(wchar_t *Buf, intptr_t Length) noexcept;
	void     WINAPI apiLowerBuf(wchar_t *Buf, intptr_t Length) noexcept;
	void     WINAPI apiStrUpper(wchar_t *s1) noexcept;
	void     WINAPI apiStrLower(wchar_t *s1) noexcept;
	wchar_t  WINAPI apiUpper(wchar_t Ch) noexcept;
	wchar_t  WINAPI apiLower(wchar_t Ch) noexcept;
	int      WINAPI apiStrCmpNI(const wchar_t *Str1, const wchar_t *Str2, intptr_t MaxSize) noexcept;
	int      WINAPI apiStrCmpI(const wchar_t *Str1, const wchar_t *Str2) noexcept;
	int      WINAPI apiIsLower(wchar_t Ch) noexcept;
	int      WINAPI apiIsUpper(wchar_t Ch) noexcept;
	int      WINAPI apiIsAlpha(wchar_t Ch) noexcept;
	int      WINAPI apiIsAlphaNum(wchar_t Ch) noexcept;
	wchar_t* WINAPI apiTruncStr(wchar_t *Str, intptr_t MaxLength) noexcept;
	wchar_t* WINAPI apiTruncStrFromCenter(wchar_t *Str, intptr_t MaxLength) noexcept;
	wchar_t* WINAPI apiTruncStrFromEnd(wchar_t *Str, intptr_t MaxLength) noexcept;
	wchar_t* WINAPI apiTruncPathStr(wchar_t *Str, intptr_t MaxLength) noexcept;
	const wchar_t* WINAPI apiPointToName(const wchar_t* Path) noexcept;
	size_t   WINAPI apiGetFileOwner(const wchar_t *Computer, const wchar_t *Name, wchar_t *Owner, size_t Size) noexcept;
	size_t   WINAPI apiConvertPath(CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, size_t DestSize) noexcept;
	size_t   WINAPI apiGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest, size_t DestSize) noexcept;
	size_t   WINAPI apiGetNumberOfLinks(const wchar_t* Name) noexcept;
	size_t   WINAPI apiGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize) noexcept;
	BOOL     WINAPI apiCopyToClipboard(enum FARCLIPBOARD_TYPE Type, const wchar_t *Data) noexcept;
	size_t   WINAPI apiPasteFromClipboard(enum FARCLIPBOARD_TYPE Type, wchar_t *Data, size_t Length) noexcept;
	intptr_t WINAPI apiGetPluginDirList(const UUID* PluginId, HANDLE hPlugin, const wchar_t* Dir, PluginPanelItem** pPanelItem, size_t* pItemsNumber) noexcept;
	void     WINAPI apiFreePluginDirList(HANDLE hPlugin, PluginPanelItem *PanelItems, size_t ItemsNumber) noexcept;
	intptr_t WINAPI apiMenuFn(const UUID* PluginId, const UUID* Id, intptr_t X, intptr_t Y, intptr_t MaxHeight, unsigned long long Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const FarKey* BreakKeys, intptr_t* BreakCode, const FarMenuItem* Item, size_t ItemsNumber) noexcept;
	const wchar_t* WINAPI apiGetMsgFn(const UUID* PluginId, intptr_t MsgId) noexcept;
	intptr_t WINAPI apiMessageFn(const UUID* PluginId, const UUID* Id, unsigned long long Flags, const wchar_t* HelpTopic, const wchar_t* const* Items, size_t ItemsNumber, intptr_t ButtonsNumber) noexcept;
	intptr_t WINAPI apiPanelControl(HANDLE hPlugin, FILE_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	HANDLE   WINAPI apiSaveScreen(intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2) noexcept;
	void     WINAPI apiRestoreScreen(HANDLE hScreen) noexcept;
	void     WINAPI apiFreeScreen(HANDLE hScreen) noexcept;
	intptr_t WINAPI apiGetDirList(const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber) noexcept;
	void     WINAPI apiFreeDirList(PluginPanelItem *PanelItems, size_t ItemsNumber) noexcept;
	intptr_t WINAPI apiViewer(const wchar_t *FileName, const wchar_t *Title, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, unsigned long long Flags, uintptr_t CodePage) noexcept;
	intptr_t WINAPI apiEditor(const wchar_t* FileName, const wchar_t* Title, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, unsigned long long Flags, intptr_t StartLine, intptr_t StartChar, uintptr_t CodePage) noexcept;
	void     WINAPI apiText(intptr_t X, intptr_t Y, const FarColor* Color, const wchar_t *Str) noexcept;
	intptr_t WINAPI apiEditorControl(intptr_t EditorID, EDITOR_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiViewerControl(intptr_t ViewerID, VIEWER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	BOOL     WINAPI apiShowHelp(const wchar_t *ModuleName, const wchar_t *HelpTopic, FARHELPFLAGS Flags) noexcept;
	intptr_t WINAPI apiInputBox(const UUID* PluginId, const UUID* Id, const wchar_t* Title, const wchar_t* Prompt, const wchar_t* HistoryName, const wchar_t* SrcText, wchar_t* DestText, size_t DestSize, const wchar_t* HelpTopic, unsigned long long Flags) noexcept;
	intptr_t WINAPI apiAdvControl(const UUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	HANDLE   WINAPI apiDialogInit(const UUID* PluginId, const UUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, const wchar_t* HelpTopic, const FarDialogItem* Item, size_t ItemsNumber, intptr_t Reserved, unsigned long long Flags, FARWINDOWPROC DlgProc, void* Param) noexcept;
	intptr_t WINAPI apiDialogRun(HANDLE hDlg) noexcept;
	void     WINAPI apiDialogFree(HANDLE hDlg) noexcept;
	intptr_t WINAPI apiDefDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiSendDlgMessage(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiPluginsControl(HANDLE Handle, FAR_PLUGINS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiMacroControl(const UUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	intptr_t WINAPI apiSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept;
	size_t   WINAPI apiGetCurrentDirectory(size_t Size, wchar_t* Buffer) noexcept;
	size_t   WINAPI apiFormatFileSize(unsigned long long Size, intptr_t Width, FARFORMATFILESIZEFLAGS Flags, wchar_t *Dest, size_t DestSize) noexcept;
	void     WINAPI apiRecursiveSearch(const wchar_t *InitDir, const wchar_t *Mask, FRSUSERFUNC Func, unsigned long long flags, void *param) noexcept;
	size_t   WINAPI apiMkTemp(wchar_t *Dest, size_t DestSize, const wchar_t *Prefix) noexcept;
	size_t   WINAPI apiProcessName(const wchar_t *param1, wchar_t *param2, size_t size, PROCESSNAME_FLAGS flags) noexcept;
	BOOL     WINAPI apiColorDialog(const UUID* PluginId, COLORDIALOGFLAGS Flags, FarColor* Color) noexcept;
	size_t   WINAPI apiInputRecordToKeyName(const INPUT_RECORD* Key, wchar_t *KeyText, size_t Size) noexcept;
	BOOL     WINAPI apiKeyNameToInputRecord(const wchar_t *Name, INPUT_RECORD* RecKey) noexcept;
	BOOL     WINAPI apiMkLink(const wchar_t *Target, const wchar_t *LinkName, LINK_TYPE Type, MKLINK_FLAGS Flags) noexcept;
	BOOL     WINAPI apiAddEndSlash(wchar_t *Path) noexcept;
	wchar_t* WINAPI apiXlat(wchar_t *Line, intptr_t StartPos, intptr_t EndPos, XLAT_FLAGS Flags) noexcept;
	unsigned long long  WINAPI apiFarClock() noexcept;
	int      WINAPI apiCompareStrings(const wchar_t* Str1, size_t Size1, const wchar_t* Str2, size_t Size2) noexcept;
	//arclite
	HANDLE   WINAPI apiCreateFile(const wchar_t *Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile) noexcept;
	DWORD    WINAPI apiGetFileAttributes(const wchar_t *FileName) noexcept;
	BOOL     WINAPI apiSetFileAttributes(const wchar_t *FileName, DWORD dwFileAttributes) noexcept;
	BOOL     WINAPI apiMoveFileEx(const wchar_t *ExistingFileName, const wchar_t *NewFileName, DWORD dwFlags) noexcept;
	BOOL     WINAPI apiDeleteFile(const wchar_t *FileName) noexcept;
	BOOL     WINAPI apiRemoveDirectory(const wchar_t *DirName) noexcept;
	BOOL     WINAPI apiCreateDirectory(const wchar_t *PathName, LPSECURITY_ATTRIBUTES SecurityAttributes) noexcept;
	//luamacro
	intptr_t WINAPI apiCallFar(intptr_t CheckCode, FarMacroCall* Data) noexcept;
}

void CreatePluginStartupInfo(PluginStartupInfo* PSI, FarStandardFunctions* FSF);

#endif // PLUGAPI_HPP_2389ECC5_6302_4627_9495_F76642AA9B56
