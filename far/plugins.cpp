﻿/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в filelist.cpp)
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

#include "plugins.hpp"

#include "keys.hpp"
#include "scantree.hpp"
#include "chgprior.hpp"
#include "constitle.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "fileedit.hpp"
#include "refreshwindowmanager.hpp"
#include "plugapi.hpp"
#include "taskbar.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "processname.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "FarGuid.hpp"
#include "configdb.hpp"
#include "FarDlgBuilder.hpp"
#include "DlgGuid.hpp"
#include "mix.hpp"
#include "manager.hpp"
#include "lang.hpp"
#include "desktop.hpp"
#include "PluginA.hpp"
#include "string_sort.hpp"
#include "cvtname.hpp"
#include "delete.hpp"
#include "global.hpp"

#include "platform.env.hpp"
#include "platform.memory.hpp"

#include "common/enum_tokens.hpp"
#include "common/scope_exit.hpp"

static void ReadUserBackgound(SaveScreen *SaveScr)
{
	if (Global->KeepUserScreen)
	{
		if (SaveScr)
			SaveScr->Discard();

		Global->ScrBuf->FillBuf();
		Global->WindowManager->Desktop()->TakeSnapshot();
	}
}

static string GetHotKeyPluginKey(Plugin *pPlugin)
{
	/*
	FarPath
	C:\Program Files\Far\

	ModuleName                                             PluginName
	---------------------------------------------------------------------------------------
	C:\Program Files\Far\Plugins\MultiArc\MULTIARC.DLL  -> Plugins\MultiArc\MULTIARC.DLL
	C:\MultiArc\MULTIARC.DLL                            -> C:\MultiArc\MULTIARC.DLL
	---------------------------------------------------------------------------------------
	*/
	auto PluginKey = pPlugin->GetHotkeyName();
#ifndef NO_WRAPPER
	if (pPlugin->IsOemPlugin() && starts_with_icase(pPlugin->ModuleName(), Global->g_strFarPath))
		PluginKey.erase(0, Global->g_strFarPath.size());
#endif // NO_WRAPPER
	return PluginKey;
}

static wchar_t GetPluginHotKey(Plugin *pPlugin, const GUID& Guid, hotkey_type HotKeyType)
{
	const auto strHotKey = ConfigProvider().PlHotkeyCfg()->GetHotkey(GetHotKeyPluginKey(pPlugin), Guid, HotKeyType);
	return strHotKey.empty()? L'\0' : strHotKey.front();
}

bool PluginManager::plugin_less::operator()(const Plugin* a, const Plugin *b) const
{
	return string_sort::less(PointToName(a->ModuleName()), PointToName(b->ModuleName()));
}

static void CallPluginSynchroEvent(const std::any& Payload)
{
	const auto& Data = std::any_cast<const std::pair<GUID, void*>&>(Payload);
	if (const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(Data.first))
	{
		ProcessSynchroEventInfo Info = { sizeof(Info) };
		Info.Event = SE_COMMONSYNCHRO;
		Info.Param = Data.second;
		pPlugin->ProcessSynchroEvent(&Info);
	}
}

static void EnsureLuaCpuCompatibility()
{
// All AMD64 processors have SSE2
#ifndef _WIN64
	if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
		return;

	static os::rtdl::module LuaModule(path::join(Global->g_strFarPath, L"legacy"sv, L"lua51.dll"sv));
	// modules are lazy loaded
	LuaModule.operator bool();
#endif
}

PluginManager::PluginManager():
	m_PluginSynchro(plugin_synchro, &CallPluginSynchroEvent),
#ifndef NO_WRAPPER
	OemPluginsCount(),
#endif // NO_WRAPPER
	m_PluginsLoaded()
{
}

PluginManager::~PluginManager()
{
	Plugin *Luamacro=nullptr; // обеспечить выгрузку данного плагина последним.

	std::for_each(CONST_RANGE(SortedPlugins, i)
	{
		if (!Luamacro && i->Id() == Global->Opt->KnownIDs.Luamacro.Id)
		{
			Luamacro=i;
		}
		else
		{
			i->Unload(true);
		}
	});

	if (Luamacro)
	{
		Luamacro->Unload(true);
	}

	// some plugins might still have dialogs (if DialogFree wasn't called)
	// better to delete them explicitly while manager is still alive
	m_Plugins.clear();
}

Plugin* PluginManager::AddPlugin(std::unique_ptr<Plugin>&& pPlugin)
{
	const auto Result = m_Plugins.emplace(pPlugin->Id(), nullptr);
	if (!Result.second)
	{
		pPlugin->Unload(true);
		return nullptr;
	}
	Result.first->second = std::move(pPlugin);

	const auto PluginPtr = Result.first->second.get();

	SortedPlugins.emplace(PluginPtr);
#ifndef NO_WRAPPER
	if (PluginPtr->IsOemPlugin())
	{
		OemPluginsCount++;
	}
#endif // NO_WRAPPER
	return PluginPtr;
}

bool PluginManager::UpdateId(Plugin *pPlugin, const GUID& Id)
{
	const auto Iterator = m_Plugins.find(pPlugin->Id());
	// important, do not delete Plugin instance
	Iterator->second.release();
	m_Plugins.erase(Iterator);
	pPlugin->SetGuid(Id);
	const auto Result = m_Plugins.emplace(pPlugin->Id(), nullptr);
	if (!Result.second)
	{
		return false;
	}
	Result.first->second.reset(pPlugin);
	return true;
}

bool PluginManager::RemovePlugin(Plugin *pPlugin)
{
#ifndef NO_WRAPPER
	if(pPlugin->IsOemPlugin())
	{
		OemPluginsCount--;
	}
#endif // NO_WRAPPER
	SortedPlugins.erase(std::find(SortedPlugins.begin(), SortedPlugins.end(), pPlugin));
	m_Plugins.erase(pPlugin->Id());
	return true;
}


Plugin* PluginManager::LoadPlugin(const string& FileName, const os::fs::find_data &FindData, bool LoadToMem)
{
	std::unique_ptr<Plugin> pPlugin;

	if (!std::any_of(CONST_RANGE(PluginFactories, i) { return (pPlugin = i->CreatePlugin(FileName)) != nullptr; }))
		return nullptr;

	auto Result = LoadToMem? false : pPlugin->LoadFromCache(FindData);

	auto bDataLoaded = false;

	if (!Result && (pPlugin->CheckWorkFlags(PIWF_PRELOADED) || !Global->Opt->LoadPlug.PluginsCacheOnly))
		Result = bDataLoaded = pPlugin->LoadData();

	if (!Result)
		return nullptr;

	const auto PluginPtr = AddPlugin(std::move(pPlugin));

	if (!PluginPtr)
		return nullptr;

	if (bDataLoaded && !PluginPtr->Load())
	{
		PluginPtr->Unload(true);
		RemovePlugin(PluginPtr);
		return nullptr;
	}

	return PluginPtr;
}

Plugin* PluginManager::LoadPluginExternal(const string& ModuleName, bool LoadToMem)
{
	auto pPlugin = FindPlugin(ModuleName);

	if (pPlugin)
	{
		if ((LoadToMem || pPlugin->bPendingRemove) && !pPlugin->Load())
		{
			if (!pPlugin->bPendingRemove)
			{
				UnloadedPlugins.emplace_back(pPlugin);
			}
			return nullptr;
		}
	}
	else
	{
		os::fs::find_data FindData;

		if (os::fs::get_find_data(ModuleName, FindData))
		{
			pPlugin = LoadPlugin(ModuleName, FindData, LoadToMem);
		}
	}
	return pPlugin;
}

int PluginManager::UnloadPlugin(Plugin *pPlugin, int From)
{
	int nResult = FALSE;

	if (pPlugin && (From != iExitFAR))   //схитрим, если упали в EXITFAR, не полезем в рекурсию, мы и так в Unload
	{
		for(int i = static_cast<int>(Global->WindowManager->GetWindowCount()-1); i >= 0; --i)
		{
			const auto Window = Global->WindowManager->GetWindow(i);
			if((Window->GetType()==windowtype_dialog && std::static_pointer_cast<Dialog>(Window)->GetPluginOwner() == pPlugin) || Window->GetType()==windowtype_help)
			{
				Global->WindowManager->DeleteWindow(Window);
				Global->WindowManager->PluginCommit();
			}
		}

		bool bPanelPlugin = pPlugin->IsPanelPlugin();

		nResult = pPlugin->Unload(true);

		pPlugin->WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (bPanelPlugin /*&& bUpdatePanels*/)
		{
			Global->CtrlObject->Cp()->ActivePanel()->SetCurDir(L"."s, true);
			const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
			ActivePanel->Update(UPDATE_KEEP_SELECTION);
			ActivePanel->Redraw();
			const auto AnotherPanel = Global->CtrlObject->Cp()->PassivePanel();
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}

		UnloadedPlugins.emplace_back(pPlugin);
	}

	return nResult;
}

bool PluginManager::IsPluginUnloaded(const Plugin* pPlugin) const
{
	return contains(UnloadedPlugins, pPlugin);
}

