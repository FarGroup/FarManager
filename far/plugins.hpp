#pragma once

/*
plugins.hpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "language.hpp"
#include "bitflags.hpp"
#include "plugin.hpp"
#include "plclass.hpp"
#include "PluginA.hpp"
#include "PluginW.hpp"

extern const wchar_t *FmtPluginsCache_PluginS;
extern const wchar_t *FmtDiskMenuStringD;
extern const wchar_t *FmtDiskMenuNumberD;
extern const wchar_t *FmtPluginMenuStringD;
extern const wchar_t *FmtPluginConfigStringD;


class SaveScreen;
class FileEditor;
class Viewer;
class Frame;
class Panel;
struct FileListItem;

enum
{
	SYSID_PRINTMANAGER      =0x6E614D50,
	SYSID_NETWORK           =0x5774654E,
};

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
};

// флаги для поля Plugin.FuncFlags - активности функций
enum PLUGINITEMCALLFUNCFLAGS
{
	PICFF_LOADED               = 0x00000001, // DLL загружен ;-)
	PICFF_SETSTARTUPINFO       = 0x00000002, //
	PICFF_OPENPLUGIN           = 0x00000004, //
	PICFF_OPENFILEPLUGIN       = 0x00000008, //
	PICFF_CLOSEPLUGIN          = 0x00000010, //
	PICFF_GETPLUGININFO        = 0x00000020, //
	PICFF_GETOPENPLUGININFO    = 0x00000040, //
	PICFF_GETFINDDATA          = 0x00000080, //
	PICFF_FREEFINDDATA         = 0x00000100, //
	PICFF_GETVIRTUALFINDDATA   = 0x00000200, //
	PICFF_FREEVIRTUALFINDDATA  = 0x00000400, //
	PICFF_SETDIRECTORY         = 0x00000800, //
	PICFF_GETFILES             = 0x00001000, //
	PICFF_PUTFILES             = 0x00002000, //
	PICFF_DELETEFILES          = 0x00004000, //
	PICFF_MAKEDIRECTORY        = 0x00008000, //
	PICFF_PROCESSHOSTFILE      = 0x00010000, //
	PICFF_SETFINDLIST          = 0x00020000, //
	PICFF_CONFIGURE            = 0x00040000, //
	PICFF_EXITFAR              = 0x00080000, //
	PICFF_PROCESSKEY           = 0x00100000, //
	PICFF_PROCESSEVENT         = 0x00200000, //
	PICFF_PROCESSEDITOREVENT   = 0x00400000, //
	PICFF_COMPARE              = 0x00800000, //
	PICFF_PROCESSEDITORINPUT   = 0x01000000, //
	PICFF_MINFARVERSION        = 0x02000000, //
	PICFF_PROCESSVIEWEREVENT   = 0x04000000, //
	PICFF_PROCESSDIALOGEVENT   = 0x08000000, //
	PICFF_PROCESSSYNCHROEVENT  = 0x10000000, //

	// PICFF_PANELPLUGIN - первая попытка определиться с понятием "это панель"
	PICFF_PANELPLUGIN          = PICFF_OPENFILEPLUGIN|
	PICFF_GETFINDDATA|
	PICFF_FREEFINDDATA|
	PICFF_GETVIRTUALFINDDATA|
	PICFF_FREEVIRTUALFINDDATA|
	PICFF_SETDIRECTORY|
	PICFF_GETFILES|
	PICFF_PUTFILES|
	PICFF_DELETEFILES|
	PICFF_MAKEDIRECTORY|
	PICFF_PROCESSHOSTFILE|
	PICFF_SETFINDLIST|
	PICFF_PROCESSKEY|
	PICFF_PROCESSEVENT|
	PICFF_COMPARE|
	PICFF_GETOPENPLUGININFO,
};

// флаги для поля PluginManager.Flags
enum PLUGINSETFLAGS
{
	PSIF_ENTERTOOPENPLUGIN        = 0x00000001, // ввалились в плагин OpenPlugin
	PSIF_DIALOG                   = 0x00000002, // была бадяга с диалогом
	PSIF_PLUGINSLOADDED           = 0x80000000, // плагины загружены
};

struct PluginHandle
{
	HANDLE hPlugin;
	class Plugin *pPlugin;
};

class PluginManager
{
	private:

		Plugin **PluginsData;
		int PluginsCount;
		int OemPluginsCount;

	public:

		BitFlags Flags;        // флаги манагера плагинов

		Plugin *CurPluginItem;

		FileEditor *CurEditor;
		Viewer *CurViewer;     // 27.09.2000 SVS: Указатель на текущий Viewer

	private:

		void LoadIfCacheAbsent();
		void ReadUserBackgound(SaveScreen *SaveScr);

		void GetPluginHotKey(Plugin *pPlugin,int ItemNumber,const wchar_t *HotKeyType,string &strHotKey);

		bool TestPluginInfo(Plugin *Item,PluginInfo *Info);
		bool TestOpenPluginInfo(Plugin *Item,OpenPluginInfo *Info);

		bool LoadPlugin(const wchar_t *lpwszModuleName, const FAR_FIND_DATA_EX &FindData);

		bool AddPlugin(Plugin *pPlugin);
		bool RemovePlugin(Plugin *pPlugin);

		void LoadPluginsFromCache();

		void SetFlags(DWORD NewFlags) { Flags.Set(NewFlags); }
		void SkipFlags(DWORD NewFlags) { Flags.Clear(NewFlags); }

	public:

		PluginManager();
		~PluginManager();

	public:

		bool LoadPluginExternal(const wchar_t *lpwszModuleName);

		int UnloadPlugin(Plugin *pPlugin, DWORD dwException, bool bRemove = false);
		int UnloadPluginExternal(const wchar_t *lpwszModuleName);

		void LoadPlugins();

		Plugin *GetPlugin(const wchar_t *lpwszModuleName);
		Plugin *GetPlugin(int PluginNumber);

		int GetPluginsCount() { return PluginsCount; }
		int GetOemPluginsCount() { return OemPluginsCount; }

		BOOL IsPluginsLoaded() { return Flags.Check(PSIF_PLUGINSLOADDED); }

		BOOL CheckFlags(DWORD NewFlags) { return Flags.Check(NewFlags); }

		void Configure(int StartPos=0);
		void ConfigureCurrent(Plugin *pPlugin,int INum);
		int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=nullptr);
		bool GetDiskMenuItem(Plugin *pPlugin,int PluginItem,bool &ItemPresent, wchar_t& PluginHotkey, string &strPluginText);

		int UseFarCommand(HANDLE hPlugin,int CommandType);
		void ReloadLanguage();
		void DiscardCache();
		int ProcessCommandLine(const wchar_t *Command,Panel *Target=nullptr);

		bool SetHotKeyDialog(const wchar_t *DlgPluginTitle,const wchar_t *RegKey,const wchar_t *RegValueName);
		void GetHotKeyRegKey(Plugin *pPlugin,int ItemNumber,string &strRegKey);

		// $ .09.2000 SVS - Функция CallPlugin - найти плагин по ID и запустить OpenFrom = OPEN_*
		int CallPlugin(DWORD SysID,int OpenFrom, void *Data);
		Plugin *FindPlugin(DWORD SysID);

//api functions

	public:

		Plugin *Analyse(const AnalyseData *pData);

		HANDLE OpenPlugin(Plugin *pPlugin,int OpenFrom,INT_PTR Item);
		HANDLE OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode);
		HANDLE OpenFindListPlugin(const PluginPanelItem *PanelItem,int ItemsNumber);
		void ClosePlugin(HANDLE hPlugin);
		void GetOpenPluginInfo(HANDLE hPlugin, OpenPluginInfo *Info);
		int GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,int Silent);
		void FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
		int GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
		void FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
		int SetDirectory(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
		int GetFile(HANDLE hPlugin,PluginPanelItem *PanelItem,const wchar_t *DestPath,string &strResultName,int OpMode);
		int GetFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode);
		int PutFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
		int DeleteFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
		int MakeDirectory(HANDLE hPlugin,const wchar_t **Name,int OpMode);
		int ProcessHostFile(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
		int ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
		int ProcessEvent(HANDLE hPlugin,int Event,void *Param);
		int Compare(HANDLE hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
		int ProcessEditorInput(INPUT_RECORD *Rec);
		int ProcessEditorEvent(int Event,void *Param);
		int ProcessViewerEvent(int Event,void *Param);
		int ProcessDialogEvent(int Event,void *Param);

		void GetCustomData(FileListItem *ListItem);

		friend class Plugin;
};
