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
#include "processname.hpp"
#include "language.hpp"

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
typedef void     (WINAPI *iCloseAnalysePrototype)        (const CloseAnalyseInfo *Info);
typedef intptr_t (WINAPI *iGetCustomDataPrototype)       (const wchar_t *FilePath, wchar_t **CustomData);
typedef void     (WINAPI *iFreeCustomDataPrototype)      (wchar_t *CustomData);

Plugin* GenericPluginModel::CreatePlugin(const string& filename)
{
	return IsPlugin(filename)? new Plugin(this, filename) : nullptr;
}

void GenericPluginModel::SaveExportsToCache(PluginsCacheConfig* cache, unsigned long long id, void* const * exports)
{
	auto ExportPtr = exports;

	std::for_each(m_ExportsNames, m_ExportsNames + ExportsCount, [&](const export_name& i)
	{
		if (*i.UName)
			cache->SetExport(id, i.UName, *ExportPtr != nullptr);
		++ExportPtr;
	});
}

void GenericPluginModel::LoadExportsFromCache(PluginsCacheConfig* cache, unsigned long long id, void** exports)
{
	std::transform(m_ExportsNames, m_ExportsNames + ExportsCount, exports, [&](const export_name& i)
	{
		return *i.UName? cache->GetExport(id, i.UName) : nullptr;
	});
}


GenericPluginModel::GenericPluginModel(PluginManager* owner):
	m_owner(owner)
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
		WA("CloseAnalyseW"),
		WA("GetCustomDataW"),
		WA("FreeCustomDataW"),

		WA(""), // OpenFilePlugin not used
		WA(""), // GetMinFarVersion not used
	};
	static_assert(ARRAYSIZE(ExportsNames) == ExportsCount, "Not all exports names are defined");

	m_ExportsNames = ExportsNames;

}

bool NativePluginModel::IsPlugin(const string& filename)
{
	if (!CmpName(L"*.dll", filename.data(), false))
		return false;

	bool Result = false;
	HANDLE hModuleFile = api::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING);

	if (hModuleFile != INVALID_HANDLE_VALUE)
	{
		HANDLE hModuleMapping = CreateFileMapping(hModuleFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (hModuleMapping)
		{
			const void* pData = MapViewOfFile(hModuleMapping, FILE_MAP_READ, 0, 0, 0);
			if (pData)
			{
				Result = IsPlugin2(pData);
				UnmapViewOfFile(pData);
			}
			CloseHandle(hModuleMapping);
		}
		CloseHandle(hModuleFile);
	}
	return Result;
}

GenericPluginModel::plugin_instance NativePluginModel::Create(const string& filename)
{
	plugin_instance module = LoadLibraryEx(filename.data(), nullptr, 0);
	if(!module)
		module = LoadLibraryEx(filename.data(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
	Global->CatchError();
	return module;
}

bool NativePluginModel::Destroy(GenericPluginModel::plugin_instance instance)
{
	return FreeLibrary(static_cast<HMODULE>(instance)) != FALSE;
}

void NativePluginModel::InitExports(GenericPluginModel::plugin_instance instance, void** exports)
{
	std::transform(m_ExportsNames, m_ExportsNames + ExportsCount, exports, [&](const export_name& i)
	{
		return *i.AName? reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(instance), i.AName)) : nullptr;
	});
}

bool NativePluginModel::FindExport(const char* ExportName)
{
	// only module with GetGlobalInfoW can be native plugin
	return !strcmp(ExportName, m_ExportsNames[iGetGlobalInfo].AName);
}

