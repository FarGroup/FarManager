/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)
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

#include "headers.hpp"
#pragma hdrstop

#include "plugins.hpp"
#include "keys.hpp"
#include "flink.hpp"
#include "scantree.hpp"
#include "chgprior.hpp"
#include "constitle.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "fileedit.hpp"
#include "RefreshFrameManager.hpp"
#include "plugapi.hpp"
#include "TaskBar.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "processname.hpp"
#include "interf.hpp"
#include "filelist.hpp"
#include "message.hpp"
#include "FarGuid.hpp"
#include "configdb.hpp"
#include "FarDlgBuilder.hpp"
#include "DlgGuid.hpp"
#include "mix.hpp"
#include "manager.hpp"

static const wchar_t *PluginsFolderName=L"Plugins";

static const struct plugin_less
{
	bool operator ()(const Plugin* a, const Plugin *b)
	{
		return StrCmpI(PointToName(a->GetModuleName()),PointToName(b->GetModuleName())) < 0;
	}
}
PluginLess;

PluginManager::PluginManager():
#ifndef NO_WRAPPER
	OemPluginsCount(0),
#endif // NO_WRAPPER
	m_CurEditor(nullptr),
	m_CurViewer(nullptr)
{
}

PluginManager::~PluginManager()
{
	Plugin *Luamacro=nullptr; // обеспечить выгрузку данного плагина последним.

	std::for_each(CONST_RANGE(SortedPlugins, i)
	{
		if (i->GetGUID() == LuamacroGuid)
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
}

bool PluginManager::AddPlugin(Plugin *pPlugin)
{
	auto Result = Plugins.emplace(VALUE_TYPE(Plugins)(pPlugin->GetGUID(), VALUE_TYPE(Plugins)::second_type()));
	if (!Result.second)
	{
		return false;
	}
	Result.first->second.reset(pPlugin);

	SortedPlugins.emplace_back(pPlugin);
#ifndef NO_WRAPPER
	if(pPlugin->IsOemPlugin())
	{
		OemPluginsCount++;
	}
#endif // NO_WRAPPER
	return true;
}

bool PluginManager::UpdateId(Plugin *pPlugin, const GUID& Id)
{
	auto Iterator = Plugins.find(pPlugin->GetGUID());
	// important, do not delete Plugin instance
	Iterator->second.release();
	Plugins.erase(Iterator);
	pPlugin->SetGuid(Id);
	auto Result = Plugins.emplace(VALUE_TYPE(Plugins)(pPlugin->GetGUID(), VALUE_TYPE(Plugins)::second_type()));
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
	Plugins.erase(pPlugin->GetGUID());
	return true;
}


Plugin* PluginManager::LoadPlugin(const string& FileName, const api::FAR_FIND_DATA &FindData, bool LoadToMem)
{
	Plugin *pPlugin = nullptr;

	std::any_of(CONST_RANGE(PluginModels, i) { return pPlugin = i->CreatePlugin(FileName); });

	if (pPlugin)
	{
		bool Result = false, bDataLoaded = false;

		if (!LoadToMem)
		{
			Result = pPlugin->LoadFromCache(FindData);
		}

		if (!Result && (pPlugin->CheckWorkFlags(PIWF_PRELOADED) || !Global->Opt->LoadPlug.PluginsCacheOnly))
		{
			Result = bDataLoaded = pPlugin->LoadData();
		}

		if (!Result || !AddPlugin(pPlugin))
		{
			pPlugin->Unload(true);
			delete pPlugin;
			return nullptr;
		}

		if (bDataLoaded && !pPlugin->Load())
		{
			pPlugin->Unload(true);
			RemovePlugin(pPlugin);
			pPlugin = nullptr;
		}
	}
	return pPlugin;
}

HANDLE PluginManager::LoadPluginExternal(const string& lpwszModuleName, bool LoadToMem)
{
	Plugin *pPlugin = GetPlugin(lpwszModuleName);

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
		api::FAR_FIND_DATA FindData;

		if (api::GetFindDataEx(lpwszModuleName, FindData))
		{
			pPlugin = LoadPlugin(lpwszModuleName, FindData, LoadToMem);
			if (!pPlugin)
				return nullptr;
			SortedPlugins.sort(PluginLess);
		}
	}
	return pPlugin;
}

int PluginManager::UnloadPlugin(Plugin *pPlugin, int From)
{
	int nResult = FALSE;

	if (pPlugin && (From != iExitFAR))   //схитрим, если упали в EXITFAR, не полезем в рекурсию, мы и так в Unload
	{
		for(int i = static_cast<int>(FrameManager->GetModalStackCount()-1); i >= 0; --i)
		{
			Frame *frame = FrameManager->GetModalFrame(i);
			if((frame->GetType()==MODALTYPE_DIALOG && static_cast<Dialog*>(frame)->GetPluginOwner() == pPlugin) || frame->GetType()==MODALTYPE_HELP)
			{
				frame->Lock();
				if(i)
				{
					FrameManager->GetModalFrame(i-1)->Lock();
				}
				FrameManager->DeleteFrame(frame);
				FrameManager->PluginCommit();
			}
		}

		bool bPanelPlugin = pPlugin->IsPanelPlugin();

		nResult = pPlugin->Unload(true);

		pPlugin->WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (bPanelPlugin /*&& bUpdatePanels*/)
		{
			Global->CtrlObject->Cp()->ActivePanel->SetCurDir(L".",true);
			Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
			ActivePanel->Update(UPDATE_KEEP_SELECTION);
			ActivePanel->Redraw();
			Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}

		UnloadedPlugins.emplace_back(pPlugin);
	}

	return nResult;
}

bool PluginManager::IsPluginUnloaded(Plugin* pPlugin)
{
	return std::find(UnloadedPlugins.cbegin(), UnloadedPlugins.cend(), pPlugin) != UnloadedPlugins.cend();
}

int PluginManager::UnloadPluginExternal(HANDLE hPlugin)
{
	//BUGBUG нужны проверки на легальность выгрузки
	int nResult = FALSE;
	Plugin* pPlugin = reinterpret_cast<Plugin*>(hPlugin);
	if(pPlugin->Active())
	{
		nResult = TRUE;
		if(!IsPluginUnloaded(pPlugin))
		{
			UnloadedPlugins.emplace_back(pPlugin);
		}
	}
	else
	{
		UnloadedPlugins.remove(pPlugin);
		nResult = pPlugin->Unload(true);
		RemovePlugin(pPlugin);
	}
	return nResult;
}

Plugin *PluginManager::GetPlugin(const string& ModuleName)
{
	auto ItemIterator = std::find_if(CONST_RANGE(SortedPlugins, i)
	{
		return !StrCmpI(i->GetModuleName(), ModuleName);
	});
	return ItemIterator == SortedPlugins.cend()? nullptr : *ItemIterator;
}

void PluginManager::LoadModels()
{
	PluginModels.emplace_back(new NativePluginModel(this));
#ifndef NO_WRAPPER
	if (Global->Opt->LoadPlug.OEMPluginsSupport)
		PluginModels.emplace_back(new wrapper::OEMPluginModel(this));
#endif // NO_WRAPPER

	ScanTree ScTree(false, true, Global->Opt->LoadPlug.ScanSymlinks);
	api::FAR_FIND_DATA FindData;
	ScTree.SetFindPath(Global->g_strFarPath + L"\\Adapters", L"*");

	string filename;
	while (ScTree.GetNextName(&FindData, filename))
	{
		if (CmpName(L"*.dll", filename.data(), false) && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			auto CustomModel = std::make_unique<CustomPluginModel>(this, filename);
			if (CustomModel->Success())
			{
				PluginModels.emplace_back(std::move(CustomModel));
			}
		}
	}
}

void PluginManager::LoadPlugins()
{
	TaskBar TB(false);
	Flags.Clear(PSIF_PLUGINSLOADDED);

	LoadModels();

	if (Global->Opt->LoadPlug.PluginsCacheOnly)  // $ 01.09.2000 tran  '/co' switch
	{
		LoadPluginsFromCache();
	}
	else if (Global->Opt->LoadPlug.MainPluginDir || !Global->Opt->LoadPlug.strCustomPluginsPath.empty() || (Global->Opt->LoadPlug.PluginsPersonal && !Global->Opt->LoadPlug.strPersonalPluginsPath.empty()))
	{
		ScanTree ScTree(FALSE,TRUE,Global->Opt->LoadPlug.ScanSymlinks);
		string strPluginsDir;
		string strFullName;
		api::FAR_FIND_DATA FindData;

		// сначала подготовим список
		if (Global->Opt->LoadPlug.MainPluginDir) // только основные и персональные?
		{
			strPluginsDir=Global->g_strFarPath+PluginsFolderName;
			// ...а персональные есть?
			if (Global->Opt->LoadPlug.PluginsPersonal && !Global->Opt->LoadPlug.strPersonalPluginsPath.empty() && !(Global->Opt->Policies.DisabledOptions&FFPOL_PERSONALPATH))
				strPluginsDir += L";" + Global->Opt->LoadPlug.strPersonalPluginsPath;
		}
		else if (!Global->Opt->LoadPlug.strCustomPluginsPath.empty())  // только "заказные" пути?
		{
			strPluginsDir = Global->Opt->LoadPlug.strCustomPluginsPath;
		}
		auto PluginPathList(StringToList(strPluginsDir, STLF_UNIQUE));

		// теперь пройдемся по всему ранее собранному списку
		std::for_each(CONST_RANGE(PluginPathList, i)
		{
			// расширяем значение пути
			strFullName = api::ExpandEnvironmentStrings(i);
			Unquote(strFullName); //??? здесь ХЗ

			if (!IsAbsolutePath(strFullName))
			{
				strPluginsDir = Global->g_strFarPath;
				strPluginsDir += strFullName;
				strFullName = strPluginsDir;
			}

			// Получим реальное значение полного длинного пути
			ConvertNameToFull(strFullName,strFullName);
			ConvertNameToLong(strFullName,strFullName);
			strPluginsDir = strFullName;

			// ставим на поток очередной путь из списка...
			ScTree.SetFindPath(strPluginsDir,L"*");

			// ...и пройдемся по нему
			while (ScTree.GetNextName(&FindData,strFullName))
			{
				if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					LoadPlugin(strFullName, FindData, false);
				}
			} // end while
		});
	}

	Flags.Set(PSIF_PLUGINSLOADDED);
	SortedPlugins.sort(PluginLess);
}