int PluginManager::UnloadPluginExternal(Plugin* pPlugin)
{
	//BUGBUG нужны проверки на легальность выгрузки
	if(pPlugin->Active())
	{
		if(!IsPluginUnloaded(pPlugin))
		{
			UnloadedPlugins.emplace_back(pPlugin);
		}
		return TRUE;
	}

	UnloadedPlugins.remove(pPlugin);
	const auto Result = pPlugin->Unload(true);
	RemovePlugin(pPlugin);
	return Result;
}

Plugin *PluginManager::FindPlugin(const string& ModuleName) const
{
	const auto ItemIterator = std::find_if(CONST_RANGE(SortedPlugins, i)
	{
		return equal_icase(i->ModuleName(), ModuleName);
	});
	return ItemIterator == SortedPlugins.cend()? nullptr : *ItemIterator;
}

void PluginManager::LoadFactories()
{
	PluginFactories.emplace_back(std::make_unique<native_plugin_factory>(this));
#ifndef NO_WRAPPER
	if (Global->Opt->LoadPlug.OEMPluginsSupport)
		PluginFactories.emplace_back(CreateOemPluginFactory(this));
#endif // NO_WRAPPER

	ScanTree ScTree(false, true, Global->Opt->LoadPlug.ScanSymlinks);
	os::fs::find_data FindData;
	ScTree.SetFindPath(path::join(Global->g_strFarPath, L"Adapters"sv), L"*"sv);

	string filename;
	while (ScTree.GetNextName(FindData, filename))
	{
		if (CmpName(L"*.dll"sv, filename, false) && !(FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (auto CustomModel = CreateCustomPluginFactory(this, filename))
			{
				if (std::find_if(ALL_CONST_RANGE(PluginFactories), [&](const auto& i){ return i->Id() == CustomModel->Id(); }) == PluginFactories.cend())
					PluginFactories.emplace_back(std::move(CustomModel));
			}
		}
	}
}

void PluginManager::LoadPlugins()
{
	const auto& PluginLoadOptions = Global->Opt->LoadPlug;

	const auto AnyPluginsPossible =
		PluginLoadOptions.PluginsCacheOnly ||
		PluginLoadOptions.MainPluginDir ||
		PluginLoadOptions.PluginsPersonal ||
		!PluginLoadOptions.strCustomPluginsPath.empty();

	if (!AnyPluginsPossible)
		return;

	SCOPED_ACTION(IndeterminateTaskbar)(false);

	EnsureLuaCpuCompatibility();

	m_PluginsLoaded = false;

	LoadFactories();

	if (PluginLoadOptions.PluginsCacheOnly)  // $ 01.09.2000 tran  '/co' switch
	{
		LoadPluginsFromCache();
	}
	else
	{
		ScanTree ScTree(false, true, PluginLoadOptions.ScanSymlinks);
		std::vector<string> PluginDirectories;
		os::fs::find_data FindData;

		// сначала подготовим список
		if (PluginLoadOptions.MainPluginDir) // только основные и персональные?
		{
			PluginDirectories = { path::join(Global->g_strFarPath, L"Plugins"sv) };
			// ...а персональные есть?
			if (PluginLoadOptions.PluginsPersonal)
				PluginDirectories.emplace_back(PluginLoadOptions.strPersonalPluginsPath);
		}
		else if (!PluginLoadOptions.strCustomPluginsPath.empty())  // только "заказные" пути?
		{
			for (const auto& i: enum_tokens_with_quotes_t<with_trim>(PluginLoadOptions.strCustomPluginsPath, L";"sv))
			{
				if (i.empty())
					continue;

				PluginDirectories.emplace_back(i.data(), i.size());
			}
		}

		// теперь пройдемся по всему ранее собранному списку
		for (const auto& i: PluginDirectories)
		{
			auto strFullName = unquote(os::env::expand(i)); //??? здесь ХЗ

			if (!IsAbsolutePath(strFullName))
			{
				strFullName.insert(0, Global->g_strFarPath);
			}

			ScTree.SetFindPath(ConvertNameToLong(ConvertNameToFull(strFullName)), L"*"sv);

			while (ScTree.GetNextName(FindData,strFullName))
			{
				if (!(FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					LoadPlugin(strFullName, FindData, false);
				}
			}
		}
	}

	m_PluginsLoaded = true;
}

/* $ 01.09.2000 tran
   Load cache only plugins  - '/co' switch */
void PluginManager::LoadPluginsFromCache()
{
	string strModuleName;

	for (DWORD i=0; ConfigProvider().PlCacheCfg()->EnumPlugins(i, strModuleName); i++)
	{
		ReplaceSlashToBackslash(strModuleName);

		os::fs::find_data FindData;

		if (os::fs::get_find_data(strModuleName, FindData))
			LoadPlugin(strModuleName, FindData, false);
	}
}

std::unique_ptr<plugin_panel> PluginManager::OpenFilePlugin(const string* Name, OPERATION_MODES OpMode, OPENFILEPLUGINTYPE Type, bool* StopProcessingPtr)
{
	bool StopProcessing_Unused;
	auto& StopProcessing = StopProcessingPtr? *StopProcessingPtr : StopProcessing_Unused;

	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	// We're conditionally messing with the title down there.
	// However, we save & restore it unconditionally, as plugins could mess with it too.

	string OldTitle(ConsoleTitle::GetTitle());
	SCOPE_EXIT
	{
		// Invalidate cache
		ConsoleTitle::SetFarTitle({});
		// Set & flush
		ConsoleTitle::SetFarTitle(OldTitle, true);
	};

	if (Global->Opt->ShowCheckingFile)
	{
		OldTitle = ConsoleTitle::GetTitle();
		ConsoleTitle::SetFarTitle(msg(lng::MCheckingFileInPlugin), true);
	}

	class plugin_panel_holder: public plugin_panel
	{
	public:
		MOVABLE(plugin_panel_holder);

		plugin_panel_holder(Plugin* PluginInstance, HANDLE Panel, HANDLE Analyse):
			plugin_panel(PluginInstance, Panel),
			m_Analyse(Analyse)
		{
		}

		~plugin_panel_holder()
		{
			if (!plugin())
				return;

			if (m_Analyse)
			{
				CloseAnalyseInfo Info{ sizeof(Info), m_Analyse };
				plugin()->CloseAnalyse(&Info);
			}

			if (panel())
			{
				ClosePanelInfo Info{ sizeof(Info), panel() };
				plugin()->ClosePanel(&Info);
			}
		}

		HANDLE analyse() const
		{
			return m_Analyse;
		}

		void set_analyse(HANDLE Analyse)
		{
			m_Analyse = Analyse;
		}

	private:
		HANDLE m_Analyse;
	};

	std::vector<plugin_panel_holder> items;

	string strFullName;

	if (Name)
	{
		strFullName = ConvertNameToFull(*Name);
		Name = &strFullName;
	}

	const auto ShowMenu = Global->Opt->PluginConfirm.OpenFilePlugin==BSTATE_3STATE? !(Type == OFP_NORMAL || Type == OFP_SEARCH) : Global->Opt->PluginConfirm.OpenFilePlugin != 0;
	const auto ShowWarning = OpMode == OPM_NONE;
	 //у анси плагинов OpMode нет.
	if(Type==OFP_ALTERNATIVE) OpMode|=OPM_PGDN;
	if(Type==OFP_COMMANDS) OpMode|=OPM_COMMANDS;

	AnalyseInfo Info{ sizeof(Info), Name? Name->c_str() : nullptr, nullptr, 0, OpMode };
	std::vector<BYTE> Buffer(Global->Opt->PluginMaxReadData);

	bool DataRead = false;
	for (const auto& i: SortedPlugins)
	{
		if (!(i->has(iAnalyse) && i->has(iOpen)) && !i->has(iOpenFilePlugin))
			continue;

		if(Name && !DataRead)
		{
			if (const auto File = os::fs::file(*Name, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
			{
				size_t DataSize = 0;
				if (File.Read(Buffer.data(), Buffer.size(), DataSize))
				{
					Info.Buffer = Buffer.data();
					Info.BufferSize = DataSize;
					DataRead = true;
				}
			}

			if(!DataRead)
			{
				if(ShowWarning)
				{
					const auto ErrorState = error_state::fetch();

					Message(MSG_WARNING, ErrorState,
						{},
						{
							msg(lng::MOpenPluginCannotOpenFile),
							*Name
						},
						{ lng::MOk });
				}
				break;
			}
		}

		if (i->has(iAnalyse))
		{
			if (const auto analyse = i->Analyse(&Info))
			{
				items.emplace_back(i, nullptr, analyse);
			}
		}
		else
		{
			if (Global->Opt->ShowCheckingFile)
				ConsoleTitle::SetFarTitle(concat(msg(lng::MCheckingFileInPlugin), L" - ["sv, PointToName(i->ModuleName()), L"]..."sv), true);

			const auto hPlugin = i->OpenFilePlugin(Name? Name->c_str() : nullptr, (BYTE*)Info.Buffer, Info.BufferSize, OpMode);
			if (!hPlugin)
				continue;

			if (hPlugin == PANEL_STOP)   //сразу на выход, плагин решил нагло обработать все сам (Autorun/PictureView)!!!
			{
				StopProcessing = true;
				return nullptr;
			}

			items.emplace_back(i, hPlugin, nullptr);
		}

		if (!items.empty() && !ShowMenu)
			break;
	}

	if (items.empty())
		return nullptr;

	const auto OnlyOne = items.size() == 1 && !(Name && Global->Opt->PluginConfirm.OpenFilePlugin && Global->Opt->PluginConfirm.StandardAssociation && Global->Opt->PluginConfirm.EvenIfOnlyOnePlugin);

	auto PluginIterator = items.begin();

	if (!OnlyOne && ShowMenu)
	{
		const auto menu = VMenu2::create(msg(lng::MPluginConfirmationTitle), {}, ScrY - 4);
		menu->SetPosition(-1, -1, 0, 0);
		menu->SetHelp(L"ChoosePluginMenu"sv);
		menu->SetMenuFlags(VMENU_SHOWAMPERSAND | VMENU_WRAPMODE);

		for (const auto& i: items)
			menu->AddItem(i.plugin()->Title());

		if (Global->Opt->PluginConfirm.StandardAssociation && Type == OFP_NORMAL)
		{
			menu->AddItem(MenuItemEx{ {}, MIF_SEPARATOR });
			menu->AddItem(msg(lng::MMenuPluginStdAssociation));
		}

		const auto ExitCode = menu->Run();

		if (ExitCode < 0)
		{
			StopProcessing = true;
			return nullptr;
		}

		if (static_cast<size_t>(ExitCode) >= items.size())
		{
			return nullptr;
		}

		PluginIterator = items.begin() + ExitCode;
	}

	if (PluginIterator->analyse())
	{
		OpenAnalyseInfo oainfo{ sizeof(OpenAnalyseInfo), &Info, PluginIterator->analyse() };

		OpenInfo oInfo{ sizeof(oInfo) };
		oInfo.OpenFrom = OPEN_ANALYSE;
		oInfo.Guid = &FarGuid;
		oInfo.Data = reinterpret_cast<intptr_t>(&oainfo);

		// If we have reached this point, the analyse handle will be passed to the plugin
		// and supposed to be deleted by it, so we should not call CloseAnalyse.
		PluginIterator->set_analyse(nullptr);

		const auto PanelHandle = PluginIterator->plugin()->Open(&oInfo);

		if (!PanelHandle)
			return nullptr;

		if (PanelHandle == PANEL_STOP)
		{
			StopProcessing = true;
			return nullptr;
		}

		PluginIterator->set_panel(PanelHandle);
	}

	return std::make_unique<plugin_panel>(std::move(*PluginIterator));
}

std::unique_ptr<plugin_panel> PluginManager::OpenFindListPlugin(const PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	class plugin_panel_holder: public plugin_panel
	{
	public:
		MOVABLE(plugin_panel_holder);

		using plugin_panel::plugin_panel;

		~plugin_panel_holder()
		{
			if (!plugin())
				return;

			ClosePanelInfo Info{ sizeof(Info), panel() };
			plugin()->ClosePanel(&Info);
		}
	};

	std::vector<plugin_panel_holder> items;

	for (const auto& i: SortedPlugins)
	{
		if (!i->has(iSetFindList))
			continue;

		OpenInfo Info{ sizeof(Info) };
		Info.OpenFrom = OPEN_FINDLIST;
		Info.Guid = &FarGuid;
		Info.Data = 0;

		const auto PluginHandle = i->Open(&Info);
		if (!PluginHandle)
			continue;

		items.emplace_back(i, PluginHandle);

		if (!Global->Opt->PluginConfirm.SetFindList)
			break;
	}

	if (items.empty())
		return nullptr;

	auto PluginIterator = items.begin();

	if (items.size() != 1)
	{
		const auto menu = VMenu2::create(msg(lng::MPluginConfirmationTitle), {}, ScrY - 4);
		menu->SetPosition(-1, -1, 0, 0);
		menu->SetHelp(L"ChoosePluginMenu"sv);
		menu->SetMenuFlags(VMENU_SHOWAMPERSAND | VMENU_WRAPMODE);

		for (const auto& i: items)
			menu->AddItem(i.plugin()->Title());

		const auto ExitCode = menu->Run();

		if (ExitCode < 0 || static_cast<size_t>(ExitCode) >= items.size())
			return nullptr;

		PluginIterator = items.begin() + ExitCode;
	}

	SetFindListInfo Info{ sizeof(Info) };
	Info.hPanel = PluginIterator->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;

	if (!PluginIterator->plugin()->SetFindList(&Info))
		return nullptr;

	return std::make_unique<plugin_panel>(std::move(*PluginIterator));
}


void PluginManager::ClosePanel(std::unique_ptr<plugin_panel>&& hPlugin)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	ClosePanelInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	hPlugin->plugin()->ClosePanel(&Info);
}


int PluginManager::ProcessEditorInput(const INPUT_RECORD *Rec) const
{
	ProcessEditorInputInfo Info={sizeof(Info)};
	Info.Rec=*Rec;

	return std::any_of(CONST_RANGE(SortedPlugins, i) {return i->has(iProcessEditorInput) && i->ProcessEditorInput(&Info);});
}


int PluginManager::ProcessEditorEvent(int Event, void *Param, const Editor* EditorInstance) const
{
	int nResult = 0;
	if (const auto Container = EditorInstance->GetOwner())
	{
		if (Event == EE_REDRAW)
		{
			const auto FED = std::dynamic_pointer_cast<FileEditor>(Container).get();
			FED->AutoDeleteColors();
		}

		ProcessEditorEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.EditorID = EditorInstance->GetId();

		SCOPED_ACTION(auto)(Container->GetPinner());
		std::for_each(CONST_RANGE(SortedPlugins, i)
		{
			if (i->has(iProcessEditorEvent))
				nResult = i->ProcessEditorEvent(&Info);
		});
	}

	return nResult;
}


int PluginManager::ProcessSubscribedEditorEvent(int Event, void *Param, const Editor* EditorInstance, const std::unordered_set<UUID>& PluginIds) const
{
	int nResult = 0;
	if (const auto Container = EditorInstance->GetOwner())
	{
		ProcessEditorEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.EditorID = EditorInstance->GetId();

		SCOPED_ACTION(auto)(Container->GetPinner());
		std::for_each(CONST_RANGE(SortedPlugins, i)
		{
			if (i->has(iProcessEditorEvent) && PluginIds.count(i->Id()))
			{
				nResult = i->ProcessEditorEvent(&Info);
			}
		});
	}

	return nResult;
}


int PluginManager::ProcessViewerEvent(int Event, void *Param, const Viewer* ViewerInstance) const
{
	int nResult = 0;
	if (const auto Container = ViewerInstance->GetOwner())
	{
		ProcessViewerEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.ViewerID = ViewerInstance->GetId();

		SCOPED_ACTION(auto)(Container->GetPinner());
		std::for_each(CONST_RANGE(SortedPlugins, i)
		{
			if (i->has(iProcessViewerEvent))
				nResult = i->ProcessViewerEvent(&Info);
		});
	}
	return nResult;
}

int PluginManager::ProcessDialogEvent(int Event, FarDialogEvent *Param) const
{
	ProcessDialogEventInfo Info = {sizeof(Info)};
	Info.Event = Event;
	Info.Param = Param;

	return std::any_of(CONST_RANGE(SortedPlugins, i) {return i->has(iProcessDialogEvent) && i->ProcessDialogEvent(&Info);});
}

int PluginManager::ProcessConsoleInput(ProcessConsoleInputInfo *Info) const
{
	int nResult = 0;

	for (const auto& i: SortedPlugins)
	{
		if (i->has(iProcessConsoleInput))
		{
			int n = i->ProcessConsoleInput(Info);
			if (n == 1)
			{
				nResult = 1;
				break;
			}
			else if (n == 2)
			{
				nResult = 2;
			}
		}
	}

	return nResult;
}


int PluginManager::GetFindData(const plugin_panel* hPlugin, PluginPanelItem **pPanelData, size_t *pItemsNumber, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	GetFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.OpMode = OpMode;
	int Result = hPlugin->plugin()->GetFindData(&Info);
	*pPanelData = Info.PanelItem;
	*pItemsNumber = Info.ItemsNumber;
	return Result;
}


void PluginManager::FreeFindData(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool FreeUserData)
{
	if (FreeUserData)
	{
		std::for_each(PanelItem, PanelItem + ItemsNumber, [&hPlugin](const PluginPanelItem& i)
		{
			FreePluginPanelItemUserData(const_cast<plugin_panel*>(hPlugin), i.UserData);
		});
	}

	FreeFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	hPlugin->plugin()->FreeFindData(&Info);
}


int PluginManager::GetVirtualFindData(const plugin_panel* hPlugin, PluginPanelItem **pPanelData, size_t *pItemsNumber, const string& Path)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	*pItemsNumber=0;

	GetVirtualFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.Path = Path.c_str();
	const auto Result = hPlugin->plugin()->GetVirtualFindData(&Info);
	*pPanelData = Info.PanelItem;
	*pItemsNumber = Info.ItemsNumber;
	return Result;
}


void PluginManager::FreeVirtualFindData(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	FreeFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	return hPlugin->plugin()->FreeVirtualFindData(&Info);
}


int PluginManager::SetDirectory(const plugin_panel* hPlugin, const string& Dir, int OpMode, const UserDataItem *UserData)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SetDirectoryInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.Dir = Dir.c_str();
	Info.OpMode = OpMode;
	if (UserData)
	{
		Info.UserData = *UserData;
	}
	return hPlugin->plugin()->SetDirectory(&Info);
}


bool PluginManager::GetFile(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, const string& DestPath, string &strResultName, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	std::unique_ptr<SaveScreen> SaveScr;
	bool Found = false;
	Global->KeepUserScreen=FALSE;

	if (!(OpMode & OPM_FIND))
		SaveScr = std::make_unique<SaveScreen>(); //???

	SCOPED_ACTION(UndoGlobalSaveScrPtr)(SaveScr.get());

	GetFilesInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = 1;
	Info.Move = 0;
	Info.DestPath = DestPath.c_str();
	Info.OpMode = OpMode;

	const auto GetCode = hPlugin->plugin()->GetFiles(&Info);

	const auto Find = os::fs::enum_files(path::join(Info.DestPath, L'*'));
	const auto ItemIterator = std::find_if(CONST_RANGE(Find, i) { return !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY); });
	if (ItemIterator != Find.cend())
	{
		string_view Name = PanelItem->FileName;
		const auto isADS = GetCode == 1 && starts_with(Name, ItemIterator->FileName) && starts_with(Name.substr(ItemIterator->FileName.size()), L':');
		auto Result = path::join(Info.DestPath, isADS? Name : ItemIterator->FileName);

		if (GetCode == 1)
		{
			Found = true;
			strResultName = std::move(Result);
		}
		else
		{
			os::fs::set_file_attributes(Result,FILE_ATTRIBUTE_NORMAL);
			os::fs::delete_file(Result); //BUGBUG
		}
	}

	ReadUserBackgound(SaveScr.get());
	return Found;
}