bool NativePluginModel::IsPlugin2(const void* Module)
{
	try
	{
		auto pDOSHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(Module);
		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		auto pPEHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const char*>(Module) + pDOSHeader->e_lfanew);

		if (pPEHeader->Signature != IMAGE_NT_SIGNATURE)
			return false;

		if (!(pPEHeader->FileHeader.Characteristics & IMAGE_FILE_DLL))
			return false;

		static WORD FarMachineType = 0;
		if (!FarMachineType)
		{
			HMODULE FarModule = GetModuleHandle(nullptr);
			FarMachineType = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const char*>(FarModule) + reinterpret_cast<const IMAGE_DOS_HEADER*>(FarModule)->e_lfanew)->FileHeader.Machine;
		}

		if (pPEHeader->FileHeader.Machine != FarMachineType)
			return false;

		DWORD dwExportAddr = pPEHeader->OptionalHeader.DataDirectory[0].VirtualAddress;

		if (!dwExportAddr)
			return false;

		auto pSection = IMAGE_FIRST_SECTION(pPEHeader);

		for (int i = 0; i < pPEHeader->FileHeader.NumberOfSections; i++)
		{
			if ((pSection[i].VirtualAddress == dwExportAddr) ||
				((pSection[i].VirtualAddress <= dwExportAddr) && ((pSection[i].Misc.VirtualSize+pSection[i].VirtualAddress) > dwExportAddr)))
			{
				int nDiff = pSection[i].VirtualAddress-pSection[i].PointerToRawData;
				auto pExportDir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(reinterpret_cast<const char*>(Module) + dwExportAddr - nDiff);
				auto pNames = reinterpret_cast<const DWORD*>(reinterpret_cast<const char*>(Module) + pExportDir->AddressOfNames-nDiff);
				for (DWORD n = 0; n < pExportDir->NumberOfNames; n++)
				{
					if (FindExport(reinterpret_cast<const char *>(Module) + pNames[n]-nDiff))
					{
						return true;
					}
				}
			}
		}
	}
	catch (SException&)
	{
	}
	return false;
}

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
	pluginapi::apiCallFar,
	pluginapi::apiCallPlugin
};

