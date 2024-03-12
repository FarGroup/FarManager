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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "plclass.hpp"

// Internal:
#include "plugins.hpp"
#include "pathmix.hpp"
#include "config.hpp"
#include "farversion.hpp"
#include "plugapi.hpp"
#include "message.hpp"
#include "mix.hpp"
#include "notification.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "uuids.far.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "configdb.hpp"
#include "global.hpp"
#include "encoding.hpp"
#include "exception_handler.hpp"
#include "log.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.process.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/io.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"
#include "common/view/zip.hpp"


// External:
#include "format.hpp"

//----------------------------------------------------------------------------

std::exception_ptr& GlobalExceptionPtr()
{
	static std::exception_ptr ExceptionPtr;
	return ExceptionPtr;
}

#define DECLARE_PLUGIN_FUNCTION(name, signature) DECLARE_GEN_PLUGIN_FUNCTION(name, true, signature)

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

#undef DECLARE_PLUGIN_FUNCTION

std::unique_ptr<Plugin> plugin_factory::CreatePlugin(const string& FileName)
{
	return IsPlugin(FileName)? std::make_unique<Plugin>(this, FileName) : nullptr;
}

plugin_factory::plugin_factory(PluginManager* owner):
	m_owner(owner)
{
	static const export_name ExportsNames[]
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

		{}, // OpenFilePlugin not used
		{}, // GetMinFarVersion not used
	};
	static_assert(std::size(ExportsNames) == ExportsCount);
	m_ExportsNames = ExportsNames;
}

plugin_factory::plugin_module_ptr native_plugin_factory::Create(const string& filename)
{
	auto Module = std::make_unique<native_plugin_module>(filename);
	if (!*Module)
	{
		const auto ErrorState = os::last_error();

		Module.reset();

		Message(MSG_WARNING | MSG_NOPLUGINS, ErrorState,
			msg(lng::MError),
			{
				msg(lng::MPlgLoadPluginError),
				filename
			},
			{ lng::MOk },
			L"ErrLoadPlugin"sv);

	}
	return Module;
}

bool native_plugin_factory::Destroy(plugin_module_ptr& instance)
{
	instance.reset();
	return true;
}

plugin_factory::function_address native_plugin_factory::Function(const plugin_module_ptr& Instance, const export_name& Name)
{
	return !Name.AName.empty()? static_cast<native_plugin_module*>(Instance.get())->GetProcAddress<function_address>(null_terminated_t<char>(Name.AName).c_str()) : nullptr;
}

bool native_plugin_factory::FindExport(const std::string_view ExportName) const
{
	// only module with GetGlobalInfoW can be native plugin
	return ExportName == m_ExportsNames[iGetGlobalInfo].AName;
}

