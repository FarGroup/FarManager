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

#include "transactional.hpp"
#include "bitflags.hpp"
#include "synchro.hpp"

struct VersionInfo;
class representation_source;
class representation_destination;

class representable: noncopyable
{
public:
	virtual ~representable() = default;
	virtual void Export(representation_destination& Representation) = 0;
	virtual void Import(const representation_source& Representation) = 0;
};

class GeneralConfig: public representable, virtual public transactional
{
public:
	virtual ~GeneralConfig() = default;

	virtual bool SetValue(const string& Key, const string& Name, const string& Value) = 0;
	virtual bool SetValue(const string& Key, const string& Name, const wchar_t* Value) = 0;
	virtual bool SetValue(const string& Key, const string& Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(const string& Key, const string& Name, const blob_view& Value) = 0;
	template<class T, ENABLE_IF(!std::is_pointer<T>::value && !std::is_integral<T>::value)>
	bool SetValue(const string& Key, const string& Name, const T& Value)
	{
		static_assert(std::is_pod<T>::value, "This template requires a POD type");
		return SetValue(Key, Name, &Value, sizeof(Value));
	}

	virtual bool GetValue(const string& Key, const string& Name, bool& Value, bool Default) = 0;
	virtual bool GetValue(const string& Key, const string& Name, long long& Value, long long Default) = 0;
	virtual bool GetValue(const string& Key, const string& Name, string& Value, const string& Default) = 0;
	virtual bool GetValue(const string& Key, const string& Name, string& Value, const wchar_t *Default) = 0;

	virtual bool DeleteValue(const string& Key, const string& Name) = 0;
	virtual bool EnumValues(const string& Key, DWORD Index, string &strName, string &strValue) = 0;
	virtual bool EnumValues(const string& Key, DWORD Index, string &strName, long long& Value) = 0;

	template<class T>
	class values_enumerator: public enumerator<values_enumerator<T>, std::pair<string, T>>
	{
	public:
		values_enumerator(GeneralConfig& provider, const string& key): m_provider(provider), m_key(key) {}
		bool get(size_t index, typename values_enumerator::value_type& value) const
		{
			return m_provider.EnumValues(m_key, static_cast<DWORD>(index), value.first, value.second);
		}

	private:
		GeneralConfig& m_provider;
		const string m_key;
	};

	typedef values_enumerator<string> string_values_enumerator;
	typedef values_enumerator<long long> int_values_enumerator;

	string_values_enumerator GetStringValuesEnumerator(const string& key) { return string_values_enumerator(*this, key); }
	int_values_enumerator GetIntValuesEnumerator(const string& key) { return int_values_enumerator(*this, key); }

protected:
	GeneralConfig() = default;
};

class HierarchicalConfig: public representable, virtual public transactional, public conditional<HierarchicalConfig>
{
public:
	class key: public conditional<key>
	{
	public:
		uint64_t get() const { return m_Key; }
		bool operator!() const noexcept { return m_Key == 0; }

	private:
		friend class HierarchicalConfig;

		key(uint64_t Key): m_Key(Key) {}
		uint64_t m_Key;
	};

	static key root_key() { return make_key(0); }

	virtual void AsyncFinish() = 0;
	virtual key CreateKey(const key& Root, const string& Name, const string* Description = nullptr) = 0;
	virtual key FindByName(const key& Root, const string& Name) = 0;
	virtual bool SetKeyDescription(const key& Root, const string& Description) = 0;

	virtual bool SetValue(const key& Root, const string& Name, const string& Value) = 0;
	virtual bool SetValue(const key& Root, const string& Name, const wchar_t* Value) = 0;
	virtual bool SetValue(const key& Root, const string& Name, unsigned long long Value) = 0;
	virtual bool SetValue(const key& Root, const string& Name, const blob_view& Value) = 0;
	template<class T, ENABLE_IF(!std::is_pointer<T>::value && !std::is_integral<T>::value)>
	bool SetValue(const key& Root, const string& Name, const T& Value)
	{
		return SetValue(Root, Name, make_blob_view(Value));
	}

