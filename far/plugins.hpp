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

// Internal:
#include "plclass.hpp"
#include "mix.hpp"
#include "notification.hpp"

// Platform:
#include "platform.fwd.hpp"

// Common:
#include "common/function_traits.hpp"
#include "common/range.hpp"
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

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
enum CALLPLUGINFLAGS: unsigned int
{
	CPT_MENU        = 0_bit,
	CPT_CONFIGURE   = 1_bit,
	CPT_CMDLINE     = 2_bit,
	CPT_INTERNAL    = 3_bit,
	CPT_MASK        = 0b1111,
	CPT_CHECKONLY   = 28_bit,
};

class plugin_panel
{
public:
	NONCOPYABLE(plugin_panel);
	MOVE_CONSTRUCTIBLE(plugin_panel);

	plugin_panel(Plugin* PluginInstance, HANDLE Panel);
	~plugin_panel();

	Plugin* plugin() const
	{
		return m_Plugin.get();
	}

	HANDLE panel() const
	{
		return m_Panel.get();
	}

	void set_panel(HANDLE Panel)
	{
		m_Panel.reset(Panel);
	}

	void delayed_delete(const string& Name);

private:
	movable_ptr<Plugin> m_Plugin;
	FN_RETURN_TYPE(Plugin::keep_activity) m_PluginActivity;
	movable_ptr<std::remove_pointer_t<HANDLE>> m_Panel;
	std::unique_ptr<delayed_deleter> m_DelayedDeleter;
};

class PluginManager: noncopyable
{
	struct plugin_less
	{
		bool operator()(const Plugin* a, const Plugin *b) const;
	};

public:
	PluginManager();

	// API functions
	std::unique_ptr<plugin_panel> Open(Plugin* pPlugin, int OpenFrom, const UUID& Uuid, intptr_t Item) const;
	std::unique_ptr<plugin_panel> OpenFilePlugin(const string* Name, OPERATION_MODES OpMode, OPENFILEPLUGINTYPE Type, bool* StopProcessingPtr = nullptr);
	std::unique_ptr<plugin_panel> OpenFindListPlugin(span<const PluginPanelItem> PanelItems);
	static void ClosePanel(std::unique_ptr<plugin_panel>&& hPlugin);
	static void GetOpenPanelInfo(const plugin_panel* hPlugin, OpenPanelInfo *Info);
	static intptr_t GetFindData(const plugin_panel* hPlugin, span<PluginPanelItem>& PanelItems, int OpMode);
	static void FreeFindData(const plugin_panel* hPlugin, span<PluginPanelItem>, bool FreeUserData);
	static intptr_t GetVirtualFindData(const plugin_panel* hPlugin, span<PluginPanelItem>& PanelItems, const string& Path);
	static void FreeVirtualFindData(const plugin_panel* hPlugin, span<PluginPanelItem> PanelItems);
	static intptr_t SetDirectory(const plugin_panel* hPlugin, const string& Dir, int OpMode, const UserDataItem* UserData = nullptr);
	static bool GetFile(const plugin_panel* hPlugin,PluginPanelItem *PanelItem,const string& DestPath,string &strResultName,int OpMode);
	static intptr_t GetFiles(const plugin_panel* hPlugin, span<PluginPanelItem> PanelItems, bool Move, const wchar_t** DestPath, int OpMode);
	static intptr_t PutFiles(const plugin_panel* hPlugin, span<PluginPanelItem> PanelItems, bool Move, int OpMode);
	static intptr_t DeleteFiles(const plugin_panel* hPlugin, span<PluginPanelItem> PanelItems, int OpMode);
	static intptr_t MakeDirectory(const plugin_panel* hPlugin,const wchar_t **Name,int OpMode);
	static intptr_t ProcessHostFile(const plugin_panel* hPlugin, span<PluginPanelItem> PanelItems, int OpMode);
	static intptr_t ProcessKey(const plugin_panel* hPlugin,const INPUT_RECORD *Rec,bool Pred);
	static intptr_t ProcessEvent(const plugin_panel* hPlugin,int Event,void *Param);
	static intptr_t Compare(const plugin_panel* hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
	static bool GetPluginInfo(Plugin *pPlugin, PluginInfo* Info);
	intptr_t ProcessEditorInput(const INPUT_RECORD *Rec) const;
	intptr_t ProcessEditorEvent(int Event, void *Param, const Editor* EditorInstance) const;
	intptr_t ProcessSubscribedEditorEvent(int Event, void *Param, const Editor* EditorInstance, const std::unordered_set<UUID>& PluginIds) const;
	intptr_t ProcessViewerEvent(int Event, void *Param, const Viewer* ViewerInstance) const;
	intptr_t ProcessDialogEvent(int Event,FarDialogEvent *Param) const;
	intptr_t ProcessConsoleInput(ProcessConsoleInputInfo *Info) const;
	std::vector<Plugin*> GetContentPlugins(const std::vector<const wchar_t*>& ColNames) const;
	void GetContentData(const std::vector<Plugin*>& Plugins, string_view FilePath, const std::vector<const wchar_t*>& ColNames, std::vector<const wchar_t*>& ColValues, std::unordered_map<string,string>& ContentData) const;
	Plugin* LoadPluginExternal(const string& ModuleName, bool LoadToMem);
	bool UnloadPluginExternal(Plugin* pPlugin);
	bool IsPluginUnloaded(const Plugin* pPlugin) const;
	void LoadPlugins();
	void UnloadPlugins();

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
		unsigned int CallFlags; // CALLPLUGINFLAGS
		int OpenFrom;
		union
		{
			UUID *ItemUuid;
			const wchar_t *Command;
		};
		// Используется в функции CallPluginItem для внутренних нужд
		Plugin *pPlugin;
		UUID FoundUuid;
	};