bool native_plugin_factory::IsPlugin(const string& FileName) const
{
	if (!ends_with_icase(FileName, L".dll"sv))
		return false;

	const os::fs::file ModuleFile(FileName, FILE_READ_DATA, os::fs::file_share_read, nullptr, OPEN_EXISTING);
	if (!ModuleFile)
	{
		LOGDEBUG(L"create_file({}) {}"sv, FileName, os::last_error());
		return false;
	}

	os::fs::filebuf StreamBuffer(ModuleFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	return IsPlugin(FileName, Stream);
}

bool native_plugin_factory::IsPlugin(string_view const FileName, std::istream& Stream) const
{
	IMAGE_DOS_HEADER DosHeader;
	if (io::read(Stream, edit_bytes(DosHeader)) != sizeof(DosHeader))
	{
		LOGDEBUG(L"Not a {} plugin: not enough data in {}"sv, kind(), FileName);
		return false;
	}

	if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		LOGDEBUG(L"Not a {} plugin: wrong DOS signature 0x{:04X} in {}"sv, kind(), DosHeader.e_magic, FileName);
		return false;
	}

	Stream.seekg(DosHeader.e_lfanew);

	IMAGE_NT_HEADERS NtHeaders;
	if (io::read(Stream, edit_bytes(NtHeaders)) != sizeof(NtHeaders))
	{
		LOGDEBUG(L"Not a {} plugin: not enough data in {}"sv, kind(), FileName);
		return false;
	}

	if (NtHeaders.Signature != IMAGE_NT_SIGNATURE)
	{
		LOGDEBUG(L"Not a {} plugin: wrong NT signature 0x{:08X} in {}"sv, kind(), NtHeaders.Signature, FileName);
		return false;
	}

	if (!(NtHeaders.FileHeader.Characteristics & IMAGE_FILE_DLL))
	{
		LOGDEBUG(L"Not a {} plugin: not a DLL 0x{:04X} in {}"sv, kind(), NtHeaders.FileHeader.Characteristics, FileName);
		return false;
	}

	static const auto FarMachineType = []
	{
		const auto FarModule = GetModuleHandle(nullptr);
		const auto& FarDosHeader = view_as<IMAGE_DOS_HEADER>(FarModule, 0);
		const auto& FarNtHeaders = view_as<IMAGE_NT_HEADERS>(FarModule, FarDosHeader.e_lfanew);
		return FarNtHeaders.FileHeader.Machine;
	}();

	if (NtHeaders.FileHeader.Machine != FarMachineType)
	{
		LOGDEBUG(L"Not a {} plugin: machine type is {:04X}, expected {:04X} in {}"sv, kind(), NtHeaders.FileHeader.Machine, FarMachineType, FileName);
		return false;
	}

	const auto ExportDirectoryAddress = NtHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if (!ExportDirectoryAddress)
	{
		LOGDEBUG(L"Not a {} plugin: no exports in {}"sv, kind(), FileName);
		return false;
	}

	Stream.seekg(DosHeader.e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + NtHeaders.FileHeader.SizeOfOptionalHeader);

	IMAGE_SECTION_HEADER Section;
	bool Found{};

	for ([[maybe_unused]] const auto i: std::views::iota(0uz, NtHeaders.FileHeader.NumberOfSections))
	{
		if (io::read(Stream, edit_bytes(Section)) != sizeof(Section))
		{
			LOGDEBUG(L"Not a {} plugin: not enough data in {}"sv, kind(), FileName);
			return false;
		}

		if (
			Section.VirtualAddress == ExportDirectoryAddress ||
			(Section.VirtualAddress <= ExportDirectoryAddress && ExportDirectoryAddress < Section.VirtualAddress + Section.Misc.VirtualSize)
		)
		{
			Found = true;
			break;
		}
	}

	if (!Found)
	{
		LOGDEBUG(L"Not a {} plugin: exports section not found in {}"sv, kind(), FileName);
		return false;
	}

	const auto section_address_to_real = [&](size_t const VirtualAddress)
	{
		return VirtualAddress - Section.VirtualAddress + Section.PointerToRawData;
	};

	Stream.seekg(section_address_to_real(ExportDirectoryAddress));

	IMAGE_EXPORT_DIRECTORY ExportDirectory;
	if (io::read(Stream, edit_bytes(ExportDirectory)) != sizeof(ExportDirectory))
	{
		LOGDEBUG(L"Not a {} plugin: not enough data in {}"sv, kind(), FileName);
		return false;
	}

	std::string Name;
	for (const auto i: std::views::iota(0uz, ExportDirectory.NumberOfNames))
	{
		Stream.seekg(section_address_to_real(ExportDirectory.AddressOfNames) + sizeof(DWORD) * i);

		DWORD NameAddress;
		if (io::read(Stream, edit_bytes(NameAddress)) != sizeof(NameAddress))
		{
			LOGDEBUG(L"Not a {} plugin: not enough data in {}"sv, kind(), FileName);
			return false;
		}

		Stream.seekg(section_address_to_real(NameAddress));

		Name.clear();

		for (;;)
		{
			char Ch;
			if (!io::read(Stream, edit_bytes(Ch)))
			{
				LOGDEBUG(L"Not a {} plugin: not enough data in {}"sv, kind(), FileName);
				return false;
			}

			if (!Ch)
				break;

			Name.push_back(Ch);
		}

		if (FindExport(Name))
		{
			LOGTRACE(L"Found a {} plugin: {}"sv, kind(), FileName);
			return true;
		}
	}

	LOGDEBUG(L"Not a {} plugin: no known exports found in {}"sv, kind(), FileName);
	return false;
}