/* $ 01.09.2000 tran
   Load cache only plugins  - '/co' switch */
void PluginManager::LoadPluginsFromCache()
{
	string strModuleName;

	for (DWORD i=0; Global->Db->PlCacheCfg()->EnumPlugins(i, strModuleName); i++)
	{
		ReplaceSlashToBSlash(strModuleName);

		api::FAR_FIND_DATA FindData;

		if (api::GetFindDataEx(strModuleName, FindData))
			LoadPlugin(strModuleName, FindData, false);
	}
}

HANDLE PluginManager::OpenFilePlugin(
	const string* Name,
	int OpMode,	//!!! potential future error: OPERATION_MODES is __int64
	OPENFILEPLUGINTYPE Type
)
{
	struct PluginInfo
	{
		PluginHandle Handle;
		HANDLE Analyse;
		bool operator ==(const PluginInfo& rhs) const {return Handle.hPlugin == rhs.Handle.hPlugin && Handle.pPlugin == rhs.Handle.pPlugin && Analyse == rhs.Analyse;}
		bool operator !=(const PluginInfo& rhs) const {return !(*this == rhs);}
	};
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	ConsoleTitle ct(Global->Opt->ShowCheckingFile?MSG(MCheckingFileInPlugin):L"");
	HANDLE hResult = nullptr;
	std::list<PluginInfo> items;
	string strFullName;

	if (Name)
	{
		ConvertNameToFull(*Name,strFullName);
		Name = &strFullName;
	}

	bool ShowMenu = Global->Opt->PluginConfirm.OpenFilePlugin==BSTATE_3STATE? !(Type == OFP_NORMAL || Type == OFP_SEARCH) : Global->Opt->PluginConfirm.OpenFilePlugin != 0;
	bool ShowWarning = !OpMode;
	 //у анси плагинов OpMode нет.
	if(Type==OFP_ALTERNATIVE) OpMode|=OPM_PGDN;
	if(Type==OFP_COMMANDS) OpMode|=OPM_COMMANDS;

	Plugin *pPlugin = nullptr;

	api::File file;
	AnalyseInfo Info={sizeof(Info), Name? Name->data() : nullptr, nullptr, 0, (OPERATION_MODES)OpMode};
	bool DataRead = false;
	FOR_CONST_RANGE(SortedPlugins, i)
	{
		pPlugin = *i;

		if (!pPlugin->HasOpenFilePlugin() && !(pPlugin->HasAnalyse() && pPlugin->HasOpen()))
			continue;

		if(Name && !DataRead)
		{
			if (file.Open(*Name, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
			{
				Info.Buffer = new BYTE[Global->Opt->PluginMaxReadData];
				if (Info.Buffer)
				{
					DWORD DataSize = 0;
					if (file.Read(Info.Buffer, Global->Opt->PluginMaxReadData, DataSize))
					{
						Info.BufferSize = DataSize;
						DataRead = true;
					}
				}
				file.Close();
			}
			if(!DataRead)
			{
				if(ShowWarning)
				{
					Global->CatchError();
					Message(MSG_WARNING|MSG_ERRORTYPE, 1, L"", MSG(MOpenPluginCannotOpenFile), Name->data(), MSG(MOk));
				}
				break;
			}
		}

		HANDLE hPlugin;

		if (pPlugin->HasOpenFilePlugin())
		{
			if (Global->Opt->ShowCheckingFile)
				ct << MSG(MCheckingFileInPlugin) << L" - [" << PointToName(pPlugin->GetModuleName()) << L"]..." << fmt::Flush();

			hPlugin = pPlugin->OpenFilePlugin(Name? Name->data() : nullptr, (BYTE*)Info.Buffer, Info.BufferSize, OpMode);

			if (hPlugin == PANEL_STOP)   //сразу на выход, плагин решил нагло обработать все сам (Autorun/PictureView)!!!
			{
				hResult = PANEL_STOP;
				break;
			}

			if (hPlugin)
			{
				PluginInfo handle;
				handle.Handle.hPlugin = hPlugin;
				handle.Handle.pPlugin = pPlugin;
				handle.Analyse = nullptr;
				items.emplace_back(handle);
			}
		}
		else
		{
			HANDLE analyse=pPlugin->Analyse(&Info);
			if (analyse)
			{
				PluginInfo handle;
				handle.Handle.pPlugin = pPlugin;
				handle.Handle.hPlugin = nullptr;
				handle.Analyse = analyse;
				items.emplace_back(handle);
			}
		}

		if (!items.empty() && !ShowMenu)
			break;
	}

	auto pResult = items.end();
	auto pAnalyse = items.end();
	if (!items.empty() && (hResult != PANEL_STOP))
	{
		bool OnlyOne = (items.size() == 1) && !(Name && Global->Opt->PluginConfirm.OpenFilePlugin && Global->Opt->PluginConfirm.StandardAssociation && Global->Opt->PluginConfirm.EvenIfOnlyOnePlugin);

		if(!OnlyOne && ShowMenu)
		{
			VMenu2 menu(MSG(MPluginConfirmationTitle), nullptr, 0, ScrY-4);
			menu.SetPosition(-1, -1, 0, 0);
			menu.SetHelp(L"ChoosePluginMenu");
			menu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
			MenuItemEx mitem;

			std::for_each(CONST_RANGE(items, i)
			{
				mitem.Clear();
				mitem.strName = i.Handle.pPlugin->GetTitle();
				menu.AddItem(&mitem);
			});

			if (Global->Opt->PluginConfirm.StandardAssociation && Type == OFP_NORMAL)
			{
				mitem.Clear();
				mitem.Flags |= MIF_SEPARATOR;
				menu.AddItem(&mitem);
				mitem.Clear();
				mitem.strName = MSG(MMenuPluginStdAssociation);
				menu.AddItem(&mitem);
			}

			int ExitCode = menu.Run();
			if (ExitCode == -1)
				hResult = PANEL_STOP;
			else
			{
				if(ExitCode < static_cast<int>(items.size()))
				{
					pResult = items.begin();
					std::advance(pResult, ExitCode);
				}
			}
		}
		else
		{
			pResult = items.begin();
		}

		if (pResult != items.end() && pResult->Handle.hPlugin == nullptr)
		{
			pAnalyse = pResult;
			OpenAnalyseInfo oainfo={sizeof(OpenAnalyseInfo),&Info,pResult->Analyse};

			OpenInfo Info = {sizeof(Info)};
			Info.OpenFrom = OPEN_ANALYSE;
			Info.Guid = &FarGuid;
			Info.Data = (intptr_t)&oainfo;

			HANDLE h = pResult->Handle.pPlugin->Open(&Info);

			if (h == PANEL_STOP)
			{
				hResult = PANEL_STOP;
				pResult = items.end();
			}
			else if (h)
			{
				pResult->Handle.hPlugin = h;
			}
			else
			{
				pResult = items.end();
			}
		}
	}

	if(Info.Buffer)
	{
		delete[] (BYTE*)Info.Buffer;
	}

	std::for_each(CONST_RANGE(items, i)
	{
		if (pResult == items.end() || i != *pResult)
		{
			if (i.Handle.hPlugin)
			{
				ClosePanelInfo Info = {sizeof(Info)};
				Info.hPanel = i.Handle.hPlugin;
				i.Handle.pPlugin->ClosePanel(&Info);
			}
		}
		if (pAnalyse == items.end() || i != *pAnalyse)
		{
			if(i.Analyse)
			{
				CloseAnalyseInfo Info = {sizeof(Info)};
				Info.Handle = i.Analyse;
				i.Handle.pPlugin->CloseAnalyse(&Info);
			}
		}
	});

	if (pResult != items.end())
	{
		PluginHandle* pDup=new PluginHandle;
		pDup->hPlugin=pResult->Handle.hPlugin;
		pDup->pPlugin=pResult->Handle.pPlugin;
		hResult=static_cast<HANDLE>(pDup);
	}

	return hResult;
}

HANDLE PluginManager::OpenFindListPlugin(const PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	std::list<PluginHandle> items;
	auto pResult = items.end();
	Plugin *pPlugin=nullptr;

	FOR_CONST_RANGE(SortedPlugins, i)
	{
		pPlugin = *i;

		if (!pPlugin->HasSetFindList())
			continue;

		OpenInfo Info = {sizeof(Info)};
		Info.OpenFrom = OPEN_FINDLIST;
		Info.Guid = &FarGuid;
		Info.Data = 0;

		HANDLE hPlugin = pPlugin->Open(&Info);

		if (hPlugin)
		{
			PluginHandle handle;
			handle.hPlugin = hPlugin;
			handle.pPlugin = pPlugin;
			items.emplace_back(handle);
		}

		if (!items.empty() && !Global->Opt->PluginConfirm.SetFindList)
			break;
	}

	if (!items.empty())
	{
		if (items.size()>1)
		{
			VMenu2 menu(MSG(MPluginConfirmationTitle), nullptr, 0, ScrY-4);
			menu.SetPosition(-1, -1, 0, 0);
			menu.SetHelp(L"ChoosePluginMenu");
			menu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
			MenuItemEx mitem;

			std::for_each(CONST_RANGE(items, i)
			{
				mitem.Clear();
				mitem.strName = i.pPlugin->GetTitle();
				menu.AddItem(&mitem);
			});

			int ExitCode=menu.Run();

			if (ExitCode>=0)
			{
				pResult=items.begin();
				std::advance(pResult, ExitCode);
			}
		}
		else
		{
			pResult = items.begin();
		}
	}

	if (pResult != items.end())
	{
		SetFindListInfo Info = {sizeof(Info)};
		Info.hPanel = pResult->hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;

		if (!pResult->pPlugin->SetFindList(&Info))
		{
			pResult = items.end();
		}
	}

	FOR_CONST_RANGE(items, i)
	{
		if (i!=pResult)
		{
			if (i->hPlugin)
			{
				ClosePanelInfo Info = {sizeof(Info)};
				Info.hPanel = i->hPlugin;
				i->pPlugin->ClosePanel(&Info);
			}
		}
	}

	if (pResult != items.end())
	{
		PluginHandle* pDup=new PluginHandle;
		pDup->hPlugin=pResult->hPlugin;
		pDup->pPlugin=pResult->pPlugin;
		return pDup;
	}

	return nullptr;
}


void PluginManager::ClosePanel(HANDLE hPlugin)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ClosePanelInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	ph->pPlugin->ClosePanel(&Info);
	delete ph;
}


int PluginManager::ProcessEditorInput(INPUT_RECORD *Rec)
{
	ProcessEditorInputInfo Info={sizeof(Info)};
	Info.Rec=*Rec;

	return std::any_of(CONST_RANGE(SortedPlugins, i) {return i->HasProcessEditorInput() && i->ProcessEditorInput(&Info);});
}


int PluginManager::ProcessEditorEvent(int Event,void *Param,int EditorID)
{
	int nResult = 0;

	if (Global->CtrlObject->Plugins->GetCurEditor())
	{
		ProcessEditorEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.EditorID = EditorID;

		std::for_each(CONST_RANGE(SortedPlugins, i)
		{
			if (i->HasProcessEditorEvent())
				nResult = i->ProcessEditorEvent(&Info);
		});
	}

	return nResult;
}


int PluginManager::ProcessSubscribedEditorEvent(int Event,void *Param,int EditorID, const std::list<GUID> &PluginIds)
{
	int nResult = 0;

	if (Global->CtrlObject->Plugins->GetCurEditor())
	{
		ProcessEditorEventInfo Info = {sizeof(Info)};
		Info.Event = Event;
		Info.Param = Param;
		Info.EditorID = EditorID;

		std::for_each(CONST_RANGE(PluginIds, i)
		{
			auto Plugin = FindPlugin(i);
			if (Plugin && Plugin->HasProcessEditorEvent())
				nResult = Plugin->ProcessEditorEvent(&Info);
		});
	}

	return nResult;
}


int PluginManager::ProcessViewerEvent(int Event, void *Param,int ViewerID)
{
	int nResult = 0;

	ProcessViewerEventInfo Info = {sizeof(Info)};
	Info.Event = Event;
	Info.Param = Param;
	Info.ViewerID = ViewerID;

	std::for_each(CONST_RANGE(SortedPlugins, i)
	{
		if (i->HasProcessViewerEvent())
			nResult = i->ProcessViewerEvent(&Info);
	});
	return nResult;
}

int PluginManager::ProcessDialogEvent(int Event, FarDialogEvent *Param)
{
	ProcessDialogEventInfo Info = {sizeof(Info)};
	Info.Event = Event;
	Info.Param = Param;

	return std::any_of(CONST_RANGE(SortedPlugins, i) {return i->HasProcessDialogEvent() && i->ProcessDialogEvent(&Info);});
}

int PluginManager::ProcessConsoleInput(ProcessConsoleInputInfo *Info)
{
	int nResult = 0;

	FOR_CONST_RANGE(SortedPlugins, i)
	{
		Plugin *pPlugin = *i;

		if (pPlugin->HasProcessConsoleInput())
		{
			int n = pPlugin->ProcessConsoleInput(Info);
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


int PluginManager::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelData,
    size_t *pItemsNumber,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle *)hPlugin;
	GetFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.OpMode = OpMode;
	int Result = ph->pPlugin->GetFindData(&Info);
	*pPanelData = Info.PanelItem;
	*pItemsNumber = Info.ItemsNumber;
	return Result;
}


void PluginManager::FreeFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool FreeUserData
)
{
	PluginHandle *ph = (PluginHandle *)hPlugin;

	if (FreeUserData)
		FreePluginPanelItemsUserData(hPlugin,PanelItem,ItemsNumber);

	FreeFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	ph->pPlugin->FreeFindData(&Info);
}


int PluginManager::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelData,
    size_t *pItemsNumber,
    const string& Path
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	*pItemsNumber=0;

	GetVirtualFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.Path = Path.data();
	int Result = ph->pPlugin->GetVirtualFindData(&Info);
	*pPanelData = Info.PanelItem;
	*pItemsNumber = Info.ItemsNumber;
	return Result;
}


