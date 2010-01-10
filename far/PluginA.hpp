#pragma once

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

class PluginManager;

#include "language.hpp"
#include "bitflags.hpp"
#include "plugin.hpp"
#include "plclass.hpp"
#include "pluginold.hpp"

typedef void (WINAPI *PLUGINCLOSEPLUGIN)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPARE)(HANDLE hPlugin,const oldfar::PluginPanelItem *Item1,const oldfar::PluginPanelItem *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGURE)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILES)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFAR)();
typedef void (WINAPI *PLUGINFREEFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILES)(HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSION)();
typedef void (WINAPI *PLUGINGETOPENPLUGININFO)(HANDLE hPlugin,oldfar::OpenPluginInfo *Info);
typedef void (WINAPI *PLUGINGETPLUGININFO)(oldfar::PluginInfo *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATA)(HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORY)(HANDLE hPlugin,char *Name,int OpMode);
typedef HANDLE(WINAPI *PLUGINOPENFILEPLUGIN)(char *Name,const unsigned char *Data,int DataSize);
typedef HANDLE(WINAPI *PLUGINOPENPLUGIN)(int OpenFrom,INT_PTR Item);
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

		/* $ 21.09.2000 SVS
		   поле - системный идентификатор плагина
		   Плагин должен сам задавать, например для
		   Network      = 0x5774654E (NetW)
		   PrintManager = 0x6E614D50 (PMan)  SYSID_PRINTMANAGER
		*/
		DWORD SysID;

		string strRootKey;
		char *RootKey;

		PluginInfo PI;
		OpenPluginInfo OPI;

		oldfar::PluginPanelItem  *pFDPanelItemA;
		oldfar::PluginPanelItem  *pVFDPanelItemA;

		PLUGINSETSTARTUPINFO        pSetStartupInfo;
		PLUGINOPENPLUGIN            pOpenPlugin;
		PLUGINOPENFILEPLUGIN        pOpenFilePlugin;
		PLUGINCLOSEPLUGIN           pClosePlugin;
		PLUGINGETPLUGININFO         pGetPluginInfo;
		PLUGINGETOPENPLUGININFO     pGetOpenPluginInfo;
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

		bool Load();
		bool LoadFromCache(const FAR_FIND_DATA_EX &FindData);

		bool SaveToCache();

		int Unload(bool bExitFAR = false);

		bool IsPanelPlugin();

		bool HasOpenPlugin() { return pOpenPlugin!=NULL; }
		bool HasMakeDirectory() { return pMakeDirectory!=NULL; }
		bool HasDeleteFiles() { return pDeleteFiles!=NULL; }
		bool HasPutFiles() { return pPutFiles!=NULL; }
		bool HasGetFiles() { return pGetFiles!=NULL; }
		bool HasSetStartupInfo() { return pSetStartupInfo!=NULL; }
		bool HasOpenFilePlugin() { return pOpenFilePlugin!=NULL; }
		bool HasClosePlugin() { return pClosePlugin!=NULL; }
		bool HasGetPluginInfo() { return pGetPluginInfo!=NULL; }
		bool HasGetOpenPluginInfo() { return pGetOpenPluginInfo!=NULL; }
		bool HasGetFindData() { return pGetFindData!=NULL; }
		bool HasFreeFindData() { return pFreeFindData!=NULL; }
		bool HasGetVirtualFindData() { return pGetVirtualFindData!=NULL; }
		bool HasFreeVirtualFindData() { return pFreeVirtualFindData!=NULL; }
		bool HasSetDirectory() { return pSetDirectory!=NULL; }
		bool HasProcessHostFile() { return pProcessHostFile!=NULL; }
		bool HasSetFindList() { return pSetFindList!=NULL; }
		bool HasConfigure() { return pConfigure!=NULL; }
		bool HasExitFAR() { return pExitFAR!=NULL; }
		bool HasProcessKey() { return pProcessKey!=NULL; }
		bool HasProcessEvent() { return pProcessEvent!=NULL; }
		bool HasProcessEditorEvent() { return pProcessEditorEvent!=NULL; }
		bool HasCompare() { return pCompare!=NULL; }
		bool HasProcessEditorInput() { return pProcessEditorInput!=NULL; }
		bool HasMinFarVersion() { return pMinFarVersion!=NULL; }
		bool HasProcessViewerEvent() { return pProcessViewerEvent!=NULL; }
		bool HasProcessDialogEvent() { return pProcessDialogEvent!=NULL; }
		bool HasProcessSynchroEvent() { return false; }
		bool HasAnalyse() { return false; }

		const string &GetModuleName() { return m_strModuleName; }
		const wchar_t *GetCacheName() { return m_strCacheName; }
		DWORD GetSysID() { return SysID; }
		bool CheckWorkFlags(DWORD flags) { return WorkFlags.Check(flags)==TRUE; }
		DWORD GetWorkFlags() { return WorkFlags.Flags; }
		DWORD GetFuncFlags() { return FuncFlags.Flags; }

		bool InitLang(const wchar_t *Path) { return Lang.Init(Path,false); }
		void CloseLang() { Lang.Close(); }
		const char *GetMsgA(int nID) { return Lang.GetMsgA(nID); }

	public:

		bool SetStartupInfo(bool &bUnloaded);
		bool CheckMinFarVersion(bool &bUnloaded);

		HANDLE OpenPlugin(int OpenFrom, INT_PTR Item);
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
		int ProcessKey(HANDLE hPlugin, int Key, unsigned int dwControlState);
		int ProcessEvent(HANDLE hPlugin, int Event, PVOID Param);
		int Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode);

		void GetOpenPluginInfo(HANDLE hPlugin, OpenPluginInfo *Info);
		void FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber);
		void FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber);
		void ClosePlugin(HANDLE hPlugin);

		int ProcessEditorInput(const INPUT_RECORD *D);
		int ProcessEditorEvent(int Event, PVOID Param);
		int ProcessViewerEvent(int Event, PVOID Param);
		int ProcessDialogEvent(int Event, PVOID Param);

		int ProcessSynchroEvent(int Event, PVOID Param) {return 0;};

		bool GetPluginInfo(PluginInfo *pi);
		int Configure(int MenuItem);

		void ExitFAR();

	private:

		void ClearExports();

		void FreePluginInfo();
		void ConvertPluginInfo(oldfar::PluginInfo &Src, PluginInfo *Dest);
		void FreeOpenPluginInfo();
		void ConvertOpenPluginInfo(oldfar::OpenPluginInfo &Src, OpenPluginInfo *Dest);
};