static void PrepareModulePath(string_view const ModuleName)
{
	auto strModulePath = ModuleName;
	CutToSlash(strModulePath); //??
	FarChDir(strModulePath);
}

static const ArclitePrivateInfo ArcliteInfo
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

static const NetBoxPrivateInfo NetBoxInfo
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

static const MacroPrivateInfo MacroInfo
{
	sizeof(MacroPrivateInfo),
	pluginapi::apiCallFar,
};

static void CreatePluginStartupInfo(const Plugin* pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF)
{
	CreatePluginStartupInfo(PSI, FSF);

	PSI->ModuleName = pPlugin->ModuleName().c_str();
	if (pPlugin->Id() == Global->Opt->KnownIDs.Arclite.Id)
	{
		PSI->Private = &ArcliteInfo;
	}
	else if (pPlugin->Id() == Global->Opt->KnownIDs.Netbox.Id)
	{
		PSI->Private = &NetBoxInfo;
	}
	else if (pPlugin->Id() == Global->Opt->KnownIDs.Luamacro.Id)
	{
		PSI->Private = &MacroInfo;
	}
}

static void ShowMessageAboutIllegalPluginVersion(const string& plg, const VersionInfo& required)
{
	Message(MSG_WARNING|MSG_NOPLUGINS,
		msg(lng::MError),
		{
			msg(lng::MPlgBadVers),
			plg,
			far::vformat(msg(lng::MPlgRequired), version_to_string(required)),
			far::vformat(msg(lng::MPlgRequired2), version_to_string(build::version()))
		},
		{ lng::MOk }
	);
}

static auto MakeSignature(const os::fs::find_data& Data)
{
	return concat
	(
		to_hex_wstring(Data.FileSize),
		to_hex_wstring(Data.CreationTime.time_since_epoch().count()),
		to_hex_wstring(Data.LastWriteTime.time_since_epoch().count())
	);
}

bool Plugin::SaveToCache()
{
	PluginInfo Info{ sizeof(Info) };
	GetPluginInfo(&Info);

	auto& PlCache = ConfigProvider().PlCacheCfg();

	SCOPED_ACTION(auto)(PlCache->ScopedTransaction());

	const auto id = PlCache->CreateCache(m_strCacheName);

	const bool bPreload = (Info.Flags & PF_PRELOAD);
	PlCache->SetPreload(id, bPreload);
	WorkFlags.Change(PIWF_PRELOADED, bPreload);

	if (bPreload)
	{
		return true;
	}

	os::fs::find_data fdata;
	if (!os::fs::get_find_data(m_strModuleName, fdata))
		return false;

	PlCache->SetSignature(id, MakeSignature(fdata));

	const auto SaveItems = [&PlCache, &id](const auto& Setter, const PluginMenuItem& Item)
	{
		for (const auto i: std::views::iota(0uz, Item.Count))
		{
			std::invoke(Setter, PlCache, id, i, Item.Strings[i], Item.Guids[i]);
		}
	};

	SaveItems(&PluginsCacheConfig::SetPluginsMenuItem, Info.PluginMenu);
	SaveItems(&PluginsCacheConfig::SetDiskMenuItem, Info.DiskMenu);
	SaveItems(&PluginsCacheConfig::SetPluginsConfigMenuItem, Info.PluginConfig);

	PlCache->SetCommandPrefix(id, NullToEmpty(Info.CommandPrefix));
	PlCache->SetFlags(id, Info.Flags);

	PlCache->SetMinFarVersion(id, m_MinFarVersion);
	PlCache->SetUuid(id, m_strUuid);
	PlCache->SetVersion(id, m_PluginVersion);
	PlCache->SetTitle(id, strTitle);
	PlCache->SetDescription(id, strDescription);
	PlCache->SetAuthor(id, strAuthor);

	for (const auto& [Name, Export]: zip(m_Factory->ExportsNames(), Exports))
	{
		PlCache->SetExportState(id, Name.UName, Export != nullptr);
	}

	return true;
}

