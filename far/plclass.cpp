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
typedef intptr_t (WINAPI *iProcessConsoleInputPrototype) (const ProcessConsoleInputInfo *Info);
typedef HANDLE   (WINAPI *iAnalysePrototype)             (const AnalyseInfo *Info);
typedef intptr_t (WINAPI *iGetCustomDataPrototype)       (const wchar_t *FilePath, wchar_t **CustomData);
typedef void     (WINAPI *iFreeCustomDataPrototype)      (wchar_t *CustomData);
typedef void     (WINAPI *iCloseAnalysePrototype)        (const CloseAnalyseInfo *Info);

static const export_name* GetExportsNames()
{
	static const export_name ExportsNames[] =
	{
		WA("GetGlobalInfoW"),
		WA("SetStartupInfoW"),
		WA("OpenW"),
		WA("ClosePanelW"),
		WA("GetPluginInfoW"),
		WA("GetOpenPanelInfoW"),
		WA("GetFindDataW"),
		WA("FreeFindDataW"),
		WA("GetVirtualFindDataW"),
		WA("FreeVirtualFindDataW"),
		WA("SetDirectoryW"),
		WA("GetFilesW"),
		WA("PutFilesW"),
		WA("DeleteFilesW"),
		WA("MakeDirectoryW"),
		WA("ProcessHostFileW"),
		WA("SetFindListW"),
		WA("ConfigureW"),
		WA("ExitFARW"),
		WA("ProcessPanelInputW"),
		WA("ProcessPanelEventW"),
		WA("ProcessEditorEventW"),
		WA("CompareW"),
		WA("ProcessEditorInputW"),
		WA("ProcessViewerEventW"),
		WA("ProcessDialogEventW"),
		WA("ProcessSynchroEventW"),
		WA("ProcessConsoleInputW"),
		WA("AnalyseW"),
		WA("GetCustomDataW"),
		WA("FreeCustomDataW"),
		WA("CloseAnalyseW"),

		WA(""), // OpenFilePlugin not used
		WA(""), // GetMinFarVersion not used
	};
	static_assert(ARRAYSIZE(ExportsNames) == i_LAST, "Not all exports names are defined");
	return ExportsNames;
};

static BOOL PrepareModulePath(const string& ModuleName)
{
	string strModulePath = ModuleName;
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

NetBoxPrivateInfo NetBoxInfo =
{
	sizeof(NetBoxInfo),
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
		PSI->ModuleName = pPlugin->GetModuleName().data();
		if(pPlugin->GetGUID() == ArcliteGuid)
		{
			PSI->Private = &ArcliteInfo;
		}
		else if(pPlugin->GetGUID() == NetBoxGuid)
		{
			PSI->Private = &NetBoxInfo;
		}
		else if(pPlugin->GetGUID() == LuamacroGuid)
		{
			PSI->Private = &MacroInfo;
		}
	}
}

static void ShowMessageAboutIllegalPluginVersion(const string& plg,const VersionInfo& required)
{
	Message(MSG_WARNING|MSG_NOPLUGINS, 1,
		MSG(MError),
		MSG(MPlgBadVers),
		plg.data(),
		(LangString(MPlgRequired) << (FormatString() << required.Major << L'.' << required.Minor << L'.' << required.Revision << L'.' << required.Build)).data(),
		(LangString(MPlgRequired2) << (FormatString() << FAR_VERSION.Major << L'.' << FAR_VERSION.Minor << L'.' << FAR_VERSION.Revision << L'.' << FAR_VERSION.Build)).data(),
		MSG(MOk));
}

bool Plugin::SaveToCache()
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
		FAR_FIND_DATA fdata;
		apiGetFindDataEx(m_strModuleName, fdata);
		strCurPluginID = str_printf(
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
	PlCache.SetFlags(id, Info.Flags);

	PlCache.SetMinFarVersion(id, &MinFarVersion);
	PlCache.SetGuid(id, m_strGuid);
	PlCache.SetVersion(id, &PluginVersion);
	PlCache.SetTitle(id, strTitle);
	PlCache.SetDescription(id, strDescription);
	PlCache.SetAuthor(id, strAuthor);

	auto ExportPtr = Exports;
	std::for_each(ExportsNames, ExportsNames + i_LAST, [&](const export_name& i)
	{
		if (*i.UName)
			PlCache.SetExport(id, i.UName, *ExportPtr != nullptr);
		++ExportPtr;
	});

	PlCache.EndTransaction();

	return true;
}

void Plugin::InitExports()
{
	std::transform(ExportsNames, ExportsNames + i_LAST, Exports, [&](const export_name& i)
	{
		return *i.AName? reinterpret_cast<void*>(GetProcAddress(m_hModule, i.AName)) : nullptr;
	});
}