int PluginManager::DeleteFiles(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	DeleteFilesInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.OpMode = OpMode;

	int Code = hPlugin->plugin()->DeleteFiles(&Info);

	ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::MakeDirectory(const plugin_panel* hPlugin, const wchar_t **Name, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	MakeDirectoryInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.Name = *Name;
	Info.OpMode = OpMode;

	int Code = hPlugin->plugin()->MakeDirectory(&Info);

	*Name = Info.Name;

	ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::ProcessHostFile(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	ProcessHostFileInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.OpMode = OpMode;

	int Code = hPlugin->plugin()->ProcessHostFile(&Info);

	ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::GetFiles(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, const wchar_t **DestPath, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	GetFilesInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.Move = Move;
	Info.DestPath = *DestPath;
	Info.OpMode = OpMode;

	int Result = hPlugin->plugin()->GetFiles(&Info);
	*DestPath = Info.DestPath;
	return Result;
}


int PluginManager::PutFiles(const plugin_panel* hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, int OpMode)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	static string strCurrentDirectory;
	strCurrentDirectory = os::fs::GetCurrentDirectory();
	PutFilesInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.Move = Move;
	Info.SrcPath = strCurrentDirectory.c_str();
	Info.OpMode = OpMode;

	const auto Result = hPlugin->plugin()->PutFiles(&Info);

	ReadUserBackgound(&SaveScr);

	return Result;
}

void PluginManager::GetOpenPanelInfo(const plugin_panel* hPlugin, OpenPanelInfo *Info)
{
	if (!Info)
		return;

	*Info = {};

	Info->StructSize = sizeof(OpenPanelInfo);
	Info->hPanel = hPlugin->panel();
	hPlugin->plugin()->GetOpenPanelInfo(Info);

	if (Info->CurDir && *Info->CurDir && (Info->Flags & OPIF_REALNAMES) && (Global->CtrlObject->Cp()->ActivePanel()->GetPluginHandle() == hPlugin) && ParsePath(Info->CurDir)!=root_type::unknown)
		os::fs::SetCurrentDirectory(Info->CurDir, false);
}


int PluginManager::ProcessKey(const plugin_panel* hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	ProcessPanelInputInfo Info={sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.Rec=*Rec;

#ifndef NO_WRAPPER
	if (Pred && hPlugin->plugin()->IsOemPlugin())
		Info.Rec.EventType |= 0x4000;
#endif
	return hPlugin->plugin()->ProcessPanelInput(&Info);
}


int PluginManager::ProcessEvent(const plugin_panel* hPlugin, int Event, void *Param)
{
	ProcessPanelEventInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.Event = Event;
	Info.Param = Param;

	return hPlugin->plugin()->ProcessPanelEvent(&Info);
}


int PluginManager::Compare(const plugin_panel* hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode)
{
	CompareInfo Info = {sizeof(Info)};
	Info.hPanel = hPlugin->panel();
	Info.Item1 = Item1;
	Info.Item2 = Item2;
	Info.Mode = static_cast<OPENPANELINFO_SORTMODES>(Mode);

	return hPlugin->plugin()->Compare(&Info);
}

void PluginManager::ConfigureCurrent(Plugin *pPlugin, const GUID& Guid)
{
	ConfigureInfo Info = {sizeof(Info)};
	Info.Guid = &Guid;

	if (pPlugin->Configure(&Info))
	{
		panel_ptr Panels[] =
		{
			Global->CtrlObject->Cp()->LeftPanel(),
			Global->CtrlObject->Cp()->RightPanel(),
		};

		std::for_each(CONST_RANGE(Panels, i)
		{
			if (i->GetMode() == panel_mode::PLUGIN_PANEL)
			{
				i->Update(UPDATE_KEEP_SELECTION);
				i->SetViewMode(i->GetViewMode());
				i->Redraw();
			}
		});
		pPlugin->SaveToCache();
	}
}

struct PluginMenuItemData
{
	Plugin *pPlugin;
	GUID Guid;
};

static string AddHotkey(const string& Item, wchar_t Hotkey)
{
	return concat(!Hotkey?  L" "s : Hotkey == L'&'? L"&&&"s : L"&"s + Hotkey, L"  "sv, Item);
}

/* $ 29.05.2001 IS
   ! При настройке "параметров внешних модулей" закрывать окно с их
     списком только при нажатии на ESC
*/
void PluginManager::Configure(int StartPos)
{
	const auto PluginList = VMenu2::create(msg(lng::MPluginConfigTitle), {}, ScrY - 4);
	PluginList->SetMenuFlags(VMENU_WRAPMODE);
	PluginList->SetHelp(L"PluginsConfig"sv);
	PluginList->SetId(PluginsConfigMenuId);

	bool NeedUpdateItems = true;

	while (!Global->CloseFAR)
	{
		bool HotKeysPresent = ConfigProvider().PlHotkeyCfg()->HotkeysPresent(hotkey_type::config_menu);

		if (NeedUpdateItems)
		{
			PluginList->clear();
			LoadIfCacheAbsent();
			string strName;
			GUID guid;

			for (const auto& i: SortedPlugins)
			{
				bool bCached = i->CheckWorkFlags(PIWF_CACHED);
				unsigned long long id = 0;

				PluginInfo Info = {sizeof(Info)};
				if (bCached)
				{
					id = ConfigProvider().PlCacheCfg()->GetCacheID(i->CacheName());
				}
				else
				{
					if (!GetPluginInfo(i, &Info))
						continue;
				}

				for (size_t J=0; ; J++)
				{
					if (bCached)
					{
						if (!ConfigProvider().PlCacheCfg()->GetPluginsConfigMenuItem(id, J, strName, guid))
							break;
					}
					else
					{
						if (J >= Info.PluginConfig.Count)
							break;

						strName = NullToEmpty(Info.PluginConfig.Strings[J]);
						guid = Info.PluginConfig.Guids[J];
					}

					const auto Hotkey = GetPluginHotKey(i, guid, hotkey_type::config_menu);
					MenuItemEx ListItem;
#ifndef NO_WRAPPER
					if (i->IsOemPlugin())
						ListItem.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
					if (!HotKeysPresent)
						ListItem.Name = strName;
					else
						ListItem.Name = AddHotkey(strName, Hotkey);

					PluginMenuItemData item = { i, guid };

					ListItem.ComplexUserData = item;

					PluginList->AddItem(ListItem);
				}
			}

			PluginList->AssignHighlights(FALSE);
			PluginList->SetBottomTitle(msg(lng::MPluginHotKeyBottom));
			PluginList->SortItems(false, HotKeysPresent? 3 : 0);
			PluginList->SetSelectPos(StartPos,1);
			NeedUpdateItems = false;
		}

		string strPluginModuleName;

		PluginList->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			int SelPos=PluginList->GetSelectPos();
			const auto item = PluginList->GetComplexUserDataPtr<PluginMenuItemData>(SelPos);
			int KeyProcessed = 1;

			switch (Key)
			{
				case KEY_SHIFTF1:
					if (item)
					{
						strPluginModuleName = item->pPlugin->ModuleName();
						if (!pluginapi::apiShowHelp(strPluginModuleName.c_str(), L"Config", FHELP_SELFHELP | FHELP_NOSHOWERROR) &&
							!pluginapi::apiShowHelp(strPluginModuleName.c_str(), L"Configure", FHELP_SELFHELP | FHELP_NOSHOWERROR))
						{
							pluginapi::apiShowHelp(strPluginModuleName.c_str(), nullptr, FHELP_SELFHELP | FHELP_NOSHOWERROR);
						}
					}
					break;

				case KEY_F3:
					if (item)
					{
						ShowPluginInfo(item->pPlugin, item->Guid);
					}
					break;

				case KEY_F4:
					if (item)
					{
						int nOffset = HotKeysPresent?3:0;
						if (SetHotKeyDialog(item->pPlugin, item->Guid, hotkey_type::config_menu, trim(string_view(PluginList->current().Name).substr(nOffset))))
						{
							NeedUpdateItems = true;
							StartPos = SelPos;
							PluginList->Close(SelPos);
							break;
						}
					}
					break;

				default:
					KeyProcessed = 0;
			}
			return KeyProcessed;
		});

		if (!NeedUpdateItems)
		{
			StartPos=PluginList->GetExitCode();

			if (StartPos<0)
				break;

			const auto item = PluginList->GetComplexUserDataPtr<PluginMenuItemData>(StartPos);
			ConfigureCurrent(item->pPlugin, item->Guid);
		}
	}
}

int PluginManager::CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName)
{
	if (ModalType == windowtype_dialog || ModalType == windowtype_menu)
	{
		const auto dlg = std::static_pointer_cast<Dialog>(Global->WindowManager->GetCurrentWindow());
		if (dlg->CheckDialogMode(DMODE_NOPLUGINS) || dlg->GetId()==PluginsMenuId)
		{
			return 0;
		}
	}

	bool Editor = ModalType==windowtype_editor;
	bool Viewer = ModalType==windowtype_viewer;
	bool Dialog = ModalType==windowtype_dialog || ModalType==windowtype_menu;

	PluginMenuItemData item;

	{
		const auto PluginList = VMenu2::create(msg(lng::MPluginCommandsMenuTitle), {}, ScrY - 4);
		PluginList->SetMenuFlags(VMENU_WRAPMODE);
		PluginList->SetHelp(L"PluginCommands"sv);
		PluginList->SetId(PluginsMenuId);
		bool NeedUpdateItems = true;

		while (NeedUpdateItems)
		{
				bool HotKeysPresent = ConfigProvider().PlHotkeyCfg()->HotkeysPresent(hotkey_type::plugins_menu);

				PluginList->clear();
				LoadIfCacheAbsent();
				string strName;
				GUID guid;

				for (const auto& i: SortedPlugins)
				{
					bool bCached = i->CheckWorkFlags(PIWF_CACHED);
					unsigned long long IFlags;
					unsigned long long id = 0;

					PluginInfo Info = {sizeof(Info)};
					if (bCached)
					{
						id = ConfigProvider().PlCacheCfg()->GetCacheID(i->CacheName());
						IFlags = ConfigProvider().PlCacheCfg()->GetFlags(id);
					}
					else
					{
						if (!GetPluginInfo(i, &Info))
							continue;

						IFlags = Info.Flags;
					}

					if ((Editor && !(IFlags & PF_EDITOR)) ||
					        (Viewer && !(IFlags & PF_VIEWER)) ||
					        (Dialog && !(IFlags & PF_DIALOG)) ||
					        (!Editor && !Viewer && !Dialog && (IFlags & PF_DISABLEPANELS)))
						continue;

					for (size_t J=0; ; J++)
					{
						if (bCached)
						{
							if (!ConfigProvider().PlCacheCfg()->GetPluginsMenuItem(id, J, strName, guid))
								break;
						}
						else
						{
							if (J >= Info.PluginMenu.Count)
								break;

							strName = NullToEmpty(Info.PluginMenu.Strings[J]);
							guid = Info.PluginMenu.Guids[J];
						}

						const auto Hotkey = GetPluginHotKey(i, guid, hotkey_type::plugins_menu);
						MenuItemEx ListItem;
#ifndef NO_WRAPPER
						if (i->IsOemPlugin())
							ListItem.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
						if (!HotKeysPresent)
							ListItem.Name = strName;
						else
							ListItem.Name = AddHotkey(strName, Hotkey);

						PluginMenuItemData itemdata;
						itemdata.pPlugin = i;
						itemdata.Guid = guid;

						ListItem.ComplexUserData = itemdata;

						PluginList->AddItem(ListItem);
					}
				}

				PluginList->AssignHighlights(FALSE);
				PluginList->SetBottomTitle(msg(lng::MPluginHotKeyBottom));
				PluginList->SortItems(false, HotKeysPresent? 3 : 0);
				PluginList->SetSelectPos(StartPos,1);
				NeedUpdateItems = false;


			PluginList->Run([&](const Manager::Key& RawKey)
			{
				const auto Key=RawKey();
				int SelPos=PluginList->GetSelectPos();
				const auto ItemPtr = PluginList->GetComplexUserDataPtr<PluginMenuItemData>(SelPos);
				int KeyProcessed = 1;

				switch (Key)
				{
					case KEY_SHIFTF1:
						// Вызываем нужный топик, который передали в CommandsMenu()
						if (ItemPtr)
							pluginapi::apiShowHelp(ItemPtr->pPlugin->ModuleName().c_str(), HistoryName, FHELP_SELFHELP | FHELP_NOSHOWERROR | FHELP_USECONTENTS);
						break;

					case KEY_F3:
						if (ItemPtr)
						{
							ShowPluginInfo(ItemPtr->pPlugin, ItemPtr->Guid);
						}
						break;

					case KEY_F4:
						if (ItemPtr)
						{
							int nOffset = HotKeysPresent?3:0;
							if (SetHotKeyDialog(ItemPtr->pPlugin, ItemPtr->Guid, hotkey_type::plugins_menu, trim(string_view(PluginList->current().Name).substr(nOffset))))
							{
								NeedUpdateItems = true;
								StartPos = SelPos;
								PluginList->Close(SelPos);
							}
						}
						break;

					case KEY_ALTSHIFTF9:
					case KEY_RALTSHIFTF9:
					{
						if (ItemPtr)
						{
							NeedUpdateItems = true;
							StartPos = SelPos;
							Configure();
							PluginList->Close(SelPos);
						}
						break;
					}

					case KEY_SHIFTF9:
					{
						if (ItemPtr)
						{
							NeedUpdateItems = true;
							StartPos=SelPos;

							if (ItemPtr->pPlugin->has(iConfigure))
								ConfigureCurrent(ItemPtr->pPlugin, ItemPtr->Guid);

							PluginList->Close(SelPos);
						}

						break;
					}

					default:
						KeyProcessed = 0;
				}
				return KeyProcessed;
			});
		}

		int ExitCode=PluginList->GetExitCode();

		if (ExitCode<0)
		{
			return FALSE;
		}

		Global->ScrBuf->Flush();
		item = *PluginList->GetComplexUserDataPtr<PluginMenuItemData>(ExitCode);
	}

	const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	int OpenCode=OPEN_PLUGINSMENU;
	intptr_t Item=0;
	OpenDlgPluginData pd={sizeof(OpenDlgPluginData)};

	if (Editor)
	{
		OpenCode=OPEN_EDITOR;
	}
	else if (Viewer)
	{
		OpenCode=OPEN_VIEWER;
	}
	else if (Dialog)
	{
		OpenCode=OPEN_DIALOG;
		pd.hDlg=(HANDLE)Global->WindowManager->GetCurrentWindow().get();
		Item=(intptr_t)&pd;
	}

	auto hPlugin = Open(item.pPlugin, OpenCode, item.Guid, Item);

	if (hPlugin && !Editor && !Viewer && !Dialog)
	{
		if (ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		{
			ClosePanel(std::move(hPlugin));
			return FALSE;
		}

		const auto NewPanel = Global->CtrlObject->Cp()->ChangePanel(ActivePanel, panel_type::FILE_PANEL, TRUE, TRUE);
		NewPanel->SetPluginMode(std::move(hPlugin), {}, true);
		NewPanel->Update(0);
		NewPanel->Show();
	}

	// restore title for old plugins only.
#ifndef NO_WRAPPER
	if (item.pPlugin->IsOemPlugin() && Editor && Global->WindowManager->GetCurrentEditor())
	{
		Global->WindowManager->GetCurrentEditor()->SetPluginTitle(nullptr);
	}
#endif // NO_WRAPPER
	return TRUE;
}

bool PluginManager::SetHotKeyDialog(Plugin* const pPlugin, const GUID& Guid, const hotkey_type HotKeyType, const string_view DlgPluginTitle)
{
	const auto strPluginKey = GetHotKeyPluginKey(pPlugin);
	auto strHotKey = ConfigProvider().PlHotkeyCfg()->GetHotkey(strPluginKey, Guid, HotKeyType);

	DialogBuilder Builder(lng::MPluginHotKeyTitle, L"SetHotKeyDialog"sv);
	Builder.AddText(lng::MPluginHotKey);
	Builder.AddTextAfter(Builder.AddFixEditField(strHotKey, 1), null_terminated(DlgPluginTitle).c_str());
	Builder.AddOKCancel();
	if(Builder.ShowDialog())
	{
		if (!strHotKey.empty() && strHotKey.front() != L' ')
			ConfigProvider().PlHotkeyCfg()->SetHotkey(strPluginKey, Guid, HotKeyType, strHotKey);
		else
			ConfigProvider().PlHotkeyCfg()->DelHotkey(strPluginKey, Guid, HotKeyType);
		return true;
	}
	return false;
}

void PluginManager::ShowPluginInfo(Plugin *pPlugin, const GUID& Guid)
{
	const auto strPluginGuid = GuidToStr(pPlugin->Id());
	const auto strItemGuid = GuidToStr(Guid);
	string strPluginPrefix;
	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		unsigned long long id = ConfigProvider().PlCacheCfg()->GetCacheID(pPlugin->CacheName());
		strPluginPrefix = ConfigProvider().PlCacheCfg()->GetCommandPrefix(id);
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};
		if (GetPluginInfo(pPlugin, &Info))
		{
			strPluginPrefix = NullToEmpty(Info.CommandPrefix);
		}
	}
	const int Width = 36;
	DialogBuilder Builder(lng::MPluginInformation, L"ShowPluginInfo"sv);
	Builder.SetId(PluginInformationId);
	Builder.AddText(lng::MPluginModuleTitle);
	Builder.AddConstEditField(pPlugin->Title(), Width);
	Builder.AddText(lng::MPluginDescription);
	Builder.AddConstEditField(pPlugin->Description(), Width);
	Builder.AddText(lng::MPluginAuthor);
	Builder.AddConstEditField(pPlugin->Author(), Width);
	Builder.AddText(lng::MPluginVersion);
	Builder.AddConstEditField(pPlugin->VersionString(), Width);
	Builder.AddText(lng::MPluginModulePath);
	Builder.AddConstEditField(pPlugin->ModuleName(), Width);
	Builder.AddText(lng::MPluginGUID);
	Builder.AddConstEditField(strPluginGuid, Width);
	Builder.AddText(lng::MPluginItemGUID);
	Builder.AddConstEditField(strItemGuid, Width);
	Builder.AddText(lng::MPluginPrefix);
	Builder.AddConstEditField(strPluginPrefix, Width);
	Builder.AddOK();
	Builder.ShowDialog();
}

