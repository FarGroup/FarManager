#ifndef PLCLASS_HPP_E324EC16_24F2_4402_BA87_74212799246D
#define PLCLASS_HPP_E324EC16_24F2_4402_BA87_74212799246D
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

class plugin_factory: noncopyable
{
public:
	using plugin_instance = void*;
	using function_address = void*;
	using exports_array = std::array<std::pair<function_address, bool>, ExportsCount>;

	struct export_name
	{
		const wchar_t* UName;
		const char* AName;
	};

	plugin_factory(PluginManager* owner);
	virtual ~plugin_factory() = default;

	virtual std::unique_ptr<Plugin> CreatePlugin(const string& filename);

	virtual bool IsPlugin(const string& filename) const = 0;
	virtual plugin_instance Create(const string& filename) = 0;
	virtual bool Destroy(plugin_instance module) = 0;
	virtual function_address GetFunction(plugin_instance Instance, const export_name& Name) = 0;

	PluginManager* GetOwner() const { return m_owner; }
	const auto& ExportsNames() const { return m_ExportsNames; }

protected:
	range<const export_name*> m_ExportsNames;
	PluginManager* const m_owner;
};

using plugin_factory_ptr = std::unique_ptr<plugin_factory>;

class native_plugin_factory : public plugin_factory
{
public:
	native_plugin_factory(PluginManager* owner) : plugin_factory(owner) {}

	virtual bool IsPlugin(const string& filename) const override;
	virtual plugin_instance Create(const string& filename) override;
	virtual bool Destroy(plugin_instance module) override;
	virtual function_address GetFunction(plugin_instance Instance, const export_name& Name) override;

private:
	// the rest shouldn't be here, just an optimization for OEM plugins
	bool IsPlugin2(const void* Module) const;
	virtual bool FindExport(const char* ExportName) const;
};

template<EXPORTS_ENUM id>
struct prototype;

template<EXPORTS_ENUM id>
using prototype_t = typename prototype<id>::type;

namespace detail
{
	struct ExecuteStruct
	{
		ExecuteStruct& operator =(intptr_t value) { Result = value; return *this; }
		ExecuteStruct& operator =(HANDLE value) { Result = reinterpret_cast<intptr_t>(value); return *this; }
		operator intptr_t() const { return Result; }
		operator void*() const { return ToPtr(Result); }

		EXPORTS_ENUM id;

		intptr_t Default, Result;
	};

	template<typename function, typename... args, ENABLE_IF(std::is_void<std::result_of_t<function(args...)>>::value)>
	void ExecuteFunctionImpl(ExecuteStruct&, const function& Function, args&&... Args)
	{
		Function(std::forward<args>(Args)...);
	}

	template<typename function, typename... args, ENABLE_IF(!std::is_void<std::result_of_t<function(args...)>>::value)>
	void ExecuteFunctionImpl(ExecuteStruct& es, const function& Function, args&&... Args)
	{
		es = Function(std::forward<args>(Args)...);
	}
}

class Plugin: noncopyable
{
public:
	Plugin(plugin_factory* Factory, const string& ModuleName);
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

	virtual int GetContentFields(const GetContentFieldsInfo *Info);
	virtual int GetContentData(GetContentDataInfo *Info);
	virtual void FreeContentData(const GetContentDataInfo *Info);

	virtual void* OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode);
	virtual bool CheckMinFarVersion();


#ifndef NO_WRAPPER
	virtual bool IsOemPlugin() const { return false; }
#endif // NO_WRAPPER
	virtual const string& GetHotkeyName() const { return m_strGuid; }

	virtual bool InitLang(const string& Path);
	void CloseLang();

	bool has(size_t ExportIndex) const { return Exports[ExportIndex].second; }

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
	using ExecuteStruct = detail::ExecuteStruct;

	template<EXPORTS_ENUM id, class... args>
	void ExecuteFunction(ExecuteStruct& es, args&&... Args)
	{
		Prologue(); ++Activity;
		SCOPE_EXIT{ --Activity; Epilogue(); };

		try
		{
			detail::ExecuteFunctionImpl(es, reinterpret_cast<prototype_t<id>>(Exports[id].first), std::forward<args>(Args)...);
		}
		catch (const SException &e)
		{
			ProcessSEHException(e.GetInfo(), m_Factory->ExportsNames()[es.id].UName, this)? HandleFailure(es) : throw;
		}
		catch (const std::exception &e)
		{
			ProcessStdException(e, m_Factory->ExportsNames()[es.id].UName, this)? HandleFailure(es) : throw;
		}
	}

	void HandleFailure(ExecuteStruct& es);

	virtual void Prologue() {}
	virtual void Epilogue() {}

	plugin_factory::exports_array Exports;

	std::unordered_set<window_ptr> m_dialogs;
	plugin_factory* m_Factory;
	std::unique_ptr<Language> PluginLang;
	size_t Activity;
	bool bPendingRemove;

private:
	friend class PluginManager;
	friend class plugin_factory;
	friend class native_plugin_factory;

	void InitExports();
	void ClearExports();
	void SetGuid(const GUID& Guid);

	string strTitle;
	string strDescription;
	string strAuthor;

	string m_strModuleName;
	string m_strCacheName;

	BitFlags WorkFlags;      // рабочие флаги текущего плагина

	plugin_factory::plugin_instance m_Instance;

	VersionInfo MinFarVersion;
	VersionInfo PluginVersion;

	string VersionString;

	GUID m_Guid;
	string m_strGuid;
};

plugin_factory_ptr CreateCustomPluginFactory(PluginManager* Owner, const string& Filename);

#define DECLARE_PLUGIN_FUNCTION(name, signature) template<> struct prototype<name>  { using type = signature; };
#define WA(string) {L##string, string}

#endif // PLCLASS_HPP_E324EC16_24F2_4402_BA87_74212799246D
