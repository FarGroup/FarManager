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

#include "language.hpp"
#include "bitflags.hpp"
#include "plugin.hpp"

class AncientPlugin
{
	public:
		virtual ~AncientPlugin() {}
		virtual const GUID& GetGUID(void) = 0;
};

class PluginManager;
struct ExecuteStruct
{
	int id; //function id
	union
	{
		INT_PTR nResult;
		HANDLE hResult;
		BOOL bResult;
	};

	union
	{
		INT_PTR nDefaultResult;
		HANDLE hDefaultResult;
		BOOL bDefaultResult;
	};

	bool bUnloaded;
};

#define EXECUTE_FUNCTION(function, es) \
{ \
	__Prolog(); \
	es.nResult = 0; \
	es.nDefaultResult = 0; \
	es.bUnloaded = false; \
	if ( Opt.ExceptRules ) \
	{ \
		__try \
		{ \
			function; \
		} \
		__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
		{ \
			m_owner->UnloadPlugin(this, es.id, true); \
			es.bUnloaded = true; \
			es.nResult = es.nDefaultResult; \
			ProcessException=FALSE; \
		} \
	} \
	else \
	{ \
		function; \
	} \
	__Epilog(); \
}

#define EXECUTE_FUNCTION_EX(function, es) EXECUTE_FUNCTION(es.nResult = (INT_PTR)function, es)

class Plugin: public AncientPlugin
{

protected:
		PluginManager *m_owner; //BUGBUG

		string m_strModuleName;
		string m_strCacheName;

		BitFlags WorkFlags;      // рабочие флаги текущего плагина
		BitFlags FuncFlags;      // битовые маски вызова эксп.функций плагина

		HMODULE m_hModule;
		Language PluginLang;

		VersionInfo MinFarVersion;
		VersionInfo PluginVersion;
		string strTitle;
		string strDescription;
		string strAuthor;

		GUID m_Guid;
		string m_strGuid;

		virtual void InitExports() = 0;
		virtual void ClearExports() = 0;
		virtual void ReadCache(unsigned __int64 id) = 0;

		void SetGuid(const GUID& Guid);

		virtual void __Prolog() {};
		virtual void __Epilog() {};


	public:

		Plugin(PluginManager *owner, const wchar_t *lpwszModuleName);
		virtual ~Plugin();

		bool Load();
		int Unload(bool bExitFAR = false);
		bool LoadData();
		bool LoadFromCache(const FAR_FIND_DATA_EX &FindData);

		virtual bool IsOemPlugin() = 0;

		virtual bool SaveToCache() = 0;

		virtual bool IsPanelPlugin() = 0;

		virtual bool HasGetGlobalInfo() = 0;
		virtual bool HasOpenPanel() = 0;
		virtual bool HasMakeDirectory() = 0;
		virtual bool HasDeleteFiles() = 0;
		virtual bool HasPutFiles() = 0;
		virtual bool HasGetFiles() = 0;
		virtual bool HasSetStartupInfo() = 0;
		virtual bool HasOpenFilePlugin() = 0;
		virtual bool HasClosePanel() = 0;
		virtual bool HasGetPluginInfo() = 0;
		virtual bool HasGetOpenPanelInfo() = 0;
		virtual bool HasGetFindData() = 0;
		virtual bool HasFreeFindData() = 0;
		virtual bool HasGetVirtualFindData() = 0;
		virtual bool HasFreeVirtualFindData() = 0;
		virtual bool HasSetDirectory() = 0;
		virtual bool HasProcessHostFile() = 0;
		virtual bool HasSetFindList() = 0;
		virtual bool HasConfigure() = 0;
		virtual bool HasExitFAR() = 0;
		virtual bool HasProcessKey() = 0;
		virtual bool HasProcessEvent() = 0;
		virtual bool HasProcessEditorEvent() = 0;
		virtual bool HasCompare() = 0;
		virtual bool HasProcessEditorInput() = 0;
		virtual bool HasMinFarVersion() = 0;
		virtual bool HasProcessViewerEvent() = 0;
		virtual bool HasProcessDialogEvent() = 0;
		virtual bool HasProcessSynchroEvent() = 0;
		virtual bool HasAnalyse() = 0;
		virtual bool HasGetCustomData() = 0;
		virtual bool HasFreeCustomData() = 0;
#if defined(PROCPLUGINMACROFUNC)
		virtual bool HasProcessMacroFunc() = 0;
#endif

		virtual const string &GetModuleName() = 0;
		virtual const wchar_t *GetCacheName() = 0;
		virtual const wchar_t *GetHotkeyName() = 0;
		virtual bool CheckWorkFlags(DWORD flags) = 0;
		virtual DWORD GetWorkFlags() = 0;
		virtual DWORD GetFuncFlags() = 0;

		virtual bool InitLang(const wchar_t *Path) = 0;
		virtual void CloseLang() = 0;

		virtual bool GetGlobalInfo(GlobalInfo *Info) = 0;

		virtual bool SetStartupInfo(bool &bUnloaded) = 0;
		virtual bool CheckMinFarVersion(bool &bUnloaded) = 0;

		virtual HANDLE Open(int OpenFrom, const GUID& Guid, INT_PTR Item) = 0;
		virtual HANDLE OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode) = 0;

		virtual int SetFindList(HANDLE hPlugin, const PluginPanelItem *PanelItem, int ItemsNumber) = 0;
		virtual int GetFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) = 0;
		virtual int GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, const wchar_t *Path) = 0;
		virtual int SetDirectory(HANDLE hPlugin, const wchar_t *Dir, int OpMode) = 0;
		virtual int GetFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, const wchar_t **DestPath, int OpMode) = 0;
		virtual int PutFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode) = 0;
		virtual int DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode) = 0;
		virtual int MakeDirectory(HANDLE hPlugin, const wchar_t **Name, int OpMode) = 0;
		virtual int ProcessHostFile(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode) = 0;
		virtual int ProcessKey(HANDLE hPlugin, const INPUT_RECORD *Rec, bool Pred) = 0;
		virtual int ProcessEvent(HANDLE hPlugin, int Event, PVOID Param) = 0;
		virtual int Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode) = 0;

		virtual int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData) = 0;
		virtual void FreeCustomData(wchar_t *CustomData) = 0;

		virtual void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info) = 0;
		virtual void FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) = 0;
		virtual void FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) = 0;
		virtual void ClosePanel(HANDLE hPlugin) = 0;

		virtual int ProcessEditorInput(const INPUT_RECORD *D) = 0;
		virtual int ProcessEditorEvent(int Event, PVOID Param) = 0;
		virtual int ProcessViewerEvent(int Event, PVOID Param) = 0;
		virtual int ProcessDialogEvent(int Event, PVOID Param) = 0;
		virtual int ProcessSynchroEvent(int Event, PVOID Param) = 0;
#if defined(PROCPLUGINMACROFUNC)
		virtual int ProcessMacroFunc(const wchar_t *Name, const FarMacroValue *Params, int nParams, FarMacroValue **Results, int *nResults) = 0;
#endif

		virtual int Analyse(const AnalyseInfo *Info) = 0;

		virtual bool GetPluginInfo(PluginInfo *pi) = 0;
		virtual int Configure(const GUID& Guid) = 0;

		virtual void ExitFAR() = 0;
		virtual	const wchar_t* GetTitle(void) = 0;
};