void PluginManager::FreeVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	FreeFindDataInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	return ph->pPlugin->FreeVirtualFindData(&Info);
}


int PluginManager::SetDirectory(
    HANDLE hPlugin,
    const string& Dir,
    int OpMode,
    struct UserDataItem *UserData
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SetDirectoryInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.Dir = Dir.data();
	Info.OpMode = OpMode;
	if (UserData)
	{
		Info.UserData.Data = UserData->Data;
		Info.UserData.FreeData = UserData->FreeData;
	}
	return ph->pPlugin->SetDirectory(&Info);
}


int PluginManager::GetFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    const string& DestPath,
    string &strResultName,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen *SaveScr=nullptr;
	int Found=FALSE;
	Global->KeepUserScreen=FALSE;

	if (!(OpMode & OPM_FIND))
		SaveScr = new SaveScreen; //???

	UndoGlobalSaveScrPtr UndSaveScr(SaveScr);

	GetFilesInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = 1;
	Info.Move = 0;
	Info.DestPath = DestPath.data();
	Info.OpMode = OpMode;

	int GetCode = ph->pPlugin->GetFiles(&Info);

	string strFindPath;
	strFindPath = Info.DestPath;
	AddEndSlash(strFindPath);
	strFindPath += L"*";
	api::FAR_FIND_DATA fdata;
	api::FindFile Find(strFindPath);
	bool Done = true;
	while(Find.Get(fdata))
	{
		if(!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			Done = false;
			break;
		}
	}

	if (!Done)
	{
		strResultName = Info.DestPath;
		AddEndSlash(strResultName);
		strResultName += fdata.strFileName;

		if (GetCode!=1)
		{
			api::SetFileAttributes(strResultName,FILE_ATTRIBUTE_NORMAL);
			api::DeleteFile(strResultName); //BUGBUG
		}
		else
			Found=TRUE;
	}

	ReadUserBackgound(SaveScr);
	delete SaveScr;
	return Found;
}