void Plugin::InitExports()
{
	for (const auto& [Name, Export]: zip(m_Factory->ExportsNames(), Exports))
	{
		Export = m_Factory->Function(m_Instance, Name);
	}
}

Plugin::Plugin(plugin_factory* Factory, const string_view ModuleName):
	m_Factory(Factory),
	m_strModuleName(ModuleName),
	m_strCacheName(ModuleName)
{
	SetUuid(FarUuid);
}

Plugin::~Plugin() = default;

void Plugin::SetUuid(const UUID& Uuid)
{
	m_Uuid = Uuid;
	m_strUuid = uuid::str(m_Uuid);
}

void Plugin::increase_activity()
{
	++m_Activity;
}

void Plugin::decrease_activity()
{
	--m_Activity;
}

bool Plugin::LoadData()
{
	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (WorkFlags.Check(PIWF_DATALOADED))
		return true;

	if (m_Instance)
		return true;

	string strCurPlugDiskPath;
	wchar_t Drive[]{ L'=', 0, L':', 0 };
	const auto strCurPath = os::fs::get_current_directory();

	if (ParsePath(m_strModuleName) == root_type::drive_letter)  // если указан локальный путь, то...
	{
		Drive[1] = m_strModuleName.front();
		strCurPlugDiskPath = os::env::get(Drive);
	}

	PrepareModulePath(m_strModuleName);

	// Factory can spawn error messages, which in turn can cause redraw events
	// and load plugins recursively again and again, eventually overflowing the stack.
	// Expect nothing and you will never be disappointed.
	WorkFlags.Set(PIWF_DONTLOADAGAIN);

	m_Instance = m_Factory->Create(m_strModuleName);
	FarChDir(strCurPath);

	if (Drive[1]) // вернем ее (переменную окружения) обратно
		os::env::set(Drive, strCurPlugDiskPath);

	if (!m_Instance)
		return false;

	WorkFlags.Clear(PIWF_DONTLOADAGAIN | PIWF_CACHED);

	if(bPendingRemove)
	{
		bPendingRemove = false;
		m_Factory->Owner()->UndoRemove(this);
	}
	InitExports();

	GlobalInfo Info{ sizeof(Info) };

	if(GetGlobalInfo(&Info) &&
		Info.StructSize &&
		Info.Title && *Info.Title &&
		Info.Description && *Info.Description &&
		Info.Author && *Info.Author)
	{
		m_MinFarVersion = Info.MinFarVersion;
		m_PluginVersion = Info.Version;
		strTitle = Info.Title;
		strDescription = Info.Description;
		strAuthor = Info.Author;

		bool ok = false;

		if (Info.Guid != FarUuid)
		{
			if (m_Uuid != FarUuid && m_Uuid != Info.Guid)
			{
				ok = m_Factory->Owner()->UpdateId(this, Info.Guid);
			}
			else
			{
				SetUuid(Info.Guid);
				ok = true;
			}
		}

		if (ok)
		{
			SubscribeToSynchroEvents();
			WorkFlags.Set(PIWF_DATALOADED);
			return true;
		}
	}
	Unload(false);
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
			Unload(false);
		}

		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		return false;
	}

	SaveToCache();
	return true;
}