static char* BufReserve(char*& Buf, size_t Count, size_t& Rest, size_t& Size)
{
	char* Res = nullptr;

	if (Buf)
	{
		if (Rest >= Count)
		{
			Res = Buf;
			Buf += Count;
			Rest -= Count;
		}
		else
		{
			Buf += Rest;
			Rest = 0;
		}
	}

	Size += Count;
	return Res;
}


static wchar_t* StrToBuf(const string& Str, char*& Buf, size_t& Rest, size_t& Size)
{
	const auto Count = (Str.size() + 1) * sizeof(wchar_t);
	const auto Res = reinterpret_cast<wchar_t*>(BufReserve(Buf, Count, Rest, Size));
	if (Res)
	{
		*std::copy(ALL_CONST_RANGE(Str), Res) = L'\0';
	}
	return Res;
}


static void ItemsToBuf(PluginMenuItem& Menu, const std::vector<string>& NamesArray, const std::vector<GUID>& GuidsArray, char*& Buf, size_t& Rest, size_t& Size)
{
	Menu.Count = NamesArray.size();
	Menu.Strings = nullptr;
	Menu.Guids = nullptr;

	if (Menu.Count)
	{
		const auto Items = reinterpret_cast<wchar_t**>(BufReserve(Buf, Menu.Count * sizeof(wchar_t*), Rest, Size));
		const auto Guids = reinterpret_cast<GUID*>(BufReserve(Buf, Menu.Count * sizeof(GUID), Rest, Size));
		Menu.Strings = Items;
		Menu.Guids = Guids;

		for (size_t i = 0; i < Menu.Count; ++i)
		{
			wchar_t* pStr = StrToBuf(NamesArray[i], Buf, Rest, Size);
			if (Items)
			{
				Items[i] = pStr;
			}

			if (Guids)
			{
				Guids[i] = GuidsArray[i];
			}
		}
	}
}