void CreatePluginStartupInfo(const Plugin* pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF)
{
	*PSI=NativeInfo;
	*FSF=NativeFSF;
	PSI->FSF=FSF;

	if (pPlugin)
	{
		PSI->ModuleName = pPlugin->GetModuleName().data();
		if (pPlugin->GetGUID() == Global->Opt->KnownIDs.Arclite.Id)
		{
			PSI->Private = &ArcliteInfo;
		}
		else if (pPlugin->GetGUID() == Global->Opt->KnownIDs.Netbox.Id)
		{
			PSI->Private = &NetBoxInfo;
		}
		else if (pPlugin->GetGUID() == Global->Opt->KnownIDs.Luamacro.Id)
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

void Plugin::ExecuteFunction(ExecuteStruct& es, const std::function<void()>& f)
{
	Prologue();
	++Activity;
	try
	{
		f();
	}
	catch (SException &e)
	{
		if (ProcessSEHException(this, m_model->GetExportName(es.id), e.GetInfo()))
		{
			m_model->GetOwner()->UnloadPlugin(this, es.id);
			es.Result = es.Default;
			Global->ProcessException=FALSE;
		}
		else
		{
			throw;
		}
	}
	catch (std::exception &e)
	{
		if (ProcessStdException(e, this, m_model->GetExportName(es.id)))
		{
			m_model->GetOwner()->UnloadPlugin(this, es.id);
			es.Result = es.Default;
			Global->ProcessException = FALSE;
		}
		else
		{
			throw;
		}
	}
	--Activity;
	Epilogue();
}

bool Plugin::SaveToCache()
{
	PluginInfo Info = {sizeof(Info)};
	GetPluginInfo(&Info);

	PluginsCacheConfig& PlCache = *Global->Db->PlCacheCfg();

	SCOPED_ACTION(auto)(PlCache.ScopedTransaction());

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
		api::FAR_FIND_DATA fdata;
		api::GetFindDataEx(m_strModuleName, fdata);
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

	m_model->SaveExportsToCache(&PlCache, id, Exports);

	return true;
}

void Plugin::InitExports()
{
	m_model->InitExports(m_Instance, Exports);
}

Plugin::Plugin(GenericPluginModel* model, const string& ModuleName):
	m_model(model),
	Activity(0),
	bPendingRemove(false),
	m_strModuleName(ModuleName),
	m_strCacheName(ModuleName),
	m_Instance(nullptr)
{
	std::replace(ALL_RANGE(m_strCacheName), L'\\', L'/');
	ClearExports();
	SetGuid(FarGuid);
}

Plugin::~Plugin()
{
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
	if(PluginVersion.Stage != VS_RELEASE && static_cast<size_t>(PluginVersion.Stage) < ARRAYSIZE(Stage))
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

	if (m_Instance)
		return true;

	if (!m_Instance)
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[]={0,L' ',L':',0}; //ставим 0, как признак того, что вертать обратно ненадо!
		api::GetCurrentDirectory(strCurPath);

		if (ParsePath(m_strModuleName) == PATH_DRIVELETTER)  // если указан локальный путь, то...
		{
			Drive[0] = L'=';
			Drive[1] = m_strModuleName.front();
			api::env::get_variable(Drive, strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);
		m_Instance = m_model->Create(m_strModuleName);
		FarChDir(strCurPath);

		if (Drive[0]) // вернем ее (переменную окружения) обратно
			api::env::set_variable(Drive, strCurPlugDiskPath);
	}

	if (!m_Instance)
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
		m_model->GetOwner()->UndoRemove(this);
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
			ok = m_model->GetOwner()->UpdateId(this, Info.Guid);
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

	if (WorkFlags.Check(PIWF_LOADED))
		return true;

	WorkFlags.Set(PIWF_LOADED);

	bool Inited = false;

	if (CheckMinFarVersion())
	{
		PluginStartupInfo info;
		FarStandardFunctions fsf;
		CreatePluginStartupInfo(this, &info, &fsf);
		Inited = SetStartupInfo(&info);
	}

	if (!Inited)
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

bool Plugin::LoadFromCache(const api::FAR_FIND_DATA &FindData)
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

		m_model->LoadExportsFromCache(&PlCache, id, Exports);

		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags
		return true;
	}
	return false;
}

int Plugin::Unload(bool bExitFAR)
{
	int nResult = TRUE;

	if (WorkFlags.Check(PIWF_LOADED))
	{
		if (bExitFAR)
		{
			ExitInfo Info={sizeof(Info)};
			ExitFAR(&Info);
		}

		if (!WorkFlags.Check(PIWF_CACHED))
		{
			nResult = m_model->Destroy(m_Instance);
			ClearExports();
		}

		m_Instance = nullptr;
		WorkFlags.Clear(PIWF_LOADED);
		WorkFlags.Clear(PIWF_DATALOADED);
		bPendingRemove = true;
	}

	return nResult;
}

void Plugin::ClearExports()
{
	ClearArray(Exports);
}

void Plugin::AddDialog(window_ptr Dlg)
{
	m_dialogs.insert(Dlg);
}

bool Plugin::RemoveDialog(window_ptr Dlg)
{
	auto ItemIterator = m_dialogs.find(Dlg);
	if (ItemIterator != m_dialogs.cend())
	{
		m_dialogs.erase(ItemIterator);
		return true;
	}
	return false;
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

bool Plugin::SetStartupInfo(PluginStartupInfo *Info)
{
	ExecuteStruct es = {iSetStartupInfo};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iSetStartupInfo)(Info));

		if (bPendingRemove)
		{
			return false;
		}
	}
	return true;
}

bool Plugin::GetGlobalInfo(GlobalInfo *gi)
{
	ExecuteStruct es = {iGetGlobalInfo};
	if (Exports[es.id])
	{
		gi->Instance = m_Instance;
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

bool Plugin::InitLang(const string& Path)
{
	bool Result = true;
	if (!PluginLang)
	{
		try
		{
			PluginLang = std::make_unique<Language>(Path);
		}
		catch (const std::exception&)
		{
			Result = false;
		}
	}
	return Result;
}

void Plugin::CloseLang()
{
	PluginLang.reset();
}

const wchar_t* Plugin::GetMsg(LNGID nID) const
{
	return PluginLang->GetMsg(nID);
}

void* Plugin::OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode)
{
	return nullptr;
}

void* Plugin::Analyse(AnalyseInfo *Info)
{
	ExecuteStruct es = {iAnalyse};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iAnalyse)(Info));
	}
	return es;
}

void Plugin::CloseAnalyse(CloseAnalyseInfo* Info)
{
	ExecuteStruct es = {iCloseAnalyse};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iCloseAnalyse)(Info));
	}
}

