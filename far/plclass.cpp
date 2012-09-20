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
#include "FarGuid.hpp"

typedef void   (WINAPI *iClosePanelPrototype)          (const ClosePanelInfo *Info);
typedef int    (WINAPI *iComparePrototype)             (const CompareInfo *Info);
typedef int    (WINAPI *iConfigurePrototype)           (const ConfigureInfo *Info);
typedef int    (WINAPI *iDeleteFilesPrototype)         (const DeleteFilesInfo *Info);
typedef void   (WINAPI *iExitFARPrototype)             (const ExitInfo *Info);
typedef void   (WINAPI *iFreeFindDataPrototype)        (const FreeFindDataInfo *Info);
typedef void   (WINAPI *iFreeVirtualFindDataPrototype) (const FreeFindDataInfo *Info);
typedef int    (WINAPI *iGetFilesPrototype)            (GetFilesInfo *Info);
typedef int    (WINAPI *iGetFindDataPrototype)         (GetFindDataInfo *Info);
typedef void   (WINAPI *iGetGlobalInfoPrototype)       (GlobalInfo *Info);
typedef void   (WINAPI *iGetOpenPanelInfoPrototype)    (OpenPanelInfo *Info);
typedef void   (WINAPI *iGetPluginInfoPrototype)       (PluginInfo *Info);
typedef int    (WINAPI *iGetVirtualFindDataPrototype)  (GetVirtualFindDataInfo *Info);
typedef int    (WINAPI *iMakeDirectoryPrototype)       (MakeDirectoryInfo *Info);
typedef HANDLE (WINAPI *iOpenPrototype)                (const OpenInfo *Info);
typedef int    (WINAPI *iProcessEditorEventPrototype)  (const ProcessEditorEventInfo *Info);
typedef int    (WINAPI *iProcessEditorInputPrototype)  (const ProcessEditorInputInfo *Info);
typedef int    (WINAPI *iProcessPanelEventPrototype)   (const ProcessPanelEventInfo *Info);
typedef int    (WINAPI *iProcessHostFilePrototype)     (const ProcessHostFileInfo *Info);
typedef int    (WINAPI *iProcessPanelInputPrototype)   (const ProcessPanelInputInfo *Info);
typedef int    (WINAPI *iPutFilesPrototype)            (const PutFilesInfo *Info);
typedef int    (WINAPI *iSetDirectoryPrototype)        (const SetDirectoryInfo *Info);
typedef int    (WINAPI *iSetFindListPrototype)         (const SetFindListInfo *Info);
typedef void   (WINAPI *iSetStartupInfoPrototype)      (const PluginStartupInfo *Info);
typedef int    (WINAPI *iProcessViewerEventPrototype)  (const ProcessViewerEventInfo *Info);
typedef int    (WINAPI *iProcessDialogEventPrototype)  (const ProcessDialogEventInfo *Info);
typedef int    (WINAPI *iProcessSynchroEventPrototype) (const ProcessSynchroEventInfo *Info);
#if defined(MANTIS_0000466)
typedef int    (WINAPI *iProcessMacroPrototype)        (const ProcessMacroInfo *Info);
#endif
#if defined(MANTIS_0001687)
typedef int    (WINAPI *iProcessConsoleInputPrototype) (const ProcessConsoleInputInfo *Info);
#endif
typedef HANDLE (WINAPI *iAnalysePrototype)             (const AnalyseInfo *Info);
typedef int    (WINAPI *iGetCustomDataPrototype)       (const wchar_t *FilePath, wchar_t **CustomData);
typedef void   (WINAPI *iFreeCustomDataPrototype)      (wchar_t *CustomData);
typedef void   (WINAPI *iCloseAnalysePrototype)        (const CloseAnalyseInfo *Info);


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
#if defined(MANTIS_0001687)
#define EXP_PROCESSCONSOLEINPUT "ProcessConsoleInputW"
#endif
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
#if defined(MANTIS_0001687)
	EXP_PROCESSCONSOLEINPUT,
#endif
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
#if defined(MANTIS_0001687)
	W(EXP_PROCESSCONSOLEINPUT),