size_t PluginManager::GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize)
{
	if(IsPluginUnloaded(pPlugin)) return 0;
	string Prefix;
	PLUGIN_FLAGS Flags = 0;

	using menu_items = std::pair<std::vector<string>, std::vector<GUID>>;
	menu_items MenuItems, DiskItems, ConfItems;

	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		unsigned long long id = ConfigProvider().PlCacheCfg()->GetCacheID(pPlugin->CacheName());
		Flags = ConfigProvider().PlCacheCfg()->GetFlags(id);
		Prefix = ConfigProvider().PlCacheCfg()->GetCommandPrefix(id);

		string Name;
		GUID Guid;

		const auto& ReadCache = [&](const auto& Getter, auto& Items)
		{
			for (size_t i = 0; std::invoke(Getter, ConfigProvider().PlCacheCfg(), id, i, Name, Guid); ++i)
			{
				Items.first.emplace_back(Name);
				Items.second.emplace_back(Guid);
			}
		};

		ReadCache(&PluginsCacheConfig::GetPluginsMenuItem, MenuItems);
		ReadCache(&PluginsCacheConfig::GetDiskMenuItem, DiskItems);
		ReadCache(&PluginsCacheConfig::GetPluginsConfigMenuItem, ConfItems);
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};
		if (GetPluginInfo(pPlugin, &Info))
		{
			Flags = Info.Flags;
			Prefix = NullToEmpty(Info.CommandPrefix);

			const auto& CopyData = [](const PluginMenuItem& Item, menu_items& Items)
			{
				for (size_t i = 0; i < Item.Count; i++)
				{
					Items.first.emplace_back(Item.Strings[i]);
					Items.second.emplace_back(Item.Guids[i]);
				}
			};

			CopyData(Info.PluginMenu, MenuItems);
			CopyData(Info.DiskMenu, DiskItems);
			CopyData(Info.PluginConfig, ConfItems);
		}
	}

	struct
	{
		FarGetPluginInformation fgpi;
		PluginInfo PInfo;
		GlobalInfo GInfo;
	} Temp;
	char* Buffer = nullptr;
	size_t Rest = 0;
	size_t Size = sizeof(Temp);

	if (pInfo)
	{
		Rest = BufferSize - Size;
		Buffer = reinterpret_cast<char*>(pInfo) + Size;
	}
	else
	{
		pInfo = &Temp.fgpi;
	}

	pInfo->PInfo = reinterpret_cast<PluginInfo*>(pInfo+1);
	pInfo->GInfo = reinterpret_cast<GlobalInfo*>(pInfo->PInfo+1);
	pInfo->ModuleName = StrToBuf(pPlugin->ModuleName(), Buffer, Rest, Size);

	pInfo->Flags = 0;

	if (pPlugin->m_Instance)
	{
		pInfo->Flags |= FPF_LOADED;
	}