bool Plugin::LoadFromCache(const os::fs::find_data &FindData)
{
	const auto& PlCache = ConfigProvider().PlCacheCfg();

	const auto id = PlCache->GetCacheID(m_strCacheName);
	if (!id)
		return false;

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

	if (!PlCache->GetMinFarVersion(id, m_MinFarVersion))
	{
		m_MinFarVersion = build::version();
	}

	if (!PlCache->GetVersion(id, m_PluginVersion))
	{
		m_PluginVersion = {};
	}

	m_strUuid = PlCache->GetUuid(id);

	if (const auto Uuid = uuid::try_parse(m_strUuid))
		SetUuid(*Uuid);
	else
		SetUuid(FarUuid);

	strTitle = PlCache->GetTitle(id);
	strDescription = PlCache->GetDescription(id);
	strAuthor = PlCache->GetAuthor(id);

	for (const auto& [Name, Export]: zip(m_Factory->ExportsNames(), Exports))
	{
		if (PlCache->GetExportState(id, Name.UName))
			Export = ToPtr(true); // Fake, will be overwritten with the real address later
	}
	SubscribeToSynchroEvents();

	WorkFlags.Set(PIWF_CACHED); //too many "cached" flags
	return true;
}

bool Plugin::Unload(bool bExitFAR)
{
	if (!WorkFlags.Check(PIWF_LOADED))
		return true;

	if (bExitFAR)
		NotifyExit();

	bool Result = true;

	if (!WorkFlags.Check(PIWF_CACHED))
	{
		Result = m_Factory->Destroy(m_Instance);
		LOGDEBUG(L"Unloaded {}"sv, ModuleName());
		ClearExports();
	}

	m_Instance = nullptr;
	WorkFlags.Clear(PIWF_LOADED);
	WorkFlags.Clear(PIWF_DATALOADED);
	bPendingRemove = true;

	return Result;
}

void Plugin::NotifyExit()
{
	if (WorkFlags.Check(PIWF_LOADED))
	{
		ExitInfo Info{sizeof(Info)};
		ExitFAR(&Info);
	}
}

void Plugin::ClearExports()
{
	Exports.fill({});
}

void Plugin::AddDialog(const window_ptr& Dlg)
{
	m_dialogs.emplace(Dlg);
}

bool Plugin::RemoveDialog(const window_ptr& Dlg)
{
	const auto ItemIterator = m_dialogs.find(Dlg);
	if (ItemIterator == m_dialogs.cend())
		return false;

	m_dialogs.erase(ItemIterator);
	return true;
}

void Plugin::SubscribeToSynchroEvents()
{
	if (!has(iProcessSynchroEvent))
		return;

	// Already initialised
	if (m_SynchroListenerCreated)
		return;

	m_SynchroListenerCreated = true;

	m_SynchroListener = std::make_unique<listener>(m_Uuid, [this](const std::any& Payload)
	{
		const auto Param = std::any_cast<void*>(Payload);

		ProcessSynchroEventInfo Info{ sizeof(Info), SE_COMMONSYNCHRO, Param };
		ProcessSynchroEvent(&Info);
	});
}

bool Plugin::IsPanelPlugin() const
{
	static const int PanelExports[]
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

	return std::ranges::any_of(PanelExports, [&](int const i)
	{
		return Exports[i] != nullptr;
	});
}

bool Plugin::SetStartupInfo(PluginStartupInfo *Info)
{
	ExecuteStruct<iSetStartupInfo> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return !bPendingRemove;
}

bool Plugin::GetGlobalInfo(GlobalInfo* Info)
{
	ExecuteStruct<iGetGlobalInfo> es;
	if (!has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return !bPendingRemove;
}

static bool CheckFarVersion(const VersionInfo& Desired)
{
	const auto FarVersion = build::version();
	return CheckVersion(&FarVersion, &Desired) != FALSE;
}

bool Plugin::CheckMinFarVersion()
{
	if (CheckFarVersion(m_MinFarVersion))
		return true;

	ShowMessageAboutIllegalPluginVersion(m_strModuleName, m_MinFarVersion);
	return false;
}

bool Plugin::InitLang(string_view const Path, string_view const Language)
{
	if (PluginLang)
		return true;

	try
	{
		PluginLang = std::make_unique<plugin_language>(Path, Language);
		return true;
	}
	catch (std::exception const& e)
	{
		LOGERROR(L"{}"sv, e);
		return false;
	}
}

void Plugin::CloseLang()
{
	PluginLang.reset();
}

const wchar_t* Plugin::Msg(intptr_t Id) const
{
	return static_cast<const plugin_language&>(*PluginLang).Msg(Id);
}

void* Plugin::OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode)
{
	return nullptr;
}