int PluginManager::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	DeleteFilesInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.OpMode = OpMode;

	int Code = ph->pPlugin->DeleteFiles(&Info);

	if (Code)
		ReadUserBackgound(&SaveScr); //???

	return Code;
}


int PluginManager::MakeDirectory(
    HANDLE hPlugin,
    const wchar_t **Name,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph = (PluginHandle*)hPlugin;
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	MakeDirectoryInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.Name = *Name;
	Info.OpMode = OpMode;

	int Code = ph->pPlugin->MakeDirectory(&Info);

	*Name = Info.Name;

	if (Code != -1)   //???BUGBUG
		ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::ProcessHostFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	ProcessHostFileInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.OpMode = OpMode;

	int Code = ph->pPlugin->ProcessHostFile(&Info);

	if (Code)   //BUGBUG
		ReadUserBackgound(&SaveScr);

	return Code;
}


int PluginManager::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	PluginHandle *ph=(PluginHandle*)hPlugin;

	GetFilesInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.Move = Move;
	Info.DestPath = *DestPath;
	Info.OpMode = OpMode;

	int Result = ph->pPlugin->GetFiles(&Info);
	*DestPath = Info.DestPath;
	return Result;
}


int PluginManager::PutFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    int OpMode
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	SaveScreen SaveScr;
	Global->KeepUserScreen=FALSE;

	static string strCurrentDirectory;
	api::GetCurrentDirectory(strCurrentDirectory);
	PutFilesInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.PanelItem = PanelItem;
	Info.ItemsNumber = ItemsNumber;
	Info.Move = Move;
	Info.SrcPath = strCurrentDirectory.data();
	Info.OpMode = OpMode;

	int Code = ph->pPlugin->PutFiles(&Info);

	if (Code)   //BUGBUG
		ReadUserBackgound(&SaveScr);

	return Code;
}

void PluginManager::GetOpenPanelInfo(
    HANDLE hPlugin,
    OpenPanelInfo *Info
)
{
	if (!Info)
		return;

	ClearStruct(*Info);
	PluginHandle *ph = (PluginHandle*)hPlugin;

	Info->StructSize = sizeof(OpenPanelInfo);
	Info->hPanel = ph->hPlugin;
	ph->pPlugin->GetOpenPanelInfo(Info);

	if (Info->CurDir && *Info->CurDir && (Info->Flags & OPIF_REALNAMES) && (Global->CtrlObject->Cp()->ActivePanel->GetPluginHandle() == hPlugin) && ParsePath(Info->CurDir)!=PATH_UNKNOWN)
		api::SetCurrentDirectory(Info->CurDir, false);
}


int PluginManager::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;

	ProcessPanelInputInfo Info={sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.Rec=*Rec;

#ifndef NO_WRAPPER
	if (Pred && ph->pPlugin->IsOemPlugin())
		Info.Rec.EventType |= 0x4000;
#endif
	return ph->pPlugin->ProcessPanelInput(&Info);
}


int PluginManager::ProcessEvent(
    HANDLE hPlugin,
    int Event,
    void *Param
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;

	ProcessPanelEventInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.Event = Event;
	Info.Param = Param;

	return ph->pPlugin->ProcessPanelEvent(&Info);
}


int PluginManager::Compare(
    HANDLE hPlugin,
    const PluginPanelItem *Item1,
    const PluginPanelItem *Item2,
    unsigned int Mode
)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;

	CompareInfo Info = {sizeof(Info)};
	Info.hPanel = ph->hPlugin;
	Info.Item1 = Item1;
	Info.Item2 = Item2;
	Info.Mode = static_cast<OPENPANELINFO_SORTMODES>(Mode);

	return ph->pPlugin->Compare(&Info);
}

