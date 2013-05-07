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

#ifndef NO_WRAPPER

#include "plclass.hpp"
namespace wrapper
{

#include "pluginold.hpp"

class PluginA: public Plugin
{
public:

	PluginA(PluginManager *owner, const string& ModuleName);
	~PluginA();

	virtual bool GetGlobalInfo(GlobalInfo *Info) override;
	virtual bool SetStartupInfo() override;
	virtual bool CheckMinFarVersion() override;
	virtual HANDLE Open(int OpenFrom, const GUID& Guid, intptr_t Item) override;
	virtual HANDLE OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode) override;
	virtual int SetFindList(HANDLE hPlugin, const PluginPanelItem *PanelItem, size_t ItemsNumber) override;
	virtual int GetFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, int OpMode) override;
	virtual int GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, const string& Path) override;
	virtual int SetDirectory(HANDLE hPlugin, const string& Dir, int OpMode,struct UserDataItem *UserData=nullptr) override;
	virtual int GetFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, const wchar_t** DestPath, int OpMode) override;
	virtual int PutFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, int OpMode) override;
	virtual int DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode) override;
	virtual int MakeDirectory(HANDLE hPlugin, const wchar_t **Name, int OpMode) override;
	virtual int ProcessHostFile(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode) override;
	virtual int ProcessKey(HANDLE hPlugin, const INPUT_RECORD *Rec, bool Pred) override;
	virtual int ProcessPanelEvent(HANDLE hPlugin, int Event, PVOID Param) override;
	virtual int Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode) override;
	virtual int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData) override { return 0; }
	virtual void FreeCustomData(wchar_t *CustomData) override {}
	virtual void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info) override;
	virtual void FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool FreeUserData) override;
	virtual void FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber) override;
	virtual void ClosePanel(HANDLE hPlugin) override;
	virtual int ProcessEditorInput(const INPUT_RECORD *D) override;
	virtual int ProcessEditorEvent(int Event, PVOID Param,int EditorID) override;
	virtual int ProcessViewerEvent(int Event, PVOID Param,int ViewerID) override;
	virtual int ProcessDialogEvent(int Event, FarDialogEvent *Param) override;
	virtual int ProcessSynchroEvent(int Event, PVOID Param) override { return 0; }
	virtual int ProcessConsoleInput(ProcessConsoleInputInfo *Info) override {return 0;}
	virtual HANDLE Analyse(const AnalyseInfo *Info) override { return nullptr; }
	virtual void CloseAnalyse(HANDLE hHandle) override {}
	virtual bool GetPluginInfo(PluginInfo *pi) override;
	virtual int Configure(const GUID& Guid) override;
	virtual void ExitFAR(const ExitInfo *Info) override;

	virtual bool IsOemPlugin() const override { return true; }
	virtual const string& GetHotkeyName() const override { return GetCacheName(); }

	virtual bool InitLang(const string& Path) override { return PluginLang.InitA(Path); }
	const char *GetMsgA(LNGID nID) const { return PluginLang.GetMsgA(nID); }

private:
	virtual void __Prolog() override { SetFileApisToOEM(); OEMApiCnt++; }
	virtual void __Epilog() override { OEMApiCnt--; if(!OEMApiCnt) SetFileApisToANSI(); }

	void FreePluginInfo();
	void ConvertPluginInfo(oldfar::PluginInfo &Src, PluginInfo *Dest);
	void FreeOpenPanelInfo();
	void ConvertOpenPanelInfo(oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest);

	string strRootKey;
	char *RootKey;

	PluginInfo PI;
	OpenPanelInfo OPI;

	oldfar::PluginPanelItem  *pFDPanelItemA;
	oldfar::PluginPanelItem  *pVFDPanelItemA;

	UINT64 OEMApiCnt;

	bool opif_shortcut;
	wchar_t_ptr p_buff;
};

void LocalUpperInit();

};
#endif // NO_WRAPPER
