#ifndef PLUGINS_HPP_1BFC0299_8B63_4CCD_AC6B_1D48977E7A97
#define PLUGINS_HPP_1BFC0299_8B63_4CCD_AC6B_1D48977E7A97
#pragma once

/*
plugins.hpp

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

#include "plclass.hpp"
#include "mix.hpp"
#include "notification.hpp"
#include "panelfwd.hpp"

class SaveScreen;
class FileEditor;
class Viewer;
class Editor;
class window;
class Panel;
class Dialog;
class delayed_deleter;

enum class hotkey_type: int;

enum
{
	PLUGIN_FARGETFILE,
	PLUGIN_FARGETFILES,
	PLUGIN_FARPUTFILES,
	PLUGIN_FARDELETEFILES,
	PLUGIN_FARMAKEDIRECTORY,
	PLUGIN_FAROTHER
};

enum OPENFILEPLUGINTYPE: int
{
	OFP_NORMAL,
	OFP_ALTERNATIVE,
	OFP_SEARCH,
	OFP_CREATE,
	OFP_EXTRACT,
	OFP_COMMANDS,
};

// параметры вызова макрофункций plugin.call и т.п.
using CALLPLUGINFLAGS = unsigned int;
static const CALLPLUGINFLAGS
	CPT_MENU        = 0x00000001L,
	CPT_CONFIGURE   = 0x00000002L,
	CPT_CMDLINE     = 0x00000004L,
	CPT_INTERNAL    = 0x00000008L,
	CPT_MASK        = 0x0000000FL,
	CPT_CHECKONLY   = 0x10000000L;

class plugin_panel
{
public:
	NONCOPYABLE(plugin_panel);
	MOVABLE(plugin_panel);

	plugin_panel(Plugin* PluginInstance, HANDLE Panel);
	~plugin_panel();

	auto plugin() const
	{
		return m_Plugin;
	}

	auto panel() const
	{
		return m_Panel;
	}

	void set_panel(HANDLE Panel)
	{
		m_Panel = Panel;
	}

	void delayed_delete(const string& Name);

private:
	Plugin* m_Plugin;
	FN_RETURN_TYPE(Plugin::keep_activity) m_PluginActivity;
	HANDLE m_Panel{};
	std::list<delayed_deleter> m_DelayedDeleters;
};

class PluginManager: noncopyable
{
	struct plugin_less { bool operator ()(const Plugin* a, const Plugin *b) const; };

public:
	PluginManager();
	~PluginManager();

	// API functions
	std::unique_ptr<plugin_panel> Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item) const;
	std::unique_ptr<plugin_panel> OpenFilePlugin(const string* Name, OPERATION_MODES OpMode, OPENFILEPLUGINTYPE Type, bool* StopProcessing = nullptr);
	std::unique_ptr<plugin_panel> OpenFindListPlugin(const PluginPanelItem *PanelItem,size_t ItemsNumber);
	static void ClosePanel(std::unique_ptr<plugin_panel>&& hPlugin);
	static void GetOpenPanelInfo(const plugin_panel* hPlugin, OpenPanelInfo *Info);
	static int GetFindData(const plugin_panel* hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,int OpMode);
	static void FreeFindData(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool FreeUserData);
	static int GetVirtualFindData(const plugin_panel* hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,const string& Path);
	static void FreeVirtualFindData(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);
	static int SetDirectory(const plugin_panel* hPlugin, const string& Dir, int OpMode, const UserDataItem* UserData = nullptr);
	static int GetFile(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,const string& DestPath,string &strResultName,int OpMode);
	static int GetFiles(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,const wchar_t **DestPath,int OpMode);
	static int PutFiles(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,int OpMode);
	static int DeleteFiles(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
	static int MakeDirectory(const plugin_panel* hPlugin,const wchar_t **Name,int OpMode);
	static int ProcessHostFile(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
	static int ProcessKey(const plugin_panel* hPlugin,const INPUT_RECORD *Rec,bool Pred);
	static int ProcessEvent(const plugin_panel* hPlugin,int Event,void *Param);
	static int Compare(const plugin_panel* hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
	int ProcessEditorInput(const INPUT_RECORD *Rec) const;
	int ProcessEditorEvent(int Event, void *Param, const Editor* EditorInstance) const;
	int ProcessSubscribedEditorEvent(int Event, void *Param, const Editor* EditorInstance, const std::unordered_set<GUID, uuid_hash, uuid_equal>& PluginIds) const;
	int ProcessViewerEvent(int Event, void *Param, const Viewer* ViewerInstance) const;
	int ProcessDialogEvent(int Event,FarDialogEvent *Param) const;
	int ProcessConsoleInput(ProcessConsoleInputInfo *Info) const;
	std::vector<Plugin*> GetContentPlugins(const std::vector<const wchar_t*>& ColNames) const;
	void GetContentData(const std::vector<Plugin*>& Plugins, const string& FilePath, const std::vector<const wchar_t*>& Names, std::vector<const wchar_t*>& Values, std::unordered_map<string,string>& ContentData) const;
	Plugin* LoadPluginExternal(const string& ModuleName, bool LoadToMem);
	int UnloadPluginExternal(Plugin* hPlugin);
	bool IsPluginUnloaded(const Plugin* pPlugin) const;
	void LoadPlugins();

private:
	using plugins_set = std::multiset<Plugin*, plugin_less>;

public:
	using value_type = Plugin*;
	using iterator = plugins_set::const_iterator;

	iterator begin() const { return SortedPlugins.cbegin(); }
	iterator end() const { return SortedPlugins.cend(); }
	iterator cbegin() const { return begin(); }
	iterator cend() const { return end(); }

	size_t size() const { return SortedPlugins.size(); }

	struct CallPluginInfo
	{
		CALLPLUGINFLAGS CallFlags;
		int OpenFrom;
		union
		{
			GUID *ItemGuid;
			const wchar_t *Command;
		};
		// Используется в функции CallPluginItem для внутренних нужд
		Plugin *pPlugin;
		GUID FoundGuid;
	};

	Plugin *FindPlugin(const string& ModuleName) const;
	Plugin *FindPlugin(const GUID& SysID) const;

#ifndef NO_WRAPPER
	bool OemPluginsPresent() const { return OemPluginsCount > 0; }
#endif // NO_WRAPPER
	bool IsPluginsLoaded() const { return m_PluginsLoaded; }
	void Configure(int StartPos=0);
	int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=nullptr);
	bool GetDiskMenuItem(Plugin *pPlugin, size_t PluginItem, bool &ItemPresent, wchar_t& PluginHotkey, string &strPluginText, GUID &Guid) const;
	void ReloadLanguage() const;
	int ProcessCommandLine(const string& Command, panel_ptr Target = nullptr);
	size_t GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize);
	// $ .09.2000 SVS - Функция CallPlugin - найти плагин по ID и запустить OpenFrom = OPEN_*
	bool CallPlugin(const GUID& SysID,int OpenFrom, void *Data, void **Ret=nullptr);
	bool CallPluginItem(const GUID& Guid, CallPluginInfo *Data);
	void RefreshPluginsList();

	static void ConfigureCurrent(Plugin *pPlugin, const GUID& Guid);
	static int UseFarCommand(plugin_panel* hPlugin, int CommandType);
	static const GUID& GetGUID(const plugin_panel* hPlugin);
	static bool SetHotKeyDialog(Plugin *pPlugin, const GUID& Guid, hotkey_type HotKeyType, const string& DlgPluginTitle);
	static void ShowPluginInfo(Plugin *pPlugin, const GUID& Guid);

private:
	friend class Plugin;

	void LoadFactories();
	void LoadIfCacheAbsent() const;
	Plugin* LoadPlugin(const string& ModuleName, const os::FAR_FIND_DATA &FindData, bool LoadToMem);
	Plugin* AddPlugin(std::unique_ptr<Plugin>&& pPlugin);
	bool RemovePlugin(Plugin *pPlugin);
	int UnloadPlugin(Plugin *pPlugin, int From);
	void UndoRemove(Plugin* plugin);
	bool UpdateId(Plugin *pPlugin, const GUID& Id);
	void LoadPluginsFromCache();

	std::vector<std::unique_ptr<plugin_factory>> PluginFactories;
	std::unordered_map<GUID, std::unique_ptr<Plugin>, uuid_hash, uuid_equal> m_Plugins;
	plugins_set SortedPlugins;
	std::list<Plugin*> UnloadedPlugins;
	listener_ex m_PluginSynchro;

#ifndef NO_WRAPPER
	size_t OemPluginsCount;
#endif // NO_WRAPPER
	bool m_PluginsLoaded;
};

#endif // PLUGINS_HPP_1BFC0299_8B63_4CCD_AC6B_1D48977E7A97
