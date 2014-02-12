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

#include "bitflags.hpp"

class PluginManager;
class Plugin;
class Language;

enum EXPORTS_ENUM
{
	iGetGlobalInfo,
	iSetStartupInfo,
	iOpen,
	iClosePanel,
	iGetPluginInfo,
	iGetOpenPanelInfo,
	iGetFindData,
	iFreeFindData,
	iGetVirtualFindData,
	iFreeVirtualFindData,
	iSetDirectory,
	iGetFiles,
	iPutFiles,
	iDeleteFiles,
	iMakeDirectory,
	iProcessHostFile,
	iSetFindList,
	iConfigure,
	iExitFAR,
	iProcessPanelInput,
	iProcessPanelEvent,
	iProcessEditorEvent,
	iCompare,
	iProcessEditorInput,
	iProcessViewerEvent,
	iProcessDialogEvent,
	iProcessSynchroEvent,
	iProcessConsoleInput,
	iAnalyse,
	iCloseAnalyse,
	iGetCustomData,
	iFreeCustomData,

	iOpenFilePlugin,
	iGetMinFarVersion,

	ExportsCount
};


extern PluginStartupInfo NativeInfo;
extern FarStandardFunctions NativeFSF;

class GenericPluginModel
{
public:
	typedef void* plugin_instance;

	GenericPluginModel(PluginManager* owner);
	virtual ~GenericPluginModel() {};

	virtual Plugin* CreatePlugin(const string& filename);

	virtual bool IsPlugin(const string& filename) = 0;
	virtual plugin_instance Create(const string& filename) = 0;
	virtual bool Destroy(plugin_instance module) = 0;
	virtual void InitExports(plugin_instance instance, void** exports) = 0;

	void SaveExportsToCache(class PluginsCacheConfig* cache, unsigned long long id, void* const * exports);
	void LoadExportsFromCache(class PluginsCacheConfig* cache, unsigned long long id, void** exports);

	template<int> struct ExportType;

	PluginManager* GetOwner() const { return m_owner; }
	const wchar_t* GetExportName(size_t index) const { return m_ExportsNames[index].UName; }

protected:
	const struct export_name
	{
		const wchar_t* UName;
		const char* AName;
	}
	* m_ExportsNames;
	PluginManager* m_owner;
};

class NativePluginModel : public GenericPluginModel
{
public:
	NativePluginModel(PluginManager* owner) : GenericPluginModel(owner) {}

	virtual bool IsPlugin(const string& filename) override;
	virtual plugin_instance Create(const string& filename) override;
	virtual bool Destroy(plugin_instance module) override;
	virtual void InitExports(plugin_instance instance, void** exports) override;

private:
	// the rest shouldn't be here, just an optimization for OEM plugins
	bool IsPlugin2(const void* Module);
	virtual bool FindExport(const char* ExportName);
};

struct PluginHandle;

class Plugin
{
public:
	Plugin(GenericPluginModel* model, const string& ModuleName);
	virtual ~Plugin();

	virtual bool GetGlobalInfo(GlobalInfo *Info);
	virtual bool SetStartupInfo(PluginStartupInfo *Info);
	virtual PluginHandle* Open(OpenInfo* Info);
	virtual void ClosePanel(ClosePanelInfo* Info);
	virtual bool GetPluginInfo(PluginInfo *pi);
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info);
	virtual int GetFindData(GetFindDataInfo* Info);
	virtual void FreeFindData(FreeFindDataInfo* Info);
	virtual int GetVirtualFindData(GetVirtualFindDataInfo* Info);
	virtual void FreeVirtualFindData(FreeFindDataInfo* Info);
	virtual int SetDirectory(SetDirectoryInfo* Info);
	virtual int GetFiles(GetFilesInfo* Info);
	virtual int PutFiles(PutFilesInfo* Info);
	virtual int DeleteFiles(DeleteFilesInfo* Info);
	virtual int MakeDirectory(MakeDirectoryInfo* Info);
	virtual int ProcessHostFile(ProcessHostFileInfo* Info);
	virtual int SetFindList(SetFindListInfo* Info);
	virtual int Configure(ConfigureInfo* Info);
	virtual void ExitFAR(ExitInfo *Info);
	virtual int ProcessPanelInput(ProcessPanelInputInfo* Info);
	virtual int ProcessPanelEvent(ProcessPanelEventInfo* Info);
	virtual int ProcessEditorEvent(ProcessEditorEventInfo* Info);
	virtual int Compare(CompareInfo* Info);
	virtual int ProcessEditorInput(ProcessEditorInputInfo* Info);
	virtual int ProcessViewerEvent(ProcessViewerEventInfo* Info);
	virtual int ProcessDialogEvent(ProcessDialogEventInfo* Info);
	virtual int ProcessSynchroEvent(ProcessSynchroEventInfo* Info);
	virtual int ProcessConsoleInput(ProcessConsoleInputInfo *Info);
	virtual PluginHandle* Analyse(AnalyseInfo *Info);
	virtual void CloseAnalyse(CloseAnalyseInfo* Info);

	virtual int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData);
	virtual void FreeCustomData(wchar_t *CustomData);

	virtual PluginHandle* OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode);
	virtual bool CheckMinFarVersion();


#ifndef NO_WRAPPER
	virtual bool IsOemPlugin() const { return false; }