Plugin::Plugin(PluginManager *owner, const string& ModuleName):
	ExportsNames(GetExportsNames()),
	m_owner(owner),
	Activity(0),
	bPendingRemove(false),
	m_strModuleName(ModuleName),
	m_strCacheName(ModuleName),
	m_hModule(nullptr)
{

	for (size_t i = 0; i != m_strCacheName.size(); ++i)
	{
		if (m_strCacheName[i] == L'\\')
			m_strCacheName[i] = L'/';
	}

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
			Drive[1] = m_strModuleName.front();
			apiGetEnvironmentVariable(Drive, strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);
		m_hModule = LoadLibraryEx(m_strModuleName.data(),nullptr,0);
		if(!m_hModule) m_hModule = LoadLibraryEx(m_strModuleName.data(),nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
		Global->CatchError();
		FarChDir(strCurPath);

		if (Drive[0]) // вернем ее (переменную окружения) обратно
			SetEnvironmentVariable(Drive,strCurPlugDiskPath.data());
	}

	if (!m_hModule)
	{
		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (!Global->Opt->LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			const wchar_t* const Items[] = {MSG(MPlgLoadPluginError), m_strModuleName.data(), MSG(MOk)};
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

bool Plugin::LoadFromCache(const FAR_FIND_DATA &FindData)
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
			string strCurPluginID = str_printf(
				L"%I64x%x%x",
				FindData.nFileSize,
				FindData.ftCreationTime.dwLowDateTime,
				FindData.ftLastWriteTime.dwLowDateTime
				);

			string strPluginID = PlCache.GetSignature(id);

			if (strPluginID != strCurPluginID)   //одинаковые ли бинарники?
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

		std::transform(ExportsNames, ExportsNames + i_LAST, Exports, [&](const export_name& i)
		{
			return  *i.UName? PlCache.GetExport(id, i.UName) : nullptr;
		});

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
	static const int PanelExports[] =
	{
		iSetFindList,
		iGetFindData,
		iGetVirtualFindData,
		iSetDirectory,
		iGetFiles,
		iPutFiles,
		iDeleteFiles,
		iMakeDirectory,
		iProcessHostFile,
		iProcessPanelInput,
		iProcessPanelEvent,
		iCompare,
		iGetOpenPanelInfo,
		iFreeFindData,
		iFreeVirtualFindData,
		iClosePanel,
	};
	return std::any_of(CONST_RANGE(PanelExports, i)
	{
		return Exports[i] != nullptr;
	});
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
		EXECUTE_FUNCTION(es = FUNCTION(iAnalyse)(Info));
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
	Global->g_strDirToSet.clear();
	ExecuteStruct es = {EXCEPT_OPEN};
	if (Load() && Exports[iOpen] && !Global->ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		OpenInfo Info = {sizeof(Info)};
		Info.OpenFrom = static_cast<OPENFROM>(OpenFrom);
		Info.Guid = &Guid;
		Info.Data = Item;
		EXECUTE_FUNCTION(es = FUNCTION(iOpen)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iSetFindList)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessEditorInput)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessEditorEvent)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessViewerEvent)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessDialogEvent)(&Info));
	}
	return es;
}

int Plugin::ProcessSynchroEvent(int Event, void *Param)
{
	ExecuteStruct es = {EXCEPT_PROCESSSYNCHROEVENT};
	if (Load() && Exports[iProcessSynchroEvent] && !Global->ProcessException)
	{
		ProcessSynchroEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessSynchroEvent)(&Info));
	}
	return es;
}

int Plugin::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	ExecuteStruct es = {EXCEPT_PROCESSCONSOLEINPUT};
	if (Load() && Exports[iProcessConsoleInput] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION(es = FUNCTION(iProcessConsoleInput)(Info));
	}
	return es;
}

int Plugin::GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, const string& Path)
{
	ExecuteStruct es = {EXCEPT_GETVIRTUALFINDDATA};
	if (Exports[iGetVirtualFindData] && !Global->ProcessException)
	{
		GetVirtualFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.Path = Path.data();
		EXECUTE_FUNCTION(es = FUNCTION(iGetVirtualFindData)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iGetFiles)(&Info));
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
		Info.SrcPath = strCurrentDirectory.data();
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION(es = FUNCTION(iPutFiles)(&Info));
	}
	return es;
}

int Plugin::DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode)
{
	ExecuteStruct es = {EXCEPT_DELETEFILES};
	if (Exports[iDeleteFiles] && !Global->ProcessException)
	{
		DeleteFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION(es = FUNCTION(iDeleteFiles)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iMakeDirectory)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessHostFile)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessPanelEvent)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iCompare)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iGetFindData)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iProcessPanelInput)(&Info));
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


int Plugin::SetDirectory(HANDLE hPlugin, const string& Dir, int OpMode, UserDataItem *UserData)
{
	ExecuteStruct es = {EXCEPT_SETDIRECTORY};
	if (Exports[iSetDirectory] && !Global->ProcessException)
	{
		SetDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Dir = Dir.data();
		Info.OpMode = OpMode;
		if (UserData)
		{
			Info.UserData.Data = UserData->Data;
			Info.UserData.FreeData = UserData->FreeData;
		}
		EXECUTE_FUNCTION(es = FUNCTION(iSetDirectory)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iConfigure)(&Info));
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
		EXECUTE_FUNCTION(es = FUNCTION(iGetCustomData)(FilePath, CustomData));
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