void PluginManager::ConfigureCurrent(Plugin *pPlugin, const GUID& Guid)
{
	ConfigureInfo Info = {sizeof(Info)};
	Info.Guid = &Guid;

	if (pPlugin->Configure(&Info))
	{
		Panel* Panels[] =
		{
			Global->CtrlObject->Cp()->LeftPanel,
			Global->CtrlObject->Cp()->RightPanel,
		};

		std::for_each(CONST_RANGE(Panels, i)
		{
			if (i->GetMode() == PLUGIN_PANEL)
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

/* $ 29.05.2001 IS
   ! При настройке "параметров внешних модулей" закрывать окно с их
     списком только при нажатии на ESC
*/
void PluginManager::Configure(int StartPos)
{
	// Полиция 4 - Параметры внешних модулей
	if (Global->Opt->Policies.DisabledOptions&FFPOL_MAINMENUPLUGINS)
		return;

	FARMACROAREA PrevMacroMode = Global->CtrlObject->Macro.GetMode();
	Global->CtrlObject->Macro.SetMode(MACROAREA_MENU);

	{
		VMenu2 PluginList(MSG(MPluginConfigTitle),nullptr,0,ScrY-4);
		PluginList.SetFlags(VMENU_WRAPMODE);
		PluginList.SetHelp(L"PluginsConfig");

		while (!Global->CloseFAR)
		{
			bool NeedUpdateItems = true;
			bool HotKeysPresent = Global->Db->PlHotkeyCfg()->HotkeysPresent(PluginsHotkeysConfig::CONFIG_MENU);

			if (NeedUpdateItems)
			{
				PluginList.DeleteItems();
				LoadIfCacheAbsent();
				string strHotKey, strName;
				GUID guid;

				FOR_CONST_RANGE(SortedPlugins, i)
				{
					Plugin *pPlugin = *i;
					bool bCached = pPlugin->CheckWorkFlags(PIWF_CACHED);
					unsigned __int64 id = 0;

					PluginInfo Info = {sizeof(Info)};
					if (bCached)
					{
						id = Global->Db->PlCacheCfg()->GetCacheID(pPlugin->GetCacheName());
					}
					else
					{
						if (!pPlugin->GetPluginInfo(&Info))
							continue;
					}

					for (size_t J=0; ; J++)
					{
						if (bCached)
						{
							string strGuid;

							if (!Global->Db->PlCacheCfg()->GetPluginsConfigMenuItem(id, J, strName, strGuid))
								break;
							if (!StrToGuid(strGuid,guid))
								break;
						}
						else
						{
							if (J >= Info.PluginConfig.Count)
								break;

							strName = NullToEmpty(Info.PluginConfig.Strings[J]);
							guid = Info.PluginConfig.Guids[J];
						}

						GetPluginHotKey(pPlugin,guid,PluginsHotkeysConfig::CONFIG_MENU,strHotKey);
						MenuItemEx ListItem;
						ListItem.Clear();

#ifndef NO_WRAPPER
						if (pPlugin->IsOemPlugin())
							ListItem.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
						if (!HotKeysPresent)
							ListItem.strName = strName;
						else if (!strHotKey.empty())
							ListItem.strName = str_printf(L"&%c%s  %s", static_cast<wchar_t>(strHotKey.front()),(strHotKey.front()==L'&'?L"&":L""), strName.data());
						else
							ListItem.strName = str_printf(L"   %s", strName.data());

						PluginMenuItemData item;
						item.pPlugin = pPlugin;
						item.Guid = guid;
						PluginList.SetUserData(&item, sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
					}
				}

				PluginList.AssignHighlights(FALSE);
				PluginList.SetBottomTitle(MSG(MPluginHotKeyBottom));
				PluginList.SortItems(false, HotKeysPresent? 3 : 0);
				PluginList.SetSelectPos(StartPos,1);
				NeedUpdateItems = false;
			}

			string strPluginModuleName;

			PluginList.Run([&](int Key)->int
			{
				Global->CtrlObject->Macro.SetMode(MACROAREA_MENU);
				int SelPos=PluginList.GetSelectPos();
				PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(nullptr,0,SelPos);
				int KeyProcessed = 1;

				switch (Key)
				{
					case KEY_SHIFTF1:
						if (item)
						{
							strPluginModuleName = item->pPlugin->GetModuleName();
							if (!pluginapi::apiShowHelp(strPluginModuleName.data(),L"Config",FHELP_SELFHELP|FHELP_NOSHOWERROR) &&
							        !pluginapi::apiShowHelp(strPluginModuleName.data(),L"Configure",FHELP_SELFHELP|FHELP_NOSHOWERROR))
							{
								pluginapi::apiShowHelp(strPluginModuleName.data(),nullptr,FHELP_SELFHELP|FHELP_NOSHOWERROR);
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
							string strTitle;
							int nOffset = HotKeysPresent?3:0;
							strTitle = PluginList.GetItemPtr()->strName.data()+nOffset;
							RemoveExternalSpaces(strTitle);

							if (SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::CONFIG_MENU, strTitle))
							{
								NeedUpdateItems = true;
								StartPos = SelPos;
								PluginList.Close(SelPos);
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
				StartPos=PluginList.GetExitCode();

				if (StartPos<0)
					break;

				PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(nullptr,0,StartPos);
				ConfigureCurrent(item->pPlugin, item->Guid);
			}
		}
	}

	Global->CtrlObject->Macro.SetMode(PrevMacroMode);
}

int PluginManager::CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName)
{
	if (ModalType == MODALTYPE_DIALOG || ModalType == MODALTYPE_VMENU)
	{
		Dialog *dlg=static_cast<Dialog*>(FrameManager->GetCurrentFrame());
		if (dlg->CheckDialogMode(DMODE_NOPLUGINS) || dlg->GetId()==PluginsMenuId)
		{
			return 0;
		}
	}

	FARMACROAREA PrevMacroMode = Global->CtrlObject->Macro.GetMode();
	Global->CtrlObject->Macro.SetMode(MACROAREA_MENU);

	bool Editor = ModalType==MODALTYPE_EDITOR;
	bool Viewer = ModalType==MODALTYPE_VIEWER;
	bool Dialog = ModalType==MODALTYPE_DIALOG || ModalType==MODALTYPE_VMENU;

	PluginMenuItemData item;

	{
		VMenu2 PluginList(MSG(MPluginCommandsMenuTitle),nullptr,0,ScrY-4);
		PluginList.SetFlags(VMENU_WRAPMODE);
		PluginList.SetHelp(L"PluginCommands");
		PluginList.SetId(PluginsMenuId);
		bool NeedUpdateItems = true;

		while (NeedUpdateItems)
		{
			bool HotKeysPresent = Global->Db->PlHotkeyCfg()->HotkeysPresent(PluginsHotkeysConfig::PLUGINS_MENU);

			if (NeedUpdateItems)
			{
				PluginList.DeleteItems();
				LoadIfCacheAbsent();
				string strHotKey, strName;
				GUID guid;

				FOR_CONST_RANGE(SortedPlugins, i)
				{
					Plugin *pPlugin = *i;
					bool bCached = pPlugin->CheckWorkFlags(PIWF_CACHED);
					UINT64 IFlags;
					unsigned __int64 id = 0;

					PluginInfo Info = {sizeof(Info)};
					if (bCached)
					{
						id = Global->Db->PlCacheCfg()->GetCacheID(pPlugin->GetCacheName());
						IFlags = Global->Db->PlCacheCfg()->GetFlags(id);
					}
					else
					{
						if (!pPlugin->GetPluginInfo(&Info))
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
							string strGuid;

							if (!Global->Db->PlCacheCfg()->GetPluginsMenuItem(id, J, strName, strGuid))
								break;
							if (!StrToGuid(strGuid,guid))
								break;
						}
						else
						{
							if (J >= Info.PluginMenu.Count)
								break;

							strName = NullToEmpty(Info.PluginMenu.Strings[J]);
							guid = Info.PluginMenu.Guids[J];
						}

						GetPluginHotKey(pPlugin,guid,PluginsHotkeysConfig::PLUGINS_MENU,strHotKey);
						MenuItemEx ListItem;
						ListItem.Clear();
#ifndef NO_WRAPPER
						if (pPlugin->IsOemPlugin())
							ListItem.Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
						if (!HotKeysPresent)
							ListItem.strName = strName;
						else if (!strHotKey.empty())
							ListItem.strName = str_printf(L"&%c%s  %s", static_cast<wchar_t>(strHotKey.front()),(strHotKey.front()==L'&'?L"&":L""), strName.data());
						else
							ListItem.strName = str_printf(L"   %s", strName.data());

						PluginMenuItemData item;
						item.pPlugin = pPlugin;
						item.Guid = guid;
						PluginList.SetUserData(&item, sizeof(PluginMenuItemData),PluginList.AddItem(&ListItem));
					}
				}

				PluginList.AssignHighlights(FALSE);
				PluginList.SetBottomTitle(MSG(MPluginHotKeyBottom));
				PluginList.SortItems(false, HotKeysPresent? 3 : 0);
				PluginList.SetSelectPos(StartPos,1);
				NeedUpdateItems = false;
			}

			PluginList.Run([&](int Key)->int
			{
				Global->CtrlObject->Macro.SetMode(MACROAREA_MENU);
				int SelPos=PluginList.GetSelectPos();
				PluginMenuItemData *item = (PluginMenuItemData*)PluginList.GetUserData(nullptr,0,SelPos);
				int KeyProcessed = 1;

				switch (Key)
				{
					case KEY_SHIFTF1:
						// Вызываем нужный топик, который передали в CommandsMenu()
						if (item)
							pluginapi::apiShowHelp(item->pPlugin->GetModuleName().data(),HistoryName,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
						break;

					case KEY_ALTF11:
					case KEY_RALTF11:
						WriteEvent(FLOG_PLUGINSINFO);
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
							string strTitle;
							int nOffset = HotKeysPresent?3:0;
							strTitle = PluginList.GetItemPtr()->strName.data()+nOffset;
							RemoveExternalSpaces(strTitle);

							if (SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::PLUGINS_MENU, strTitle))
							{
								NeedUpdateItems = true;
								StartPos = SelPos;
								PluginList.Close(SelPos);
							}
						}
						break;

					case KEY_ALTSHIFTF9:
					case KEY_RALTSHIFTF9:
					{
						if (item)
						{
							NeedUpdateItems = true;
							StartPos = SelPos;
							Configure();
							PluginList.Close(SelPos);
						}
						break;
					}

					case KEY_SHIFTF9:
					{
						if (item)
						{
							NeedUpdateItems = true;
							StartPos=SelPos;

							if (item->pPlugin->HasConfigure())
								ConfigureCurrent(item->pPlugin, item->Guid);

							PluginList.Close(SelPos);
						}

						break;
					}

					default:
						KeyProcessed = 0;
				}
				return KeyProcessed;
			});
		}

		int ExitCode=PluginList.GetExitCode();

		if (ExitCode<0)
		{
			Global->CtrlObject->Macro.SetMode(PrevMacroMode);
			return FALSE;
		}

		Global->ScrBuf->Flush();
		item = *(PluginMenuItemData*)PluginList.GetUserData(nullptr,0,ExitCode);
	}

	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
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
		pd.hDlg=(HANDLE)FrameManager->GetCurrentFrame();
		Item=(intptr_t)&pd;
	}

	HANDLE hPlugin=Open(item.pPlugin,OpenCode,item.Guid,Item);

	if (hPlugin && !Editor && !Viewer && !Dialog)
	{
		if (ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		{
			ClosePanel(hPlugin);
			return FALSE;
		}

		Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
		NewPanel->SetPluginMode(hPlugin,L"",true);
		NewPanel->Update(0);
		NewPanel->Show();
	}

	// restore title for old plugins only.
#ifndef NO_WRAPPER
	if (item.pPlugin->IsOemPlugin() && Editor && m_CurEditor)
	{
		m_CurEditor->SetPluginTitle(nullptr);
	}
#endif // NO_WRAPPER
	Global->CtrlObject->Macro.SetMode(PrevMacroMode);
	return TRUE;
}

void PluginManager::GetHotKeyPluginKey(Plugin *pPlugin, string &strPluginKey)
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
	strPluginKey = pPlugin->GetHotkeyName();
#ifndef NO_WRAPPER
	size_t FarPathLength=Global->g_strFarPath.size();
	if (pPlugin->IsOemPlugin() && FarPathLength < pPlugin->GetModuleName().size() && !StrCmpNI(pPlugin->GetModuleName().data(), Global->g_strFarPath.data(), (int)FarPathLength))
		strPluginKey.erase(0, FarPathLength);
#endif // NO_WRAPPER
}

void PluginManager::GetPluginHotKey(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, string &strHotKey)
{
	string strPluginKey;
	strHotKey.clear();
	GetHotKeyPluginKey(pPlugin, strPluginKey);
	strHotKey = Global->Db->PlHotkeyCfg()->GetHotkey(strPluginKey, GuidToStr(Guid), HotKeyType);
}

bool PluginManager::SetHotKeyDialog(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, const string& DlgPluginTitle)
{
	string strPluginKey;
	GetHotKeyPluginKey(pPlugin, strPluginKey);
	string strGuid = GuidToStr(Guid);
	string strHotKey = Global->Db->PlHotkeyCfg()->GetHotkey(strPluginKey, strGuid, HotKeyType);

	DialogBuilder Builder(MPluginHotKeyTitle, L"SetHotKeyDialog");
	Builder.AddText(MPluginHotKey);
	Builder.AddTextAfter(Builder.AddFixEditField(&strHotKey, 1), DlgPluginTitle.data());
	Builder.AddOKCancel();
	if(Builder.ShowDialog())
	{
		if (!strHotKey.empty() && strHotKey.front() != L' ')
			Global->Db->PlHotkeyCfg()->SetHotkey(strPluginKey, strGuid, HotKeyType, strHotKey);
		else
			Global->Db->PlHotkeyCfg()->DelHotkey(strPluginKey, strGuid, HotKeyType);
		return true;
	}
	return false;
}

void PluginManager::ShowPluginInfo(Plugin *pPlugin, const GUID& Guid)
{
	string strPluginGuid = GuidToStr(pPlugin->GetGUID());
	string strItemGuid = GuidToStr(Guid);
	string strPluginPrefix;
	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		unsigned __int64 id = Global->Db->PlCacheCfg()->GetCacheID(pPlugin->GetCacheName());
		strPluginPrefix = Global->Db->PlCacheCfg()->GetCommandPrefix(id);
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};
		if (pPlugin->GetPluginInfo(&Info))
		{
			strPluginPrefix = NullToEmpty(Info.CommandPrefix);
		}
	}
	const int Width = 36;
	DialogBuilder Builder(MPluginInformation, L"ShowPluginInfo");
	Builder.AddText(MPluginModuleTitle);
	Builder.AddConstEditField(pPlugin->GetTitle(), Width);
	Builder.AddText(MPluginDescription);
	Builder.AddConstEditField(pPlugin->GetDescription(), Width);
	Builder.AddText(MPluginAuthor);
	Builder.AddConstEditField(pPlugin->GetAuthor(), Width);
	Builder.AddText(MPluginVersion);
	Builder.AddConstEditField(pPlugin->GetVersionString(), Width);
	Builder.AddText(MPluginModulePath);
	Builder.AddConstEditField(pPlugin->GetModuleName(), Width);
	Builder.AddText(MPluginGUID);
	Builder.AddConstEditField(strPluginGuid, Width);
	Builder.AddText(MPluginItemGUID);
	Builder.AddConstEditField(strItemGuid, Width);
	Builder.AddText(MPluginPrefix);
	Builder.AddConstEditField(strPluginPrefix, Width);
	Builder.AddOK();
	Builder.ShowDialog();
}

char* BufReserve(char*& Buf, size_t Count, size_t& Rest, size_t& Size)
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


wchar_t* StrToBuf(const string& Str, char*& Buf, size_t& Rest, size_t& Size)
{
	size_t Count = (Str.size() + 1) * sizeof(wchar_t);
	wchar_t* Res = reinterpret_cast<wchar_t*>(BufReserve(Buf, Count, Rest, Size));
	if (Res)
	{
		wcscpy(Res, Str.data());
	}
	return Res;
}


void ItemsToBuf(PluginMenuItem& Menu, const std::vector<string>& NamesArray, const std::vector<string>& GuidsArray, char*& Buf, size_t& Rest, size_t& Size)
{
	Menu.Count = NamesArray.size();
	Menu.Strings = nullptr;
	Menu.Guids = nullptr;

	if (Menu.Count)
	{
		wchar_t** Items = reinterpret_cast<wchar_t**>(BufReserve(Buf, Menu.Count * sizeof(wchar_t*), Rest, Size));
		GUID* Guids = reinterpret_cast<GUID*>(BufReserve(Buf, Menu.Count * sizeof(GUID), Rest, Size));
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
				GUID Guid;
				if (StrToGuid(GuidsArray[i], Guid))
				{
					Guids[i] = Guid;
				}
			}
		}
	}
}

size_t PluginManager::GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize)
{
	if(IsPluginUnloaded(pPlugin)) return 0;
	string Prefix;
	PLUGIN_FLAGS Flags = 0;
	std::vector<string> MenuNames, MenuGuids, DiskNames, DiskGuids, ConfNames, ConfGuids;

	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		unsigned __int64 id = Global->Db->PlCacheCfg()->GetCacheID(pPlugin->GetCacheName());
		Flags = Global->Db->PlCacheCfg()->GetFlags(id);
		Prefix = Global->Db->PlCacheCfg()->GetCommandPrefix(id);

		string Name, Guid;

		for(int i = 0; Global->Db->PlCacheCfg()->GetPluginsMenuItem(id, i, Name, Guid); ++i)
		{
			MenuNames.emplace_back(Name);
			MenuGuids.emplace_back(Guid);
		}

		for(int i = 0; Global->Db->PlCacheCfg()->GetDiskMenuItem(id, i, Name, Guid); ++i)
		{
			DiskNames.emplace_back(Name);
			DiskGuids.emplace_back(Guid);
		}

		for(int i = 0; Global->Db->PlCacheCfg()->GetPluginsConfigMenuItem(id, i, Name, Guid); ++i)
		{
			ConfNames.emplace_back(Name);
			ConfGuids.emplace_back(Guid);
		}
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};
		if (pPlugin->GetPluginInfo(&Info))
		{
			Flags = Info.Flags;
			Prefix = NullToEmpty(Info.CommandPrefix);

			for (size_t i = 0; i < Info.PluginMenu.Count; i++)
			{
					MenuNames.emplace_back(Info.PluginMenu.Strings[i]);
					MenuGuids.emplace_back(GuidToStr(Info.PluginMenu.Guids[i]));
			}

			for (size_t i = 0; i < Info.DiskMenu.Count; i++)
			{
				DiskNames.emplace_back(Info.DiskMenu.Strings[i]);
				DiskGuids.emplace_back(GuidToStr(Info.DiskMenu.Guids[i]));
			}

			for (size_t i = 0; i < Info.PluginConfig.Count; i++)
			{
				ConfNames.emplace_back(Info.PluginConfig.Strings[i]);
				ConfGuids.emplace_back(GuidToStr(Info.PluginConfig.Guids[i]));
			}
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
	pInfo->ModuleName = StrToBuf(pPlugin->GetModuleName(), Buffer, Rest, Size);

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
	pInfo->GInfo->Guid = pPlugin->GetGUID();
	pInfo->GInfo->Version = pPlugin->GetVersion();
	pInfo->GInfo->MinFarVersion = pPlugin->GetMinFarVersion();
	pInfo->GInfo->Title = StrToBuf(pPlugin->strTitle, Buffer, Rest, Size);
	pInfo->GInfo->Description = StrToBuf(pPlugin->strDescription, Buffer, Rest, Size);
	pInfo->GInfo->Author = StrToBuf(pPlugin->strAuthor, Buffer, Rest, Size);

	pInfo->PInfo->StructSize = sizeof(PluginInfo);
	pInfo->PInfo->Flags = Flags;
	pInfo->PInfo->CommandPrefix = StrToBuf(Prefix, Buffer, Rest, Size);

	ItemsToBuf(pInfo->PInfo->DiskMenu, DiskNames, DiskGuids, Buffer, Rest, Size);
	ItemsToBuf(pInfo->PInfo->PluginMenu, MenuNames, MenuGuids, Buffer, Rest, Size);
	ItemsToBuf(pInfo->PInfo->PluginConfig, ConfNames, ConfGuids, Buffer, Rest, Size);

	return Size;
}

