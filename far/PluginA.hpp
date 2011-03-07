#pragma once

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

class PluginManager;

#include "language.hpp"
#include "bitflags.hpp"
#include "plugin.hpp"
#include "plclass.hpp"
#include "pluginold.hpp"
#include "FarGuid.hpp"

typedef void (WINAPI *PLUGINCLOSEPANEL)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPARE)(HANDLE hPlugin,const oldfar::PluginPanelItem *Item1,const oldfar::PluginPanelItem *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGURE)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILES)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFAR)();
typedef void (WINAPI *PLUGINFREEFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILES)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSION)();
typedef void (WINAPI *PLUGINGETOPENPANELINFO)(HANDLE hPlugin,oldfar::OpenPanelInfo *Info);
typedef void (WINAPI *PLUGINGETPLUGININFO)(oldfar::PluginInfo *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORY)(HANDLE hPlugin,char *Name,int OpMode);
typedef HANDLE(WINAPI *PLUGINOPENFILEPLUGIN)(char *Name,const unsigned char *Data,int DataSize);
typedef HANDLE(WINAPI *PLUGINOPENPANEL)(int OpenFrom,INT_PTR Item);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENT)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUT)(const INPUT_RECORD *Rec);
typedef int (WINAPI *PLUGINPROCESSEVENT)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSHOSTFILE)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINPROCESSKEY)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPUTFILES)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int (WINAPI *PLUGINSETDIRECTORY)(HANDLE hPlugin,const char *Dir,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLIST)(HANDLE hPlugin,const oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINSETSTARTUPINFO)(const oldfar::PluginStartupInfo *Info);
typedef int (WINAPI *PLUGINPROCESSVIEWEREVENT)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSDIALOGEVENT)(int Event,void *Param);


class PluginA: public Plugin
{
	private:

		PluginManager *m_owner; //BUGBUG

		string m_strModuleName;
		string m_strCacheName;

		BitFlags WorkFlags;      // рабочие флаги текущего плагина
		BitFlags FuncFlags;      // битовые маски вызова эксп.функций плагина

		HMODULE m_hModule;
		Language Lang;

		string strRootKey;
		char *RootKey;

		PluginInfo PI;
		OpenPanelInfo OPI;

		oldfar::PluginPanelItem  *pFDPanelItemA;
		oldfar::PluginPanelItem  *pVFDPanelItemA;

		PLUGINSETSTARTUPINFO        pSetStartupInfo;
		PLUGINOPENPANEL             pOpenPanel;
		PLUGINOPENFILEPLUGIN        pOpenFilePlugin;
		PLUGINCLOSEPANEL            pClosePanel;
		PLUGINGETPLUGININFO         pGetPluginInfo;
		PLUGINGETOPENPANELINFO      pGetOpenPanelInfo;
		PLUGINGETFINDDATA           pGetFindData;
		PLUGINFREEFINDDATA          pFreeFindData;
		PLUGINGETVIRTUALFINDDATA    pGetVirtualFindData;
		PLUGINFREEVIRTUALFINDDATA   pFreeVirtualFindData;
		PLUGINSETDIRECTORY          pSetDirectory;
		PLUGINGETFILES              pGetFiles;
		PLUGINPUTFILES              pPutFiles;
		PLUGINDELETEFILES           pDeleteFiles;
		PLUGINMAKEDIRECTORY         pMakeDirectory;
		PLUGINPROCESSHOSTFILE       pProcessHostFile;
		PLUGINSETFINDLIST           pSetFindList;
		PLUGINCONFIGURE             pConfigure;
		PLUGINEXITFAR               pExitFAR;
		PLUGINPROCESSKEY            pProcessKey;
		PLUGINPROCESSEVENT          pProcessEvent;
		PLUGINPROCESSEDITOREVENT    pProcessEditorEvent;
		PLUGINCOMPARE               pCompare;
		PLUGINPROCESSEDITORINPUT    pProcessEditorInput;
		PLUGINMINFARVERSION         pMinFarVersion;
		PLUGINPROCESSVIEWEREVENT    pProcessViewerEvent;
		PLUGINPROCESSDIALOGEVENT    pProcessDialogEvent;

	public:

		PluginA(PluginManager *owner, const wchar_t *lpwzModuleName);
		~PluginA();

		bool IsOemPlugin() {return true;}

		bool LoadData(void);
		bool Load();
		bool LoadFromCache(const FAR_FIND_DATA_EX &FindData);

		bool SaveToCache();

		int Unload(bool bExitFAR = false);

		bool IsPanelPlugin();

