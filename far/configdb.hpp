#ifndef CONFIGDB_HPP_552309E5_DEA6_42FD_BD7B_0F59C839FE62
#define CONFIGDB_HPP_552309E5_DEA6_42FD_BD7B_0F59C839FE62
#pragma once

/*
configdb.hpp

хранение настроек в базе sqlite.
*/
/*
Copyright © 2011 Far Group
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
#include "transactional.hpp"
#include "bitflags.hpp"

// Platform:
#include "platform.chrono.hpp"

// Common:
#include "common/bytes_view.hpp"
#include "common/enumerator.hpp"
#include "common/keep_alive.hpp"
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

struct FarColor;
struct VersionInfo;
class representation_source;
class representation_destination;

namespace os
{
	namespace concurrency
	{
		class thread;
	}

	using concurrency::thread;
}

class representable: noncopyable
{
public:
	virtual ~representable() = default;
	virtual void Export(representation_destination& Representation) const = 0;
	virtual void Import(const representation_source& Representation) = 0;
};

class GeneralConfig : public representable, virtual public transactional
{
public:
	virtual void SetValue(string_view Key, string_view Name, string_view Value) = 0;
	virtual void SetValue(string_view Key, string_view Name, unsigned long long Value) = 0;
	virtual void SetValue(string_view Key, string_view Name, bytes_view Value) = 0;

	[[nodiscard]]
	virtual bool GetValue(string_view Key, string_view Name, bool& Value) const = 0;
	[[nodiscard]]
	virtual bool GetValue(string_view Key, string_view Name, long long& Value) const = 0;
	[[nodiscard]]
	virtual bool GetValue(string_view Key, string_view Name, string& Value) const = 0;

	template<typename value_type, typename default_type = value_type>
	[[nodiscard]]
	value_type GetValue(string_view Key, string_view const Name, const default_type& Default = {}) const
	{
		value_type Value;
		return GetValue(Key, Name, Value)? Value : value_type{ Default };
	}

	virtual void DeleteValue(string_view Key, string_view Name) = 0;

	template<typename T, typename key_type>
	[[nodiscard]]
	auto ValuesEnumerator(key_type&& Key) const
	{
		static_assert(std::is_convertible_v<key_type, string_view>);

		using value_type = std::pair<string, T>;
		return make_inline_enumerator<value_type>([this, Key = keep_alive(FWD(Key))](const bool Reset, value_type& Value)
		{
			return EnumValues(Key.get(), Reset, Value.first, Value.second);
		},
		[this]
		{
			CloseEnum();
		});
	}

protected:
	GeneralConfig() = default;

private:
	[[nodiscard]]
	virtual bool EnumValues(string_view Key, bool Reset, string& strName, string& strValue) const = 0;
	[[nodiscard]]
	virtual bool EnumValues(string_view Key, bool Reset, string& strName, long long& Value) const = 0;

	virtual void CloseEnum() const = 0;
};

class async_delete
{
public:
	virtual void finish() = 0;

protected:
	virtual ~async_delete() = default;
};

class async_delete_impl;


class HierarchicalConfig: public representable, virtual public async_delete, virtual public transactional
{
public:
	class key
	{
	public:
		key() = default;
		explicit key(unsigned long long Key): m_Key(Key) {}

		unsigned long long get() const noexcept { return m_Key; }
		explicit operator bool() const noexcept { return m_Key != 0; }

	private:
		unsigned long long m_Key{};
	};

	static inline const key root_key{0};

	[[nodiscard]]
	virtual key CreateKey(const key& Root, string_view Name) = 0;
	[[nodiscard]]
	virtual key FindByName(const key& Root, string_view Name) const = 0;
	[[nodiscard]]
	virtual bool GetKeyName(const key& Root, const key& Key, string& Name) const = 0;
	virtual void SetKeyDescription(const key& Root, string_view Description) = 0;

	virtual void SetValue(const key& Root, string_view Name, string_view Value) = 0;
	virtual void SetValue(const key& Root, string_view Name, unsigned long long Value) = 0;
	virtual void SetValue(const key& Root, string_view Name, bytes_view Value) = 0;

	virtual bool GetValue(const key& Root, string_view Name, unsigned long long& Value) const = 0;
	virtual bool GetValue(const key& Root, string_view Name, string& strValue) const = 0;
	virtual bool GetValue(const key& Root, string_view Name, bytes& Value) const = 0;

	bool GetValue(const key& Root, string_view const Name, bool& Value) const
	{
		unsigned long long iValue;
		if (!GetValue(Root, Name, iValue))
			return false;

		Value = iValue != 0;
		return true;
	}

	template<typename value_type, typename default_type = value_type>
	value_type GetValue(const key& Root, string_view const Name, const default_type& Default = {})
	{
		value_type Value;
		return GetValue(Root, Name, Value)? Value : value_type{ Default };
	}


	virtual void DeleteKeyTree(const key& Key) = 0;
	virtual void DeleteValue(const key& Root, string_view Name) = 0;
	virtual void Flush() = 0;

	[[nodiscard]]
	virtual const string& GetName() const = 0;

	[[nodiscard]]
	auto KeysEnumerator(key const Root, string_view const Pattern = {}) const
	{
		using value_type = key;
		return make_inline_enumerator<value_type>([this, Root, Pattern = string{Pattern}](const bool Reset, value_type& Value)
		{
			return EnumKeys(Root, Reset, Value, Pattern);
		},
		[this, Pattern = string{Pattern}]
		{
			CloseEnumKeys(Pattern);
		});
	}

	[[nodiscard]]
	auto ValuesEnumerator(key const Root, string_view const Pattern = {}) const
	{
		using value_type = std::pair<string, int>;
		return make_inline_enumerator<value_type>([this, Root, Pattern = string{Pattern}](const bool Reset, value_type& Value)
		{
			return EnumValues(Root, Reset, Value.first, Value.second, Pattern);
		},
		[this, Pattern = string{Pattern}]
		{
			CloseEnumValues(Pattern);
		});
	}

	[[nodiscard]]
	static int ToSettingsType(int Type);

protected:
	HierarchicalConfig() = default;

private:
	[[nodiscard]]
	virtual bool EnumKeys(const key& Root, bool Reset, key& Key, string_view Pattern = {}) const = 0;
	virtual void CloseEnumKeys(string_view Pattern) const = 0;
	[[nodiscard]]
	virtual bool EnumValues(const key& Root, bool Reset, string& Name, int& Type, string_view Pattern = {}) const = 0;
	virtual void CloseEnumValues(string_view Pattern) const = 0;
};

namespace detail
{
	struct async_deleter
	{
		void operator()(async_delete* Ptr) const noexcept
		{
			Ptr->finish();
		}
	};
}

using HierarchicalConfigUniquePtr = std::unique_ptr<HierarchicalConfig, detail::async_deleter>;

class ColorsConfig: public representable, virtual public transactional
{
public:
	virtual void SetValue(string_view Name, const FarColor& Value) = 0;
	virtual bool GetValue(string_view Name, FarColor& Value) const = 0;

protected:
	ColorsConfig() = default;
};

class AssociationsConfig: public representable, virtual public transactional
{
public:
	virtual bool GetMask(unsigned long long id, string &strMask) = 0;
	virtual bool GetDescription(unsigned long long id, string &strDescription) = 0;
	virtual bool GetCommand(unsigned long long id, int Type, string &strCommand, bool *Enabled=nullptr) = 0;
	virtual void SetCommand(unsigned long long id, int Type, string_view Command, bool Enabled) = 0;
	[[nodiscard]]
	virtual bool SwapPositions(unsigned long long id1, unsigned long long id2) = 0;
	[[nodiscard]]
	virtual unsigned long long AddType(unsigned long long after_id, string_view Mask, string_view Description) = 0;
	virtual void UpdateType(unsigned long long id, string_view Mask, string_view Description) = 0;
	virtual void DelType(unsigned long long id) = 0;

	[[nodiscard]]
	auto MasksEnumerator()
	{
		using value_type = std::pair<unsigned long long, string>;
		return make_inline_enumerator<value_type>([this](const bool Reset, value_type& Value)
		{
			return EnumMasks(Reset, &Value.first, Value.second);
		},
		[this]
		{
			CloseEnumMasks();
		});
	}

	[[nodiscard]]
	auto TypedMasksEnumerator(int Type)
	{
		using value_type = std::pair<unsigned long long, string>;
		return make_inline_enumerator<value_type>([this, Type](const bool Reset, value_type& Value)
		{
			return EnumMasksForType(Reset, Type, &Value.first, Value.second);
		},
		[this]
		{
			CloseEnumMasksForType();
		});
	}

private:
	[[nodiscard]]
	virtual bool EnumMasks(bool Reset, unsigned long long *id, string &strMask) const = 0;
	virtual void CloseEnumMasks() const = 0;
	[[nodiscard]]
	virtual bool EnumMasksForType(bool Reset, int Type, unsigned long long *id, string &strMask) const = 0;
	virtual void CloseEnumMasksForType() const = 0;

protected:
	AssociationsConfig() = default;
};

class PluginsCacheConfig: public representable, virtual public transactional
{
public:
	[[nodiscard]]
	virtual unsigned long long CreateCache(string_view CacheName) = 0;
	[[nodiscard]]
	virtual unsigned long long GetCacheID(string_view CacheName) const = 0;
	[[nodiscard]]
	virtual bool IsPreload(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual string GetSignature(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual bool GetExportState(unsigned long long id, string_view ExportName) const = 0;
	[[nodiscard]]
	virtual string GetUuid(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual string GetTitle(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual string GetAuthor(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual string GetDescription(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual bool GetMinFarVersion(unsigned long long id, VersionInfo& Version) const = 0;
	[[nodiscard]]
	virtual bool GetVersion(unsigned long long id, VersionInfo& Version) const = 0;
	[[nodiscard]]
	virtual bool GetDiskMenuItem(unsigned long long id, size_t index, string &Text, UUID& Uuid) const = 0;
	[[nodiscard]]
	virtual bool GetPluginsMenuItem(unsigned long long id, size_t index, string &Text, UUID& Uuid) const = 0;
	[[nodiscard]]
	virtual bool GetPluginsConfigMenuItem(unsigned long long id, size_t index, string &Text, UUID& Uuid) const = 0;
	[[nodiscard]]
	virtual string GetCommandPrefix(unsigned long long id) const = 0;
	[[nodiscard]]
	virtual unsigned long long GetFlags(unsigned long long id) const = 0;
	virtual void SetPreload(unsigned long long id, bool Preload) = 0;
	virtual void SetSignature(unsigned long long id, string_view Signature) = 0;
	virtual void SetDiskMenuItem(unsigned long long id, size_t index, string_view Text, const UUID& Uuid) = 0;
	virtual void SetPluginsMenuItem(unsigned long long id, size_t index, string_view Text, const UUID& Uuid) = 0;
	virtual void SetPluginsConfigMenuItem(unsigned long long id, size_t index, string_view Text, const UUID& Uuid) = 0;
	virtual void SetCommandPrefix(unsigned long long id, string_view Prefix) = 0;
	virtual void SetFlags(unsigned long long id, unsigned long long Flags) = 0;
	virtual void SetExportState(unsigned long long id, string_view ExportName, bool Exists) = 0;
	virtual void SetMinFarVersion(unsigned long long id, const VersionInfo& Version) = 0;
	virtual void SetVersion(unsigned long long id, const VersionInfo& Version) = 0;
	virtual void SetUuid(unsigned long long id, string_view Uuid) = 0;
	virtual void SetTitle(unsigned long long id, string_view Title) = 0;
	virtual void SetAuthor(unsigned long long id, string_view Author) = 0;
	virtual void SetDescription(unsigned long long id, string_view Description) = 0;
	[[nodiscard]]
	virtual bool EnumPlugins(size_t Index, string &CacheName) const = 0;
	virtual void DiscardCache() = 0;
	[[nodiscard]]
	virtual bool IsCacheEmpty() const = 0;

protected:
	PluginsCacheConfig() = default;
};

enum class hotkey_type: int
{
	drive_menu,
	plugins_menu,
	config_menu,
};

class PluginsHotkeysConfig: public representable, virtual public transactional
{
public:
	[[nodiscard]]
	virtual bool HotkeysPresent(hotkey_type HotKeyType) = 0;
	[[nodiscard]]
	virtual string GetHotkey(string_view PluginKey, const UUID& MenuUuid, hotkey_type HotKeyType) = 0;
	virtual void SetHotkey(string_view PluginKey, const UUID& MenuUuid, hotkey_type HotKeyType, string_view HotKey) = 0;
	virtual void DelHotkey(string_view PluginKey, const UUID& MenuUuid, hotkey_type HotKeyType) = 0;

protected:
	PluginsHotkeysConfig() = default;
};

enum history_record_type: int;

class HistoryConfig: public representable, virtual public transactional
{
public:
	//command,view,edit,folder,dialog history
	virtual void Delete(unsigned long long id) = 0;

	virtual void DeleteAndAddAsync(unsigned long long DeleteId, unsigned int TypeHistory, string_view HistoryName, string_view Name, int Type, bool Lock, string_view Uuid, string_view File, string_view Data) = 0;
	virtual void DeleteOldUnlocked(unsigned int TypeHistory, string_view HistoryName, int DaysToKeep, int MinimumEntries) = 0;
	[[nodiscard]]
	virtual bool GetNewest(unsigned int TypeHistory, string_view HistoryName, string &strName) = 0;
	[[nodiscard]]
	virtual bool Get(unsigned long long id, string* Name = {}, history_record_type* Type = {}, string* Uuid = {}, string* File = {}, string* Data = {}) = 0;
	[[nodiscard]]
	virtual DWORD Count(unsigned int TypeHistory, string_view HistoryName) = 0;
	virtual void FlipLock(unsigned long long id) = 0;
	[[nodiscard]]
	virtual bool IsLocked(unsigned long long id) = 0;
	virtual void DeleteAllUnlocked(unsigned int TypeHistory, string_view HistoryName) = 0;
	[[nodiscard]]
	virtual unsigned long long GetNext(unsigned int TypeHistory, string_view HistoryName, unsigned long long id, string &strName) = 0;
	[[nodiscard]]
	virtual unsigned long long GetPrev(unsigned int TypeHistory, string_view HistoryName, unsigned long long id, string &strName) = 0;
	[[nodiscard]]
	virtual unsigned long long CyclicGetPrev(unsigned int TypeHistory, string_view HistoryName, unsigned long long id, string &strName) = 0;

	//view,edit file positions and bookmarks history
	[[nodiscard]]
	virtual unsigned long long SetEditorPos(string_view Name, int Line, int LinePos, int ScreenLine, int LeftPos, uintptr_t CodePage) = 0;
	[[nodiscard]]
	virtual unsigned long long GetEditorPos(string_view Name, int& Line, int& LinePos, int& ScreenLine, int& LeftPos, uintptr_t& CodePage) = 0;
	virtual void SetEditorBookmark(unsigned long long id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos) = 0;
	virtual bool GetEditorBookmark(unsigned long long id, size_t i, int& Line, int& LinePos, int& ScreenLine, int& LeftPos) = 0;
	[[nodiscard]]
	virtual unsigned long long SetViewerPos(string_view Name, long long FilePos, long long LeftPos, int HexWrap, uintptr_t CodePage) = 0;
	[[nodiscard]]
	virtual unsigned long long GetViewerPos(string_view Name, long long& FilePos, long long& LeftPos, int& HexWrap, uintptr_t& CodePage) = 0;
	virtual void SetViewerBookmark(unsigned long long id, size_t i, long long FilePos, long long LeftPos) = 0;
	virtual bool GetViewerBookmark(unsigned long long id, size_t i, long long& FilePos, long long& LeftPos) = 0;
	virtual void DeleteOldPositions(int DaysToKeep, int MinimumEntries) = 0;

	struct enum_data
	{
		unsigned long long Id;
		string Name;
		history_record_type Type;
		bool Lock;
		os::chrono::time_point Time;
		string Uuid;
		string File;
		string Data;
	};

	template<typename type>
	[[nodiscard]]
	auto Enumerator(unsigned int HistoryType, type&& HistoryName, bool Reverse = false)
	{
		static_assert(std::is_convertible_v<type, string_view>);

		using value_type = enum_data;
		return make_inline_enumerator<value_type>([this, HistoryType, HistoryName = keep_alive(FWD(HistoryName)), Reverse](const bool Reset, value_type& Value)
		{
			return Enum(Reset, HistoryType, HistoryName.get(), Value.Id, Value.Name, Value.Type, Value.Lock, Value.Time, Value.Uuid, Value.File, Value.Data, Reverse);
		},
		[this, Reverse]
		{
			CloseEnum(Reverse);
		});
	}

	[[nodiscard]]
	auto LargeHistoriesEnumerator(unsigned int HistoryType, int MinimumEntries)
	{
		using value_type = string;
		return make_inline_enumerator<value_type>([this, HistoryType, MinimumEntries](const bool Reset, value_type& Value)
		{
			return EnumLargeHistories(Reset, HistoryType, MinimumEntries, Value);
		},
		[this]
		{
			CloseEnumLargeHistories();
		});
	}

protected:
	HistoryConfig() = default;

private:
	//command,view,edit,folder,dialog history
	[[nodiscard]]
	virtual bool Enum(bool Reset, unsigned int TypeHistory, string_view HistoryName, unsigned long long& id, string& strName, history_record_type& Type, bool& Lock, os::chrono::time_point& Time, string& strUuid, string& strFile, string& strData, bool Reverse) = 0;
	virtual void CloseEnum(bool Reverse) const = 0;
	[[nodiscard]]
	virtual bool EnumLargeHistories(bool Reset, unsigned int TypeHistory, int MinimumEntries, string& strHistoryName) = 0;
	virtual void CloseEnumLargeHistories() const = 0;
};

enum dbcheck: int;

class config_provider: noncopyable
{
public:
	enum class mode { m_default, m_import, m_export };
	struct clear_cache{};

	explicit config_provider(mode Mode = mode::m_default);
	explicit config_provider(clear_cache);
	~config_provider();
	[[nodiscard]]
	bool ShowProblems() const;
	void ServiceMode(string_view File);

	class async_key
	{
		friend async_delete_impl;
		async_key() = default;
	};

	void AsyncCall(async_key, const std::function<void()>& Routine);

	[[nodiscard]]
	const auto& GeneralCfg() const { return m_GeneralCfg; }
	[[nodiscard]]
	const auto& LocalGeneralCfg() const { return m_LocalGeneralCfg; }
	[[nodiscard]]
	const auto& ColorsCfg() const { return m_ColorsCfg; }
	[[nodiscard]]
	const auto& AssocConfig() const { return m_AssocConfig; }
	[[nodiscard]]
	const auto& PlCacheCfg() const { return m_PlCacheCfg; }
	[[nodiscard]]
	const auto& PlHotkeyCfg() const { return m_PlHotkeyCfg; }
	[[nodiscard]]
	const auto& HistoryCfg() const { return m_HistoryCfg; }
	[[nodiscard]]
	const auto& HistoryCfgMem() const { return m_HistoryCfgMem; }

	HierarchicalConfigUniquePtr CreatePluginsConfig(string_view Uuid, bool Local = false, bool UseFallback = true);
	HierarchicalConfigUniquePtr CreateFiltersConfig();
	HierarchicalConfigUniquePtr CreateHighlightConfig();
	HierarchicalConfigUniquePtr CreateShortcutsConfig();
	HierarchicalConfigUniquePtr CreatePanelModesConfig();

private:
	template<class T>
	void ImportDatabase(T& Database, const char* ImportNodeName, bool IsPlugin);

	template<class T>
	[[nodiscard]]
	std::unique_ptr<T> CreateWithFallback(string_view Name);

	template<class T>
	[[nodiscard]]
	std::unique_ptr<T> CreateDatabase(string_view Name, bool Local);

	template<class T>
	[[nodiscard]]
	HierarchicalConfigUniquePtr CreateHierarchicalConfig(dbcheck DbId, string_view DbName, const char* ImportNodeName, bool Local = false, bool IsPlugin = false, bool UseFallback = true);

	void Import(string_view File);
	void Export(string_view File);
	void TryImportDatabase(representable& p, const char* NodeName = nullptr, bool IsPlugin = false);

	struct implementation
	{
		NONCOPYABLE(implementation);

		implementation();
		~implementation();
	}
	m_Impl;

	std::vector<os::thread> m_Threads;
	mutable std::vector<string> m_Problems;
	std::unique_ptr<representation_source> m_TemplateSource;
	mode m_Mode{ mode::m_default };

	std::unique_ptr<GeneralConfig> m_GeneralCfg;
	std::unique_ptr<GeneralConfig> m_LocalGeneralCfg;
	std::unique_ptr<ColorsConfig> m_ColorsCfg;
	std::unique_ptr<AssociationsConfig> m_AssocConfig;
	std::unique_ptr<PluginsCacheConfig> m_PlCacheCfg;
	std::unique_ptr<PluginsHotkeysConfig> m_PlHotkeyCfg;
	std::unique_ptr<HistoryConfig> m_HistoryCfg;
	std::unique_ptr<HistoryConfig> m_HistoryCfgMem;

	BitFlags m_CheckedDb;
};

[[nodiscard]]
config_provider& ConfigProvider();

#endif // CONFIGDB_HPP_552309E5_DEA6_42FD_BD7B_0F59C839FE62