bool PluginManager::GetDiskMenuItem(
     Plugin *pPlugin,
     size_t PluginItem,
     bool &ItemPresent,
     wchar_t& PluginHotkey,
     string &strPluginText,
     GUID &Guid
)
{
	LoadIfCacheAbsent();

	ItemPresent = false;

	if (pPlugin->CheckWorkFlags(PIWF_CACHED))
	{
		string strGuid;
		if (Global->Db->PlCacheCfg()->GetDiskMenuItem(Global->Db->PlCacheCfg()->GetCacheID(pPlugin->GetCacheName()), PluginItem, strPluginText, strGuid))
			if (StrToGuid(strGuid,Guid))
				ItemPresent = true;
		ItemPresent = ItemPresent && !strPluginText.empty();
	}
	else
	{
		PluginInfo Info = {sizeof(Info)};

		if (!pPlugin->GetPluginInfo(&Info) || Info.DiskMenu.Count <= PluginItem)
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
		string strHotKey;
		GetPluginHotKey(pPlugin,Guid,PluginsHotkeysConfig::DRIVE_MENU,strHotKey);
		PluginHotkey = strHotKey.empty() ? 0 : strHotKey[0];
	}

	return true;
}

int PluginManager::UseFarCommand(HANDLE hPlugin,int CommandType)
{
	OpenPanelInfo Info;
	GetOpenPanelInfo(hPlugin,&Info);

	if (!(Info.Flags & OPIF_REALNAMES))
		return FALSE;

	PluginHandle *ph = (PluginHandle*)hPlugin;

	switch (CommandType)
	{
		case PLUGIN_FARGETFILE:
		case PLUGIN_FARGETFILES:
			return(!ph->pPlugin->HasGetFiles() || (Info.Flags & OPIF_EXTERNALGET));
		case PLUGIN_FARPUTFILES:
			return(!ph->pPlugin->HasPutFiles() || (Info.Flags & OPIF_EXTERNALPUT));
		case PLUGIN_FARDELETEFILES:
			return(!ph->pPlugin->HasDeleteFiles() || (Info.Flags & OPIF_EXTERNALDELETE));
		case PLUGIN_FARMAKEDIRECTORY:
			return(!ph->pPlugin->HasMakeDirectory() || (Info.Flags & OPIF_EXTERNALMKDIR));
	}

	return TRUE;
}


