#ifndef PLCLASS_HPP_E324EC16_24F2_4402_BA87_74212799246D
#define PLCLASS_HPP_E324EC16_24F2_4402_BA87_74212799246D
#pragma once

/*
plclass.hpp
*/
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

// Internal:
#include "bitflags.hpp"
#include "windowsfwd.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fwd.hpp"

// Common:
#include "common.hpp"
#include "common/function_ref.hpp"
#include "common/range.hpp"
#include "common/smart_ptr.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum class lng : int;
class PluginManager;
class Plugin;
class language;
class PluginsCacheConfig;

std::exception_ptr& GlobalExceptionPtr();

enum export_index
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
	PIWF_LOADED        = 0_bit, // DLL загружена
	PIWF_CACHED        = 1_bit, // кешируется
	PIWF_PRELOADED     = 2_bit, //
	PIWF_DONTLOADAGAIN = 3_bit, // не загружать плагин снова, ставится в результате проверки требуемой версии фара
	PIWF_DATALOADED    = 4_bit, // LoadData успешно выполнилась
};

class i_plugin_module
{
public:
	virtual ~i_plugin_module() = default;
	virtual void* opaque() const = 0;
};

class plugin_factory
{
public:
	explicit plugin_factory(PluginManager* owner);
	using plugin_module_ptr = std::unique_ptr<i_plugin_module>;
	using function_address = void*;
	using exports_array = std::array<function_address, ExportsCount>;

	struct export_name
	{
		string_view UName;
		std::string_view AName;
	};

	virtual ~plugin_factory() = default;

	virtual std::unique_ptr<Plugin> CreatePlugin(const string& filename);
	virtual bool IsPlugin(const string& filename) const = 0;
	virtual plugin_module_ptr Create(const string& filename) = 0;
	virtual bool Destroy(plugin_module_ptr& module) = 0;
	virtual function_address Function(const plugin_module_ptr& Instance, const export_name& Name) = 0;
	virtual void ProcessError(std::string_view Function) const {}
	virtual bool IsExternal() const { return false; }
	virtual string Title() const { return {}; }
	virtual VersionInfo version() const { return {}; }

	const auto& Id() const { return m_Id; }
	auto Owner() const { return m_owner; }
	auto ExportsNames() const { return m_ExportsNames; }

protected:
	span<const export_name> m_ExportsNames;
	PluginManager* const m_owner;
	UUID m_Id{};
};

using plugin_factory_ptr = std::unique_ptr<plugin_factory>;

class native_plugin_module: public i_plugin_module
{
public:
	NONCOPYABLE(native_plugin_module);
	explicit native_plugin_module(const string& Name): m_Module(Name, true) {}

	void* opaque() const override
	{
		return nullptr;
	}

	template<typename T>
	auto GetProcAddress(const char* Name) const
	{
		return m_Module.GetProcAddress<T>(Name);
	}

	explicit operator bool() const noexcept
	{
		return m_Module.operator bool();
	}

private:
	os::rtdl::module m_Module;
};

class native_plugin_factory : public plugin_factory
{
public:
	NONCOPYABLE(native_plugin_factory);
	using plugin_factory::plugin_factory;

	bool IsPlugin(const string& FileName) const override;
	plugin_module_ptr Create(const string& filename) override;
	bool Destroy(plugin_module_ptr& instance) override;
	function_address Function(const plugin_module_ptr& Instance, const export_name& Name) override;

private:
	// This shouldn't be here, just an optimization for OEM plugins
	virtual bool FindExport(std::string_view ExportName) const;
	bool IsPlugin(const void* Data) const;
};

template<export_index id, bool native>
struct export_type;

namespace detail
{
	struct ExecuteStruct
	{
		auto& operator=(intptr_t value) { Result = value; return *this; }
		auto& operator=(HANDLE value) { Result = reinterpret_cast<intptr_t>(value); return *this; }
		operator intptr_t() const { return Result; }
		operator void*() const { return ToPtr(Result); }
		operator bool() const { return Result != 0; }
		intptr_t Result;
	};

	// A workaround for 2017.
	// TODO: remove once we drop support for VS2017.
	template<typename result_type, typename function, typename... args>
	void assign(result_type& Result, function const& Function, args&&... Args)
	{
		Result = Function(FWD(Args)...);
	}
}

class Plugin
{
public:
	NONCOPYABLE(Plugin);

	Plugin(plugin_factory* Factory, const string& ModuleName);
	virtual ~Plugin();