void* Plugin::Analyse(AnalyseInfo* Info)
{
	ExecuteStruct<iAnalyse> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

void Plugin::CloseAnalyse(CloseAnalyseInfo* Info)
{
	ExecuteStruct<iCloseAnalyse> es;
	if (exception_handling_in_progress() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}

void* Plugin::Open(OpenInfo* Info)
{
	ExecuteStruct<iOpen> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::SetFindList(SetFindListInfo* Info)
{
	ExecuteStruct<iSetFindList> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessEditorInput(ProcessEditorInputInfo* Info)
{
	ExecuteStruct<iProcessEditorInput> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessEditorEvent(ProcessEditorEventInfo* Info)
{
	ExecuteStruct<iProcessEditorEvent> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessViewerEvent(ProcessViewerEventInfo* Info)
{
	ExecuteStruct<iProcessViewerEvent> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessDialogEvent(ProcessDialogEventInfo* Info)
{
	ExecuteStruct<iProcessDialogEvent> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessSynchroEvent(ProcessSynchroEventInfo* Info)
{
	ExecuteStruct<iProcessSynchroEvent> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	ExecuteStruct<iProcessConsoleInput> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::GetVirtualFindData(GetVirtualFindDataInfo* Info)
{
	ExecuteStruct<iGetVirtualFindData> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

void Plugin::FreeVirtualFindData(FreeFindDataInfo* Info)
{
	ExecuteStruct<iFreeVirtualFindData> es;
	if (exception_handling_in_progress() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}

intptr_t Plugin::GetFiles(GetFilesInfo* Info)
{
	ExecuteStruct<iGetFiles> es(-1);
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::PutFiles(PutFilesInfo* Info)
{
	ExecuteStruct<iPutFiles> es(-1);
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::DeleteFiles(DeleteFilesInfo* Info)
{
	ExecuteStruct<iDeleteFiles> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::MakeDirectory(MakeDirectoryInfo* Info)
{
	ExecuteStruct<iMakeDirectory> es(-1);
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessHostFile(ProcessHostFileInfo* Info)
{
	ExecuteStruct<iProcessHostFile> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::ProcessPanelEvent(ProcessPanelEventInfo* Info)
{
	ExecuteStruct<iProcessPanelEvent> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::Compare(CompareInfo* Info)
{
	ExecuteStruct<iCompare> es(-2);
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::GetFindData(GetFindDataInfo* Info)
{
	ExecuteStruct<iGetFindData> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

void Plugin::FreeFindData(FreeFindDataInfo* Info)
{
	ExecuteStruct<iFreeFindData> es;
	if (exception_handling_in_progress() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}

intptr_t Plugin::ProcessPanelInput(ProcessPanelInputInfo* Info)
{
	ExecuteStruct<iProcessPanelInput> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}


void Plugin::ClosePanel(ClosePanelInfo* Info)
{
	ExecuteStruct<iClosePanel> es;
	if (exception_handling_in_progress() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}


intptr_t Plugin::SetDirectory(SetDirectoryInfo* Info)
{
	ExecuteStruct<iSetDirectory> es;
	if (exception_handling_in_progress() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

void Plugin::GetOpenPanelInfo(OpenPanelInfo* Info)
{
	ExecuteStruct<iGetOpenPanelInfo> es;
	if (exception_handling_in_progress() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}


intptr_t Plugin::Configure(ConfigureInfo* Info)
{
	ExecuteStruct<iConfigure> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}


bool Plugin::GetPluginInfo(PluginInfo* Info)
{
	ExecuteStruct<iGetPluginInfo> es;
	if (exception_handling_in_progress() || !has(es))
		return false;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return !bPendingRemove;
}

intptr_t Plugin::GetContentFields(GetContentFieldsInfo *Info)
{
	ExecuteStruct<iGetContentFields> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

intptr_t Plugin::GetContentData(GetContentDataInfo *Info)
{
	ExecuteStruct<iGetContentData> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return es;

	SetInstance(Info);
	ExecuteFunction(es, Info);
	return es;
}

void Plugin::FreeContentData(GetContentDataInfo *Info)
{
	ExecuteStruct<iFreeContentData> es;
	if (exception_handling_in_progress() || !Load() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}

void Plugin::ExitFAR(ExitInfo *Info)
{
	ExecuteStruct<iExitFAR> es;
	if (exception_handling_in_progress() || !has(es))
		return;

	SetInstance(Info);
	ExecuteFunction(es, Info);
}

void Plugin::ExecuteFunctionImpl(export_index const ExportId, function_ref<void()> const Callback, source_location const& Location)
{
	const auto HandleFailure = [&](DWORD const ExceptionCode = EXIT_FAILURE)
	{
		if (use_terminate_handler())
			os::process::terminate_by_user(ExceptionCode);

		m_Factory->Owner()->UnloadPlugin(this, ExportId);
	};

	seh_try_with_ui(
	[&]
	{
		Prologue();
		increase_activity();

		SCOPE_EXIT
		{
			decrease_activity();
			Epilogue();
		};

		const auto HandleException = [&](const auto& Handler, auto&&... ProcArgs)
		{
			Handler(FWD(ProcArgs)..., this, Location)? HandleFailure() : throw;
		};

		cpp_try(
		[&]
		{
			Callback();
			rethrow_if(GlobalExceptionPtr());
			m_Factory->ProcessError(m_Factory->ExportsNames()[ExportId].AName);
		},
		[&](source_location const&)
		{
			HandleException(handle_unknown_exception);
		},
		[&](std::exception const& e, source_location const&)
		{
			HandleException(handle_std_exception, e);
		},
		Location);
	},
	HandleFailure,
	this,
	Location);
}

class custom_plugin_module final: public i_plugin_module
{
public:
	NONCOPYABLE(custom_plugin_module);
	explicit custom_plugin_module(void* Opaque) : m_Opaque(Opaque) {}
	void* opaque() const override { return m_Opaque; }

private:
	void* m_Opaque;
};

class custom_plugin_factory final: public plugin_factory
{
public:
	NONCOPYABLE(custom_plugin_factory);
	custom_plugin_factory(PluginManager* Owner, const string_view Filename):
		plugin_factory(Owner),
		m_Imports(Filename)
	{
		GlobalInfo Info{ sizeof(Info) };

		if (m_Imports.IsValid() &&
			m_Imports.pInitialize(&Info) &&
			Info.StructSize &&
			Info.Title && *Info.Title &&
			Info.Description && *Info.Description &&
			Info.Author && *Info.Author)
		{
			m_Success = CheckFarVersion(Info.MinFarVersion);
			if (m_Success)
			{
				m_Version = Info.Version;
				m_Title = Info.Title;
				m_Id = Info.Guid;
			}
			// TODO: store info, show message if version is bad
		}
		custom_plugin_factory::ProcessError(m_Imports.pInitialize.name());
	}

	~custom_plugin_factory() override
	{
		if (!m_Success)
			return;

		ExitInfo Info{ sizeof(Info) };
		m_Imports.pFree(&Info);
		custom_plugin_factory::ProcessError(m_Imports.pFree.name());
	}

	bool Success() const { return m_Success; }

	bool IsPlugin(const string& FileName) const override
	{
		const auto Result = m_Imports.pIsPlugin(FileName.c_str()) != FALSE;
		ProcessError(m_Imports.pIsPlugin.name());
		return Result;
	}

	plugin_module_ptr Create(const string& Filename) override
	{
		auto Module = std::make_unique<custom_plugin_module>(m_Imports.pCreateInstance(Filename.c_str()));
		if (!Module->opaque())
		{
			Module.reset();
		}
		ProcessError(m_Imports.pCreateInstance.name());
		return Module;
	}

	bool Destroy(plugin_module_ptr& Module) override
	{
		const auto Result = m_Imports.pDestroyInstance(static_cast<custom_plugin_module*>(Module.get())->opaque()) != FALSE;
		Module.reset();
		ProcessError(m_Imports.pDestroyInstance.name());
		return Result;
	}

	function_address Function(const plugin_module_ptr& Instance, const export_name& Name) override
	{
		if (Name.UName.empty())
			return nullptr;
		const auto Result = m_Imports.pGetFunctionAddress(static_cast<custom_plugin_module*>(Instance.get())->opaque(), null_terminated(Name.UName).c_str());
		ProcessError(m_Imports.pGetFunctionAddress.name());
		return Result;
	}

	void ProcessError(const std::string_view Function) const override
	{
		if (!m_Imports.pGetError)
			return;

		ErrorInfo Info{ sizeof(Info) };
		if (!m_Imports.pGetError(&Info))
			return;

		std::vector<string> MessageLines;
		const string Summary = concat(Info.Summary, L" ("sv, encoding::ansi::get_chars(Function), L')');
		const auto Enumerator = enum_tokens(Info.Description, L"\n"sv);
		std::ranges::transform(Enumerator, std::back_inserter(MessageLines), [](const string_view View) { return string(View); });
		Message(MSG_WARNING | MSG_LEFTALIGN,
			Summary,
			MessageLines,
			{ lng::MOk });
	}

	bool IsExternal() const override
	{
		return true;
	}

	string Title() const override
	{
		return m_Title;
	}

	VersionInfo version() const override
	{
		return m_Version;
	}

private:
	struct ModuleImports
	{
	private:
		os::rtdl::module m_Module;

	public:
#define DECLARE_IMPORT_FUNCTION(name, ...) os::rtdl::function_pointer<__VA_ARGS__> p ## name{ m_Module, #name }

		DECLARE_IMPORT_FUNCTION(Initialize,            BOOL   WINAPI(GlobalInfo* info));
		DECLARE_IMPORT_FUNCTION(IsPlugin,              BOOL   WINAPI(const wchar_t* filename));
		DECLARE_IMPORT_FUNCTION(CreateInstance,        HANDLE WINAPI(const wchar_t* filename));
		DECLARE_IMPORT_FUNCTION(GetFunctionAddress,    void*  WINAPI(HANDLE Instance, const wchar_t* functionname));
		DECLARE_IMPORT_FUNCTION(GetError,              BOOL   WINAPI(ErrorInfo* info));
		DECLARE_IMPORT_FUNCTION(DestroyInstance,       BOOL   WINAPI(HANDLE Instance));
		DECLARE_IMPORT_FUNCTION(Free,                  void   WINAPI(const ExitInfo* info));

#undef DECLARE_IMPORT_FUNCTION

		explicit ModuleImports(const string_view Filename):
			m_Module(Filename),
			m_IsValid(pInitialize && pIsPlugin && pCreateInstance && pGetFunctionAddress && pGetError && pDestroyInstance && pFree)
		{
		}

		bool IsValid() const { return m_IsValid; }

	private:
		bool m_IsValid;
	}
	m_Imports;
	string m_Title;
	VersionInfo m_Version{};
	bool m_Success{};
};

plugin_factory_ptr CreateCustomPluginFactory(PluginManager* const Owner, const string_view Filename)
{
	auto Model = std::make_unique<custom_plugin_factory>(Owner, Filename);
	if (!Model->Success())
	{
		Model.reset();
	}
	return Model;
}
