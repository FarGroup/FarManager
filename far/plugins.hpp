#pragma once

/*
plugins.hpp

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

#include "bitflags.hpp"
#include "plclass.hpp"
#include "PluginA.hpp"
#include "configdb.hpp"
#include "mix.hpp"

class SaveScreen;
class FileEditor;
class Viewer;
class Frame;
class Panel;

enum
{
	PLUGIN_FARGETFILE,
	PLUGIN_FARGETFILES,
	PLUGIN_FARPUTFILES,
	PLUGIN_FARDELETEFILES,
	PLUGIN_FARMAKEDIRECTORY,
	PLUGIN_FAROTHER
};

// флаги для поля Plugin.WorkFlags
enum PLUGINITEMWORKFLAGS
{
	PIWF_CACHED        = 0x00000001, // кешируется
	PIWF_PRELOADED     = 0x00000002, //
	PIWF_DONTLOADAGAIN = 0x00000004, // не загружать плагин снова, ставится в
	//   результате проверки требуемой версии фара
	PIWF_DATALOADED    = 0x00000008, // LoadData успешно выполнилась
};

// флаги для поля Plugin.FuncFlags - активности функций
enum PLUGINITEMCALLFUNCFLAGS
{
	PICFF_LOADED               = 0x00000001, // DLL загружен ;-)
	PICFF_OPENPANEL            = 0x00000004, //
	PICFF_ANALYSE              = 0x00000008, //
	PICFF_CLOSEPANEL           = 0x00000010, //
	PICFF_GETGLOBALINFO        = 0x00000002, //
	PICFF_SETSTARTUPINFO       = 0x00000004, //
	PICFF_OPENPLUGIN           = 0x00000008, //
	PICFF_OPENFILEPLUGIN       = 0x00000010, //
	PICFF_CLOSEPLUGIN          = 0x00000020, //
	PICFF_GETPLUGININFO        = 0x00000040, //
	PICFF_GETOPENPANELINFO     = 0x00000080, //
	PICFF_GETFINDDATA          = 0x00000100, //
	PICFF_FREEFINDDATA         = 0x00000200, //
	PICFF_GETVIRTUALFINDDATA   = 0x00000400, //
	PICFF_FREEVIRTUALFINDDATA  = 0x00000800, //
	PICFF_SETDIRECTORY         = 0x00001000, //
	PICFF_GETFILES             = 0x00002000, //
	PICFF_PUTFILES             = 0x00004000, //
	PICFF_DELETEFILES          = 0x00008000, //
	PICFF_MAKEDIRECTORY        = 0x00010000, //
	PICFF_PROCESSHOSTFILE      = 0x00020000, //
	PICFF_SETFINDLIST          = 0x00040000, //
	PICFF_CONFIGURE            = 0x00080000, //
	PICFF_EXITFAR              = 0x00100000, //
	PICFF_PROCESSPANELINPUT    = 0x00200000, //
	PICFF_PROCESSPANELEVENT    = 0x00400000, //
	PICFF_PROCESSEDITOREVENT   = 0x00800000, //
	PICFF_COMPARE              = 0x01000000, //
	PICFF_PROCESSEDITORINPUT   = 0x02000000, //
	PICFF_MINFARVERSION        = 0x04000000, //
	PICFF_PROCESSVIEWEREVENT   = 0x08000000, //
	PICFF_PROCESSDIALOGEVENT   = 0x10000000, //
	PICFF_PROCESSSYNCHROEVENT  = 0x20000000, //
	PICFF_PROCESSCONSOLEINPUT  = 0x40000000, //
	PICFF_CLOSEANALYSE         = 0x80000000, //
};

// флаги для поля PluginManager.Flags
enum PLUGINSETFLAGS
{
	PSIF_ENTERTOOPENPLUGIN        = 0x00000001, // ввалились в плагин OpenPlugin
	PSIF_PLUGINSLOADDED           = 0x80000000, // плагины загружены
};

ENUM(OPENFILEPLUGINTYPE)
{
	OFP_NORMAL,
	OFP_ALTERNATIVE,
	OFP_SEARCH,
	OFP_CREATE,
	OFP_EXTRACT,
	OFP_COMMANDS,
};

// параметры вызова макрофункций plugin.call и т.п.
typedef unsigned int CALLPLUGINFLAGS;
static const CALLPLUGINFLAGS
	CPT_MENU        = 0x00000001L,
	CPT_CONFIGURE   = 0x00000002L,
	CPT_CMDLINE     = 0x00000004L,
	CPT_INTERNAL    = 0x00000008L,
	CPT_MASK        = 0x0000000FL,
	CPT_CHECKONLY   = 0x10000000L;

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

struct PluginHandle
{
	HANDLE hPlugin;
	Plugin *pPlugin;
};

class Dialog;

class PluginManager: NonCopyable
{
	struct plugin_less { bool operator ()(const Plugin* a, const Plugin *b) const; };

public:
	PluginManager();
	~PluginManager();

	// API functions
	PluginHandle* Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item);
	PluginHandle* OpenFilePlugin(const string* Name, int OpMode, OPENFILEPLUGINTYPE Type);
	PluginHandle* OpenFindListPlugin(const PluginPanelItem *PanelItem,size_t ItemsNumber);
	static void ClosePanel(PluginHandle* hPlugin);
	static void GetOpenPanelInfo(PluginHandle* hPlugin, OpenPanelInfo *Info);
	static int GetFindData(PluginHandle* hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,int OpMode);
	static void FreeFindData(PluginHandle* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool FreeUserData);
	static int GetVirtualFindData(PluginHandle* hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,const string& Path);
	static void FreeVirtualFindData(PluginHandle* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);
	static int SetDirectory(PluginHandle* hPlugin, const string& Dir, int OpMode, UserDataItem *UserData=nullptr);
	static int GetFile(PluginHandle* hPlugin,PluginPanelItem *PanelItem,const string& DestPath,string &strResultName,int OpMode);
	static int GetFiles(PluginHandle* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,const wchar_t **DestPath,int OpMode);
	static int PutFiles(PluginHandle* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,int OpMode);
	static int DeleteFiles(PluginHandle* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
	static int MakeDirectory(PluginHandle* hPlugin,const wchar_t **Name,int OpMode);
	static int ProcessHostFile(PluginHandle* hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
	static int ProcessKey(PluginHandle* hPlugin,const INPUT_RECORD *Rec,bool Pred);
	static int ProcessEvent(PluginHandle* hPlugin,int Event,void *Param);
	static int Compare(PluginHandle* hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
	int ProcessEditorInput(const INPUT_RECORD *Rec) const;
	int ProcessEditorEvent(int Event,void *Param,int EditorID) const;
	int ProcessSubscribedEditorEvent(int Event,void *Param,int EditorID, const std::list<GUID> &PluginIds) const;
	int ProcessViewerEvent(int Event,void *Param,int ViewerID) const;
	int ProcessDialogEvent(int Event,FarDialogEvent *Param) const;
	int ProcessConsoleInput(ProcessConsoleInputInfo *Info) const;
	string GetCustomData(const string& Name) const;

	int UnloadPlugin(Plugin *pPlugin, int From);
	Plugin* LoadPluginExternal(const string& lpwszModuleName, bool LoadToMem);
	int UnloadPluginExternal(Plugin* hPlugin);
	bool IsPluginUnloaded(Plugin* pPlugin);
	void LoadModels();
	void LoadPlugins();

	typedef Plugin* value_type;
	typedef std::multiset<Plugin*, plugin_less> plugins_set;
	typedef plugins_set::const_iterator iterator;

	iterator begin() const { return SortedPlugins.cbegin(); }
	iterator end() const { return SortedPlugins.cend(); }
	iterator cbegin() const { return begin(); }
	iterator cend() const { return end(); }

	Plugin *GetPlugin(const string& ModuleName);
	size_t GetPluginsCount() const { return SortedPlugins.size(); }
#ifndef NO_WRAPPER
	size_t OemPluginsPresent() const { return OemPluginsCount > 0; }
#endif // NO_WRAPPER
	bool IsPluginsLoaded() const { return Flags.Check(PSIF_PLUGINSLOADDED); }
	bool CheckFlags(DWORD NewFlags) const { return Flags.Check(NewFlags); }
	void Configure(int StartPos=0);
	int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=nullptr);
	bool GetDiskMenuItem(Plugin *pPlugin, size_t PluginItem, bool &ItemPresent, wchar_t& PluginHotkey, string &strPluginText, GUID &Guid);
	void ReloadLanguage();
	void DiscardCache();
	int ProcessCommandLine(const string& Command,Panel *Target=nullptr);
	size_t GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize);
	// $ .09.2000 SVS - Функция CallPlugin - найти плагин по ID и запустить OpenFrom = OPEN_*
	int CallPlugin(const GUID& SysID,int OpenFrom, void *Data, void **Ret=nullptr);
	int CallPluginItem(const GUID& Guid, CallPluginInfo *Data);
	Plugin *FindPlugin(const GUID& SysID) const;
	void RefreshPluginsList();
	void UndoRemove(Plugin* plugin);
	FileEditor* GetCurEditor() const { return m_CurEditor; }
	void SetCurEditor(FileEditor* Editor) { m_CurEditor = Editor; }
	Viewer* GetCurViewer() const { return m_CurViewer; }
	void SetCurViewer(Viewer* Viewer) { m_CurViewer = Viewer; }

	static void ConfigureCurrent(Plugin *pPlugin, const GUID& Guid);
	static int UseFarCommand(PluginHandle* hPlugin, int CommandType);
	static const GUID& GetGUID(const PluginHandle* hPlugin);
	static bool SetHotKeyDialog(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, const string& DlgPluginTitle);
	static void ShowPluginInfo(Plugin *pPlugin, const GUID& Guid);

private:
	friend class Plugin;

	void LoadIfCacheAbsent();
	Plugin* LoadPlugin(const string& lpwszModuleName, const api::FAR_FIND_DATA &FindData, bool LoadToMem);
	bool AddPlugin(Plugin *pPlugin);
	bool RemovePlugin(Plugin *pPlugin);
	bool UpdateId(Plugin *pPlugin, const GUID& Id);
	void LoadPluginsFromCache();

	static void ReadUserBackgound(SaveScreen *SaveScr);
	static void GetHotKeyPluginKey(Plugin *pPlugin, string &strPluginKey);
	static void GetPluginHotKey(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, string &strHotKey);

	std::vector<std::unique_ptr<GenericPluginModel>> PluginModels;
	std::unordered_map<GUID, std::unique_ptr<Plugin>, uuid_hash, uuid_equal> Plugins;
	plugins_set SortedPlugins;
	std::list<Plugin*> UnloadedPlugins;
	BitFlags Flags;
#ifndef NO_WRAPPER
	size_t OemPluginsCount;
#endif // NO_WRAPPER
	FileEditor* m_CurEditor;
	Viewer* m_CurViewer;
};