#ifndef NO_WRAPPER
	if (pPlugin->IsOemPlugin())
	{
		pInfo->Flags |= FPF_ANSI;
	}
#endif // NO_WRAPPER

	pInfo->GInfo->StructSize = sizeof(GlobalInfo);
	pInfo->GInfo->Guid = pPlugin->Id();
	pInfo->GInfo->Version = pPlugin->Version();
	pInfo->GInfo->MinFarVersion = pPlugin->MinFarVersion();
	pInfo->GInfo->Title = StrToBuf(pPlugin->strTitle, Buffer, Rest, Size);
	pInfo->GInfo->Description = StrToBuf(pPlugin->strDescription, Buffer, Rest, Size);
	pInfo->GInfo->Author = StrToBuf(pPlugin->strAuthor, Buffer, Rest, Size);

	pInfo->PInfo->StructSize = sizeof(PluginInfo);
	pInfo->PInfo->Flags = Flags;
	pInfo->PInfo->CommandPrefix = StrToBuf(Prefix, Buffer, Rest, Size);

	ItemsToBuf(pInfo->PInfo->DiskMenu, DiskItems.first, DiskItems.second, Buffer, Rest, Size);
	ItemsToBuf(pInfo->PInfo->PluginMenu, MenuItems.first, MenuItems.second, Buffer, Rest, Size);
	ItemsToBuf(pInfo->PInfo->PluginConfig, ConfItems.first, ConfItems.second, Buffer, Rest, Size);

	return Size;
}

