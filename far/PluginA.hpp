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

class OEMPluginModel : public NativePluginModel
{
public:
	OEMPluginModel(PluginManager* owner);

	virtual Plugin* CreatePlugin(const string& filename) override;
	std::string getUserName();

private:
	virtual bool FindExport(const char* ExportName) override;
	std::string m_userName;
};

class file_version;

// TODO: PluginA class shouldn't be derived from Plugin.
// All exports should be provided by OEMPluginModel.
class PluginA: public Plugin
{
public:
	PluginA(OEMPluginModel* model, const string& ModuleName);
	~PluginA();

	virtual bool GetGlobalInfo(GlobalInfo *Info) override;
	virtual bool SetStartupInfo(PluginStartupInfo* Info) override;
	virtual PluginHandle* Open(OpenInfo* OpenInfo) override;
	virtual void ClosePanel(ClosePanelInfo* Info) override;
	virtual bool GetPluginInfo(PluginInfo *pi) override;
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info) override;
	virtual int GetFindData(GetFindDataInfo* Info) override;
	virtual void FreeFindData(FreeFindDataInfo* Info) override;
	virtual int GetVirtualFindData(GetVirtualFindDataInfo* Info) override;
	virtual void FreeVirtualFindData(FreeFindDataInfo* Info) override;
	virtual int SetDirectory(SetDirectoryInfo* Info) override;
	virtual int GetFiles(GetFilesInfo* Info) override;
	virtual int PutFiles(PutFilesInfo* Info) override;
	virtual int DeleteFiles(DeleteFilesInfo* Info) override;
	virtual int MakeDirectory(MakeDirectoryInfo* Info) override;
	virtual int ProcessHostFile(ProcessHostFileInfo* Info) override;
	virtual int SetFindList(SetFindListInfo* Info) override;
	virtual int Configure(ConfigureInfo* Info) override;
	virtual void ExitFAR(ExitInfo *Info) override;
	virtual int ProcessPanelInput(ProcessPanelInputInfo* Info) override;
	virtual int ProcessPanelEvent(ProcessPanelEventInfo* Info) override;
	virtual int ProcessEditorEvent(ProcessEditorEventInfo* Info) override;
	virtual int Compare(CompareInfo* Info) override;
	virtual int ProcessEditorInput(ProcessEditorInputInfo* Info) override;
	virtual int ProcessViewerEvent(ProcessViewerEventInfo* Info) override;
	virtual int ProcessDialogEvent(ProcessDialogEventInfo* Info) override;
	virtual int ProcessSynchroEvent(ProcessSynchroEventInfo* Info) override { return 0; }
	virtual int ProcessConsoleInput(ProcessConsoleInputInfo *Info) override { return 0; }
	virtual PluginHandle* Analyse(AnalyseInfo *Info) override { return nullptr; }
	virtual void CloseAnalyse(CloseAnalyseInfo* Info) override {}

	virtual int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData) override { return 0; }
	virtual void FreeCustomData(wchar_t *CustomData) override {}

	virtual PluginHandle* OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode) override;
	virtual bool CheckMinFarVersion() override;


	virtual bool IsOemPlugin() const override { return true; }
	virtual const string& GetHotkeyName() const override { return GetCacheName(); }

	virtual bool InitLang(const string& Path) override { return PluginLang.InitA(Path); }
	const char *GetMsgA(LNGID nID) const { return PluginLang.GetMsgA(nID); }

private:
	virtual void Prologue() override { SetFileApisToOEM(); OEMApiCnt++; }
	virtual void Epilogue() override { OEMApiCnt--; if(!OEMApiCnt) SetFileApisToANSI(); }

	void FreePluginInfo();
	void ConvertPluginInfo(const oldfar::PluginInfo &Src, PluginInfo *Dest);
	void FreeOpenPanelInfo();
	void ConvertOpenPanelInfo(const oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest);

	PluginInfo PI;
	OpenPanelInfo OPI;

	oldfar::PluginPanelItem  *pFDPanelItemA;
	oldfar::PluginPanelItem  *pVFDPanelItemA;

	UINT64 OEMApiCnt;

	bool opif_shortcut;
	std::unique_ptr<file_version> FileVersion;
};

};
#endif // NO_WRAPPER