void PluginManager::ReloadLanguage()
{
	std::for_each(ALL_CONST_RANGE(SortedPlugins), std::mem_fn(&Plugin::CloseLang));
	DiscardCache();
}


void PluginManager::DiscardCache()
{
	std::for_each(ALL_CONST_RANGE(SortedPlugins), std::mem_fn(&Plugin::Load));
	Global->Db->PlCacheCfg()->DiscardCache();
}


void PluginManager::LoadIfCacheAbsent()
{
	if (Global->Db->PlCacheCfg()->IsCacheEmpty())
	{
		std::for_each(ALL_CONST_RANGE(SortedPlugins), std::mem_fn(&Plugin::Load));
	}
}

//template parameters must have external linkage
struct PluginData
{
	Plugin *pPlugin;
	UINT64 PluginFlags;
};

int PluginManager::ProcessCommandLine(const string& CommandParam,Panel *Target)
{
	size_t PrefixLength=0;
	string strCommand=CommandParam;
	UnquoteExternal(strCommand);
	RemoveLeadingSpaces(strCommand);

	if (!IsPluginPrefixPath(strCommand))
		return FALSE;

	LoadIfCacheAbsent();
	string strPrefix = strCommand.substr(0, strCommand.find(L':'));
	string strPluginPrefix;
	std::list<PluginData> items;

	FOR_CONST_RANGE(SortedPlugins, i)
	{
		UINT64 PluginFlags=0;

		if ((*i)->CheckWorkFlags(PIWF_CACHED))
		{
			unsigned __int64 id = Global->Db->PlCacheCfg()->GetCacheID((*i)->GetCacheName());
			strPluginPrefix = Global->Db->PlCacheCfg()->GetCommandPrefix(id);
			PluginFlags = Global->Db->PlCacheCfg()->GetFlags(id);
		}
		else
		{
			PluginInfo Info = {sizeof(Info)};

			if ((*i)->GetPluginInfo(&Info))
			{
				strPluginPrefix = NullToEmpty(Info.CommandPrefix);
				PluginFlags = Info.Flags;
			}
			else
				continue;
		}

		if (strPluginPrefix.empty())
			continue;

		const wchar_t *PrStart = strPluginPrefix.data();
		PrefixLength=strPrefix.size();

		for (;;)
		{
			const wchar_t *PrEnd = wcschr(PrStart, L':');
			size_t Len=PrEnd ? (PrEnd-PrStart):StrLength(PrStart);

			if (Len<PrefixLength)Len=PrefixLength;

			if (!StrCmpNI(strPrefix.data(), PrStart, (int)Len))
			{
				if ((*i)->Load() && (*i)->HasOpen())
				{
					PluginData pD;
					pD.pPlugin=*i;
					pD.PluginFlags=PluginFlags;
					items.emplace_back(pD);
					break;
				}
			}

			if (!PrEnd)
				break;

			PrStart = ++PrEnd;
		}

		if (!items.empty() && !Global->Opt->PluginConfirm.Prefix)
			break;
	}

	if (items.empty())
		return FALSE;

	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
	Panel *CurPanel=(Target)?Target:ActivePanel;

	if (CurPanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		return FALSE;

	auto PData = items.begin();

	if (items.size()>1)
	{
		VMenu2 menu(MSG(MPluginConfirmationTitle), nullptr, 0, ScrY-4);
		menu.SetPosition(-1, -1, 0, 0);
		menu.SetHelp(L"ChoosePluginMenu");
		menu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
		MenuItemEx mitem;

		std::for_each(CONST_RANGE(items, i)
		{
			mitem.Clear();
			mitem.strName=PointToName(i.pPlugin->GetModuleName());
			menu.AddItem(&mitem);
		});

		int ExitCode=menu.Run();

		if (ExitCode>=0)
		{
			std::advance(PData, ExitCode);
		}
	}

	Global->CtrlObject->CmdLine->SetString(L"");
	string strPluginCommand=strCommand.data()+(PData->PluginFlags & PF_FULLCMDLINE ? 0:PrefixLength+1);
	RemoveTrailingSpaces(strPluginCommand);
	OpenCommandLineInfo info={sizeof(OpenCommandLineInfo),strPluginCommand.data()}; //BUGBUG
	HANDLE hPlugin=Open(PData->pPlugin,OPEN_COMMANDLINE,FarGuid,(intptr_t)&info);

	if (hPlugin)
	{
		Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(CurPanel,FILE_PANEL,TRUE,TRUE);
		NewPanel->SetPluginMode(hPlugin,L"",!Target || Target == ActivePanel);
		NewPanel->Update(0);
		NewPanel->Show();
	}

	return TRUE;
}


void PluginManager::ReadUserBackgound(SaveScreen *SaveScr)
{
	FilePanels *FPanel=Global->CtrlObject->Cp();
	FPanel->LeftPanel->ProcessingPluginCommand++;
	FPanel->RightPanel->ProcessingPluginCommand++;

	if (Global->KeepUserScreen)
	{
		if (SaveScr)
			SaveScr->Discard();

		RedrawDesktop Redraw;
	}

	FPanel->LeftPanel->ProcessingPluginCommand--;
	FPanel->RightPanel->ProcessingPluginCommand--;
}


/* $ 27.09.2000 SVS
  Функция CallPlugin - найти плагин по ID и запустить
  в зачаточном состоянии!
*/
int PluginManager::CallPlugin(const GUID& SysID,int OpenFrom, void *Data,void **Ret)
{
	if (FrameManager->GetCurrentFrame() && FrameManager->GetCurrentFrame()->GetType() == MODALTYPE_DIALOG)
	{
		if (static_cast<Dialog*>(FrameManager->GetCurrentFrame())->CheckDialogMode(DMODE_NOPLUGINS))
		{
			return FALSE;
		}
	}

	Plugin *pPlugin = FindPlugin(SysID);

	if (pPlugin)
	{
		if (pPlugin->HasOpen() && !Global->ProcessException)
		{
			HANDLE hNewPlugin=Open(pPlugin,OpenFrom,FarGuid,(intptr_t)Data);
			bool process=false;

			if (OpenFrom == OPEN_FROMMACRO)
			{
				// <????>
				;
				// </????>
			}
			else
			{
				process=OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_FILEPANEL;
			}

			if (hNewPlugin && process)
			{
				int CurFocus=Global->CtrlObject->Cp()->ActivePanel->GetFocus();
				Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(Global->CtrlObject->Cp()->ActivePanel,FILE_PANEL,TRUE,TRUE);
				NewPanel->SetPluginMode(hNewPlugin,L"",CurFocus || !Global->CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());
				if (Data && *(const wchar_t *)Data)
				{
					struct UserDataItem UserData={0};  // !!! NEED CHECK !!!
					SetDirectory(hNewPlugin,(const wchar_t *)Data,0,&UserData);
				}
				// $ 04.04.2001 SVS
				//	Код закомментирован! Попытка исключить ненужные вызовы в CallPlugin()
				//	Если что-то не так - раскомментировать!!!

				//NewPanel->Update(0);
				//NewPanel->Show();
			}

			if (Ret)
			{
				PluginHandle *handle=(PluginHandle *)hNewPlugin;
				*Ret = hNewPlugin?handle->hPlugin:nullptr;
				delete handle;
			}

			return TRUE;
		}
	}
	return FALSE;
}

// поддержка макрофункций plugin.call, plugin.cmd, plugin.config и т.п
int PluginManager::CallPluginItem(const GUID& Guid, CallPluginInfo *Data)
{
	BOOL Result=FALSE;

	if (!Global->ProcessException)
	{
		int curType = FrameManager->GetCurrentFrame()->GetType();

		if (curType==MODALTYPE_DIALOG)
		{
			if (static_cast<Dialog*>(FrameManager->GetCurrentFrame())->CheckDialogMode(DMODE_NOPLUGINS))
			{
				return FALSE;
			}
		}

		bool Editor = curType==MODALTYPE_EDITOR;
		bool Viewer = curType==MODALTYPE_VIEWER;
		bool Dialog = curType==MODALTYPE_DIALOG;

		if (Data->CallFlags & CPT_CHECKONLY)
		{
			Data->pPlugin = FindPlugin(Guid);
			if (Data->pPlugin && Data->pPlugin->Load())
			{
				// Разрешен ли вызов данного типа в текущей области (предварительная проверка)
				switch ((Data->CallFlags & CPT_MASK))
				{
					case CPT_MENU:
						if (!Data->pPlugin->HasOpen())
							return FALSE;
						break;
					case CPT_CONFIGURE:
						if (curType!=MODALTYPE_PANELS)
						{
							//TODO: Автокомплит не влияет?
							return FALSE;
						}
						if (!Data->pPlugin->HasConfigure())
							return FALSE;
						break;
					case CPT_CMDLINE:
						if (curType!=MODALTYPE_PANELS)
						{
							//TODO: Автокомплит не влияет?
							return FALSE;
						}
						//TODO: OpenPanel или OpenFilePlugin?
						if (!Data->pPlugin->HasOpen())
							return FALSE;
						break;
					case CPT_INTERNAL:
						//TODO: Уточнить функцию
						if (!Data->pPlugin->HasOpen())
							return FALSE;
						break;
				}

				UINT64 IFlags;
				PluginInfo Info = {sizeof(Info)};
				if (!Data->pPlugin->GetPluginInfo(&Info))
					return FALSE;
				else
					IFlags = Info.Flags;

				PluginMenuItem *MenuItems=nullptr;

				// Разрешен ли вызов данного типа в текущей области
				switch ((Data->CallFlags & CPT_MASK))
				{
					case CPT_MENU:
						if ((Editor && !(IFlags & PF_EDITOR)) ||
								(Viewer && !(IFlags & PF_VIEWER)) ||
								(Dialog && !(IFlags & PF_DIALOG)) ||
								(!Editor && !Viewer && !Dialog && (IFlags & PF_DISABLEPANELS)))
							return FALSE;
						MenuItems = &Info.PluginMenu;
						break;
					case CPT_CONFIGURE:
						MenuItems = &Info.PluginConfig;
						break;
					case CPT_CMDLINE:
						if (!Info.CommandPrefix || !*Info.CommandPrefix)
							return FALSE;
						break;
					case CPT_INTERNAL:
						break;
				}

				if ((Data->CallFlags & CPT_MASK)==CPT_MENU || (Data->CallFlags & CPT_MASK)==CPT_CONFIGURE)
				{
					bool ItemFound = false;
					if (Data->ItemGuid==nullptr)
					{
						if (MenuItems->Count==1)
						{
							Data->FoundGuid=MenuItems->Guids[0];
							Data->ItemGuid=&Data->FoundGuid;
							ItemFound=true;
						}
					}
					else
					{
						for (size_t i = 0; i < MenuItems->Count; i++)
						{
							if (memcmp(Data->ItemGuid, &(MenuItems->Guids[i]), sizeof(GUID)) == 0)
							{
								Data->FoundGuid=*Data->ItemGuid;
								Data->ItemGuid=&Data->FoundGuid;
								ItemFound=true;
								break;
							}
						}
					}
					if (!ItemFound)
						return FALSE;
				}

				Result=TRUE;
			}
		}
		else
		{
			if (!Data->pPlugin)
				return FALSE;

			HANDLE hPlugin=nullptr;
			Panel *ActivePanel=nullptr;

			switch ((Data->CallFlags & CPT_MASK))
			{
				case CPT_MENU:
				{
					ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
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
						pd.hDlg=(HANDLE)FrameManager->GetCurrentFrame();
						Item=(intptr_t)&pd;
					}

					hPlugin=Open(Data->pPlugin,OpenCode,Data->FoundGuid,Item);

					Result=TRUE;
					break;
				}

				case CPT_CONFIGURE:
					Global->CtrlObject->Plugins->ConfigureCurrent(Data->pPlugin,Data->FoundGuid);
					return TRUE;

				case CPT_CMDLINE:
				{
					ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
					string command=Data->Command; // Нужна копия строки
					OpenCommandLineInfo info={sizeof(OpenCommandLineInfo),command.data()};
					hPlugin=Open(Data->pPlugin,OPEN_COMMANDLINE,FarGuid,(intptr_t)&info);

					Result=TRUE;
					break;
				}
				case CPT_INTERNAL:
					//TODO: бывший CallPlugin
					//WARNING: учесть, что он срабатывает без переключения MacroState
					break;
			}

			if (hPlugin && !Editor && !Viewer && !Dialog)
			{
				//BUGBUG: Закрытие панели? Нужно ли оно?
				//BUGBUG: В ProcessCommandLine зовется перед Open, а в CPT_MENU - после
				if (ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
				{
					ClosePanel(hPlugin);
					return FALSE;
				}

				Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
				NewPanel->SetPluginMode(hPlugin,L"",true);
				NewPanel->Update(0);
				NewPanel->Show();
			}

			// restore title for old plugins only.
			#ifndef NO_WRAPPER
			if (Data->pPlugin->IsOemPlugin() && Editor && m_CurEditor)
			{
				m_CurEditor->SetPluginTitle(nullptr);
			}
			#endif // NO_WRAPPER
		}
	}

	return Result;
}

Plugin *PluginManager::FindPlugin(const GUID& SysID) const
{
	auto Iterator = Plugins.find(SysID);
	return Iterator == Plugins.cend()? nullptr : Iterator->second.get();
}

HANDLE PluginManager::Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item)
{
	OpenInfo Info = {sizeof(Info)};
	Info.OpenFrom = static_cast<OPENFROM>(OpenFrom);
	Info.Guid = &Guid;
	Info.Data = Item;

	HANDLE hPlugin = pPlugin->Open(&Info);
	if (hPlugin)
	{
		PluginHandle *handle = new PluginHandle;
		handle->hPlugin = hPlugin;
		handle->pPlugin = pPlugin;
		return handle;
	}

	return hPlugin;
}

string PluginManager::GetCustomData(const string& Name) const
{
	const NTPath FilePath(Name);

	string strCustomData;

	std::for_each(CONST_RANGE(SortedPlugins, i)
	{
		wchar_t *CustomData = nullptr;

		if (i->HasGetCustomData() && i->GetCustomData(FilePath.data(), &CustomData))
		{
			if (!strCustomData.empty())
				strCustomData += L" ";
			strCustomData += CustomData;

			if (i->HasFreeCustomData())
				i->FreeCustomData(CustomData);
		}
	});
	return strCustomData;
}

const GUID& PluginManager::GetGUID(HANDLE hPlugin)
{
	PluginHandle *ph = (PluginHandle*)hPlugin;
	return ph->pPlugin->GetGUID();
}

void PluginManager::RefreshPluginsList()
{
	if(!UnloadedPlugins.empty())
	{
		UnloadedPlugins.remove_if([&](const T_CONST_VALUE_TYPE(UnloadedPlugins)& i) -> bool
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
	auto i = std::find(UnloadedPlugins.begin(), UnloadedPlugins.end(), plugin);
	if(i != UnloadedPlugins.end())
		UnloadedPlugins.erase(i);
}