	virtual bool GetValue(const key& Root, const string& Name, unsigned long long& Value) = 0;
	virtual bool GetValue(const key& Root, const string& Name, string &strValue) = 0;
	virtual bool GetValue(const key& Root, const string& Name, writable_blob_view& Value) = 0;
	template<class T, ENABLE_IF(!std::is_pointer<T>::value && !std::is_integral<T>::value)>
	bool GetValue(const key& Root, const string& Name, T& Value)
	{
		static_assert(std::is_pod<T>::value, "This template requires a POD type");
		writable_blob_view Blob(&Value, sizeof(Value));
		return GetValue(Root, Name, Blob);
	}

	virtual bool DeleteKeyTree(const key& Key) = 0;
	virtual bool DeleteValue(const key& Root, const string& Name) = 0;
	virtual bool EnumKeys(const key& Root, DWORD Index, string &strName) = 0;
	virtual bool EnumValues(const key& Root, DWORD Index, string &strName, int& Type) = 0;
	virtual bool Flush() = 0;

protected:
	HierarchicalConfig() = default;
	virtual ~HierarchicalConfig() = default;

	static key make_key(uint64_t Key) { return key(Key); }
};

namespace detail
{
	struct HierarchicalConfigDeleter
	{
		void operator()(HierarchicalConfig *ptr) const { ptr->AsyncFinish(); }
	};
}

typedef std::unique_ptr<HierarchicalConfig, detail::HierarchicalConfigDeleter> HierarchicalConfigUniquePtr;

class ColorsConfig: public representable, virtual public transactional
{
public:
	virtual ~ColorsConfig() = default;
	virtual bool SetValue(const string& Name, const FarColor& Value) = 0;
	virtual bool GetValue(const string& Name, FarColor& Value) = 0;

protected:
	ColorsConfig() = default;
};

class AssociationsConfig: public representable, virtual public transactional {

public:

	virtual ~AssociationsConfig() = default;
	virtual bool EnumMasks(DWORD Index, unsigned __int64 *id, string &strMask) = 0;
	virtual bool EnumMasksForType(int Type, DWORD Index, unsigned __int64 *id, string &strMask) = 0;
	virtual bool GetMask(unsigned __int64 id, string &strMask) = 0;
	virtual bool GetDescription(unsigned __int64 id, string &strDescription) = 0;
	virtual bool GetCommand(unsigned __int64 id, int Type, string &strCommand, bool *Enabled=nullptr) = 0;
	virtual bool SetCommand(unsigned __int64 id, int Type, const string& Command, bool Enabled) = 0;
	virtual bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2) = 0;
	virtual unsigned __int64 AddType(unsigned __int64 after_id, const string& Mask, const string& Description) = 0;
	virtual bool UpdateType(unsigned __int64 id, const string& Mask, const string& Description) = 0;
	virtual bool DelType(unsigned __int64 id) = 0;

protected:
	AssociationsConfig() = default;
};

class PluginsCacheConfig: public representable, virtual public transactional
{
public:
	virtual ~PluginsCacheConfig() = default;
	virtual unsigned __int64 CreateCache(const string& CacheName) = 0;
	virtual unsigned __int64 GetCacheID(const string& CacheName) const = 0;
	virtual bool DeleteCache(const string& CacheName) = 0;
	virtual bool IsPreload(unsigned __int64 id) const = 0;
	virtual string GetSignature(unsigned __int64 id) const = 0;
	virtual bool GetExportState(unsigned __int64 id, const wchar_t* ExportName) const = 0;
	virtual string GetGuid(unsigned __int64 id) const = 0;
	virtual string GetTitle(unsigned __int64 id) const = 0;
	virtual string GetAuthor(unsigned __int64 id) const = 0;
	virtual string GetDescription(unsigned __int64 id) const = 0;
	virtual bool GetMinFarVersion(unsigned __int64 id, VersionInfo *Version) const = 0;
	virtual bool GetVersion(unsigned __int64 id, VersionInfo *Version) const = 0;
	virtual bool GetDiskMenuItem(unsigned __int64 id, size_t index, string &Text, GUID& Guid) const = 0;
	virtual bool GetPluginsMenuItem(unsigned __int64 id, size_t index, string &Text, GUID& Guid) const = 0;
	virtual bool GetPluginsConfigMenuItem(unsigned __int64 id, size_t index, string &Text, GUID& Guid) const = 0;
	virtual string GetCommandPrefix(unsigned __int64 id) const = 0;
	virtual unsigned __int64 GetFlags(unsigned __int64 id) const = 0;
	virtual bool SetPreload(unsigned __int64 id, bool Preload) = 0;
	virtual bool SetSignature(unsigned __int64 id, const string& Signature) = 0;
	virtual bool SetDiskMenuItem(unsigned __int64 id, size_t index, const string& Text, const GUID& Guid) = 0;
	virtual bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const string& Text, const GUID& Guid) = 0;
	virtual bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const string& Text, const GUID& Guid) = 0;
	virtual bool SetCommandPrefix(unsigned __int64 id, const string& Prefix) = 0;
	virtual bool SetFlags(unsigned __int64 id, unsigned __int64 Flags) = 0;
	virtual bool SetExportState(unsigned __int64 id, const wchar_t* ExportName, bool Exists) = 0;
	virtual bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version) = 0;
	virtual bool SetVersion(unsigned __int64 id, const VersionInfo *Version) = 0;
	virtual bool SetGuid(unsigned __int64 id, const string& Guid) = 0;
	virtual bool SetTitle(unsigned __int64 id, const string& Title) = 0;
	virtual bool SetAuthor(unsigned __int64 id, const string& Author) = 0;
	virtual bool SetDescription(unsigned __int64 id, const string& Description) = 0;
	virtual bool EnumPlugins(DWORD index, string &CacheName) const = 0;
	virtual bool DiscardCache() = 0;
	virtual bool IsCacheEmpty() const = 0;

