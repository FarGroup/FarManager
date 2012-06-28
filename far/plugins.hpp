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
#include "plugin.hpp"
#include "plclass.hpp"
#include "PluginA.hpp"
#include "tree.hpp"
#include "configdb.hpp"

class SaveScreen;
class FileEditor;
class Viewer;
class Frame;
class Panel;
struct FileListItem;

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
#if defined(MANTIS_0000466)
	PICFF_PROCESSMACRO         = 0x40000000, //
#endif
#if defined(MANTIS_0001687)
	PICFF_PROCESSCONSOLEINPUT  = 0x80000000, //
#endif
};

// флаги для поля PluginManager.Flags
enum PLUGINSETFLAGS
{
	PSIF_ENTERTOOPENPLUGIN        = 0x00000001, // ввалились в плагин OpenPlugin
	PSIF_PLUGINSLOADDED           = 0x80000000, // плагины загружены
};

enum OPENFILEPLUGINTYPE
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
	class Plugin *pPlugin;
};

class PluginTree: public Tree<class AncientPlugin*>
{
	public:
		PluginTree();
		~PluginTree();
		long compare(Node<class AncientPlugin*>* first,class AncientPlugin** second);
		class AncientPlugin** query(const GUID& value);
};

class PluginManager
{
	private:

		Plugin **PluginsData;
		size_t PluginsCount;
#ifndef NO_WRAPPER
		size_t OemPluginsCount;
#endif // NO_WRAPPER
		PluginTree* PluginsCache;

		DList<Plugin*> UnloadedPlugins;

	public:

		BitFlags Flags;        // флаги манагера плагинов

		Plugin *CurPluginItem;

		FileEditor *CurEditor;
		Viewer *CurViewer;     // 27.09.2000 SVS: Указатель на текущий Viewer

	private:

		void LoadIfCacheAbsent();
		void ReadUserBackgound(SaveScreen *SaveScr);

		void GetHotKeyPluginKey(Plugin *pPlugin, string &strPluginKey);
		void GetPluginHotKey(Plugin *pPlugin,const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType,string &strHotKey);

		bool TestPluginInfo(Plugin *Item,PluginInfo *Info);
		bool TestOPENPANELINFO(Plugin *Item,OpenPanelInfo *Info);

		Plugin* LoadPlugin(const string& lpwszModuleName, const FAR_FIND_DATA_EX &FindData, bool LoadToMem);

		bool AddPlugin(Plugin *pPlugin);
		bool RemovePlugin(Plugin *pPlugin);

		bool UpdateId(Plugin *pPlugin, const GUID& Id);

		void LoadPluginsFromCache();

		void SetFlags(DWORD NewFlags) { Flags.Set(NewFlags); }
		void SkipFlags(DWORD NewFlags) { Flags.Clear(NewFlags); }

	public:

		PluginManager();
		~PluginManager();

	public:


		int UnloadPlugin(Plugin *pPlugin, DWORD dwException);

		HANDLE LoadPluginExternal(const string& lpwszModuleName, bool LoadToMem);
		int UnloadPluginExternal(HANDLE hPlugin);
		bool IsPluginUnloaded(Plugin* pPlugin);

		void LoadPlugins();

		Plugin *GetPlugin(const wchar_t *lpwszModuleName);
		Plugin *GetPlugin(size_t PluginNumber);

		size_t GetPluginsCount() { return PluginsCount; }
#ifndef NO_WRAPPER
		size_t GetOemPluginsCount() { return OemPluginsCount; }
#endif // NO_WRAPPER

		BOOL IsPluginsLoaded() { return Flags.Check(PSIF_PLUGINSLOADDED); }

		BOOL CheckFlags(DWORD NewFlags) { return Flags.Check(NewFlags); }

		void Configure(int StartPos=0);
		void ConfigureCurrent(Plugin *pPlugin,const GUID& Guid);
		int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=nullptr);
		bool GetDiskMenuItem(Plugin *pPlugin,size_t PluginItem,bool &ItemPresent, wchar_t& PluginHotkey, string &strPluginText, GUID &Guid);

		int UseFarCommand(HANDLE hPlugin,int CommandType);
		void ReloadLanguage();
		void DiscardCache();
		int ProcessCommandLine(const wchar_t *Command,Panel *Target=nullptr);

		bool SetHotKeyDialog(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, const wchar_t *DlgPluginTitle);
		void ShowPluginInfo(Plugin *pPlugin, const GUID& Guid);
		size_t GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize);

		// $ .09.2000 SVS - Функция CallPlugin - найти плагин по ID и запустить OpenFrom = OPEN_*
#ifdef FAR_LUA
		int CallPlugin(const GUID& SysID,int OpenFrom, void *Data, void **Ret=nullptr);
#else
		int CallPlugin(const GUID& SysID,int OpenFrom, void *Data, int *Ret=nullptr);
#endif
		int CallPluginItem(const GUID& Guid, CallPluginInfo *Data, int *Ret=nullptr);
		Plugin *FindPlugin(const GUID& SysID);
		static const GUID& GetGUID(HANDLE hPlugin);

		void RefreshPluginsList();
		void UndoRemove(Plugin* plugin);

//api functions

	public:
		HANDLE Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item);
		HANDLE OpenFilePlugin(const string* Name, int OpMode, OPENFILEPLUGINTYPE Type);
		HANDLE OpenFindListPlugin(const PluginPanelItem *PanelItem,size_t ItemsNumber);
		void ClosePanel(HANDLE hPlugin);
		void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info);
		int GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,int Silent);
		void FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);
		int GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,const wchar_t *Path);
		void FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);
		int SetDirectory(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
		int GetFile(HANDLE hPlugin,PluginPanelItem *PanelItem,const wchar_t *DestPath,string &strResultName,int OpMode);
		int GetFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,const wchar_t **DestPath,int OpMode);
		int PutFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,int OpMode);
		int DeleteFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
		int MakeDirectory(HANDLE hPlugin,const wchar_t **Name,int OpMode);
		int ProcessHostFile(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
		int ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec,bool Pred);
		int ProcessEvent(HANDLE hPlugin,int Event,void *Param);
		int Compare(HANDLE hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
		int ProcessEditorInput(INPUT_RECORD *Rec);
		int ProcessEditorEvent(int Event,void *Param,int EditorID);
		int ProcessViewerEvent(int Event,void *Param,int ViewerID);
		int ProcessDialogEvent(int Event,FarDialogEvent *Param);
#if defined(MANTIS_0000466)
		int ProcessMacro(const GUID& guid,ProcessMacroInfo *Info);
#endif
#if defined(MANTIS_0001687)
		int ProcessConsoleInput(ProcessConsoleInputInfo *Info);
#endif
		void GetCustomData(FileListItem *ListItem);

		friend class Plugin;
};