#endif
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
	if (ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey())
	{
//		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
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
	swscanf,
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
	pluginapi::apiDeleteBuffer,
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
	0, //Reserved
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

void CreatePluginStartupInfo(const Plugin* pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF)
{
	*PSI=NativeInfo;
	*FSF=NativeFSF;
	PSI->FSF=FSF;

	if (pPlugin)
	{
		PSI->ModuleName = pPlugin->GetModuleName().CPtr();
		if(IsEqualGUID(pPlugin->GetGUID(),ArcliteGuid))
		{
			PSI->Private = &ArcliteInfo;
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
#if defined(MANTIS_0001687)
		Exports[iProcessConsoleInput] ||
#endif
		Exports[iAnalyse] ||
		Exports[iGetCustomData]
	)
	{
		PluginInfo Info = {sizeof(Info)};
		GetPluginInfo(&Info);

		PlCacheCfg->BeginTransaction();

		PlCacheCfg->DeleteCache(m_strCacheName);
		unsigned __int64 id = PlCacheCfg->CreateCache(m_strCacheName);

		{
			bool bPreload = (Info.Flags & PF_PRELOAD);
			PlCacheCfg->SetPreload(id, bPreload);
			WorkFlags.Change(PIWF_PRELOADED, bPreload);

			if (bPreload)
			{
				PlCacheCfg->EndTransaction();
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
			PlCacheCfg->SetSignature(id, strCurPluginID);
		}

		for (size_t i = 0; i < Info.DiskMenu.Count; i++)
		{
			PlCacheCfg->SetDiskMenuItem(id, i, Info.DiskMenu.Strings[i], GuidToStr(Info.DiskMenu.Guids[i]));
		}

		for (size_t i = 0; i < Info.PluginMenu.Count; i++)
		{
			PlCacheCfg->SetPluginsMenuItem(id, i, Info.PluginMenu.Strings[i], GuidToStr(Info.PluginMenu.Guids[i]));
		}

		for (size_t i = 0; i < Info.PluginConfig.Count; i++)
		{
			PlCacheCfg->SetPluginsConfigMenuItem(id, i, Info.PluginConfig.Strings[i], GuidToStr(Info.PluginConfig.Guids[i]));
		}

		PlCacheCfg->SetCommandPrefix(id, NullToEmpty(Info.CommandPrefix));
#if defined(MANTIS_0000466)
		PlCacheCfg->SetMacroFunctions(id, NullToEmpty(Info.MacroFunctions));
#endif
		PlCacheCfg->SetFlags(id, Info.Flags);

		PlCacheCfg->SetMinFarVersion(id, &MinFarVersion);
		PlCacheCfg->SetGuid(id, m_strGuid);
		PlCacheCfg->SetVersion(id, &PluginVersion);
		PlCacheCfg->SetTitle(id, strTitle);
		PlCacheCfg->SetDescription(id, strDescription);
		PlCacheCfg->SetAuthor(id, strAuthor);

#define OPT_SETEXPORT(i) if (*ExportsNamesW[i]) PlCacheCfg->SetExport(id, ExportsNamesW[i], Exports[i]!=nullptr)

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
#if defined(MANTIS_0001687)
		OPT_SETEXPORT(iProcessConsoleInput);
#endif
		OPT_SETEXPORT(iConfigure);
		OPT_SETEXPORT(iAnalyse);
		OPT_SETEXPORT(iGetCustomData);

		PlCacheCfg->EndTransaction();

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
#if defined(MANTIS_0001687)
	OPT_GetProcAddress(iProcessConsoleInput);
#endif
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

		if (!Opt.LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			SetMessageHelp(L"ErrLoadPlugin");
			Message(MSG_WARNING|MSG_ERRORTYPE|MSG_NOPLUGINS,1,MSG(MError),MSG(MPlgLoadPluginError),m_strModuleName,MSG(MOk));
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
	unsigned __int64 id = PlCacheCfg->GetCacheID(m_strCacheName);

	if (id)
	{
		if (PlCacheCfg->IsPreload(id))   //PF_PRELOAD plugin, skip cache
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

			string strPluginID = PlCacheCfg->GetSignature(id);

			if (StrCmp(strPluginID, strCurPluginID))   //одинаковые ли бинарники?
				return false;
		}

		if (!PlCacheCfg->GetMinFarVersion(id, &MinFarVersion))
		{
			MinFarVersion = FAR_VERSION;
		}

		if (!PlCacheCfg->GetVersion(id, &PluginVersion))
		{
			ClearStruct(PluginVersion);
		}
		InitVersionString(PluginVersion, VersionString);

		m_strGuid = PlCacheCfg->GetGuid(id);
		SetGuid(StrToGuid(m_strGuid,m_Guid)?m_Guid:FarGuid);
		strTitle = PlCacheCfg->GetTitle(id);
		strDescription = PlCacheCfg->GetDescription(id);
		strAuthor = PlCacheCfg->GetAuthor(id);

#define OPT_GETEXPORT(i) Exports[i] = *ExportsNamesW[i]? PlCacheCfg->GetExport(id, ExportsNamesW[i]) : nullptr

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
#if defined(MANTIS_0001687)
		OPT_GETEXPORT(iProcessConsoleInput);
#endif
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
	if (Exports[iSetStartupInfo] && !ProcessException)
	{
		PluginStartupInfo _info;
		FarStandardFunctions _fsf;
		CreatePluginStartupInfo(this, &_info, &_fsf);
		// скорректирем адреса и плагино-зависимые поля
		ExecuteStruct es;
		es.id = EXCEPT_SETSTARTUPINFO;
		EXECUTE_FUNCTION(FUNCTION(iSetStartupInfo)(&_info), es);

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
		ExecuteStruct es;
		es.id = EXCEPT_GETGLOBALINFO;
		EXECUTE_FUNCTION(FUNCTION(iGetGlobalInfo)(gi), es);
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

HANDLE Plugin::OpenFilePlugin(
    const wchar_t *Name,
    const unsigned char *Data,
    size_t DataSize,
    int OpMode
)
{
	return nullptr;
}

HANDLE Plugin::Analyse(const AnalyseInfo *Info)
{
	if (Load() && Exports[iAnalyse] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_ANALYSE;
		es.bDefaultResult = FALSE;
		es.hResult = nullptr;
		EXECUTE_FUNCTION_EX(FUNCTION(iAnalyse)(Info), es);
		return es.hResult;
	}

	return nullptr;
}

void Plugin::CloseAnalyse(HANDLE hHandle)
{
	if (Exports[iCloseAnalyse] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CLOSEANALYSE;
		CloseAnalyseInfo Info = {sizeof(Info)};
		Info.Handle = hHandle;
		EXECUTE_FUNCTION(FUNCTION(iCloseAnalyse)(&Info), es);
	}
}

HANDLE Plugin::Open(int OpenFrom, const GUID& Guid, intptr_t Item)
{
	ChangePriority *ChPriority = new ChangePriority(THREAD_PRIORITY_NORMAL);

	CheckScreenLock(); //??

	{
//		string strCurDir;
//		CtrlObject->CmdLine->GetCurDir(strCurDir);
//		FarChDir(strCurDir);
		g_strDirToSet.Clear();
	}

	HANDLE hResult = nullptr;

	if (Load() && Exports[iOpen] && !ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		ExecuteStruct es;
		es.id = EXCEPT_OPEN;
		es.hDefaultResult = nullptr;
		es.hResult = nullptr;
		OpenInfo Info = {sizeof(Info)};
		Info.OpenFrom = static_cast<OPENFROM>(OpenFrom);
		Info.Guid = &Guid;
		Info.Data = Item;
		EXECUTE_FUNCTION_EX(FUNCTION(iOpen)(&Info), es);
		hResult = es.hResult;
		//CurPluginItem=nullptr; //BUGBUG
		/*    CtrlObject->Macro.SetRedrawEditor(TRUE); //BUGBUG

		    if ( !bUnloaded )
		    {

		      if(OpenFrom == OPEN_EDITOR &&
		         !CtrlObject->Macro.IsExecuting() &&
		         CtrlObject->Plugins->CurEditor &&
		         CtrlObject->Plugins->CurEditor->IsVisible() )
		      {
		        CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
		        CtrlObject->Plugins->CurEditor->Show();
		      }
		      if (hInternal!=INVALID_HANDLE_VALUE)
		      {
		        PluginHandle *hPlugin=new PluginHandle;
		        hPlugin->InternalHandle=es.hResult;
		        hPlugin->PluginNumber=(intptr_t)this;
		        return((HANDLE)hPlugin);
		      }
		      else
		        if ( !g_strDirToSet.IsEmpty() )
		        {
							CtrlObject->Cp()->ActivePanel->SetCurDir(g_strDirToSet,TRUE);
		          CtrlObject->Cp()->ActivePanel->Redraw();
		        }
		    } */
	}

	delete ChPriority;

	return hResult;
}

//////////////////////////////////
int Plugin::SetFindList(
    HANDLE hPlugin,
    const PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	BOOL bResult = FALSE;

	if (Exports[iSetFindList] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;
		SetFindListInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION_EX(FUNCTION(iSetFindList)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}

int Plugin::ProcessEditorInput(
    const INPUT_RECORD *D
)
{
	BOOL bResult = FALSE;

	if (Load() && Exports[iProcessEditorInput] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITORINPUT;
		es.bDefaultResult = TRUE; //(TRUE) treat the result as a completed request on exception!
		ProcessEditorInputInfo Info={sizeof(Info)};
		Info.Rec=*D;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessEditorInput)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}

int Plugin::ProcessEditorEvent(
    int Event,
    PVOID Param,
    int EditorID
)
{
	if (Load() && Exports[iProcessEditorEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;
		ProcessEditorEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.EditorID = EditorID;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessEditorEvent)(&Info), es);
	}

	return 0; //oops!
}

int Plugin::ProcessViewerEvent(
    int Event,
    void *Param,
    int ViewerID
)
{
	if (Load() && Exports[iProcessViewerEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;
		ProcessViewerEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.ViewerID = ViewerID;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessViewerEvent)(&Info), es);
	}

	return 0; //oops, again!
}

int Plugin::ProcessDialogEvent(
    int Event,
    FarDialogEvent *Param
)
{
	BOOL bResult = FALSE;

	if (Load() && Exports[iProcessDialogEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSDIALOGEVENT;
		es.bDefaultResult = FALSE;
		ProcessDialogEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessDialogEvent)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}

int Plugin::ProcessSynchroEvent(
    int Event,
    void *Param
)
{
	if (Load() && Exports[iProcessSynchroEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSSYNCHROEVENT;
		es.nDefaultResult = 0;
		ProcessSynchroEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessSynchroEvent)(&Info), es);
	}

	return 0; //oops, again!
}

#if defined(MANTIS_0000466)
int Plugin::ProcessMacro(
	ProcessMacroInfo *Info
)
{
	int nResult = 0;

	if (Load() && Exports[iProcessMacro] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSMACRO;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(FUNCTION(iProcessMacro)(Info), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}
#endif

#if defined(MANTIS_0001687)
int Plugin::ProcessConsoleInput(
	ProcessConsoleInputInfo *Info
)
{
	int nResult = 0;

	if (Load() && Exports[iProcessConsoleInput] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSCONSOLEINPUT;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(FUNCTION(iProcessConsoleInput)(Info), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}
#endif


int Plugin::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    size_t *pItemsNumber,
    const wchar_t *Path
)
{
	BOOL bResult = FALSE;

	if (Exports[iGetVirtualFindData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;
		GetVirtualFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.Path = Path;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetVirtualFindData)(&Info), es);
		*pPanelItem = Info.PanelItem;
		*pItemsNumber = Info.ItemsNumber;
		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::FreeVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	if (Exports[iFreeVirtualFindData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;
		FreeFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION(FUNCTION(iFreeVirtualFindData)(&Info), es);
	}
}



int Plugin::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	int nResult = -1;

	if (Exports[iGetFiles] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;
		GetFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.Move = Move;
		Info.DestPath = *DestPath;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetFiles)(&Info), es);
		*DestPath = Info.DestPath;
		nResult = (int)es.nResult;
	}

	return nResult;
}


int Plugin::PutFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    int OpMode
)
{
	int nResult = -1;

	if (Exports[iPutFiles] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;
		static string strCurrentDirectory;
		apiGetCurrentDirectory(strCurrentDirectory);
		PutFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.Move = Move;
		Info.SrcPath = strCurrentDirectory;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iPutFiles)(&Info), es);
		nResult = (int)es.nResult;
	}

	return nResult;
}

int Plugin::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iDeleteFiles] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;
		DeleteFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iDeleteFiles)(&Info), es);
		bResult = (int)es.bResult;
	}

	return bResult;
}


