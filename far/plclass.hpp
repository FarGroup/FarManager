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
#include "windowsfwd.hpp"

class PluginManager;
class Plugin;
class Language;
class PluginsCacheConfig;

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
	iGetContentFields,
	iGetContentData,
	iFreeContentData,

	iOpenFilePlugin,
	iGetMinFarVersion,

	ExportsCount
};

enum PLUGINWORKFLAGS
{
	PIWF_LOADED        = BIT(0), // DLL загружена
	PIWF_CACHED        = BIT(1), // кешируется
	PIWF_PRELOADED     = BIT(2), //
	PIWF_DONTLOADAGAIN = BIT(3), // не загружать плагин снова, ставится в результате проверки требуемой версии фара
	PIWF_DATALOADED    = BIT(4), // LoadData успешно выполнилась
};

extern PluginStartupInfo NativeInfo;
extern FarStandardFunctions NativeFSF;

class GenericPluginModel: noncopyable
{
public:
	typedef void* plugin_instance;
	typedef std::array<void*, ExportsCount> exports_array;

	GenericPluginModel(PluginManager* owner);
	virtual ~GenericPluginModel() {};

	virtual Plugin* CreatePlugin(const string& filename);

	virtual bool IsPlugin(const string& filename) = 0;
	virtual plugin_instance Create(const string& filename) = 0;
	virtual bool Destroy(plugin_instance module) = 0;
	virtual void InitExports(plugin_instance instance, exports_array& exports) = 0;

	void SaveExportsToCache(PluginsCacheConfig& cache, unsigned long long id, const exports_array& exports);
	void LoadExportsFromCache(const PluginsCacheConfig& cache, unsigned long long id, exports_array& exports);

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
	virtual void InitExports(plugin_instance instance, exports_array& exports) override;

private:
	// the rest shouldn't be here, just an optimization for OEM plugins
	bool IsPlugin2(const void* Module);
	virtual bool FindExport(const char* ExportName);
};

class Plugin: noncopyable
{
public:
	Plugin(GenericPluginModel* model, const string& ModuleName);
	virtual ~Plugin();

	virtual bool GetGlobalInfo(GlobalInfo *Info);
	virtual bool SetStartupInfo(PluginStartupInfo *Info);
	virtual void* Open(OpenInfo* Info);
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
	virtual void* Analyse(AnalyseInfo *Info);
	virtual void CloseAnalyse(CloseAnalyseInfo* Info);

	virtual int GetContentFields(GetContentFieldsInfo *Info);
	virtual int GetContentData(GetContentDataInfo *Info);
	virtual void FreeContentData(GetContentDataInfo *Info);

	virtual void* OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode);
	virtual bool CheckMinFarVersion();


#ifndef NO_WRAPPER
	virtual bool IsOemPlugin() const { return false; }
#endif // NO_WRAPPER
	virtual const string& GetHotkeyName() const { return m_strGuid; }

	virtual bool InitLang(const string& Path);
	void CloseLang();
	void* GetOpen() const { return Exports[iOpen]; }

	template<EXPORTS_ENUM N>
	bool has() { static_assert(N != ExportsCount, "Wrong index"); return Exports[N] != nullptr; }

	const string& GetModuleName() const { return m_strModuleName; }
	const string& GetCacheName() const  { return m_strCacheName; }
	const string& GetTitle() const { return strTitle; }
	const string& GetDescription() const { return strDescription; }
	const string& GetAuthor() const { return strAuthor; }
	const VersionInfo& GetVersion() const { return PluginVersion; }
	const VersionInfo& GetMinFarVersion() const { return MinFarVersion; }
	const string& GetVersionString() const { return VersionString; }
	const GUID& GetGUID() const { return m_Guid; }
	bool IsPendingRemove() const { return bPendingRemove; }
	const wchar_t *GetMsg(LNGID nID) const;

	bool CheckWorkFlags(DWORD flags) const { return WorkFlags.Check(flags); }

	bool Load();
	int Unload(bool bExitFAR = false);
	bool LoadData();
	bool LoadFromCache(const os::FAR_FIND_DATA &FindData);
	bool SaveToCache();
	bool IsPanelPlugin();
	bool Active() const {return Activity != 0;}
	void AddDialog(window_ptr_ref Dlg);
	bool RemoveDialog(window_ptr_ref Dlg);

protected:
	struct ExecuteStruct
	{
		ExecuteStruct& operator =(intptr_t value) { Result = value; return *this; }
		ExecuteStruct& operator =(HANDLE value) { Result = reinterpret_cast<intptr_t>(value); return *this; }
		operator intptr_t() const { return Result; }
		operator void*() const { return ToPtr(Result); }

		EXPORTS_ENUM id;

		intptr_t Default, Result;
	};

	void ExecuteFunction(ExecuteStruct& es, const std::function<void()>& f);

	GenericPluginModel::exports_array Exports;

	std::unordered_set<window_ptr> m_dialogs;
	GenericPluginModel *m_model;
	std::unique_ptr<Language> PluginLang;
	size_t Activity;
	bool bPendingRemove;

private:
	friend class PluginManager;
	friend class GenericPluginModel;
	friend class NativePluginModel;

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

	GenericPluginModel::plugin_instance m_Instance;

	VersionInfo MinFarVersion;
	VersionInfo PluginVersion;

	string VersionString;

	GUID m_Guid;
	string m_strGuid;
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
	virtual void InitExports(plugin_instance instance, exports_array& exports) override;

private:
	os::rtdl::module m_Module;
	struct ModuleImports
	{
		os::rtdl::function_pointer<BOOL(WINAPI*)(GlobalInfo* info)> pInitialize;
		os::rtdl::function_pointer<BOOL(WINAPI*)(const wchar_t* filename)> pIsPlugin;
		os::rtdl::function_pointer<HANDLE(WINAPI*)(const wchar_t* filename)> pCreateInstance;
		os::rtdl::function_pointer<FARPROC(WINAPI*)(HANDLE Instance, const wchar_t* functionname)> pGetFunctionAddress;
		os::rtdl::function_pointer<BOOL(WINAPI*)(HANDLE Instance)> pDestroyInstance;
		os::rtdl::function_pointer<void (WINAPI*)(const ExitInfo* info)> pFree;

		ModuleImports(const os::rtdl::module& Module);
	}
	m_Imports;
	bool m_Success;
};

#define EXECUTE_FUNCTION(f) ExecuteFunction(es, [&]{ f; });
#define FUNCTION(id) reinterpret_cast<id##Prototype>(Exports[id])
#define WA(string) {L##string, string}