bool PluginManager::GetDiskMenuItem(Plugin *pPlugin, size_t PluginItem, bool &ItemPresent, wchar_t& PluginHotkey, string &strPluginText, GUID &Guid) const
{
	LoadIfCacheAbsent();

	ItemPresent = false;

	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		ItemPresent = ConfigProvider().PlCacheCfg()->GetDiskMenuItem(ConfigProvider().PlCacheCfg()->GetCacheID(pPlugin->CacheName()), PluginItem, strPluginText, Guid) && !strPluginText.empty();
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};

		if (!GetPluginInfo(pPlugin, &Info) || Info.DiskMenu.Count <= PluginItem)
		{
			ItemPresent = false;
		}
		else
		{
			strPluginText = NullToEmpty(Info.DiskMenu.Strings[PluginItem]);
			Guid = Info.DiskMenu.Guids[PluginItem];
			ItemPresent = true;
		}
	}
	if (ItemPresent)
	{
		PluginHotkey = GetPluginHotKey(pPlugin, Guid, hotkey_type::drive_menu);
	}

	return true;
}

int PluginManager::UseFarCommand(const plugin_panel* const hPlugin, int const CommandType)
{
	OpenPanelInfo Info;
	GetOpenPanelInfo(hPlugin,&Info);

	if (!(Info.Flags & OPIF_REALNAMES))
		return FALSE;

	switch (CommandType)
	{
		case PLUGIN_FARGETFILE:
		case PLUGIN_FARGETFILES:
			return !hPlugin->plugin()->has(iGetFiles) || (Info.Flags & OPIF_EXTERNALGET);
		case PLUGIN_FARPUTFILES:
			return !hPlugin->plugin()->has(iPutFiles) || (Info.Flags & OPIF_EXTERNALPUT);
		case PLUGIN_FARDELETEFILES:
			return !hPlugin->plugin()->has(iDeleteFiles) || (Info.Flags & OPIF_EXTERNALDELETE);
		case PLUGIN_FARMAKEDIRECTORY:
			return !hPlugin->plugin()->has(iMakeDirectory) || (Info.Flags & OPIF_EXTERNALMKDIR);
	}

	return TRUE;
}


void PluginManager::ReloadLanguage() const
{
	std::for_each(ALL_CONST_RANGE(SortedPlugins), std::mem_fn(&Plugin::CloseLang));
	ConfigProvider().PlCacheCfg()->DiscardCache();
}

void PluginManager::LoadIfCacheAbsent() const
{
	if (ConfigProvider().PlCacheCfg()->IsCacheEmpty())
	{
		std::for_each(ALL_CONST_RANGE(SortedPlugins), std::mem_fn(&Plugin::Load));
	}
}

//template parameters must have external linkage
struct PluginData
{
	Plugin* pPlugin;
	unsigned long long PluginFlags;
};

bool PluginManager::ProcessCommandLine(const string& Command)
{
	if (!IsPluginPrefixPath(Command))
		return false;

	LoadIfCacheAbsent();
	const auto Prefix = string_view(Command).substr(0, Command.find(L':'));
	std::vector<PluginData> items;

	for (const auto& i: SortedPlugins)
	{
		string PluginPrefixes;
		unsigned long long PluginFlags;

		if (i->CheckWorkFlags(PIWF_CACHED))
		{
			unsigned long long id = ConfigProvider().PlCacheCfg()->GetCacheID(i->CacheName());
			PluginPrefixes = ConfigProvider().PlCacheCfg()->GetCommandPrefix(id);
			PluginFlags = ConfigProvider().PlCacheCfg()->GetFlags(id);
		}
		else
		{
			PluginInfo Info{sizeof(Info)};
			if (!GetPluginInfo(i, &Info))
				continue;

			PluginPrefixes = NullToEmpty(Info.CommandPrefix);
			PluginFlags = Info.Flags;
		}

		if (PluginPrefixes.empty())
			continue;

		const auto Enumerator = enum_tokens(PluginPrefixes, L":"sv);
		if (!std::any_of(ALL_CONST_RANGE(Enumerator), [&](const auto& p) { return equal_icase(p, Prefix); }))
			continue;

		if (!i->Load() || !i->has(iOpen))
			continue;

		items.emplace_back(PluginData{ i, PluginFlags });

		if (!Global->Opt->PluginConfirm.Prefix)
			break;
	}

	if (items.empty())
		return false;

	if (Global->CtrlObject->Cp()->ActivePanel()->ProcessPluginEvent(FE_CLOSE,nullptr))
		return false;

	auto PluginIterator = items.cbegin();

	if (items.size() > 1)
	{
		const auto Menu = VMenu2::create(msg(lng::MPluginConfirmationTitle), {}, ScrY - 4);
		Menu->SetPosition(-1, -1, 0, 0);
		Menu->SetHelp(L"ChoosePluginMenu"sv);
		Menu->SetMenuFlags(VMENU_SHOWAMPERSAND | VMENU_WRAPMODE);

		for (const auto& i: items)
		{
			Menu->AddItem(string(PointToName(i.pPlugin->ModuleName())));
		}

		const auto ExitCode = Menu->Run();

		if (ExitCode < 0)
			return false;

		PluginIterator += ExitCode;
	}

	// Copy instead of string_view as it goes into the wild
	const auto PluginCommand = Command.substr(PluginIterator->PluginFlags & PF_FULLCMDLINE? 0 : Prefix.size() + 1);
	const OpenCommandLineInfo info{ sizeof(OpenCommandLineInfo), PluginCommand.c_str() };
	if (auto hPlugin = Global->CtrlObject->Plugins->Open(PluginIterator->pPlugin, OPEN_COMMANDLINE, FarGuid, reinterpret_cast<intptr_t>(&info)))
	{
		const auto NewPanel = Global->CtrlObject->Cp()->ChangePanel(Global->CtrlObject->Cp()->ActivePanel(), panel_type::FILE_PANEL, TRUE, TRUE);
		NewPanel->SetPluginMode(std::move(hPlugin), {}, true);
		NewPanel->Update(0);
		NewPanel->Show();
	}
	return true;
}


/* $ 27.09.2000 SVS
  Функция CallPlugin - найти плагин по ID и запустить
  в зачаточном состоянии!
*/
bool PluginManager::CallPlugin(const GUID& SysID,int OpenFrom, void *Data,void **Ret)
{
	if (const auto Dlg = std::dynamic_pointer_cast<Dialog>(Global->WindowManager->GetCurrentWindow()))
	{
		if (Dlg->CheckDialogMode(DMODE_NOPLUGINS))
		{
			return false;
		}
	}

	const auto pPlugin = FindPlugin(SysID);

	if (!pPlugin || !pPlugin->has(iOpen) || Global->ProcessException)
		return false;

	auto PluginPanel = Open(pPlugin, OpenFrom, FarGuid, reinterpret_cast<intptr_t>(Data));
	bool process=false;

	if (OpenFrom == OPEN_FROMMACRO)
	{
		if (PluginPanel)
		{
			if (os::memory::is_pointer(PluginPanel->panel()) && PluginPanel->panel() != INVALID_HANDLE_VALUE)
			{
				const auto fmc = reinterpret_cast<FarMacroCall*>(PluginPanel->panel());
				if (fmc->Count > 0 && fmc->Values[0].Type == FMVT_PANEL)
				{
					process = true;
					PluginPanel->set_panel(fmc->Values[0].Pointer);
					if (fmc->Callback)
						fmc->Callback(fmc->CallbackData, fmc->Values, fmc->Count);
				}
			}
		}
	}
	else
	{
		process=OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_FILEPANEL;
	}

	if (PluginPanel && process)
	{
		const auto PluginPanelCopy = PluginPanel.get();
		const auto NewPanel = Global->CtrlObject->Cp()->ChangePanel(Global->CtrlObject->Cp()->ActivePanel(), panel_type::FILE_PANEL, TRUE, TRUE);

		NewPanel->SetPluginMode(std::move(PluginPanel), {}, true);
		if (OpenFrom != OPEN_FROMMACRO)
		{
			if (Data && *static_cast<const wchar_t *>(Data))
			{
				UserDataItem UserData = {};  // !!! NEED CHECK !!!
				SetDirectory(PluginPanelCopy, static_cast<const wchar_t *>(Data), 0, &UserData);
			}
		}
		else
		{
			NewPanel->Update(0);
			NewPanel->Show();
		}
	}

	if (Ret)
	{
		if (OpenFrom == OPEN_FROMMACRO && process)
		{
			*Ret = ToPtr(1);
		}
		else
		{
			*Ret = PluginPanel? PluginPanel->panel(): nullptr;
		}
	}

	return true;
}