protected:
	PluginsCacheConfig() = default;
};

class PluginsHotkeysConfig: public representable, virtual public transactional
{
public:
	enum class hotkey_type: int
	{
		drive_menu,
		plugins_menu,
		config_menu,
	};

	virtual ~PluginsHotkeysConfig() = default;
	virtual bool HotkeysPresent(hotkey_type HotKeyType) = 0;
	virtual string GetHotkey(const string& PluginKey, const GUID& MenuGuid, hotkey_type HotKeyType) = 0;
	virtual bool SetHotkey(const string& PluginKey, const GUID& MenuGuid, hotkey_type HotKeyType, const string& HotKey) = 0;
	virtual bool DelHotkey(const string& PluginKey, const GUID& MenuGuid, hotkey_type HotKeyType) = 0;

protected:
	PluginsHotkeysConfig() = default;
};

enum history_record_type: int;

class HistoryConfig: public representable, virtual public transactional
{
public:
	virtual ~HistoryConfig() = default;

	//command,view,edit,folder,dialog history
	virtual bool Enum(DWORD index, unsigned int TypeHistory, const string& HistoryName, unsigned __int64 *id, string &strName, history_record_type* Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false) = 0;
	virtual bool Delete(unsigned __int64 id) = 0;
	virtual bool DeleteAndAddAsync(unsigned __int64 DeleteId, unsigned int TypeHistory, const string& HistoryName, string strName, int Type, bool Lock, string &strGuid, string &strFile, string &strData) = 0;
	virtual bool DeleteOldUnlocked(unsigned int TypeHistory, const string& HistoryName, int DaysToKeep, int MinimumEntries) = 0;
	virtual bool EnumLargeHistories(DWORD index, int MinimumEntries, unsigned int TypeHistory, string &strHistoryName) = 0;
	virtual bool GetNewest(unsigned int TypeHistory, const string& HistoryName, string &strName) = 0;
	virtual bool Get(unsigned __int64 id, string &strName) = 0;
	virtual bool Get(unsigned __int64 id, string &strName, history_record_type& Type, string &strGuid, string &strFile, string &strData) = 0;
	virtual DWORD Count(unsigned int TypeHistory, const string& HistoryName) = 0;
	virtual bool FlipLock(unsigned __int64 id) = 0;
	virtual bool IsLocked(unsigned __int64 id) = 0;
	virtual bool DeleteAllUnlocked(unsigned int TypeHistory, const string& HistoryName) = 0;
	virtual unsigned __int64 GetNext(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string &strName) = 0;
	virtual unsigned __int64 GetPrev(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string &strName) = 0;
	virtual unsigned __int64 CyclicGetPrev(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string &strName) = 0;

