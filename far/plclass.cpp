/*
plclass.cpp

*/
/*
Copyright © 2011 Far Group
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

#include "plclass.hpp"
#include "plugins.hpp"
#include "pathmix.hpp"
#include "lasterror.hpp"
#include "config.hpp"
#include "farexcpt.hpp"
#include "chgprior.hpp"
#include "scrbuf.hpp"
#include "ctrlobj.hpp"
#include "farversion.hpp"
#include "plugapi.hpp"
#include "message.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "FarGuid.hpp"
#include "manager.hpp"
#include "console.hpp"
#include "plugsettings.hpp"
#include "savescr.hpp"

typedef void     (WINAPI *iClosePanelPrototype)          (const ClosePanelInfo *Info);
typedef intptr_t (WINAPI *iComparePrototype)             (const CompareInfo *Info);
typedef intptr_t (WINAPI *iConfigurePrototype)           (const ConfigureInfo *Info);
typedef intptr_t (WINAPI *iDeleteFilesPrototype)         (const DeleteFilesInfo *Info);
typedef void     (WINAPI *iExitFARPrototype)             (const ExitInfo *Info);
typedef void     (WINAPI *iFreeFindDataPrototype)        (const FreeFindDataInfo *Info);
typedef void     (WINAPI *iFreeVirtualFindDataPrototype) (const FreeFindDataInfo *Info);
typedef intptr_t (WINAPI *iGetFilesPrototype)            (GetFilesInfo *Info);
typedef intptr_t (WINAPI *iGetFindDataPrototype)         (GetFindDataInfo *Info);
typedef void     (WINAPI *iGetGlobalInfoPrototype)       (GlobalInfo *Info);
typedef void     (WINAPI *iGetOpenPanelInfoPrototype)    (OpenPanelInfo *Info);
typedef void     (WINAPI *iGetPluginInfoPrototype)       (PluginInfo *Info);
typedef intptr_t (WINAPI *iGetVirtualFindDataPrototype)  (GetVirtualFindDataInfo *Info);
typedef intptr_t (WINAPI *iMakeDirectoryPrototype)       (MakeDirectoryInfo *Info);
typedef HANDLE   (WINAPI *iOpenPrototype)                (const OpenInfo *Info);
typedef intptr_t (WINAPI *iProcessEditorEventPrototype)  (const ProcessEditorEventInfo *Info);
typedef intptr_t (WINAPI *iProcessEditorInputPrototype)  (const ProcessEditorInputInfo *Info);
typedef intptr_t (WINAPI *iProcessPanelEventPrototype)   (const ProcessPanelEventInfo *Info);
typedef intptr_t (WINAPI *iProcessHostFilePrototype)     (const ProcessHostFileInfo *Info);
typedef intptr_t (WINAPI *iProcessPanelInputPrototype)   (const ProcessPanelInputInfo *Info);
typedef intptr_t (WINAPI *iPutFilesPrototype)            (const PutFilesInfo *Info);
typedef intptr_t (WINAPI *iSetDirectoryPrototype)        (const SetDirectoryInfo *Info);
typedef intptr_t (WINAPI *iSetFindListPrototype)         (const SetFindListInfo *Info);
typedef void     (WINAPI *iSetStartupInfoPrototype)      (const PluginStartupInfo *Info);
typedef intptr_t (WINAPI *iProcessViewerEventPrototype)  (const ProcessViewerEventInfo *Info);
typedef intptr_t (WINAPI *iProcessDialogEventPrototype)  (const ProcessDialogEventInfo *Info);
typedef intptr_t (WINAPI *iProcessSynchroEventPrototype) (const ProcessSynchroEventInfo *Info);
#if defined(MANTIS_0000466)
typedef intptr_t (WINAPI *iProcessMacroPrototype)        (const ProcessMacroInfo *Info);
#endif
typedef intptr_t (WINAPI *iProcessConsoleInputPrototype) (const ProcessConsoleInputInfo *Info);
typedef HANDLE   (WINAPI *iAnalysePrototype)             (const AnalyseInfo *Info);
typedef intptr_t (WINAPI *iGetCustomDataPrototype)       (const wchar_t *FilePath, wchar_t **CustomData);
typedef void     (WINAPI *iFreeCustomDataPrototype)      (wchar_t *CustomData);
typedef void     (WINAPI *iCloseAnalysePrototype)        (const CloseAnalyseInfo *Info);


#define EXP_GETGLOBALINFO       "GetGlobalInfoW"
#define EXP_SETSTARTUPINFO      "SetStartupInfoW"
#define EXP_OPEN                "OpenW"
#define EXP_CLOSEPANEL          "ClosePanelW"
#define EXP_GETPLUGININFO       "GetPluginInfoW"
#define EXP_GETOPENPANELINFO    "GetOpenPanelInfoW"
#define EXP_GETFINDDATA         "GetFindDataW"
#define EXP_FREEFINDDATA        "FreeFindDataW"
#define EXP_GETVIRTUALFINDDATA  "GetVirtualFindDataW"
#define EXP_FREEVIRTUALFINDDATA "FreeVirtualFindDataW"
#define EXP_SETDIRECTORY        "SetDirectoryW"
#define EXP_GETFILES            "GetFilesW"
#define EXP_PUTFILES            "PutFilesW"
#define EXP_DELETEFILES         "DeleteFilesW"
#define EXP_MAKEDIRECTORY       "MakeDirectoryW"
#define EXP_PROCESSHOSTFILE     "ProcessHostFileW"
#define EXP_SETFINDLIST         "SetFindListW"
#define EXP_CONFIGURE           "ConfigureW"
#define EXP_EXITFAR             "ExitFARW"
#define EXP_PROCESSPANELINPUT   "ProcessPanelInputW"
#define EXP_PROCESSPANELEVENT   "ProcessPanelEventW"
#define EXP_PROCESSEDITOREVENT  "ProcessEditorEventW"
#define EXP_COMPARE             "CompareW"
#define EXP_PROCESSEDITORINPUT  "ProcessEditorInputW"
#define EXP_PROCESSVIEWEREVENT  "ProcessViewerEventW"
#define EXP_PROCESSDIALOGEVENT  "ProcessDialogEventW"
#define EXP_PROCESSSYNCHROEVENT "ProcessSynchroEventW"
#if defined(MANTIS_0000466)
#define EXP_PROCESSMACRO        "ProcessMacroW"
#endif
#define EXP_PROCESSCONSOLEINPUT "ProcessConsoleInputW"
#define EXP_ANALYSE             "AnalyseW"
#define EXP_GETCUSTOMDATA       "GetCustomDataW"
#define EXP_FREECUSTOMDATA      "FreeCustomDataW"
#define EXP_CLOSEANALYSE        "CloseAnalyseW"

#define EXP_OPENFILEPLUGIN      ""
#define EXP_GETMINFARVERSION    ""


static const char* _ExportsNamesA[i_LAST] =
{
	EXP_GETGLOBALINFO,
	EXP_SETSTARTUPINFO,
	EXP_OPEN,
	EXP_CLOSEPANEL,
	EXP_GETPLUGININFO,
	EXP_GETOPENPANELINFO,
	EXP_GETFINDDATA,
	EXP_FREEFINDDATA,
	EXP_GETVIRTUALFINDDATA,
	EXP_FREEVIRTUALFINDDATA,
	EXP_SETDIRECTORY,
	EXP_GETFILES,
	EXP_PUTFILES,
	EXP_DELETEFILES,
	EXP_MAKEDIRECTORY,
	EXP_PROCESSHOSTFILE,
	EXP_SETFINDLIST,
	EXP_CONFIGURE,
	EXP_EXITFAR,
	EXP_PROCESSPANELINPUT,
	EXP_PROCESSPANELEVENT,
	EXP_PROCESSEDITOREVENT,
	EXP_COMPARE,
	EXP_PROCESSEDITORINPUT,
	EXP_PROCESSVIEWEREVENT,
	EXP_PROCESSDIALOGEVENT,
	EXP_PROCESSSYNCHROEVENT,
#if defined(MANTIS_0000466)
	EXP_PROCESSMACRO,
#endif
	EXP_PROCESSCONSOLEINPUT,
	EXP_ANALYSE,
	EXP_GETCUSTOMDATA,
	EXP_FREECUSTOMDATA,
	EXP_CLOSEANALYSE,

	EXP_OPENFILEPLUGIN,
	EXP_GETMINFARVERSION,
};


static const wchar_t* _ExportsNamesW[i_LAST] =
{
	W(EXP_GETGLOBALINFO),
	W(EXP_SETSTARTUPINFO),
	W(EXP_OPEN),
	W(EXP_CLOSEPANEL),
	W(EXP_GETPLUGININFO),
	W(EXP_GETOPENPANELINFO),
	W(EXP_GETFINDDATA),
	W(EXP_FREEFINDDATA),
	W(EXP_GETVIRTUALFINDDATA),
	W(EXP_FREEVIRTUALFINDDATA),
	W(EXP_SETDIRECTORY),
	W(EXP_GETFILES),
	W(EXP_PUTFILES),
	W(EXP_DELETEFILES),
	W(EXP_MAKEDIRECTORY),
	W(EXP_PROCESSHOSTFILE),
	W(EXP_SETFINDLIST),
	W(EXP_CONFIGURE),
	W(EXP_EXITFAR),
	W(EXP_PROCESSPANELINPUT),
	W(EXP_PROCESSPANELEVENT),
	W(EXP_PROCESSEDITOREVENT),
	W(EXP_COMPARE),
	W(EXP_PROCESSEDITORINPUT),
	W(EXP_PROCESSVIEWEREVENT),
	W(EXP_PROCESSDIALOGEVENT),
	W(EXP_PROCESSSYNCHROEVENT),
#if defined(MANTIS_0000466)
	W(EXP_PROCESSMACRO),
#endif
	W(EXP_PROCESSCONSOLEINPUT),
	W(EXP_ANALYSE),
	W(EXP_GETCUSTOMDATA),
	W(EXP_FREECUSTOMDATA),
	W(EXP_CLOSEANALYSE),

	W(EXP_OPENFILEPLUGIN),
	W(EXP_GETMINFARVERSION),
};

static BOOL PrepareModulePath(const wchar_t *ModuleName)
{
	string strModulePath;
	strModulePath = ModuleName;
	CutToSlash(strModulePath); //??
	return FarChDir(strModulePath);
}

static void CheckScreenLock()
{
	if (Global->ScrBuf->GetLockCount() > 0 && !Global->CtrlObject->Macro.PeekKey())
	{
//		Global->ScrBuf->SetLockCount(0);
		Global->ScrBuf->Flush();
	}
}

FarStandardFunctions NativeFSF =
{
	sizeof(NativeFSF),
	pluginapi::apiAtoi,
	pluginapi::apiAtoi64,
	pluginapi::apiItoa,
	pluginapi::apiItoa64,
	pluginapi::apiSprintf,
#ifndef _MSC_VER
	pluginapi::apiSscanf,
#else
	swscanf,
#endif
	pluginapi::apiQsort,
	pluginapi::apiBsearch,
	pluginapi::apiSnprintf,
	pluginapi::apiIsLower,
	pluginapi::apiIsUpper,
	pluginapi::apiIsAlpha,
	pluginapi::apiIsAlphaNum,
	pluginapi::apiUpper,
	pluginapi::apiLower,
	pluginapi::apiUpperBuf,
	pluginapi::apiLowerBuf,
	pluginapi::apiStrUpper,
	pluginapi::apiStrLower,
	pluginapi::apiStrCmpI,
	pluginapi::apiStrCmpNI,
	pluginapi::apiUnquote,
	pluginapi::apiRemoveLeadingSpaces,
	pluginapi::apiRemoveTrailingSpaces,
	pluginapi::apiRemoveExternalSpaces,
	pluginapi::apiTruncStr,
	pluginapi::apiTruncPathStr,
	pluginapi::apiQuoteSpaceOnly,
	pluginapi::apiPointToName,
	pluginapi::apiGetPathRoot,
	pluginapi::apiAddEndSlash,
	pluginapi::apiCopyToClipboard,
	pluginapi::apiPasteFromClipboard,
	pluginapi::apiInputRecordToKeyName,
	pluginapi::apiKeyNameToInputRecord,
	pluginapi::apiXlat,
	pluginapi::apiGetFileOwner,
	pluginapi::apiGetNumberOfLinks,
	pluginapi::apiRecursiveSearch,
	pluginapi::apiMkTemp,
	pluginapi::apiProcessName,
	pluginapi::apiMkLink,
	pluginapi::apiConvertPath,
	pluginapi::apiGetReparsePointInfo,
	pluginapi::apiGetCurrentDirectory,
	pluginapi::apiFormatFileSize,
};

PluginStartupInfo NativeInfo =
{
	sizeof(NativeInfo),
	nullptr, //ModuleName, dynamic
	pluginapi::apiMenuFn,
	pluginapi::apiMessageFn,
	pluginapi::apiGetMsgFn,
	pluginapi::apiPanelControl,
	pluginapi::apiSaveScreen,
	pluginapi::apiRestoreScreen,
	pluginapi::apiGetDirList,
	pluginapi::apiGetPluginDirList,
	pluginapi::apiFreeDirList,
	pluginapi::apiFreePluginDirList,
	pluginapi::apiViewer,
	pluginapi::apiEditor,
	pluginapi::apiText,
	pluginapi::apiEditorControl,
	nullptr, // FSF, dynamic
	pluginapi::apiShowHelp,
	pluginapi::apiAdvControl,
	pluginapi::apiInputBox,
	pluginapi::apiColorDialog,
	pluginapi::apiDialogInit,
	pluginapi::apiDialogRun,
	pluginapi::apiDialogFree,
	pluginapi::apiSendDlgMessage,
	pluginapi::apiDefDlgProc,
	pluginapi::apiViewerControl,
	pluginapi::apiPluginsControl,
	pluginapi::apiFileFilterControl,
	pluginapi::apiRegExpControl,
	pluginapi::apiMacroControl,
	pluginapi::apiSettingsControl,
	nullptr, //Private, dynamic
};

ArclitePrivateInfo ArcliteInfo =
{
	sizeof(ArcliteInfo),
	pluginapi::apiCreateFile,
	pluginapi::apiGetFileAttributes,
	pluginapi::apiSetFileAttributes,
	pluginapi::apiMoveFileEx,
	pluginapi::apiDeleteFile,
	pluginapi::apiRemoveDirectory,
	pluginapi::apiCreateDirectory
};

MacroPrivateInfo MacroInfo =
{
	sizeof(MacroPrivateInfo),
	pluginapi::apiCallFar
};

void CreatePluginStartupInfo(const Plugin* pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF)
{
	*PSI=NativeInfo;
	*FSF=NativeFSF;
	PSI->FSF=FSF;

	if (pPlugin)
	{
		PSI->ModuleName = pPlugin->GetModuleName().CPtr();
		if(pPlugin->GetGUID() == ArcliteGuid)
		{
			PSI->Private = &ArcliteInfo;
		}
		else if(pPlugin->GetGUID() == LuamacroGuid)
		{
			PSI->Private = &MacroInfo;
		}
	}
}

static void ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,const VersionInfo& required)
{
	Message(MSG_WARNING|MSG_NOPLUGINS, 1,
		MSG(MError),
		MSG(MPlgBadVers),
		plg,
		LangString(MPlgRequired) << (FormatString() << required.Major << L'.' << required.Minor << L'.' << required.Revision << L'.' << required.Build),
		LangString(MPlgRequired2) << (FormatString() << FAR_VERSION.Major << L'.' << FAR_VERSION.Minor << L'.' << FAR_VERSION.Revision << L'.' << FAR_VERSION.Build),
		MSG(MOk));
}

bool Plugin::SaveToCache()
{
	if (Exports[iGetGlobalInfo] ||
		Exports[iGetPluginInfo] ||
		Exports[iOpen] ||
		Exports[iSetFindList] ||
		Exports[iProcessEditorInput] ||
		Exports[iProcessEditorEvent] ||
		Exports[iProcessViewerEvent] ||
		Exports[iProcessDialogEvent] ||
		Exports[iProcessSynchroEvent] ||
#if defined(MANTIS_0000466)
		Exports[iProcessMacro] ||
#endif
		Exports[iProcessConsoleInput] ||
		Exports[iAnalyse] ||
		Exports[iGetCustomData]
	)
	{
		PluginInfo Info = {sizeof(Info)};
		GetPluginInfo(&Info);

		PluginsCacheConfig& PlCache = *Global->Db->PlCacheCfg();

		PlCache.BeginTransaction();

		PlCache.DeleteCache(m_strCacheName);
		unsigned __int64 id = PlCache.CreateCache(m_strCacheName);

		{
			bool bPreload = (Info.Flags & PF_PRELOAD);
			PlCache.SetPreload(id, bPreload);
			WorkFlags.Change(PIWF_PRELOADED, bPreload);

			if (bPreload)
			{
				PlCache.EndTransaction();
				return true;
			}
		}

		{
			string strCurPluginID;
			FAR_FIND_DATA_EX fdata;
			apiGetFindDataEx(m_strModuleName, fdata);
			strCurPluginID.Format(
				L"%I64x%x%x",
				fdata.nFileSize,
				fdata.ftCreationTime.dwLowDateTime,
				fdata.ftLastWriteTime.dwLowDateTime
				);
			PlCache.SetSignature(id, strCurPluginID);
		}

		for (size_t i = 0; i < Info.DiskMenu.Count; i++)
		{
			PlCache.SetDiskMenuItem(id, i, Info.DiskMenu.Strings[i], GuidToStr(Info.DiskMenu.Guids[i]));
		}

		for (size_t i = 0; i < Info.PluginMenu.Count; i++)
		{
			PlCache.SetPluginsMenuItem(id, i, Info.PluginMenu.Strings[i], GuidToStr(Info.PluginMenu.Guids[i]));
		}

		for (size_t i = 0; i < Info.PluginConfig.Count; i++)
		{
			PlCache.SetPluginsConfigMenuItem(id, i, Info.PluginConfig.Strings[i], GuidToStr(Info.PluginConfig.Guids[i]));
		}

		PlCache.SetCommandPrefix(id, NullToEmpty(Info.CommandPrefix));
#if defined(MANTIS_0000466)
		PlCache.SetMacroFunctions(id, NullToEmpty(Info.MacroFunctions));
#endif
		PlCache.SetFlags(id, Info.Flags);

		PlCache.SetMinFarVersion(id, &MinFarVersion);
		PlCache.SetGuid(id, m_strGuid);
		PlCache.SetVersion(id, &PluginVersion);
		PlCache.SetTitle(id, strTitle);
		PlCache.SetDescription(id, strDescription);
		PlCache.SetAuthor(id, strAuthor);

#define OPT_SETEXPORT(i) if (*ExportsNamesW[i]) PlCache.SetExport(id, ExportsNamesW[i], Exports[i]!=nullptr)

		OPT_SETEXPORT(iOpen);
		OPT_SETEXPORT(iOpenFilePlugin);
		OPT_SETEXPORT(iSetFindList);
		OPT_SETEXPORT(iProcessEditorInput);
		OPT_SETEXPORT(iProcessEditorEvent);
		OPT_SETEXPORT(iProcessViewerEvent);
		OPT_SETEXPORT(iProcessDialogEvent);
		OPT_SETEXPORT(iProcessSynchroEvent);
#if defined(MANTIS_0000466)
		OPT_SETEXPORT(iProcessMacro);
#endif
		OPT_SETEXPORT(iProcessConsoleInput);
		OPT_SETEXPORT(iConfigure);
		OPT_SETEXPORT(iAnalyse);
		OPT_SETEXPORT(iGetCustomData);

		PlCache.EndTransaction();

		return true;
	}

	return false;
}

void Plugin::InitExports()
{
#define OPT_GetProcAddress(id) Exports[id] = *ExportsNamesA[id]? reinterpret_cast<void*>(GetProcAddress(m_hModule, ExportsNamesA[id])): nullptr;

	OPT_GetProcAddress(iGetGlobalInfo);
	OPT_GetProcAddress(iSetStartupInfo);
	OPT_GetProcAddress(iOpen);
	OPT_GetProcAddress(iClosePanel);
	OPT_GetProcAddress(iGetPluginInfo);
	OPT_GetProcAddress(iGetOpenPanelInfo);
	OPT_GetProcAddress(iGetFindData);
	OPT_GetProcAddress(iFreeFindData);
	OPT_GetProcAddress(iGetVirtualFindData);
	OPT_GetProcAddress(iFreeVirtualFindData);
	OPT_GetProcAddress(iSetDirectory);
	OPT_GetProcAddress(iGetFiles);
	OPT_GetProcAddress(iPutFiles);
	OPT_GetProcAddress(iDeleteFiles);
	OPT_GetProcAddress(iMakeDirectory);
	OPT_GetProcAddress(iProcessHostFile);
	OPT_GetProcAddress(iSetFindList);
	OPT_GetProcAddress(iConfigure);
	OPT_GetProcAddress(iExitFAR);
	OPT_GetProcAddress(iProcessPanelInput);
	OPT_GetProcAddress(iProcessPanelEvent);
	OPT_GetProcAddress(iCompare);
	OPT_GetProcAddress(iProcessEditorInput);
	OPT_GetProcAddress(iProcessEditorEvent);
	OPT_GetProcAddress(iProcessViewerEvent);
	OPT_GetProcAddress(iProcessDialogEvent);
	OPT_GetProcAddress(iProcessSynchroEvent);
#if defined(MANTIS_0000466)
	OPT_GetProcAddress(iProcessMacro);
#endif
	OPT_GetProcAddress(iProcessConsoleInput);
	OPT_GetProcAddress(iAnalyse);
	OPT_GetProcAddress(iGetCustomData);
	OPT_GetProcAddress(iFreeCustomData);
	OPT_GetProcAddress(iCloseAnalyse);

	OPT_GetProcAddress(iOpenFilePlugin);
	OPT_GetProcAddress(iGetMinFarVersion);

#undef OPT_GetProcAddress
}

Plugin::Plugin(PluginManager *owner, const wchar_t *lpwszModuleName):
	ExportsNamesW(_ExportsNamesW),
	ExportsNamesA(_ExportsNamesA),
	m_owner(owner),
	Activity(0),
	bPendingRemove(false)
{
	m_strModuleName = lpwszModuleName;
	m_strCacheName = lpwszModuleName;
	m_hModule = nullptr;
	wchar_t *p = m_strCacheName.GetBuffer();
	while (*p)
	{
		if (*p == L'\\')
			*p = L'/';

		p++;
	}
	m_strCacheName.ReleaseBuffer();
	ClearExports();
	SetGuid(FarGuid);
}

Plugin::~Plugin()
{
	PluginLang.Close();
}

void Plugin::SetGuid(const GUID& Guid)
{
	m_Guid = Guid;
	m_strGuid = GuidToStr(m_Guid);
}

void InitVersionString(const VersionInfo& PluginVersion, string& VersionString)
{
		const wchar_t* Stage[] = { L" Release", L" Alpha", L" Beta", L" RC"};
		FormatString strVersion;
		strVersion << PluginVersion.Major << L"." << PluginVersion.Minor << L"." << PluginVersion.Revision << L" (build " << PluginVersion.Build <<L")";
		if(PluginVersion.Stage != VS_RELEASE && PluginVersion.Stage < ARRAYSIZE(Stage))
		{
			strVersion << Stage[PluginVersion.Stage];
		}
		VersionString = strVersion;
}

bool Plugin::LoadData()
{
	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (WorkFlags.Check(PIWF_DATALOADED))
		return true;

	if (m_hModule)
		return true;

	if (!m_hModule)
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[]={0,L' ',L':',0}; //ставим 0, как признак того, что вертать обратно ненадо!
		apiGetCurrentDirectory(strCurPath);

		if (ParsePath(m_strModuleName) == PATH_DRIVELETTER)  // если указан локальный путь, то...
		{
			Drive[0] = L'=';
			Drive[1] = m_strModuleName.At(0);
			apiGetEnvironmentVariable(Drive, strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);
		m_hModule = LoadLibraryEx(m_strModuleName,nullptr,0);
		if(!m_hModule) m_hModule = LoadLibraryEx(m_strModuleName,nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
		GuardLastError Err;
		FarChDir(strCurPath);

		if (Drive[0]) // вернем ее (переменную окружения) обратно
			SetEnvironmentVariable(Drive,strCurPlugDiskPath);
	}

	if (!m_hModule)
	{
		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (!Global->Opt->LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			const wchar_t* const Items[] = {MSG(MPlgLoadPluginError), m_strModuleName, MSG(MOk)};
			Message(MSG_WARNING|MSG_ERRORTYPE|MSG_NOPLUGINS, 1, MSG(MError), Items, ARRAYSIZE(Items), L"ErrLoadPlugin");
		}

		return false;
	}

	WorkFlags.Clear(PIWF_CACHED);

	if(bPendingRemove)
	{
		bPendingRemove = false;
		m_owner->UndoRemove(this);
	}
	InitExports();

	GlobalInfo Info={sizeof(Info)};

	if(GetGlobalInfo(&Info) &&
		Info.StructSize &&
		Info.Title && *Info.Title &&
		Info.Description && *Info.Description &&
		Info.Author && *Info.Author)
	{
		MinFarVersion = Info.MinFarVersion;
		PluginVersion = Info.Version;
		InitVersionString(PluginVersion, VersionString);
		strTitle = Info.Title;
		strDescription = Info.Description;
		strAuthor = Info.Author;

		bool ok=true;
		if(m_Guid != FarGuid && m_Guid != Info.Guid)
		{
			ok = m_owner->UpdateId(this, Info.Guid);
		}
		else
		{
			SetGuid(Info.Guid);
		}

		if (ok)
		{
			WorkFlags.Set(PIWF_DATALOADED);
			return true;
		}
	}
	Unload();
	//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
	WorkFlags.Set(PIWF_DONTLOADAGAIN);
	return false;
}

bool Plugin::Load()
{

	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (!WorkFlags.Check(PIWF_DATALOADED)&&!LoadData())
		return false;

	if (FuncFlags.Check(PICFF_LOADED))
		return true;

	FuncFlags.Set(PICFF_LOADED);

	if (!CheckMinFarVersion() || !SetStartupInfo())
	{
		if (!bPendingRemove)
		{
			Unload();
		}

		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		return false;
	}

	SaveToCache();
	return true;
}

bool Plugin::LoadFromCache(const FAR_FIND_DATA_EX &FindData)
{
	PluginsCacheConfig& PlCache = *Global->Db->PlCacheCfg();

	unsigned __int64 id = PlCache.GetCacheID(m_strCacheName);

	if (id)
	{
		if (PlCache.IsPreload(id))   //PF_PRELOAD plugin, skip cache
		{
			WorkFlags.Set(PIWF_PRELOADED);
			return false;
		}

		{
			string strCurPluginID;
			strCurPluginID.Format(
				L"%I64x%x%x",
				FindData.nFileSize,
				FindData.ftCreationTime.dwLowDateTime,
				FindData.ftLastWriteTime.dwLowDateTime
				);

			string strPluginID = PlCache.GetSignature(id);

			if (StrCmp(strPluginID, strCurPluginID))   //одинаковые ли бинарники?
				return false;
		}

		if (!PlCache.GetMinFarVersion(id, &MinFarVersion))
		{
			MinFarVersion = FAR_VERSION;
		}

		if (!PlCache.GetVersion(id, &PluginVersion))
		{
			ClearStruct(PluginVersion);
		}
		InitVersionString(PluginVersion, VersionString);

		m_strGuid = PlCache.GetGuid(id);
		SetGuid(StrToGuid(m_strGuid,m_Guid)?m_Guid:FarGuid);
		strTitle = PlCache.GetTitle(id);
		strDescription = PlCache.GetDescription(id);
		strAuthor = PlCache.GetAuthor(id);

#define OPT_GETEXPORT(i) Exports[i] = *ExportsNamesW[i]? PlCache.GetExport(id, ExportsNamesW[i]) : nullptr

		OPT_GETEXPORT(iOpen);
		OPT_GETEXPORT(iOpenFilePlugin);
		OPT_GETEXPORT(iSetFindList);
		OPT_GETEXPORT(iProcessEditorInput);
		OPT_GETEXPORT(iProcessEditorEvent);
		OPT_GETEXPORT(iProcessViewerEvent);
		OPT_GETEXPORT(iProcessDialogEvent);
		OPT_GETEXPORT(iProcessSynchroEvent);
#if defined(MANTIS_0000466)
		OPT_GETEXPORT(iProcessMacro);
#endif
		OPT_GETEXPORT(iProcessConsoleInput);
		OPT_GETEXPORT(iConfigure);
		OPT_GETEXPORT(iAnalyse);
		OPT_GETEXPORT(iGetCustomData);
		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags
		return true;
	}
	return false;
}

int Plugin::Unload(bool bExitFAR)
{
	int nResult = TRUE;

	if (FuncFlags.Check(PICFF_LOADED))
	{
		if (bExitFAR)
		{
			const ExitInfo Info={sizeof(Info)};
			ExitFAR(&Info);
		}

		if (!WorkFlags.Check(PIWF_CACHED))
		{
			nResult = FreeLibrary(m_hModule);
			ClearExports();
		}

		m_hModule = nullptr;
		FuncFlags.Clear(PICFF_LOADED);
		WorkFlags.Clear(PIWF_DATALOADED);
		bPendingRemove = true;
	}

	return nResult;
}

void Plugin::ClearExports()
{
	ClearArray(Exports);
}

bool Plugin::IsPanelPlugin()
{
	return Exports[iSetFindList] ||
	       Exports[iGetFindData] ||
	       Exports[iGetVirtualFindData] ||
	       Exports[iSetDirectory] ||
	       Exports[iGetFiles] ||
	       Exports[iPutFiles] ||
	       Exports[iDeleteFiles] ||
	       Exports[iMakeDirectory] ||
	       Exports[iProcessHostFile] ||
	       Exports[iProcessPanelInput] ||
	       Exports[iProcessPanelEvent] ||
	       Exports[iCompare] ||
	       Exports[iGetOpenPanelInfo] ||
	       Exports[iFreeFindData] ||
	       Exports[iFreeVirtualFindData] ||
	       Exports[iClosePanel];
}

bool Plugin::SetStartupInfo()
{
	if (Exports[iSetStartupInfo] && !Global->ProcessException)
	{
		PluginStartupInfo _info;
		FarStandardFunctions _fsf;
		CreatePluginStartupInfo(this, &_info, &_fsf);
		ExecuteStruct es = {EXCEPT_SETSTARTUPINFO};
		EXECUTE_FUNCTION(FUNCTION(iSetStartupInfo)(&_info));

		if (bPendingRemove)
		{
			return false;
		}
	}
	return true;
}

bool Plugin::GetGlobalInfo(GlobalInfo *gi)
{
	if (Exports[iGetGlobalInfo])
	{
		ExecuteStruct es = {EXCEPT_GETGLOBALINFO};
		EXECUTE_FUNCTION(FUNCTION(iGetGlobalInfo)(gi));
		return !bPendingRemove;
	}
	return false;
}

bool Plugin::CheckMinFarVersion()
{
	if (!CheckVersion(&FAR_VERSION, &MinFarVersion))
	{
		ShowMessageAboutIllegalPluginVersion(m_strModuleName,MinFarVersion);
		return false;
	}

	return true;
}

HANDLE Plugin::OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode)
{
	return nullptr;
}

HANDLE Plugin::Analyse(const AnalyseInfo *Info)
{
	ExecuteStruct es = {EXCEPT_ANALYSE};
	if (Load() && Exports[iAnalyse] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION_EX(FUNCTION(iAnalyse)(Info));
	}
	return es;
}

void Plugin::CloseAnalyse(HANDLE hHandle)
{
	if (Exports[iCloseAnalyse] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_CLOSEANALYSE};
		CloseAnalyseInfo Info = {sizeof(Info)};
		Info.Handle = hHandle;
		EXECUTE_FUNCTION(FUNCTION(iCloseAnalyse)(&Info));
	}
}

HANDLE Plugin::Open(int OpenFrom, const GUID& Guid, intptr_t Item)
{
	ChangePriority *ChPriority = new ChangePriority(THREAD_PRIORITY_NORMAL);
	CheckScreenLock(); //??
	Global->g_strDirToSet.Clear();
	ExecuteStruct es = {EXCEPT_OPEN};
	if (Load() && Exports[iOpen] && !Global->ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		OpenInfo Info = {sizeof(Info)};
		Info.OpenFrom = static_cast<OPENFROM>(OpenFrom);
		Info.Guid = &Guid;
		Info.Data = Item;
		EXECUTE_FUNCTION_EX(FUNCTION(iOpen)(&Info));
	}
	delete ChPriority;
	return es;
}

int Plugin::SetFindList(HANDLE hPlugin, const PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	ExecuteStruct es = {EXCEPT_SETFINDLIST};
	if (Exports[iSetFindList] && !Global->ProcessException)
	{
		SetFindListInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION_EX(FUNCTION(iSetFindList)(&Info));
	}
	return es;
}

int Plugin::ProcessEditorInput(const INPUT_RECORD *D)
{
	ExecuteStruct es = {EXCEPT_PROCESSEDITORINPUT};
	if (Load() && Exports[iProcessEditorInput] && !Global->ProcessException)
	{
		ProcessEditorInputInfo Info={sizeof(Info)};
		Info.Rec=*D;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessEditorInput)(&Info));
	}
	return es;
}

int Plugin::ProcessEditorEvent(int Event, PVOID Param, int EditorID)
{
	ExecuteStruct es = {EXCEPT_PROCESSEDITOREVENT};
	if (Load() && Exports[iProcessEditorEvent] && !Global->ProcessException)
	{
		ProcessEditorEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.EditorID = EditorID;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessEditorEvent)(&Info));
	}
	return es;
}

int Plugin::ProcessViewerEvent(int Event, void *Param, int ViewerID)
{
	ExecuteStruct es = {EXCEPT_PROCESSVIEWEREVENT};
	if (Load() && Exports[iProcessViewerEvent] && !Global->ProcessException)
	{
		ProcessViewerEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.ViewerID = ViewerID;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessViewerEvent)(&Info));
	}
	return es;
}

int Plugin::ProcessDialogEvent(int Event, FarDialogEvent *Param)
{
	ExecuteStruct es = {EXCEPT_PROCESSDIALOGEVENT};
	if (Load() && Exports[iProcessDialogEvent] && !Global->ProcessException)
	{
		ProcessDialogEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessDialogEvent)(&Info));
	}
	return es;
}

int Plugin::ProcessSynchroEvent(int Event, void *Param)
{
	ExecuteStruct es = {EXCEPT_PROCESSSYNCHROEVENT};
	if (Load() && Exports[iProcessSynchroEvent] && !Global->ProcessException)
	{
		es.nDefaultResult = 0;
		ProcessSynchroEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessSynchroEvent)(&Info));
	}
	return es;
}

#if defined(MANTIS_0000466)
int Plugin::ProcessMacro(ProcessMacroInfo *Info)
{
	ExecuteStruct es = {EXCEPT_PROCESSMACRO};
	if (Load() && Exports[iProcessMacro] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessMacro)(Info));
	}
	return es;
}
#endif

int Plugin::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	ExecuteStruct es = {EXCEPT_PROCESSCONSOLEINPUT};
	if (Load() && Exports[iProcessConsoleInput] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessConsoleInput)(Info));
	}
	return es;
}

int Plugin::GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, const wchar_t *Path)
{
	ExecuteStruct es = {EXCEPT_GETVIRTUALFINDDATA};
	if (Exports[iGetVirtualFindData] && !Global->ProcessException)
	{
		GetVirtualFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.Path = Path;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetVirtualFindData)(&Info));
		*pPanelItem = Info.PanelItem;
		*pItemsNumber = Info.ItemsNumber;
	}
	return es;
}

void Plugin::FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	if (Exports[iFreeVirtualFindData] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_FREEVIRTUALFINDDATA};
		FreeFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION(FUNCTION(iFreeVirtualFindData)(&Info));
	}
}

int Plugin::GetFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, const wchar_t **DestPath, int OpMode)
{
	ExecuteStruct es = {EXCEPT_GETFILES, -1};
	if (Exports[iGetFiles] && !Global->ProcessException)
	{
		GetFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.Move = Move;
		Info.DestPath = *DestPath;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetFiles)(&Info));
		*DestPath = Info.DestPath;
	}
	return es;
}

int Plugin::PutFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, int OpMode)
{
	ExecuteStruct es = {EXCEPT_PUTFILES, -1};

	if (Exports[iPutFiles] && !Global->ProcessException)
	{
		static string strCurrentDirectory;
		apiGetCurrentDirectory(strCurrentDirectory);
		PutFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.Move = Move;
		Info.SrcPath = strCurrentDirectory;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iPutFiles)(&Info));
	}
	return es;
}

int Plugin::DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode)
{
	ExecuteStruct es = {EXCEPT_DELETEFILES};
	if (Exports[iDeleteFiles] && !Global->ProcessException)
	{
		ExecuteStruct es;
		DeleteFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iDeleteFiles)(&Info));
	}
	return es;
}

int Plugin::MakeDirectory(HANDLE hPlugin, const wchar_t **Name, int OpMode)
{
	ExecuteStruct es = {EXCEPT_MAKEDIRECTORY, -1};
	if (Exports[iMakeDirectory] && !Global->ProcessException)
	{
		MakeDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Name = *Name;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iMakeDirectory)(&Info));
		*Name = Info.Name;
	}
	return es;
}

int Plugin::ProcessHostFile(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode)
{
	ExecuteStruct es = {EXCEPT_PROCESSHOSTFILE};
	if (Exports[iProcessHostFile] && !Global->ProcessException)
	{
		ProcessHostFileInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessHostFile)(&Info));
	}
	return es;
}

int Plugin::ProcessPanelEvent(HANDLE hPlugin, int Event, PVOID Param)
{
	ExecuteStruct es = {EXCEPT_PROCESSPANELEVENT};
	if (Exports[iProcessPanelEvent] && !Global->ProcessException)
	{
		ProcessPanelEventInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessPanelEvent)(&Info));
	}
	return es;
}

int Plugin::Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, DWORD Mode)
{
	ExecuteStruct es = {EXCEPT_COMPARE, -2};
	if (Exports[iCompare] && !Global->ProcessException)
	{
		CompareInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Item1 = Item1;
		Info.Item2 = Item2;
		Info.Mode = static_cast<OPENPANELINFO_SORTMODES>(Mode);
		EXECUTE_FUNCTION_EX(FUNCTION(iCompare)(&Info));
	}
	return es;
}

int Plugin::GetFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, int OpMode)
{
	ExecuteStruct es = {EXCEPT_GETFINDDATA};
	if (Exports[iGetFindData] && !Global->ProcessException)
	{
		GetFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetFindData)(&Info));
		*pPanelItem = Info.PanelItem;
		*pItemsNumber = Info.ItemsNumber;
	}
	return es;
}

void Plugin::FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool FreeUserData)
{
	if (Exports[iFreeFindData] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_FREEFINDDATA};
		FreeFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		if (FreeUserData) FreePluginPanelItemsUserData(hPlugin,PanelItem,ItemsNumber);
		EXECUTE_FUNCTION(FUNCTION(iFreeFindData)(&Info));
	}
}

int Plugin::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool)
{
	ExecuteStruct es = {EXCEPT_PROCESSPANELINPUT};
	if (Exports[iProcessPanelInput] && !Global->ProcessException)
	{
		struct ProcessPanelInputInfo Info={sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Rec=*Rec;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessPanelInput)(&Info));
	}
	return es;
}


void Plugin::ClosePanel(HANDLE hPlugin)
{
	if (Exports[iClosePanel] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_CLOSEPANEL};
		ClosePanelInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		EXECUTE_FUNCTION(FUNCTION(iClosePanel)(&Info));
	}
//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int Plugin::SetDirectory(HANDLE hPlugin, const wchar_t *Dir, int OpMode, UserDataItem *UserData)
{
	ExecuteStruct es = {EXCEPT_SETDIRECTORY};
	if (Exports[iSetDirectory] && !Global->ProcessException)
	{
		SetDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Dir = Dir;
		Info.OpMode = OpMode;
		if (UserData)
		{
			Info.UserData.Data = UserData->Data;
			Info.UserData.FreeData = UserData->FreeData;
		}
		EXECUTE_FUNCTION_EX(FUNCTION(iSetDirectory)(&Info));
	}
	return es;
}

void Plugin::GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *pInfo)
{
	//	m_pManager->m_pCurrentPlugin = this;
	pInfo->StructSize = sizeof(OpenPanelInfo);

	if (Exports[iGetOpenPanelInfo] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_GETOPENPANELINFO};
		pInfo->hPanel = hPlugin;
		EXECUTE_FUNCTION(FUNCTION(iGetOpenPanelInfo)(pInfo));
	}
}


int Plugin::Configure(const GUID& Guid)
{
	ExecuteStruct es = {EXCEPT_CONFIGURE};
	if (Load() && Exports[iConfigure] && !Global->ProcessException)
	{
		ConfigureInfo Info = {sizeof(Info)};
		Info.Guid = &Guid;
		EXECUTE_FUNCTION_EX(FUNCTION(iConfigure)(&Info));
	}
	return es;
}


bool Plugin::GetPluginInfo(PluginInfo *pi)
{
	if (Exports[iGetPluginInfo] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_GETPLUGININFO};
		EXECUTE_FUNCTION(FUNCTION(iGetPluginInfo)(pi));
		if (!bPendingRemove)
			return true;
	}
	return false;
}

int Plugin::GetCustomData(const wchar_t *FilePath, wchar_t **CustomData)
{
	ExecuteStruct es = {EXCEPT_GETCUSTOMDATA};
	if (Load() && Exports[iGetCustomData] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION_EX(FUNCTION(iGetCustomData)(FilePath, CustomData));
	}
	return es;
}

void Plugin::FreeCustomData(wchar_t *CustomData)
{
	if (Load() && Exports[iFreeCustomData] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_FREECUSTOMDATA};
		EXECUTE_FUNCTION(FUNCTION(iFreeCustomData)(CustomData));
	}
}

void Plugin::ExitFAR(const ExitInfo *Info)
{
	if (Exports[iExitFAR] && !Global->ProcessException)
	{
		ExecuteStruct es = {EXCEPT_EXITFAR};
		EXECUTE_FUNCTION(FUNCTION(iExitFAR)(Info));
	}
}