	Plugin *FindPlugin(const string& ModuleName) const;
	Plugin *FindPlugin(const UUID& SysID) const;

#ifndef NO_WRAPPER
	bool OemPluginsPresent() const { return OemPluginsCount > 0; }
#endif // NO_WRAPPER
	bool IsPluginsLoaded() const { return m_PluginsLoaded; }
	void Configure(int StartPos=0);
	int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=nullptr);
	bool GetDiskMenuItem(Plugin* pPlugin, size_t PluginItem, bool& ItemPresent, wchar_t& PluginHotkey, string& strPluginText, UUID& Uuid) const;
	void ReloadLanguage() const;
	bool ProcessCommandLine(const string& Command);
	size_t GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize);
	// $ .09.2000 SVS - Функция CallPlugin - найти плагин по ID и запустить OpenFrom = OPEN_*
	bool CallPlugin(const UUID& SysID,int OpenFrom, void *Data, void **Ret=nullptr) const;
	bool CallPluginItem(const UUID& Uuid, CallPluginInfo* Data) const;
	void RefreshPluginsList();
	const auto& Factories() const { return PluginFactories; }

	static void ConfigureCurrent(Plugin *pPlugin, const UUID& Uuid);
	static bool UseInternalCommand(const plugin_panel* hPlugin, int CommandType, OpenPanelInfo const& Info);
	static const UUID& GetUUID(const plugin_panel* hPlugin);
	static bool SetHotKeyDialog(Plugin* pPlugin, const UUID& Uuid, hotkey_type HotKeyType, string_view DlgPluginTitle);
	static void ShowPluginInfo(Plugin* pPlugin, const UUID& Uuid);

private:
	friend class Plugin;

	void LoadFactories();
	void LoadIfCacheAbsent() const;
	Plugin* LoadPlugin(const string& FileName, const os::fs::find_data &FindData, bool LoadToMem);
	Plugin* AddPlugin(std::unique_ptr<Plugin>&& pPlugin);
	bool RemovePlugin(Plugin *pPlugin);
	int UnloadPlugin(Plugin *pPlugin, int From);
	void UndoRemove(Plugin* plugin);
	bool UpdateId(Plugin *pPlugin, const UUID& Id);
	void LoadPluginsFromCache();

	std::vector<std::unique_ptr<plugin_factory>> PluginFactories;
	std::unordered_map<UUID, std::unique_ptr<Plugin>> m_Plugins;
	plugins_set SortedPlugins;
	std::list<Plugin*> UnloadedPlugins;
	listener m_PluginSynchro;

#ifndef NO_WRAPPER
	size_t OemPluginsCount;
#endif // NO_WRAPPER
	bool m_PluginsLoaded;
};

#endif // PLUGINS_HPP_1BFC0299_8B63_4CCD_AC6B_1D48977E7A97