		bool HasGetGlobalInfo() { return false; }
		bool HasOpenPanel() { return pOpenPanel!=nullptr; }
		bool HasMakeDirectory() { return pMakeDirectory!=nullptr; }
		bool HasDeleteFiles() { return pDeleteFiles!=nullptr; }
		bool HasPutFiles() { return pPutFiles!=nullptr; }
		bool HasGetFiles() { return pGetFiles!=nullptr; }
		bool HasSetStartupInfo() { return pSetStartupInfo!=nullptr; }
		bool HasOpenFilePlugin() { return pOpenFilePlugin!=nullptr; }
		bool HasClosePanel() { return pClosePanel!=nullptr; }
		bool HasGetPluginInfo() { return pGetPluginInfo!=nullptr; }
		bool HasGetOpenPanelInfo() { return pGetOpenPanelInfo!=nullptr; }
		bool HasGetFindData() { return pGetFindData!=nullptr; }
		bool HasFreeFindData() { return pFreeFindData!=nullptr; }
		bool HasGetVirtualFindData() { return pGetVirtualFindData!=nullptr; }
		bool HasFreeVirtualFindData() { return pFreeVirtualFindData!=nullptr; }
		bool HasSetDirectory() { return pSetDirectory!=nullptr; }
		bool HasProcessHostFile() { return pProcessHostFile!=nullptr; }
		bool HasSetFindList() { return pSetFindList!=nullptr; }
		bool HasConfigure() { return pConfigure!=nullptr; }
		bool HasExitFAR() { return pExitFAR!=nullptr; }
		bool HasProcessKey() { return pProcessKey!=nullptr; }
		bool HasProcessEvent() { return pProcessEvent!=nullptr; }
		bool HasProcessEditorEvent() { return pProcessEditorEvent!=nullptr; }
		bool HasCompare() { return pCompare!=nullptr; }
		bool HasProcessEditorInput() { return pProcessEditorInput!=nullptr; }
		bool HasMinFarVersion() { return pMinFarVersion!=nullptr; }
		bool HasProcessViewerEvent() { return pProcessViewerEvent!=nullptr; }
		bool HasProcessDialogEvent() { return pProcessDialogEvent!=nullptr; }
		bool HasProcessSynchroEvent() { return false; }
#if defined(PROCPLUGINMACROFUNC)
		bool HasProcessMacroFunc() { return false; }
#endif
		bool HasAnalyse() { return false; }
		bool HasGetCustomData()  { return false; }
		bool HasFreeCustomData() { return false; }

		const string &GetModuleName() { return m_strModuleName; }
		const wchar_t *GetCacheName() { return m_strCacheName; }
		const wchar_t *GetHotkeyName() { return GetCacheName(); }
		const GUID& GetGUID(void) { return FarGuid; }
		bool CheckWorkFlags(DWORD flags) { return WorkFlags.Check(flags)==TRUE; }
		DWORD GetWorkFlags() { return WorkFlags.Flags; }
		DWORD GetFuncFlags() { return FuncFlags.Flags; }

		bool InitLang(const wchar_t *Path) { return Lang.Init(Path,false); }
		void CloseLang() { Lang.Close(); }
		const char *GetMsgA(int nID) { return Lang.GetMsgA(nID); }

	public:
		bool GetGlobalInfo(GlobalInfo *Info) { return false; }
		bool SetStartupInfo(bool &bUnloaded);
		bool CheckMinFarVersion(bool &bUnloaded);

		HANDLE Open(int OpenFrom, const GUID& Guid, INT_PTR Item);
		HANDLE OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode);

		int SetFindList(HANDLE hPlugin, const PluginPanelItem *PanelItem, int ItemsNumber);
		int GetFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
		int GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, const wchar_t *Path);
		int SetDirectory(HANDLE hPlugin, const wchar_t *Dir, int OpMode);
		int GetFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, const wchar_t **DestPath, int OpMode);
		int PutFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
		int DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
		int MakeDirectory(HANDLE hPlugin, const wchar_t **Name, int OpMode);
		int ProcessHostFile(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
		int ProcessKey(HANDLE hPlugin, const INPUT_RECORD *Rec, bool Pred);
		int ProcessEvent(HANDLE hPlugin, int Event, PVOID Param);
		int Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode);

		int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData) { return 0; }
		void FreeCustomData(wchar_t *CustomData) {}

		void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info);
		void FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber);
		void FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber);
		void ClosePanel(HANDLE hPlugin);

		int ProcessEditorInput(const INPUT_RECORD *D);
		int ProcessEditorEvent(int Event, PVOID Param);
		int ProcessViewerEvent(int Event, PVOID Param);
		int ProcessDialogEvent(int Event, PVOID Param);
		int ProcessSynchroEvent(int Event, PVOID Param) { return 0; }
#if defined(PROCPLUGINMACROFUNC)
		int ProcessMacroFunc(const wchar_t *Name, const FarMacroValue *Params, int nParams, FarMacroValue **Results, int *nResults) {return 0;}
#endif

		int Analyse(const AnalyseInfo *Info) { return FALSE; }

		bool GetPluginInfo(PluginInfo *pi);
		int Configure(const GUID& Guid);

		void ExitFAR();
		const wchar_t* GetTitle(void) { return nullptr; }

	private:

		void ClearExports();

		void FreePluginInfo();
		void ConvertPluginInfo(oldfar::PluginInfo &Src, PluginInfo *Dest);
		void FreeOpenPanelInfo();
		void ConvertOpenPanelInfo(oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest);
};
