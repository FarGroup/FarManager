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
class Plugin;

//----------- PLUGIN API/FSF ---------------------------------------------------
//все эти функции, за исключение sprintf/sscanf имеют тип вызова __stdcall

namespace pluginapi
{
	intptr_t WINAPIV apiSprintf(wchar_t* Dest, const wchar_t* Format, ...);
	intptr_t WINAPIV apiSnprintf(wchar_t* Dest, size_t Count, const wchar_t* Format, ...);
#ifndef _MSC_VER
	intptr_t WINAPIV apiSscanf(const wchar_t* Src, const wchar_t* Format, ...);
#endif
	wchar_t* WINAPI apiItoa(intptr_t value, wchar_t *string, intptr_t radix);
	__int64  WINAPI apiAtoi64(const wchar_t *s);
	wchar_t* WINAPI apiItoa64(__int64 value, wchar_t *string, intptr_t radix);
	intptr_t WINAPI apiAtoi(const wchar_t *s);
	void     WINAPI apiQsort(void *base, size_t nelem, size_t width, intptr_t (WINAPI *fcmp)(const void *, const void *,void *),void *user);
	void*    WINAPI apiBsearch(const void *key, const void *base, size_t nelem, size_t width, intptr_t (WINAPI *fcmp)(const void *, const void *, void *),void *user);
	wchar_t* WINAPI apiQuoteSpace(wchar_t *Str);
	wchar_t* WINAPI apiInsertQuote(wchar_t *Str);
	void     WINAPI apiUnquote(wchar_t *Str);
	wchar_t* WINAPI apiRemoveLeadingSpaces(wchar_t *Str);
	wchar_t* WINAPI apiRemoveTrailingSpaces(wchar_t *Str);
	wchar_t* WINAPI apiRemoveExternalSpaces(wchar_t *Str);
	wchar_t* WINAPI apiQuoteSpaceOnly(wchar_t *Str);
	void     WINAPI apiDeleteBuffer(void* Buffer);
	void     WINAPI apiUpperBuf(wchar_t *Buf, intptr_t Length);
	void     WINAPI apiLowerBuf(wchar_t *Buf, intptr_t Length);
	void     WINAPI apiStrUpper(wchar_t *s1);
	void     WINAPI apiStrLower(wchar_t *s1);
	wchar_t  WINAPI apiUpper(wchar_t Ch);
	wchar_t  WINAPI apiLower(wchar_t Ch);
	intptr_t WINAPI apiStrCmpNI(const wchar_t *s1, const wchar_t *s2, intptr_t n);
	intptr_t WINAPI apiStrCmpI(const wchar_t *s1, const wchar_t *s2);
	intptr_t WINAPI apiIsLower(wchar_t Ch);
	intptr_t WINAPI apiIsUpper(wchar_t Ch);
	intptr_t WINAPI apiIsAlpha(wchar_t Ch);
	intptr_t WINAPI apiIsAlphaNum(wchar_t Ch);
	wchar_t* WINAPI apiTruncStr(wchar_t *Str,intptr_t MaxLength);
	wchar_t* WINAPI apiTruncStrFromCenter(wchar_t *Str, intptr_t MaxLength);
	wchar_t* WINAPI apiTruncStrFromEnd(wchar_t *Str,intptr_t MaxLength);
	wchar_t* WINAPI apiTruncPathStr(wchar_t *Str, intptr_t MaxLength);
	LPCWSTR  WINAPI apiPointToName(const wchar_t *lpwszPath);
	size_t   WINAPI apiGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner,size_t Size);
	size_t   WINAPI apiConvertPath(CONVERTPATHMODES Mode,const wchar_t *Src,wchar_t *Dest,size_t DestSize);
	size_t   WINAPI apiGetReparsePointInfo(const wchar_t *Src,wchar_t *Dest,size_t DestSize);
	size_t   WINAPI apiGetNumberOfLinks(const wchar_t* Name);
	size_t   WINAPI apiGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize);
	BOOL     WINAPI apiCopyToClipboard(enum FARCLIPBOARD_TYPE Type, const wchar_t *Data);
	size_t   WINAPI apiPasteFromClipboard(enum FARCLIPBOARD_TYPE Type, wchar_t *Data, size_t Length);
	intptr_t WINAPI apiGetPluginDirList(const GUID* PluginId,HANDLE hPlugin,const wchar_t *Dir,struct PluginPanelItem **pPanelItem,size_t *pItemsNumber);
	void     WINAPI apiFreePluginDirList(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber);
	intptr_t WINAPI apiMenuFn(const GUID* PluginId,const GUID* Id,intptr_t X,intptr_t Y,intptr_t MaxHeight,unsigned __int64 Flags,const wchar_t *Title,const wchar_t *Bottom,const wchar_t *HelpTopic,const FarKey *BreakKeys,intptr_t *BreakCode,const struct FarMenuItem *Item, size_t ItemsNumber);
	LPCWSTR  WINAPI apiGetMsgFn(const GUID* PluginId,intptr_t MsgId);
	intptr_t WINAPI apiMessageFn(const GUID* PluginId,const GUID* Id,unsigned __int64 Flags, const wchar_t *HelpTopic,const wchar_t * const *Items,size_t ItemsNumber,intptr_t ButtonsNumber);
	intptr_t WINAPI apiPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,intptr_t Param1,void* Param2);
	HANDLE   WINAPI apiSaveScreen(intptr_t X1,intptr_t Y1,intptr_t X2,intptr_t Y2);
	void     WINAPI apiRestoreScreen(HANDLE hScreen);
	intptr_t WINAPI apiGetDirList(const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber);
	void     WINAPI apiFreeDirList(PluginPanelItem *PanelItem, size_t nItemsNumber);
	intptr_t WINAPI apiViewer(const wchar_t *FileName,const wchar_t *Title,intptr_t X1,intptr_t Y1,intptr_t X2,intptr_t Y2,unsigned __int64 Flags, uintptr_t CodePage);
	intptr_t WINAPI apiEditor(const wchar_t* FileName, const wchar_t* Title,intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, unsigned __int64 Flags, intptr_t StartLine, intptr_t StartChar, uintptr_t CodePage);
	void     WINAPI apiText(intptr_t X,intptr_t Y,const FarColor* Color,const wchar_t *Str);
	intptr_t WINAPI apiEditorControl(intptr_t EditorID, EDITOR_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	intptr_t WINAPI apiViewerControl(intptr_t ViewerID, VIEWER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	BOOL     WINAPI apiShowHelp(const wchar_t *ModuleName,const wchar_t *HelpTopic, FARHELPFLAGS Flags);
	intptr_t WINAPI apiInputBox(const GUID* PluginId,const GUID* Id,const wchar_t *Title,const wchar_t *Prompt,const wchar_t *HistoryName,const wchar_t *SrcText,wchar_t *DestText, size_t DestSize,const wchar_t *HelpTopic,unsigned __int64 Flags);
	intptr_t WINAPI apiAdvControl(const GUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	HANDLE   WINAPI apiDialogInit(const GUID* PluginId, const GUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2,const wchar_t *HelpTopic, const struct FarDialogItem *Item,size_t ItemsNumber, intptr_t Reserved, unsigned __int64 Flags,FARWINDOWPROC Proc, void* Param);
	intptr_t WINAPI apiDialogRun(HANDLE hDlg);
	void     WINAPI apiDialogFree(HANDLE hDlg);
	intptr_t WINAPI apiDefDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t WINAPI apiSendDlgMessage(HANDLE hDlg,intptr_t Msg,intptr_t Param1, void* Param2);
	intptr_t WINAPI apiPluginsControl(HANDLE hHandle, FAR_PLUGINS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	intptr_t WINAPI apiFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	intptr_t WINAPI apiRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	intptr_t WINAPI apiMacroControl(const GUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	intptr_t WINAPI apiSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2);
	size_t   WINAPI apiGetCurrentDirectory(size_t Size,wchar_t* Buffer);
	size_t   WINAPI apiFormatFileSize(unsigned __int64 Size, intptr_t Width, FARFORMATFILESIZEFLAGS ViewFlags, wchar_t *Dest, size_t DestSize);
	typedef intptr_t (WINAPI *FRSUSERFUNC)(const PluginPanelItem *FData,const wchar_t *FullName,void *param);
	void     WINAPI apiRecursiveSearch(const wchar_t *initdir,const wchar_t *mask,FRSUSERFUNC func,unsigned __int64 flags,void *param);
	size_t   WINAPI apiMkTemp(wchar_t *Dest, size_t size, const wchar_t *Prefix);
	size_t   WINAPI apiProcessName(const wchar_t *param1, wchar_t *param2, size_t size, PROCESSNAME_FLAGS flags);
	BOOL     WINAPI apiColorDialog(const GUID* PluginId, COLORDIALOGFLAGS Flags, struct FarColor *Color);
	size_t   WINAPI apiInputRecordToKeyName(const INPUT_RECORD* Key, wchar_t *KeyText, size_t Size);
	BOOL     WINAPI apiKeyNameToInputRecord(const wchar_t *Name,INPUT_RECORD* RecKey);
	BOOL     WINAPI apiMkLink(const wchar_t *Src,const wchar_t *Dest, LINK_TYPE Type, MKLINK_FLAGS Flags);
	BOOL     WINAPI apiAddEndSlash(wchar_t *Path);
	wchar_t* WINAPI apiXlat(wchar_t *Line,intptr_t StartPos,intptr_t EndPos,XLAT_FLAGS Flags);
	//arclite
	HANDLE   WINAPI apiCreateFile(const wchar_t *Object,DWORD DesiredAccess,DWORD ShareMode,LPSECURITY_ATTRIBUTES SecurityAttributes,DWORD CreationDistribution,DWORD FlagsAndAttributes,HANDLE TemplateFile);
	DWORD    WINAPI apiGetFileAttributes(const wchar_t *FileName);
	BOOL     WINAPI apiSetFileAttributes(const wchar_t *FileName,DWORD dwFileAttributes);
	BOOL     WINAPI apiMoveFileEx(const wchar_t *ExistingFileName,const wchar_t *NewFileName,DWORD dwFlags);
	BOOL     WINAPI apiDeleteFile(const wchar_t *FileName);
	BOOL     WINAPI apiRemoveDirectory(const wchar_t *DirName);
	BOOL     WINAPI apiCreateDirectory(const wchar_t *PathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes);
};