int Plugin::MakeDirectory(
    HANDLE hPlugin,
    const wchar_t **Name,
    int OpMode
)
{
	int nResult = -1;

	if (Exports[iMakeDirectory] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;
		MakeDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Name = *Name;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iMakeDirectory)(&Info), es);
		*Name = Info.Name;
		nResult = (int)es.nResult;
	}

	return nResult;
}


int Plugin::ProcessHostFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iProcessHostFile] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;
		ProcessHostFileInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessHostFile)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


int Plugin::ProcessPanelEvent(
    HANDLE hPlugin,
    int Event,
    PVOID Param
)
{
	BOOL bResult = FALSE;

	if (Exports[iProcessPanelEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSPANELEVENT;
		es.bDefaultResult = FALSE;
		ProcessPanelEventInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessPanelEvent)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


int Plugin::Compare(
    HANDLE hPlugin,
    const PluginPanelItem *Item1,
    const PluginPanelItem *Item2,
    DWORD Mode
)
{
	int nResult = -2;

	if (Exports[iCompare] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;
		CompareInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Item1 = Item1;
		Info.Item2 = Item2;
		Info.Mode = static_cast<OPENPANELINFO_SORTMODES>(Mode);
		EXECUTE_FUNCTION_EX(FUNCTION(iCompare)(&Info), es);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int Plugin::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    size_t *pItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iGetFindData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;
		GetFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetFindData)(&Info), es);
		*pPanelItem = Info.PanelItem;
		*pItemsNumber = Info.ItemsNumber;
		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::FreeFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	if (Exports[iFreeFindData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;
		FreeFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION(FUNCTION(iFreeFindData)(&Info), es);
	}
}

int Plugin::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	(void)Pred;
	BOOL bResult = FALSE;

	if (Exports[iProcessPanelInput] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSPANELINPUT;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception
		struct ProcessPanelInputInfo Info={sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Rec=*Rec;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessPanelInput)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::ClosePanel(
    HANDLE hPlugin
)
{
	if (Exports[iClosePanel] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CLOSEPANEL;
		ClosePanelInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		EXECUTE_FUNCTION(FUNCTION(iClosePanel)(&Info), es);
	}

//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int Plugin::SetDirectory(
    HANDLE hPlugin,
    const wchar_t *Dir,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iSetDirectory] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;
		SetDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Dir = Dir;
		Info.OpMode = OpMode;
		Info.UserData = 0; //Reserved
		EXECUTE_FUNCTION_EX(FUNCTION(iSetDirectory)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::GetOpenPanelInfo(
    HANDLE hPlugin,
    OpenPanelInfo *pInfo
)
{
//	m_pManager->m_pCurrentPlugin = this;
	pInfo->StructSize = sizeof(OpenPanelInfo);

	if (Exports[iGetOpenPanelInfo] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPANELINFO;
		pInfo->hPanel = hPlugin;
		EXECUTE_FUNCTION(FUNCTION(iGetOpenPanelInfo)(pInfo), es);
	}
}


int Plugin::Configure(const GUID& Guid)
{
	BOOL bResult = FALSE;

	if (Load() && Exports[iConfigure] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CONFIGURE;
		es.bDefaultResult = FALSE;
		ConfigureInfo Info = {sizeof(Info)};
		Info.Guid = &Guid;
		EXECUTE_FUNCTION_EX(FUNCTION(iConfigure)(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


bool Plugin::GetPluginInfo(PluginInfo *pi)
{
	if (Exports[iGetPluginInfo] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETPLUGININFO;
		EXECUTE_FUNCTION(FUNCTION(iGetPluginInfo)(pi), es);

		if (!bPendingRemove)
			return true;
	}

	return false;
}

int Plugin::GetCustomData(const wchar_t *FilePath, wchar_t **CustomData)
{
	if (Load() && Exports[iGetCustomData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETCUSTOMDATA;
		es.bDefaultResult = 0;
		es.bResult = 0;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetCustomData)(FilePath, CustomData), es);
		return es.bResult;
	}

	return 0;
}

void Plugin::FreeCustomData(wchar_t *CustomData)
{
	if (Load() && Exports[iFreeCustomData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREECUSTOMDATA;
		EXECUTE_FUNCTION(FUNCTION(iFreeCustomData)(CustomData), es);
	}
}

void Plugin::ExitFAR(const ExitInfo *Info)
{
	if (Exports[iExitFAR] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_EXITFAR;
		EXECUTE_FUNCTION(FUNCTION(iExitFAR)(Info), es);
	}
}