void* Plugin::Open(OpenInfo* Info)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	CheckScreenLock(); //??
	ExecuteStruct es = {iOpen};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iOpen)(Info));
	}
	return es;
}

int Plugin::SetFindList(SetFindListInfo* Info)
{
	ExecuteStruct es = {iSetFindList};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iSetFindList)(Info));
	}
	return es;
}

int Plugin::ProcessEditorInput(ProcessEditorInputInfo* Info)
{
	ExecuteStruct es = {iProcessEditorInput};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessEditorInput)(Info));
	}
	return es;
}

int Plugin::ProcessEditorEvent(ProcessEditorEventInfo* Info)
{
	ExecuteStruct es = {iProcessEditorEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessEditorEvent)(Info));
	}
	return es;
}

int Plugin::ProcessViewerEvent(ProcessViewerEventInfo* Info)
{
	ExecuteStruct es = {iProcessViewerEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessViewerEvent)(Info));
	}
	return es;
}

int Plugin::ProcessDialogEvent(ProcessDialogEventInfo* Info)
{
	ExecuteStruct es = {iProcessDialogEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessDialogEvent)(Info));
	}
	return es;
}

int Plugin::ProcessSynchroEvent(ProcessSynchroEventInfo* Info)
{
	ExecuteStruct es = {iProcessSynchroEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessSynchroEvent)(Info));
	}
	return es;
}

int Plugin::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	ExecuteStruct es = {iProcessConsoleInput};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessConsoleInput)(Info));
	}
	return es;
}

int Plugin::GetVirtualFindData(GetVirtualFindDataInfo* Info)
{
	ExecuteStruct es = {iGetVirtualFindData};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iGetVirtualFindData)(Info));
	}
	return es;
}

void Plugin::FreeVirtualFindData(FreeFindDataInfo* Info)
{
	ExecuteStruct es = {iFreeVirtualFindData};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iFreeVirtualFindData)(Info));
	}
}

int Plugin::GetFiles(GetFilesInfo* Info)
{
	ExecuteStruct es = {iGetFiles, -1};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iGetFiles)(Info));
	}
	return es;
}

int Plugin::PutFiles(PutFilesInfo* Info)
{
	ExecuteStruct es = {iPutFiles, -1};

	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iPutFiles)(Info));
	}
	return es;
}

int Plugin::DeleteFiles(DeleteFilesInfo* Info)
{
	ExecuteStruct es = {iDeleteFiles};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iDeleteFiles)(Info));
	}
	return es;
}

int Plugin::MakeDirectory(MakeDirectoryInfo* Info)
{
	ExecuteStruct es = {iMakeDirectory, -1};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iMakeDirectory)(Info));
	}
	return es;
}

int Plugin::ProcessHostFile(ProcessHostFileInfo* Info)
{
	ExecuteStruct es = {iProcessHostFile};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessHostFile)(Info));
	}
	return es;
}

int Plugin::ProcessPanelEvent(ProcessPanelEventInfo* Info)
{
	ExecuteStruct es = {iProcessPanelEvent};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessPanelEvent)(Info));
	}
	return es;
}

int Plugin::Compare(CompareInfo* Info)
{
	ExecuteStruct es = {iCompare, -2};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iCompare)(Info));
	}
	return es;
}

int Plugin::GetFindData(GetFindDataInfo* Info)
{
	ExecuteStruct es = {iGetFindData};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iGetFindData)(Info));
	}
	return es;
}

void Plugin::FreeFindData(FreeFindDataInfo* Info)
{
	ExecuteStruct es = {iFreeFindData};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iFreeFindData)(Info));
	}
}

int Plugin::ProcessPanelInput(ProcessPanelInputInfo* Info)
{
	ExecuteStruct es = {iProcessPanelInput};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iProcessPanelInput)(Info));
	}
	return es;
}


void Plugin::ClosePanel(ClosePanelInfo* Info)
{
	ExecuteStruct es = {iClosePanel};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iClosePanel)(Info));
	}
}


int Plugin::SetDirectory(SetDirectoryInfo* Info)
{
	ExecuteStruct es = {iSetDirectory};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iSetDirectory)(Info));
	}
	return es;
}

