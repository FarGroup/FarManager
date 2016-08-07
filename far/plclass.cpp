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
#include "config.hpp"
#include "farexcpt.hpp"
#include "chgprior.hpp"
#include "farversion.hpp"
#include "plugapi.hpp"
#include "message.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "FarGuid.hpp"
#include "processname.hpp"
#include "language.hpp"

DECLARE_PLUGIN_FUNCTION(iClosePanel,          void     (WINAPI*)(const ClosePanelInfo *Info))
DECLARE_PLUGIN_FUNCTION(iCompare,             intptr_t (WINAPI*)(const CompareInfo *Info))
DECLARE_PLUGIN_FUNCTION(iConfigure,           intptr_t (WINAPI*)(const ConfigureInfo *Info))
DECLARE_PLUGIN_FUNCTION(iDeleteFiles,         intptr_t (WINAPI*)(const DeleteFilesInfo *Info))
DECLARE_PLUGIN_FUNCTION(iExitFAR,             void     (WINAPI*)(const ExitInfo *Info))
DECLARE_PLUGIN_FUNCTION(iFreeFindData,        void     (WINAPI*)(const FreeFindDataInfo *Info))
DECLARE_PLUGIN_FUNCTION(iFreeVirtualFindData, void     (WINAPI*)(const FreeFindDataInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetFiles,            intptr_t (WINAPI*)(GetFilesInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetFindData,         intptr_t (WINAPI*)(GetFindDataInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetGlobalInfo,       void     (WINAPI*)(GlobalInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetOpenPanelInfo,    void     (WINAPI*)(OpenPanelInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetPluginInfo,       void     (WINAPI*)(PluginInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetVirtualFindData,  intptr_t (WINAPI*)(GetVirtualFindDataInfo *Info))
DECLARE_PLUGIN_FUNCTION(iMakeDirectory,       intptr_t (WINAPI*)(MakeDirectoryInfo *Info))
DECLARE_PLUGIN_FUNCTION(iOpen,                HANDLE   (WINAPI*)(const OpenInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessEditorEvent,  intptr_t (WINAPI*)(const ProcessEditorEventInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessEditorInput,  intptr_t (WINAPI*)(const ProcessEditorInputInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessPanelEvent,   intptr_t (WINAPI*)(const ProcessPanelEventInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessHostFile,     intptr_t (WINAPI*)(const ProcessHostFileInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessPanelInput,   intptr_t (WINAPI*)(const ProcessPanelInputInfo *Info))
DECLARE_PLUGIN_FUNCTION(iPutFiles,            intptr_t (WINAPI*)(const PutFilesInfo *Info))
DECLARE_PLUGIN_FUNCTION(iSetDirectory,        intptr_t (WINAPI*)(const SetDirectoryInfo *Info))
DECLARE_PLUGIN_FUNCTION(iSetFindList,         intptr_t (WINAPI*)(const SetFindListInfo *Info))
DECLARE_PLUGIN_FUNCTION(iSetStartupInfo,      void     (WINAPI*)(const PluginStartupInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessViewerEvent,  intptr_t (WINAPI*)(const ProcessViewerEventInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessDialogEvent,  intptr_t (WINAPI*)(const ProcessDialogEventInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessSynchroEvent, intptr_t (WINAPI*)(const ProcessSynchroEventInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessConsoleInput, intptr_t (WINAPI*)(const ProcessConsoleInputInfo *Info))
DECLARE_PLUGIN_FUNCTION(iAnalyse,             HANDLE   (WINAPI*)(const AnalyseInfo *Info))
DECLARE_PLUGIN_FUNCTION(iCloseAnalyse,        void     (WINAPI*)(const CloseAnalyseInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetContentFields,    intptr_t (WINAPI*)(const GetContentFieldsInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetContentData,      intptr_t (WINAPI*)(GetContentDataInfo *Info))
DECLARE_PLUGIN_FUNCTION(iFreeContentData,     void     (WINAPI*)(const GetContentDataInfo *Info))

std::unique_ptr<Plugin> plugin_factory::CreatePlugin(const string& filename)
{
	return IsPlugin(filename)? std::make_unique<Plugin>(this, filename) : nullptr;
}

plugin_factory::plugin_factory(PluginManager* owner):
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
		WA("GetContentFieldsW"),
		WA("GetContentDataW"),
		WA("FreeContentDataW"),

		WA(""), // OpenFilePlugin not used
		WA(""), // GetMinFarVersion not used
	};
	static_assert(std::size(ExportsNames) == ExportsCount, "Not all exports names are defined");
	m_ExportsNames = make_range(ALL_CONST_RANGE(ExportsNames));
}

bool native_plugin_factory::IsPlugin(const string& filename) const
{
	if (!CmpName(L"*.dll", filename.data(), false))
		return false;

	const auto ModuleFile = os::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
	if (!ModuleFile)
		return false;

	const auto ModuleMapping = os::handle(CreateFileMapping(ModuleFile.native_handle(), nullptr, PAGE_READONLY, 0, 0, nullptr));
	if (!ModuleMapping)
		return false;

	const auto Data = MapViewOfFile(ModuleMapping.native_handle(), FILE_MAP_READ, 0, 0, 0);
	if (!Data)
		return false;

	SCOPE_EXIT{ UnmapViewOfFile(Data); };
	return IsPlugin2(Data);
}

plugin_factory::plugin_module_ptr native_plugin_factory::Create(const string& filename)
{
	auto Module = std::make_unique<native_plugin_module>(filename);
	if (!Module->m_Module)
	{
		Global->CatchError();
		Module.reset();
	}
	return Module;
}

bool native_plugin_factory::Destroy(plugin_factory::plugin_module_ptr& instance)
{
	instance.reset();
	return true;
}

plugin_factory::function_address native_plugin_factory::GetFunction(const plugin_factory::plugin_module_ptr& Instance, const plugin_factory::export_name& Name)
{
	return Name.AName? static_cast<native_plugin_module*>(Instance.get())->m_Module.GetProcAddress(Name.AName) : nullptr;
}

bool native_plugin_factory::FindExport(const char* ExportName) const
{
	// only module with GetGlobalInfoW can be native plugin
	return !strcmp(ExportName, m_ExportsNames[iGetGlobalInfo].AName);
}

bool native_plugin_factory::IsPlugin2(const void* Module) const
{
	try
	{
		const auto pDOSHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(Module);
		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		const auto pPEHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const char*>(Module) + pDOSHeader->e_lfanew);

		if (pPEHeader->Signature != IMAGE_NT_SIGNATURE)
			return false;

		if (!(pPEHeader->FileHeader.Characteristics & IMAGE_FILE_DLL))
			return false;

		const auto GetMachineType = []
		{
			const auto FarModule = GetModuleHandle(nullptr);
			return reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const char*>(FarModule) + reinterpret_cast<const IMAGE_DOS_HEADER*>(FarModule)->e_lfanew)->FileHeader.Machine;
		};

		static const auto FarMachineType = GetMachineType();

		if (pPEHeader->FileHeader.Machine != FarMachineType)
			return false;

		const auto dwExportAddr = pPEHeader->OptionalHeader.DataDirectory[0].VirtualAddress;

		if (!dwExportAddr)
			return false;

		for (const auto& Section: make_range(IMAGE_FIRST_SECTION(pPEHeader), pPEHeader->FileHeader.NumberOfSections))
		{
			if ((Section.VirtualAddress == dwExportAddr) ||
				((Section.VirtualAddress <= dwExportAddr) && ((Section.Misc.VirtualSize + Section.VirtualAddress) > dwExportAddr)))
			{
				const auto GetAddress = [&](size_t Offset)
				{
					return reinterpret_cast<const char*>(Module) + Section.PointerToRawData - Section.VirtualAddress + Offset;
				};

				const auto pExportDir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(GetAddress(dwExportAddr));
				const auto pNames = reinterpret_cast<const DWORD*>(GetAddress(pExportDir->AddressOfNames));

				if (std::any_of(pNames, pNames + pExportDir->NumberOfNames, [&](DWORD NameOffset)
				{
					return FindExport(reinterpret_cast<const char *>(GetAddress(NameOffset)));
				}))
				{
					return true;
				}
			}
		}
	}
	catch (const SException&)
	{
		// TODO: log
	}
	return false;
}

static BOOL PrepareModulePath(const string& ModuleName)
{
	string strModulePath = ModuleName;
	CutToSlash(strModulePath); //??
	return FarChDir(strModulePath);
}

FarStandardFunctions NativeFSF =
{
	sizeof(NativeFSF),
	pluginapi::apiAtoi,
	pluginapi::apiAtoi64,
	pluginapi::apiItoa,
	pluginapi::apiItoa64,
	pluginapi::apiSprintf,
	pluginapi::apiSscanf,
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
	pluginapi::apiFarClock,
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

static ArclitePrivateInfo ArcliteInfo =
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

static NetBoxPrivateInfo NetBoxInfo =
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

static MacroPrivateInfo MacroInfo =
{
	sizeof(MacroPrivateInfo),
	pluginapi::apiCallFar,
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
	Message(MSG_WARNING|MSG_NOPLUGINS,
		MSG(MError),
		{
			MSG(MPlgBadVers),
			plg,
			string_format(MPlgRequired, (FormatString() << required.Major << L'.' << required.Minor << L'.' << required.Revision << L'.' << required.Build)),
			string_format(MPlgRequired2, (FormatString() << FAR_VERSION.Major << L'.' << FAR_VERSION.Minor << L'.' << FAR_VERSION.Revision << L'.' << FAR_VERSION.Build))
		},
		{ MSG(MOk) }
	);
}

static string MakeSignature(const os::FAR_FIND_DATA& Data)
{
	return to_hex_wstring(Data.nFileSize) + to_hex_wstring(Data.ftCreationTime.dwLowDateTime) + to_hex_wstring(Data.ftLastWriteTime.dwLowDateTime);
}

bool Plugin::SaveToCache()
{
	PluginInfo Info = {sizeof(Info)};
	GetPluginInfo(&Info);

	auto& PlCache = ConfigProvider().PlCacheCfg();

	SCOPED_ACTION(auto)(PlCache->ScopedTransaction());

	PlCache->DeleteCache(m_strCacheName);
	const auto id = PlCache->CreateCache(m_strCacheName);

	{
		bool bPreload = (Info.Flags & PF_PRELOAD);
		PlCache->SetPreload(id, bPreload);
		WorkFlags.Change(PIWF_PRELOADED, bPreload);

		if (bPreload)
		{
			PlCache->EndTransaction();
			return true;
		}
	}

	{
		os::FAR_FIND_DATA fdata;
		os::GetFindDataEx(m_strModuleName, fdata);
		PlCache->SetSignature(id, MakeSignature(fdata));
	}

	const auto SaveItems = [&PlCache, &id](const auto& Setter, const auto& Item)
	{
		for (size_t i = 0; i != Item.Count; ++i)
		{
			((*PlCache).*Setter)(id, i, Item.Strings[i], Item.Guids[i]);
		}
	};

	SaveItems(&PluginsCacheConfig::SetPluginsMenuItem, Info.PluginMenu);
	SaveItems(&PluginsCacheConfig::SetDiskMenuItem, Info.DiskMenu);
	SaveItems(&PluginsCacheConfig::SetPluginsConfigMenuItem, Info.PluginConfig);

	PlCache->SetCommandPrefix(id, NullToEmpty(Info.CommandPrefix));
	PlCache->SetFlags(id, Info.Flags);

	PlCache->SetMinFarVersion(id, &MinFarVersion);
	PlCache->SetGuid(id, m_strGuid);
	PlCache->SetVersion(id, &PluginVersion);
	PlCache->SetTitle(id, strTitle);
	PlCache->SetDescription(id, strDescription);
	PlCache->SetAuthor(id, strAuthor);

	for (auto i: zip(m_Factory->ExportsNames(), Exports))
	{
		PlCache->SetExportState(id, std::get<0>(i).UName, std::get<1>(i).second);
	}

	return true;
}

void Plugin::InitExports()
{
	std::transform(ALL_CONST_RANGE(m_Factory->ExportsNames()), Exports.begin(), [&](const auto& i)
	{
		const auto Address = m_Factory->GetFunction(m_Instance, i);
		return std::make_pair(Address, Address != nullptr);
	});
}

Plugin::Plugin(plugin_factory* Factory, const string& ModuleName):
	Exports(),
	m_Factory(Factory),
	Activity(0),
	bPendingRemove(false),
	m_strModuleName(ModuleName),
	m_strCacheName(ModuleName)
{
	ReplaceBackslashToSlash(m_strCacheName);
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

static string VersionToString(const VersionInfo& PluginVersion)
{
	const wchar_t* Stage[] = { L" Release", L" Alpha", L" Beta", L" RC"};
	FormatString strVersion;
	strVersion << PluginVersion.Major << L"." << PluginVersion.Minor << L"." << PluginVersion.Revision << L" (build " << PluginVersion.Build <<L")";
	if(PluginVersion.Stage != VS_RELEASE && static_cast<size_t>(PluginVersion.Stage) < std::size(Stage))
	{
		strVersion << Stage[PluginVersion.Stage];
	}
	return strVersion;
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
		string strCurPlugDiskPath;
		wchar_t Drive[]={0,L' ',L':',0}; //ставим 0, как признак того, что вертать обратно ненадо!
		const auto strCurPath = os::GetCurrentDirectory();

		if (ParsePath(m_strModuleName) == PATH_DRIVELETTER)  // если указан локальный путь, то...
		{
			Drive[0] = L'=';
			Drive[1] = m_strModuleName.front();
			strCurPlugDiskPath = os::env::get_variable(Drive);
		}

		PrepareModulePath(m_strModuleName);
		m_Instance = m_Factory->Create(m_strModuleName);
		FarChDir(strCurPath);

		if (Drive[0]) // вернем ее (переменную окружения) обратно
			os::env::set_variable(Drive, strCurPlugDiskPath);
	}

	if (!m_Instance)
	{
		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (!Global->Opt->LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			Message(MSG_WARNING|MSG_ERRORTYPE|MSG_NOPLUGINS, MSG(MError),
				{ MSG(MPlgLoadPluginError), m_strModuleName },
				{ MSG(MOk) },
				L"ErrLoadPlugin");
		}

		return false;
	}

	WorkFlags.Clear(PIWF_CACHED);

	if(bPendingRemove)
	{
		bPendingRemove = false;
		m_Factory->GetOwner()->UndoRemove(this);
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
		VersionString = VersionToString(PluginVersion);
		strTitle = Info.Title;
		strDescription = Info.Description;
		strAuthor = Info.Author;

		bool ok=true;
		if(m_Guid != FarGuid && m_Guid != Info.Guid)
		{
			ok = m_Factory->GetOwner()->UpdateId(this, Info.Guid);
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

bool Plugin::LoadFromCache(const os::FAR_FIND_DATA &FindData)
{
	const auto& PlCache = ConfigProvider().PlCacheCfg();

	if (const auto id = PlCache->GetCacheID(m_strCacheName))
	{
		if (PlCache->IsPreload(id))   //PF_PRELOAD plugin, skip cache
		{
			WorkFlags.Set(PIWF_PRELOADED);
			return false;
		}

		{
			const auto strCurPluginID = MakeSignature(FindData);
			const auto strPluginID = PlCache->GetSignature(id);

			if (strPluginID != strCurPluginID)   //одинаковые ли бинарники?
				return false;
		}

		if (!PlCache->GetMinFarVersion(id, &MinFarVersion))
		{
			MinFarVersion = FAR_VERSION;
		}

		if (!PlCache->GetVersion(id, &PluginVersion))
		{
			ClearStruct(PluginVersion);
		}
		VersionString = VersionToString(PluginVersion);

		m_strGuid = PlCache->GetGuid(id);
		SetGuid(StrToGuid(m_strGuid,m_Guid)?m_Guid:FarGuid);
		strTitle = PlCache->GetTitle(id);
		strDescription = PlCache->GetDescription(id);
		strAuthor = PlCache->GetAuthor(id);

		std::transform(ALL_CONST_RANGE(m_Factory->ExportsNames()), Exports.begin(), [&PlCache, &id](const auto& i)
		{
			return std::make_pair(nullptr, PlCache->GetExportState(id, i.UName));
		});

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
			nResult = m_Factory->Destroy(m_Instance);
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
	Exports.fill({ nullptr, false });
}

void Plugin::AddDialog(window_ptr_ref Dlg)
{
	m_dialogs.emplace(Dlg);
}

bool Plugin::RemoveDialog(window_ptr_ref Dlg)
{
	const auto ItemIterator = m_dialogs.find(Dlg);
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
		return Exports[i].second;
	});
}

bool Plugin::SetStartupInfo(PluginStartupInfo *Info)
{
	ExecuteStruct<iSetStartupInfo> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);

		if (bPendingRemove)
		{
			return false;
		}
	}
	return true;
}

bool Plugin::GetGlobalInfo(GlobalInfo *gi)
{
	ExecuteStruct<iGetGlobalInfo> es;
	if (has(es))
	{
		gi->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, gi);
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
	ExecuteStruct<iAnalyse> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

void Plugin::CloseAnalyse(CloseAnalyseInfo* Info)
{
	ExecuteStruct<iCloseAnalyse> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}

void* Plugin::Open(OpenInfo* Info)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	ExecuteStruct<iOpen> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::SetFindList(SetFindListInfo* Info)
{
	ExecuteStruct<iSetFindList> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessEditorInput(ProcessEditorInputInfo* Info)
{
	ExecuteStruct<iProcessEditorInput> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessEditorEvent(ProcessEditorEventInfo* Info)
{
	ExecuteStruct<iProcessEditorEvent> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessViewerEvent(ProcessViewerEventInfo* Info)
{
	ExecuteStruct<iProcessViewerEvent> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessDialogEvent(ProcessDialogEventInfo* Info)
{
	ExecuteStruct<iProcessDialogEvent> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessSynchroEvent(ProcessSynchroEventInfo* Info)
{
	ExecuteStruct<iProcessSynchroEvent> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	ExecuteStruct<iProcessConsoleInput> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::GetVirtualFindData(GetVirtualFindDataInfo* Info)
{
	ExecuteStruct<iGetVirtualFindData> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

void Plugin::FreeVirtualFindData(FreeFindDataInfo* Info)
{
	ExecuteStruct<iFreeVirtualFindData> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}

int Plugin::GetFiles(GetFilesInfo* Info)
{
	ExecuteStruct<iGetFiles> es(-1);
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::PutFiles(PutFilesInfo* Info)
{
	ExecuteStruct<iPutFiles> es(-1);

	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::DeleteFiles(DeleteFilesInfo* Info)
{
	ExecuteStruct<iDeleteFiles> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::MakeDirectory(MakeDirectoryInfo* Info)
{
	ExecuteStruct<iMakeDirectory> es(-1);
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessHostFile(ProcessHostFileInfo* Info)
{
	ExecuteStruct<iProcessHostFile> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::ProcessPanelEvent(ProcessPanelEventInfo* Info)
{
	ExecuteStruct<iProcessPanelEvent> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::Compare(CompareInfo* Info)
{
	ExecuteStruct<iCompare> es(-2);
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::GetFindData(GetFindDataInfo* Info)
{
	ExecuteStruct<iGetFindData> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

void Plugin::FreeFindData(FreeFindDataInfo* Info)
{
	ExecuteStruct<iFreeFindData> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}

int Plugin::ProcessPanelInput(ProcessPanelInputInfo* Info)
{
	ExecuteStruct<iProcessPanelInput> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}


void Plugin::ClosePanel(ClosePanelInfo* Info)
{
	ExecuteStruct<iClosePanel> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}


int Plugin::SetDirectory(SetDirectoryInfo* Info)
{
	ExecuteStruct<iSetDirectory> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

void Plugin::GetOpenPanelInfo(OpenPanelInfo* Info)
{
	ExecuteStruct<iGetOpenPanelInfo> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}


int Plugin::Configure(ConfigureInfo* Info)
{
	ExecuteStruct<iConfigure> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}


bool Plugin::GetPluginInfo(PluginInfo* Info)
{
	ExecuteStruct<iGetPluginInfo> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
		if (!bPendingRemove)
			return true;
	}
	return false;
}

int Plugin::GetContentFields(const GetContentFieldsInfo *Info)
{
	ExecuteStruct<iGetContentFields> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		const_cast<GetContentFieldsInfo*>(Info)->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

int Plugin::GetContentData(GetContentDataInfo *Info)
{
	ExecuteStruct<iGetContentData> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
	return es;
}

void Plugin::FreeContentData(const GetContentDataInfo *Info)
{
	ExecuteStruct<iFreeContentData> es;
	if (Load() && has(es) && !Global->ProcessException)
	{
		const_cast<GetContentDataInfo*>(Info)->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}

void Plugin::ExitFAR(ExitInfo *Info)
{
	ExecuteStruct<iExitFAR> es;
	if (has(es) && !Global->ProcessException)
	{
		Info->Instance = m_Instance->get_opaque();
		ExecuteFunction(es, Info);
	}
}

void Plugin::HandleFailure(EXPORTS_ENUM id)
{
	m_Factory->GetOwner()->UnloadPlugin(this, id);
	Global->ProcessException = FALSE;
}

class custom_plugin_module: public i_plugin_module
{
public:
	NONCOPYABLE(custom_plugin_module);
	custom_plugin_module(void* Opaque) : m_Opaque(Opaque) {}
	virtual void* get_opaque() const override { return m_Opaque; }

private:
	void* m_Opaque;
};

class custom_plugin_factory: public plugin_factory
{
public:
	NONCOPYABLE(custom_plugin_factory);
	custom_plugin_factory(PluginManager* Owner, const string& Filename):
		plugin_factory(Owner),
		m_Module(Filename),
		m_Imports(m_Module),
		m_Success(false)
	{
		try
		{
			GlobalInfo Info = { sizeof(Info) };

			if (m_Imports.IsValid() &&
				m_Imports.pInitialize(&Info) &&
				Info.StructSize &&
				Info.Title && *Info.Title &&
				Info.Description && *Info.Description &&
				Info.Author && *Info.Author)
			{
				m_Success = CheckVersion(&FAR_VERSION, &Info.MinFarVersion) != FALSE;

				// TODO: store info, show message if version is bad
			}
		}
		catch (const SException&)
		{
			// TODO: notification
			throw;
		}
	}

	~custom_plugin_factory()
	{
		try
		{
			if (m_Success)
			{
				ExitInfo Info = { sizeof(Info) };
				m_Imports.pFree(&Info);
			}
		}
		catch (const SException&)
		{
			// TODO: notification
		}
	}

	bool Success() const { return m_Success; }

	virtual bool IsPlugin(const string& Filename) const override
	{
		try
		{
			return m_Imports.pIsPlugin(Filename.data()) != FALSE;
		}
		catch (const SException&)
		{
			// TODO: notification
			throw;
			//return false;
		}
	}

	virtual plugin_module_ptr Create(const string& Filename) override
	{
		try
		{
			return std::make_unique<custom_plugin_module>(m_Imports.pCreateInstance(Filename.data()));
		}
		catch (const SException&)
		{
			// TODO: notification
			throw;
			//return nullptr;
		}
	}

	virtual bool Destroy(plugin_module_ptr& Module) override
	{
		try
		{
			const auto Result = m_Imports.pDestroyInstance(static_cast<custom_plugin_module*>(Module.get())->get_opaque()) != FALSE;
			Module.reset();
			return Result;
		}
		catch (const SException&)
		{
			// TODO: notification
			throw;
			//return false;
		}
	}

	virtual function_address GetFunction(const plugin_module_ptr& Instance, const export_name& Name) override
	{
		try
		{
			return Name.UName? m_Imports.pGetFunctionAddress(static_cast<custom_plugin_module*>(Instance.get())->get_opaque(), Name.UName) : nullptr;
		}
		catch (const SException&)
		{
			// TODO: notification
			throw;
		}
	}

private:
	os::rtdl::module m_Module;
	struct ModuleImports
	{
		os::rtdl::function_pointer<BOOL(WINAPI*)(GlobalInfo* info)> pInitialize;
		os::rtdl::function_pointer<BOOL(WINAPI*)(const wchar_t* filename)> pIsPlugin;
		os::rtdl::function_pointer<HANDLE(WINAPI*)(const wchar_t* filename)> pCreateInstance;
		os::rtdl::function_pointer<void*(WINAPI*)(HANDLE Instance, const wchar_t* functionname)> pGetFunctionAddress;
		os::rtdl::function_pointer<BOOL(WINAPI*)(HANDLE Instance)> pDestroyInstance;
		os::rtdl::function_pointer<void (WINAPI*)(const ExitInfo* info)> pFree;

		ModuleImports(const os::rtdl::module& Module):
			#define INIT_IMPORT(name) p ## name(Module, #name)
			INIT_IMPORT(Initialize),
			INIT_IMPORT(IsPlugin),
			INIT_IMPORT(CreateInstance),
			INIT_IMPORT(GetFunctionAddress),
			INIT_IMPORT(DestroyInstance),
			INIT_IMPORT(Free),
			#undef INIT_IMPORT
			m_IsValid(pInitialize && pIsPlugin && pCreateInstance && pGetFunctionAddress && pDestroyInstance && pFree)
		{
		}

		bool IsValid() const { return m_IsValid; }

	private:
		bool m_IsValid;
	}
	m_Imports;
	bool m_Success;
};

plugin_factory_ptr CreateCustomPluginFactory(PluginManager* Owner, const string& Filename)
{
	auto Model = std::make_unique<custom_plugin_factory>(Owner, Filename);
	if (!Model->Success())
	{
		Model.reset();
	}
	return std::move(Model);
}
