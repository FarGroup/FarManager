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


typedef int (WINAPI *PLUGINANALYSEW)(const AnalyseData *pData);

typedef void (WINAPI *PLUGINCLOSEPLUGINW)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPAREW)(HANDLE hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGUREW)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILESW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFARW)();
typedef void (WINAPI *PLUGINFREEFINDDATAW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATAW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILESW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATAW)(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSIONW)();
typedef void (WINAPI *PLUGINGETOPENPLUGININFOW)(HANDLE hPlugin,OpenPluginInfo *Info);
typedef void (WINAPI *PLUGINGETPLUGININFOW)(PluginInfo *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATAW)(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORYW)(HANDLE hPlugin,const wchar_t **Name,int OpMode);
typedef HANDLE(WINAPI *PLUGINOPENFILEPLUGINW)(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
typedef HANDLE(WINAPI *PLUGINOPENPLUGINW)(int OpenFrom,INT_PTR Item);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENTW)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUTW)(const INPUT_RECORD *Rec);
typedef int (WINAPI *PLUGINPROCESSEVENTW)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSHOSTFILEW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINPROCESSKEYW)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPUTFILESW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode);
typedef int (WINAPI *PLUGINSETDIRECTORYW)(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLISTW)(HANDLE hPlugin,const PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINSETSTARTUPINFOW)(const PluginStartupInfo *Info);
typedef int (WINAPI *PLUGINPROCESSVIEWEREVENTW)(int Event,void *Param); //* $ 27.09.2000 SVS -  События во вьювере
typedef int (WINAPI *PLUGINPROCESSDIALOGEVENTW)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSSYNCHROEVENTW)(int Event,void *Param);




class PluginW: public Plugin
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

		PLUGINSETSTARTUPINFOW        pSetStartupInfoW;
		PLUGINOPENPLUGINW            pOpenPluginW;
		PLUGINOPENFILEPLUGINW        pOpenFilePluginW;
		PLUGINCLOSEPLUGINW           pClosePluginW;
		PLUGINGETPLUGININFOW         pGetPluginInfoW;
		PLUGINGETOPENPLUGININFOW     pGetOpenPluginInfoW;
		PLUGINGETFINDDATAW           pGetFindDataW;
		PLUGINFREEFINDDATAW          pFreeFindDataW;
		PLUGINGETVIRTUALFINDDATAW    pGetVirtualFindDataW;
		PLUGINFREEVIRTUALFINDDATAW   pFreeVirtualFindDataW;
		PLUGINSETDIRECTORYW          pSetDirectoryW;
		PLUGINGETFILESW              pGetFilesW;
		PLUGINPUTFILESW              pPutFilesW;
		PLUGINDELETEFILESW           pDeleteFilesW;
		PLUGINMAKEDIRECTORYW         pMakeDirectoryW;
		PLUGINPROCESSHOSTFILEW       pProcessHostFileW;
		PLUGINSETFINDLISTW           pSetFindListW;
		PLUGINCONFIGUREW             pConfigureW;
		PLUGINEXITFARW               pExitFARW;
		PLUGINPROCESSKEYW            pProcessKeyW;
		PLUGINPROCESSEVENTW          pProcessEventW;
		PLUGINPROCESSEDITOREVENTW    pProcessEditorEventW;
		PLUGINCOMPAREW               pCompareW;
		PLUGINPROCESSEDITORINPUTW    pProcessEditorInputW;
		PLUGINMINFARVERSIONW         pMinFarVersionW;
		PLUGINPROCESSVIEWEREVENTW    pProcessViewerEventW;
		PLUGINPROCESSDIALOGEVENTW    pProcessDialogEventW;
		PLUGINPROCESSSYNCHROEVENTW   pProcessSynchroEventW;
		PLUGINANALYSEW               pAnalyseW;

	public:

		PluginW(PluginManager *owner, const wchar_t *lpwzModuleName);
		~PluginW();

		bool IsOemPlugin() {return false;}

		bool Load();
		bool LoadFromCache(const FAR_FIND_DATA_EX &FindData);

		bool SaveToCache();

		int Unload(bool bExitFAR = false);

		bool IsPanelPlugin();

		bool HasOpenPlugin() { return pOpenPluginW!=nullptr; }
		bool HasMakeDirectory() { return pMakeDirectoryW!=nullptr; }
		bool HasDeleteFiles() { return pDeleteFilesW!=nullptr; }
		bool HasPutFiles() { return pPutFilesW!=nullptr; }
		bool HasGetFiles() { return pGetFilesW!=nullptr; }
		bool HasSetStartupInfo() { return pSetStartupInfoW!=nullptr; }
		bool HasOpenFilePlugin() { return pOpenFilePluginW!=nullptr; }
		bool HasClosePlugin() { return pClosePluginW!=nullptr; }
		bool HasGetPluginInfo() { return pGetPluginInfoW!=nullptr; }
		bool HasGetOpenPluginInfo() { return pGetOpenPluginInfoW!=nullptr; }
		bool HasGetFindData() { return pGetFindDataW!=nullptr; }
		bool HasFreeFindData() { return pFreeFindDataW!=nullptr; }
		bool HasGetVirtualFindData() { return pGetVirtualFindDataW!=nullptr; }
		bool HasFreeVirtualFindData() { return pFreeVirtualFindDataW!=nullptr; }
		bool HasSetDirectory() { return pSetDirectoryW!=nullptr; }
		bool HasProcessHostFile() { return pProcessHostFileW!=nullptr; }
		bool HasSetFindList() { return pSetFindListW!=nullptr; }
		bool HasConfigure() { return pConfigureW!=nullptr; }
		bool HasExitFAR() { return pExitFARW!=nullptr; }
		bool HasProcessKey() { return pProcessKeyW!=nullptr; }
		bool HasProcessEvent() { return pProcessEventW!=nullptr; }
		bool HasProcessEditorEvent() { return pProcessEditorEventW!=nullptr; }
		bool HasCompare() { return pCompareW!=nullptr; }
		bool HasProcessEditorInput() { return pProcessEditorInputW!=nullptr; }
		bool HasMinFarVersion() { return pMinFarVersionW!=nullptr; }
		bool HasProcessViewerEvent() { return pProcessViewerEventW!=nullptr; }
		bool HasProcessDialogEvent() { return pProcessDialogEventW!=nullptr; }
		bool HasProcessSynchroEvent() { return pProcessSynchroEventW!=nullptr; }
		bool HasAnalyse() { return pAnalyseW!=nullptr; }

		const string &GetModuleName() { return m_strModuleName; }
		const wchar_t *GetCacheName() { return m_strCacheName; }
		DWORD GetSysID() { return SysID; }
		bool CheckWorkFlags(DWORD flags) { return WorkFlags.Check(flags)==TRUE; }
		DWORD GetWorkFlags() { return WorkFlags.Flags; }
		DWORD GetFuncFlags() { return FuncFlags.Flags; }

		bool InitLang(const wchar_t *Path) { return Lang.Init(Path,true); }
		void CloseLang() { Lang.Close(); }
		const wchar_t *GetMsg(int nID) { return Lang.GetMsg(nID); }

	public:

		bool SetStartupInfo(bool &bUnloaded);
		bool CheckMinFarVersion(bool &bUnloaded);

		int Analyse(const AnalyseData *pData);

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
		int ProcessSynchroEvent(int Event, PVOID Param);

		bool GetPluginInfo(PluginInfo *pi);
		int Configure(int MenuItem);

		void ExitFAR();

	private:

		void ClearExports();
};