#endif // NO_WRAPPER
	virtual const string& GetHotkeyName() const { return m_strGuid; }

	virtual bool InitLang(const string& Path);
	void CloseLang();

	#define HAS_FUNCTION(Name) bool Has##Name() const { return Exports[i##Name] != nullptr; }

	HAS_FUNCTION(GetGlobalInfo)
	HAS_FUNCTION(Open)
	HAS_FUNCTION(MakeDirectory)
	HAS_FUNCTION(DeleteFiles)
	HAS_FUNCTION(PutFiles)
	HAS_FUNCTION(GetFiles)
	HAS_FUNCTION(SetStartupInfo)
	HAS_FUNCTION(ClosePanel)
	HAS_FUNCTION(GetPluginInfo)
	HAS_FUNCTION(GetOpenPanelInfo)
	HAS_FUNCTION(GetFindData)
	HAS_FUNCTION(FreeFindData)
	HAS_FUNCTION(GetVirtualFindData)
	HAS_FUNCTION(FreeVirtualFindData)
	HAS_FUNCTION(SetDirectory)
	HAS_FUNCTION(ProcessHostFile)
	HAS_FUNCTION(SetFindList)
	HAS_FUNCTION(Configure)
	HAS_FUNCTION(ExitFAR)
	HAS_FUNCTION(ProcessPanelInput)
	HAS_FUNCTION(ProcessPanelEvent)
	HAS_FUNCTION(ProcessEditorEvent)
	HAS_FUNCTION(Compare)
	HAS_FUNCTION(ProcessEditorInput)
	HAS_FUNCTION(ProcessViewerEvent)
	HAS_FUNCTION(ProcessDialogEvent)
	HAS_FUNCTION(ProcessSynchroEvent)
	HAS_FUNCTION(ProcessConsoleInput)
	HAS_FUNCTION(Analyse)
	HAS_FUNCTION(CloseAnalyse)
	HAS_FUNCTION(GetCustomData)
	HAS_FUNCTION(FreeCustomData)

	HAS_FUNCTION(OpenFilePlugin)
	HAS_FUNCTION(GetMinFarVersion)

	#undef HAS_FUNCTION

	const string& GetModuleName() const { return m_strModuleName; }
	const string& GetCacheName() const  { return m_strCacheName; }
	const string& GetTitle() const { return strTitle; }
	const string& GetDescription() const { return strDescription; }
	const string& GetAuthor() const { return strAuthor; }
	const VersionInfo& GetVersion() const { return PluginVersion; }
	const VersionInfo& GetMinFarVersion() const { return MinFarVersion; }
	const string& GetVersionString() const { return VersionString; }
	const GUID& GetGUID() const { return m_Guid; }
	const wchar_t *GetMsg(LNGID nID) const;

	bool CheckWorkFlags(DWORD flags) const { return WorkFlags.Check(flags); }
	DWORD GetWorkFlags() const { return WorkFlags.Flags(); }
	DWORD GetFuncFlags() const { return FuncFlags.Flags(); }

	bool Load();
	int Unload(bool bExitFAR = false);
	bool LoadData();
	bool LoadFromCache(const api::FAR_FIND_DATA &FindData);
	bool SaveToCache();
	bool IsPanelPlugin();
	bool Active() const {return Activity != 0;}

protected:
	struct ExecuteStruct
	{
		ExecuteStruct& operator =(intptr_t value) { Result = value; return *this; }
		ExecuteStruct& operator =(HANDLE value) { Result = reinterpret_cast<intptr_t>(value); return *this; }
		operator intptr_t() const { return Result; }
		operator PluginHandle*() const { return reinterpret_cast<PluginHandle*>(Result); }

		EXPORTS_ENUM id;

		intptr_t Default, Result;
	};

	void ExecuteFunction(ExecuteStruct& es, const std::function<void()>& f);

	void* Exports[ExportsCount];

	GenericPluginModel *m_model;
	std::unique_ptr<Language> PluginLang;
	size_t Activity;
	bool bPendingRemove;

private:
	virtual void Prologue() {};
	virtual void Epilogue() {};

	void InitExports();
	void ClearExports();
	void SetGuid(const GUID& Guid);

	string strTitle;
	string strDescription;
	string strAuthor;

	string m_strModuleName;
	string m_strCacheName;

	BitFlags WorkFlags;      // рабочие флаги текущего плагина
	BitFlags FuncFlags;      // битовые маски вызова эксп.функций плагина

	GenericPluginModel::plugin_instance m_Instance;

	VersionInfo MinFarVersion;
	VersionInfo PluginVersion;

	string VersionString;

	GUID m_Guid;
	string m_strGuid;

	friend class PluginManager;
	friend class GenericPluginModel;
	friend class NativePluginModel;
};


class CustomPluginModel : public GenericPluginModel
{
public:
	CustomPluginModel(PluginManager* owner, const string& filename);
	~CustomPluginModel();

	bool Success() const { return m_Success; }

	virtual bool IsPlugin(const string& filename) override;
	virtual plugin_instance Create(const string& filename) override;
	virtual bool Destroy(plugin_instance module) override;
	virtual void InitExports(plugin_instance instance, void** exports) override;

private:
	template<typename T>
	inline bool InitImport(T& Address, const char* ProcName)
	{
		return (Address = reinterpret_cast<T>(GetProcAddress(m_Module, ProcName))) != nullptr;
	}

	struct
	{
		BOOL (WINAPI *pInitialize)(GlobalInfo* info);
		BOOL (WINAPI *pIsPlugin)(const wchar_t* filename);
		HANDLE (WINAPI *pCreateInstance)(const wchar_t* filename);
		FARPROC (WINAPI *pGetFunctionAddress)(HANDLE Instance, const wchar_t* functionname);
		BOOL (WINAPI *pDestroyInstance)(HANDLE Instance);
		void (WINAPI *pFree)(const ExitInfo* info);
	}
	Imports;
	HMODULE m_Module;
	bool m_Success;
};

#define EXECUTE_FUNCTION(f) ExecuteFunction(es, [&]{ f; });
#define FUNCTION(id) reinterpret_cast<id##Prototype>(Exports[id])
#define WA(string) {L##string, string}