// поддержка макрофункций plugin.call, plugin.cmd, plugin.config и т.п
bool PluginManager::CallPluginItem(const GUID& Guid, CallPluginInfo *Data)
{
	auto Result = false;

	if (Global->ProcessException)
		return false;

	const auto curType = Global->WindowManager->GetCurrentWindow()->GetType();

	if (curType==windowtype_dialog && std::static_pointer_cast<Dialog>(Global->WindowManager->GetCurrentWindow())->CheckDialogMode(DMODE_NOPLUGINS))
		return false;

	const auto Editor = curType == windowtype_editor;
	const auto Viewer = curType == windowtype_viewer;
	const auto Dialog = curType == windowtype_dialog;

	if (Data->CallFlags & CPT_CHECKONLY)
	{
		Data->pPlugin = FindPlugin(Guid);
		if (!Data->pPlugin || !Data->pPlugin->Load())
			return false;

		// Разрешен ли вызов данного типа в текущей области (предварительная проверка)
		switch (Data->CallFlags & CPT_MASK)
		{
		case CPT_MENU:
			if (!Data->pPlugin->has(iOpen))
				return false;
			break;

		case CPT_CONFIGURE:
			//TODO: Автокомплит не влияет?
			if (curType!=windowtype_panels)
				return false;

			if (!Data->pPlugin->has(iConfigure))
				return false;
			break;

		case CPT_CMDLINE:
			//TODO: Автокомплит не влияет?
			if (curType!=windowtype_panels)
				return false;

			//TODO: OpenPanel или OpenFilePlugin?
			if (!Data->pPlugin->has(iOpen))
				return false;
			break;

		case CPT_INTERNAL:
			//TODO: Уточнить функцию
			if (!Data->pPlugin->has(iOpen))
				return false;
			break;

		default:
			break;
		}

		PluginInfo Info{sizeof(Info)};
		if (!GetPluginInfo(Data->pPlugin, &Info))
			return false;

		auto IFlags = Info.Flags;
		PluginMenuItem* MenuItems = nullptr;

		// Разрешен ли вызов данного типа в текущей области
		switch (Data->CallFlags & CPT_MASK)
		{
		case CPT_MENU:
			if (
				(Editor && !(IFlags & PF_EDITOR)) ||
				(Viewer && !(IFlags & PF_VIEWER)) ||
				(Dialog && !(IFlags & PF_DIALOG)) ||
				(!Editor && !Viewer && !Dialog && (IFlags & PF_DISABLEPANELS)))
				return false;

			MenuItems = &Info.PluginMenu;
			break;

		case CPT_CONFIGURE:
			MenuItems = &Info.PluginConfig;
			break;

		case CPT_CMDLINE:
			if (!Info.CommandPrefix || !*Info.CommandPrefix)
				return false;
			break;

		case CPT_INTERNAL:
			break;

		default:
			break;
		}

		if ((Data->CallFlags & CPT_MASK)==CPT_MENU || (Data->CallFlags & CPT_MASK)==CPT_CONFIGURE)
		{
			auto ItemFound = false;
			if (!Data->ItemGuid)
			{
				if (MenuItems->Count == 1)
				{
					Data->FoundGuid = MenuItems->Guids[0];
					Data->ItemGuid = &Data->FoundGuid;
					ItemFound = true;
				}
			}
			else
			{
				if (contains(make_range(MenuItems->Guids, MenuItems->Count), *Data->ItemGuid))
				{
					Data->FoundGuid = *Data->ItemGuid;
					Data->ItemGuid = &Data->FoundGuid;
					ItemFound = true;
				}
			}
			if (!ItemFound)
				return false;
		}

		return true;
	}

	if (!Data->pPlugin)
		return false;

	std::unique_ptr<plugin_panel> hPlugin;
	panel_ptr ActivePanel;

	switch (Data->CallFlags & CPT_MASK)
	{
	case CPT_MENU:
		{
			ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
			auto OpenCode = OPEN_PLUGINSMENU;
			intptr_t Item = 0;
			OpenDlgPluginData pd { sizeof(pd) };

			if (Editor)
			{
				OpenCode = OPEN_EDITOR;
			}
			else if (Viewer)
			{
				OpenCode = OPEN_VIEWER;
			}
			else if (Dialog)
			{
				OpenCode = OPEN_DIALOG;
				pd.hDlg = static_cast<HANDLE>(Global->WindowManager->GetCurrentWindow().get());
				Item = reinterpret_cast<intptr_t>(&pd);
			}

			hPlugin=Open(Data->pPlugin,OpenCode,Data->FoundGuid,Item);
			Result = true;
		}
		break;

	case CPT_CONFIGURE:
		Global->CtrlObject->Plugins->ConfigureCurrent(Data->pPlugin,Data->FoundGuid);
		return true;

	case CPT_CMDLINE:
		{
			ActivePanel=Global->CtrlObject->Cp()->ActivePanel();
			string command=Data->Command; // Нужна копия строки
			OpenCommandLineInfo info{ sizeof(OpenCommandLineInfo), command.c_str() };
			hPlugin=Open(Data->pPlugin,OPEN_COMMANDLINE,FarGuid,(intptr_t)&info);
			Result = true;
		}
		break;

	case CPT_INTERNAL:
		//TODO: бывший CallPlugin
		//WARNING: учесть, что он срабатывает без переключения MacroState
		break;

	default:
		break;
	}

	if (hPlugin && !Editor && !Viewer && !Dialog)
	{
		//BUGBUG: Закрытие панели? Нужно ли оно?
		//BUGBUG: В ProcessCommandLine зовется перед Open, а в CPT_MENU - после
		if (ActivePanel->ProcessPluginEvent(FE_CLOSE, nullptr))
		{
			ClosePanel(std::move(hPlugin));
			return false;
		}

		const auto NewPanel = Global->CtrlObject->Cp()->ChangePanel(ActivePanel, panel_type::FILE_PANEL, TRUE, TRUE);
		NewPanel->SetPluginMode(std::move(hPlugin), {}, true);
		NewPanel->Update(0);
		NewPanel->Show();
	}

	// restore title for old plugins only.
#ifndef NO_WRAPPER
	if (Data->pPlugin->IsOemPlugin() && Editor && Global->WindowManager->GetCurrentEditor())
	{
		Global->WindowManager->GetCurrentEditor()->SetPluginTitle(nullptr);
	}
#endif // NO_WRAPPER

	return Result;
}

Plugin *PluginManager::FindPlugin(const GUID& SysID) const
{
	const auto Iterator = m_Plugins.find(SysID);
	return Iterator == m_Plugins.cend()? nullptr : Iterator->second.get();
}

std::unique_ptr<plugin_panel> PluginManager::Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item) const
{
	OpenInfo Info = {sizeof(Info)};
	Info.OpenFrom = static_cast<OPENFROM>(OpenFrom);
	Info.Guid = &Guid;
	Info.Data = Item;

	const auto PluginHandle = pPlugin->Open(&Info);
	if (!PluginHandle)
		return nullptr;

	return std::make_unique<plugin_panel>(pPlugin, PluginHandle);
}

std::vector<Plugin*> PluginManager::GetContentPlugins(const std::vector<const wchar_t*>& ColNames) const
{
	GetContentFieldsInfo Info = { sizeof(GetContentFieldsInfo), ColNames.size(), ColNames.data() };
	std::vector<Plugin*> Plugins;
	std::copy_if(ALL_CONST_RANGE(SortedPlugins), std::back_inserter(Plugins), [&Info](Plugin* p)
	{
		return p->has(iGetContentData) && p->has(iGetContentFields) && p->GetContentFields(&Info);
	});
	return Plugins;
}

void PluginManager::GetContentData(
	const std::vector<Plugin*>& Plugins,
	const string& Name,
	const std::vector<const wchar_t*>& ColNames,
	std::vector<const wchar_t*>& ColValues,
	std::unordered_map<string,string>& ContentData
) const
{
	const NTPath FilePath(Name);
	size_t Count = ColNames.size();
	std::for_each(CONST_RANGE(Plugins, i)
	{
		GetContentDataInfo GetInfo{ sizeof(GetContentDataInfo), FilePath.c_str(), Count, ColNames.data(), ColValues.data() };
		ColValues.assign(ColValues.size(), nullptr);

		if (i->GetContentData(&GetInfo) && GetInfo.Values)
		{
			for (size_t k=0; k<Count; k++)
			{
				if (GetInfo.Values[k])
					ContentData[ColNames[k]] += GetInfo.Values[k];
			}

			if (i->has(iFreeContentData))
			{
				i->FreeContentData(&GetInfo);
			}
		}
	});
}

const GUID& PluginManager::GetGUID(const plugin_panel* hPlugin)
{
	return hPlugin->plugin()->Id();
}

void PluginManager::RefreshPluginsList()
{
	if(!UnloadedPlugins.empty())
	{
		UnloadedPlugins.remove_if([this](const auto& i)
		{
			if (!i->Active())
			{
				i->Unload(true);
				RemovePlugin(i);
				return true;
			}
			return false;
		});
	}
}

void PluginManager::UndoRemove(Plugin* plugin)
{
	const auto i = std::find(UnloadedPlugins.begin(), UnloadedPlugins.end(), plugin);
	if(i != UnloadedPlugins.end())
		UnloadedPlugins.erase(i);
}

bool PluginManager::GetPluginInfo(Plugin *pPlugin, PluginInfo* Info)
{
	return pPlugin->GetPluginInfo(Info);
}

plugin_panel::plugin_panel(Plugin* PluginInstance, HANDLE Panel):
	m_Plugin(PluginInstance),
	m_PluginActivity(PluginInstance->keep_activity()),
	m_Panel(Panel)
{
}

plugin_panel::~plugin_panel() = default;

void plugin_panel::delayed_delete(const string& Name)
{
	m_DelayedDeleters.emplace_back(Name);
}