	//view,edit file positions and bookmarks history
	virtual unsigned __int64 SetEditorPos(const string& Name, int Line, int LinePos, int ScreenLine, int LeftPos, uintptr_t CodePage) = 0;
	virtual unsigned __int64 GetEditorPos(const string& Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, uintptr_t *CodePage) = 0;
	virtual bool SetEditorBookmark(unsigned __int64 id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos) = 0;
	virtual bool GetEditorBookmark(unsigned __int64 id, size_t i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos) = 0;
	virtual unsigned __int64 SetViewerPos(const string& Name, __int64 FilePos, __int64 LeftPos, int Hex_Wrap, uintptr_t CodePage) = 0;
	virtual unsigned __int64 GetViewerPos(const string& Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, uintptr_t *CodePage) = 0;
	virtual bool SetViewerBookmark(unsigned __int64 id, size_t i, __int64 FilePos, __int64 LeftPos) = 0;
	virtual bool GetViewerBookmark(unsigned __int64 id, size_t i, __int64 *FilePos, __int64 *LeftPos) = 0;
	virtual void DeleteOldPositions(int DaysToKeep, int MinimumEntries) = 0;

protected:
	HistoryConfig() = default;
};

enum dbcheck: int;

class config_provider: noncopyable
{
public:
	enum class mode { m_default, m_import, m_export };
	config_provider(mode Mode = mode::m_default);
	~config_provider();
	int ShowProblems() const;
	bool ServiceMode(const string& File);

	void AddThread(Thread&& thread);

	static void ClearPluginsCache();

	const std::unique_ptr<GeneralConfig>& GeneralCfg() const { return m_GeneralCfg; }
	const std::unique_ptr<GeneralConfig>& LocalGeneralCfg() const { return m_LocalGeneralCfg; }
	const std::unique_ptr<ColorsConfig>& ColorsCfg() const { return m_ColorsCfg; }
	const std::unique_ptr<AssociationsConfig>& AssocConfig() const { return m_AssocConfig; }
	const std::unique_ptr<PluginsCacheConfig>& PlCacheCfg() const { return m_PlCacheCfg; }
	const std::unique_ptr<PluginsHotkeysConfig>& PlHotkeyCfg() const { return m_PlHotkeyCfg; }
	const std::unique_ptr<HistoryConfig>& HistoryCfg() const { return m_HistoryCfg; }
	const std::unique_ptr<HistoryConfig>& HistoryCfgMem() const { return m_HistoryCfgMem; }

	HierarchicalConfigUniquePtr CreatePluginsConfig(const string& guid, bool Local=false);
	HierarchicalConfigUniquePtr CreateFiltersConfig();
	HierarchicalConfigUniquePtr CreateHighlightConfig();
	HierarchicalConfigUniquePtr CreateShortcutsConfig();
	HierarchicalConfigUniquePtr CreatePanelModeConfig();

private:
	template<class T> HierarchicalConfigUniquePtr CreateHierarchicalConfig(dbcheck DbId, const string& dbn, const char *xmln, bool Local = false, bool plugin = false);
	template<class T> std::unique_ptr<T> CreateDatabase(const char *son = nullptr);
	bool Import(const string& File);
	bool Export(const string& File);
	void TryImportDatabase(representable *p, const char *son = nullptr, bool plugin=false);
	void CheckDatabase(class SQLiteDb *pDb);

	int m_LoadResult;
	std::vector<Thread> m_Threads;
	std::vector<string> m_Problems;
	std::unique_ptr<representation_source> m_TemplateSource;
	mode m_Mode;

	std::unique_ptr<GeneralConfig> m_GeneralCfg;
	std::unique_ptr<GeneralConfig> m_LocalGeneralCfg;
	std::unique_ptr<ColorsConfig> m_ColorsCfg;
	std::unique_ptr<AssociationsConfig> m_AssocConfig;
	std::unique_ptr<PluginsCacheConfig> m_PlCacheCfg;
	std::unique_ptr<PluginsHotkeysConfig> m_PlHotkeyCfg;
	std::unique_ptr<HistoryConfig> m_HistoryCfg;
	std::unique_ptr<HistoryConfig> m_HistoryCfgMem;

	BitFlags CheckedDb;
};

config_provider& ConfigProvider();

#endif // CONFIGDB_HPP_552309E5_DEA6_42FD_BD7B_0F59C839FE62