	virtual bool GetGlobalInfo(GlobalInfo *Info);
	virtual bool SetStartupInfo(PluginStartupInfo *Info);
	virtual void* Open(OpenInfo* Info);
	virtual void ClosePanel(ClosePanelInfo* Info);
	virtual bool GetPluginInfo(PluginInfo* Info);
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info);
	virtual intptr_t GetFindData(GetFindDataInfo* Info);
	virtual void FreeFindData(FreeFindDataInfo* Info);
	virtual intptr_t GetVirtualFindData(GetVirtualFindDataInfo* Info);
	virtual void FreeVirtualFindData(FreeFindDataInfo* Info);
	virtual intptr_t SetDirectory(SetDirectoryInfo* Info);
	virtual intptr_t GetFiles(GetFilesInfo* Info);
	virtual intptr_t PutFiles(PutFilesInfo* Info);
	virtual intptr_t DeleteFiles(DeleteFilesInfo* Info);
	virtual intptr_t MakeDirectory(MakeDirectoryInfo* Info);
	virtual intptr_t ProcessHostFile(ProcessHostFileInfo* Info);
	virtual intptr_t SetFindList(SetFindListInfo* Info);
	virtual intptr_t Configure(ConfigureInfo* Info);
	virtual void ExitFAR(ExitInfo *Info);
	virtual intptr_t ProcessPanelInput(ProcessPanelInputInfo* Info);
	virtual intptr_t ProcessPanelEvent(ProcessPanelEventInfo* Info);
	virtual intptr_t ProcessEditorEvent(ProcessEditorEventInfo* Info);
	virtual intptr_t Compare(CompareInfo* Info);
	virtual intptr_t ProcessEditorInput(ProcessEditorInputInfo* Info);
	virtual intptr_t ProcessViewerEvent(ProcessViewerEventInfo* Info);
	virtual intptr_t ProcessDialogEvent(ProcessDialogEventInfo* Info);
	virtual intptr_t ProcessSynchroEvent(ProcessSynchroEventInfo* Info);
	virtual intptr_t ProcessConsoleInput(ProcessConsoleInputInfo *Info);
	virtual void* Analyse(AnalyseInfo *Info);
	virtual void CloseAnalyse(CloseAnalyseInfo* Info);

	virtual intptr_t GetContentFields(GetContentFieldsInfo *Info);
	virtual intptr_t GetContentData(GetContentDataInfo *Info);
	virtual void FreeContentData(GetContentDataInfo *Info);

	virtual void* OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode);
	virtual bool CheckMinFarVersion();


#ifndef NO_WRAPPER
	virtual bool IsOemPlugin() const { return false; }
#endif // NO_WRAPPER
	virtual const string& GetHotkeyName() const { return m_strUuid; }

	virtual bool InitLang(string_view Path, string_view Language);
	void CloseLang();

	bool has(export_index id) const { return Exports[id] != nullptr; }

	template<typename T>
	bool has(const T& es) const
	{
		static_assert(std::is_base_of_v<detail::ExecuteStruct, T>);
		return has(es.export_id);
	}

	const string& ModuleName() const { return m_strModuleName; }
	const string& CacheName() const  { return m_strCacheName; }
	const string& Title() const { return strTitle; }
	const string& Description() const { return strDescription; }
	const string& Author() const { return strAuthor; }
	const VersionInfo& version() const { return m_PluginVersion; }
	const VersionInfo& MinFarVersion() const { return m_MinFarVersion; }
	const UUID& Id() const { return m_Uuid; }
	bool IsPendingRemove() const { return bPendingRemove; }
	const wchar_t* Msg(intptr_t Id) const;

	bool CheckWorkFlags(DWORD flags) const { return WorkFlags.Check(flags); }

	bool Load();
	bool Unload(bool bExitFAR = false);
	bool LoadData();
	bool LoadFromCache(const os::fs::find_data &FindData);
	bool SaveToCache();
	bool IsPanelPlugin();
	bool Active() const {return Activity != 0;}
	void AddDialog(const window_ptr& Dlg);
	bool RemoveDialog(const window_ptr& Dlg);
	[[nodiscard]]
	auto keep_activity() { return make_raii_wrapper(this, [](Plugin* p){ ++p->Activity; }, [](Plugin* p){ --p->Activity; });  }

protected:
	template<export_index Export, bool Native = true>
	struct ExecuteStruct: detail::ExecuteStruct
	{
		explicit ExecuteStruct(intptr_t FallbackValue = 0)
		{
			Result = FallbackValue;
		}

		static constexpr inline auto export_id = Export;
		using type = typename export_type<Export, Native>::type;

		using detail::ExecuteStruct::operator=;
	};

	template<typename T, typename... args>
	void ExecuteFunction(T& es, args&&... Args)
	{
		ExecuteFunctionImpl(T::export_id, [&]
		{
			using function_type = typename T::type;
			const auto Function = reinterpret_cast<function_type>(Exports[T::export_id]);

			if constexpr (std::is_void_v<std::invoke_result_t<function_type, args...>>)
				Function(FWD(Args)...);
			else
				::detail::assign(es, Function, FWD(Args)...);
		});
	}

	virtual void Prologue() {}
	virtual void Epilogue() {}

	plugin_factory::exports_array Exports{};

	std::unordered_set<window_ptr> m_dialogs;
	plugin_factory* m_Factory;
	plugin_factory::plugin_module_ptr m_Instance;
	std::unique_ptr<language> PluginLang;
	size_t Activity{};
	bool bPendingRemove{};

private:
	friend class PluginManager;

	void InitExports();
	void ClearExports();
	void SetUuid(const UUID& Uuid);

	template<typename T>
	void SetInstance(T* Object) const
	{
		Object->Instance = m_Instance->opaque();
	}

	void ExecuteFunctionImpl(export_index ExportId, function_ref<void()> Callback);

	string strTitle;
	string strDescription;
	string strAuthor;

	string m_strModuleName;
	string m_strCacheName;

	BitFlags WorkFlags;      // рабочие флаги текущего плагина

	VersionInfo m_MinFarVersion{};
	VersionInfo m_PluginVersion{};

	UUID m_Uuid;
	string m_strUuid;
};

plugin_factory_ptr CreateCustomPluginFactory(PluginManager* Owner, const string& Filename);

#define DECLARE_GEN_PLUGIN_FUNCTION(name, is_native, signature) template<> struct export_type<name, is_native>  { using type = signature; };
#define WA(string) { L##string##sv, string##sv }

#endif // PLCLASS_HPP_E324EC16_24F2_4402_BA87_74212799246D