void Plugin::GetOpenPanelInfo(OpenPanelInfo* Info)
{
	ExecuteStruct es = {iGetOpenPanelInfo};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iGetOpenPanelInfo)(Info));
	}
}


int Plugin::Configure(ConfigureInfo* Info)
{
	ExecuteStruct es = {iConfigure};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iConfigure)(Info));
	}
	return es;
}


bool Plugin::GetPluginInfo(PluginInfo* Info)
{
	ExecuteStruct es = {iGetPluginInfo};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iGetPluginInfo)(Info));
		if (!bPendingRemove)
			return true;
	}
	return false;
}

int Plugin::GetCustomData(const wchar_t *FilePath, wchar_t **CustomData)
{
	ExecuteStruct es = {iGetCustomData};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		//Info->Instance = m_Instance;
		EXECUTE_FUNCTION(es = FUNCTION(iGetCustomData)(FilePath, CustomData));
	}
	return es;
}

void Plugin::FreeCustomData(wchar_t *CustomData)
{
	ExecuteStruct es = {iFreeCustomData};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		//Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iFreeCustomData)(CustomData));
	}
}

void Plugin::ExitFAR(ExitInfo *Info)
{
	ExecuteStruct es = {iExitFAR};
	if (Exports[es.id] && !Global->ProcessException)
	{
		Info->Instance = m_Instance;
		EXECUTE_FUNCTION(FUNCTION(iExitFAR)(Info));
	}
}


CustomPluginModel::CustomPluginModel(PluginManager* owner, const string& filename):
	GenericPluginModel(owner),
	m_Success(false)
{
	try
	{
		m_Module = LoadLibraryEx(filename.data(), nullptr, 0);

#define InitImport(Name) InitImport(m_Imports.p##Name, #Name)

		if (InitImport(Initialize) &&
			InitImport(IsPlugin) &&
			InitImport(CreateInstance) &&
			InitImport(GetFunctionAddress) &&
			InitImport(DestroyInstance) &&
			InitImport(Free))

		{
			GlobalInfo Info={sizeof(Info)};

			if (m_Imports.pInitialize(&Info) &&
				Info.StructSize &&
				Info.Title && *Info.Title &&
				Info.Description && *Info.Description &&
				Info.Author && *Info.Author)
			{
				m_Success = CheckVersion(&FAR_VERSION, &Info.MinFarVersion) != FALSE;

				// TODO: store info, show message if version is bad
			}
		}
#undef InitImport
	}
	catch(const SException&)
	{
		// TODO: notification
		throw;
	}
};

CustomPluginModel::~CustomPluginModel()
{
	try
	{
		if (m_Success)
		{
			ExitInfo Info = {sizeof(Info)};
			m_Imports.pFree(&Info);
		}
		FreeLibrary(m_Module);
	}
	catch(const SException&)
	{
		// TODO: notification
	}
}

bool CustomPluginModel::IsPlugin(const string& filename)
{
	try
	{
		return m_Imports.pIsPlugin(filename.data()) != FALSE;
	}
	catch(const SException&)
	{
		// TODO: notification
		throw;
		//return false;
	}
}

GenericPluginModel::plugin_instance CustomPluginModel::Create(const string& filename)
{
	try
	{
		return m_Imports.pCreateInstance(filename.data());
	}
	catch(const SException&)
	{
		// TODO: notification
		throw;
		//return nullptr;
	}
}

void CustomPluginModel::InitExports(GenericPluginModel::plugin_instance instance, void** exports)
{
	try
	{
		std::transform(m_ExportsNames, m_ExportsNames + ExportsCount, exports, [&](const export_name& i)
		{
			return *i.UName ? reinterpret_cast<void*>(m_Imports.pGetFunctionAddress(static_cast<HANDLE>(instance), i.UName)) : nullptr;
		});
	}
	catch(const SException&)
	{
		// TODO: notification
		throw;
	}
}

bool CustomPluginModel::Destroy(GenericPluginModel::plugin_instance module)
{
	try
	{
		return m_Imports.pDestroyInstance(module) != FALSE;
	}
	catch(const SException&)
	{
		// TODO: notification
		throw;
		//return false;
	}
}
