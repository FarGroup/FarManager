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

#include "bitflags.hpp"
#include "strmix.hpp"
#include "synchro.hpp"

struct VersionInfo;
class SQLiteDb;

namespace tinyxml
{
	class TiXmlElement;
	class TiXmlHandle;
	class TiXmlDocument;
}

class XmlConfig {

public:

	virtual ~XmlConfig() {}
	virtual tinyxml::TiXmlElement *Export() { return nullptr; }
	virtual bool Import(const tinyxml::TiXmlHandle &root) { return true; }
};

class Transactional {

public:

	virtual ~Transactional() {}
	virtual bool BeginTransaction() = 0;
	virtual bool EndTransaction() = 0;
	virtual bool RollbackTransaction() = 0;
};

class GeneralConfig: public XmlConfig, public Transactional {

public:
	virtual ~GeneralConfig() {}
	virtual bool SetValue(const string& Key, const string& Name, const string& Value) = 0;
	virtual bool SetValue(const string& Key, const string& Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(const string& Key, const string& Name, const void *Value, size_t Size) = 0;

	virtual bool GetValue(const string& Key, const string& Name, long long *Value, long long Default) = 0;
	virtual bool GetValue(const string& Key, const string& Name, string &strValue, const wchar_t *Default) = 0;
	virtual int GetValue(const string& Key, const string& Name, void *Value, size_t Size, const void *Default) = 0;

	virtual bool DeleteValue(const string& Key, const string& Name) = 0;
	virtual bool EnumValues(const string& Key, DWORD Index, string &strName, string &strValue) = 0;
	virtual bool EnumValues(const string& Key, DWORD Index, string &strName, DWORD *Value) = 0;

protected:
	GeneralConfig() {}
};

class HierarchicalConfig: public XmlConfig, public Transactional {

public:
	virtual void AsyncFinish() = 0;
	virtual unsigned __int64 CreateKey(unsigned __int64 Root, const string& Name, const string* Description=nullptr) = 0;
	virtual unsigned __int64 GetKeyID(unsigned __int64 Root, const string& Name) = 0;
	virtual bool SetKeyDescription(unsigned __int64 Root, const string& Description) = 0;
	virtual bool SetValue(unsigned __int64 Root, const string& Name, const string& Value) = 0;
	virtual bool SetValue(unsigned __int64 Root, const string& Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(unsigned __int64 Root, const string& Name, const void *Value, size_t Size) = 0;
	virtual bool GetValue(unsigned __int64 Root, const string& Name, unsigned __int64 *Value) = 0;
	virtual bool GetValue(unsigned __int64 Root, const string& Name, string &strValue) = 0;
	virtual int GetValue(unsigned __int64 Root, const string& Name, void *Value, size_t Size) = 0;
	virtual bool DeleteKeyTree(unsigned __int64 KeyID) = 0;
	virtual bool DeleteValue(unsigned __int64 Root, const string& Name) = 0;
	virtual bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName) = 0;
	virtual bool EnumValues(unsigned __int64 Root, DWORD Index, string &strName, DWORD *Type) = 0;
	virtual bool Flush() = 0;

protected:
	HierarchicalConfig() {}
	virtual ~HierarchicalConfig() {}
};

class HierarchicalConfigDeletor
{
public:
	HierarchicalConfigDeletor() {}
	HierarchicalConfigDeletor(const HierarchicalConfigDeletor &d) {}
	void operator()(HierarchicalConfig *ptr) const { ptr->AsyncFinish(); }
};

typedef std::unique_ptr<HierarchicalConfig,HierarchicalConfigDeletor> HierarchicalConfigUniquePtr;

class ColorsConfig: public XmlConfig, public Transactional {

public:

	virtual ~ColorsConfig() {}
	virtual bool SetValue(const string& Name, const FarColor& Value) = 0;
	virtual bool GetValue(const string& Name, FarColor& Value) = 0;

protected:
	ColorsConfig() {}
};

class AssociationsConfig: public XmlConfig, public Transactional {

public:

	virtual ~AssociationsConfig() {}
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
	AssociationsConfig() {}
};

class PluginsCacheConfig: public XmlConfig, public Transactional {

public:

	virtual ~PluginsCacheConfig() {}
	virtual unsigned __int64 CreateCache(const string& CacheName) = 0;
	virtual unsigned __int64 GetCacheID(const string& CacheName) = 0;
	virtual bool DeleteCache(const string& CacheName) = 0;
	virtual bool IsPreload(unsigned __int64 id) = 0;
	virtual string GetSignature(unsigned __int64 id) = 0;
	virtual void *GetExport(unsigned __int64 id, const string& ExportName) = 0;
	virtual string GetGuid(unsigned __int64 id) = 0;
	virtual string GetTitle(unsigned __int64 id) = 0;
	virtual string GetAuthor(unsigned __int64 id) = 0;
	virtual string GetDescription(unsigned __int64 id) = 0;
	virtual bool GetMinFarVersion(unsigned __int64 id, VersionInfo *Version) = 0;
	virtual bool GetVersion(unsigned __int64 id, VersionInfo *Version) = 0;
	virtual bool GetDiskMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid) = 0;
	virtual bool GetPluginsMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid) = 0;
	virtual bool GetPluginsConfigMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid) = 0;
	virtual string GetCommandPrefix(unsigned __int64 id) = 0;
	virtual unsigned __int64 GetFlags(unsigned __int64 id) = 0;
	virtual bool SetPreload(unsigned __int64 id, bool Preload) = 0;
	virtual bool SetSignature(unsigned __int64 id, const string& Signature) = 0;
	virtual bool SetDiskMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid) = 0;
	virtual bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid) = 0;
	virtual bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid) = 0;
	virtual bool SetCommandPrefix(unsigned __int64 id, const string& Prefix) = 0;
	virtual bool SetFlags(unsigned __int64 id, unsigned __int64 Flags) = 0;
	virtual bool SetExport(unsigned __int64 id, const string& ExportName, bool Exists) = 0;
	virtual bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version) = 0;
	virtual bool SetVersion(unsigned __int64 id, const VersionInfo *Version) = 0;
	virtual bool SetGuid(unsigned __int64 id, const string& Guid) = 0;
	virtual bool SetTitle(unsigned __int64 id, const string& Title) = 0;
	virtual bool SetAuthor(unsigned __int64 id, const string& Author) = 0;
	virtual bool SetDescription(unsigned __int64 id, const string& Description) = 0;
	virtual bool EnumPlugins(DWORD index, string &CacheName) = 0;
	virtual bool DiscardCache() = 0;
	virtual bool IsCacheEmpty() = 0;

protected:
	PluginsCacheConfig() {}
};

class PluginsHotkeysConfig: public XmlConfig, public Transactional {

public:

	enum HotKeyTypeEnum {
		DRIVE_MENU,
		PLUGINS_MENU,
		CONFIG_MENU,
	};

	virtual ~PluginsHotkeysConfig() {}
	virtual bool HotkeysPresent(HotKeyTypeEnum HotKeyType) = 0;
	virtual string GetHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType) = 0;
	virtual bool SetHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType, const string& HotKey) = 0;
	virtual bool DelHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType) = 0;

protected:
	PluginsHotkeysConfig() {}
};

class HistoryConfig: public XmlConfig, public Transactional {

public:

	virtual ~HistoryConfig() {}

	//command,view,edit,folder,dialog history
	virtual bool Enum(DWORD index, DWORD TypeHistory, const string& HistoryName, unsigned __int64 *id, string &strName, int *Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false) = 0;
	virtual bool Delete(unsigned __int64 id) = 0;
	virtual bool DeleteAndAddAsync(unsigned __int64 DeleteId, DWORD TypeHistory, const string& HistoryName, string strName, int Type, bool Lock, string &strGuid, string &strFile, string &strData) = 0;
	virtual bool DeleteOldUnlocked(DWORD TypeHistory, const string& HistoryName, int DaysToKeep, int MinimumEntries) = 0;
	virtual bool EnumLargeHistories(DWORD index, int MinimumEntries, DWORD TypeHistory, string &strHistoryName) = 0;
	virtual bool GetNewest(DWORD TypeHistory, const string& HistoryName, string &strName) = 0;
	virtual bool Get(unsigned __int64 id, string &strName) = 0;
	virtual bool Get(unsigned __int64 id, string &strName, int *Type, string &strGuid, string &strFile, string &strData) = 0;
	virtual DWORD Count(DWORD TypeHistory, const string& HistoryName) = 0;
	virtual bool FlipLock(unsigned __int64 id) = 0;
	virtual bool IsLocked(unsigned __int64 id) = 0;
	virtual bool DeleteAllUnlocked(DWORD TypeHistory, const string& HistoryName) = 0;
	virtual unsigned __int64 GetNext(DWORD TypeHistory, const string& HistoryName, unsigned __int64 id, string &strName) = 0;
	virtual unsigned __int64 GetPrev(DWORD TypeHistory, const string& HistoryName, unsigned __int64 id, string &strName) = 0;
	virtual unsigned __int64 CyclicGetPrev(DWORD TypeHistory, const string& HistoryName, unsigned __int64 id, string &strName) = 0;

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
	HistoryConfig() {}
};

class Database
{
public:
	Database(char ImportExportMode='\0');
	~Database();
	bool Import(const string& File);
	bool Export(const string& File);
	int ShowProblems();

	void AddThread(HandleWrapper& Thread) {ThreadWaiter.Add(Thread);}
	void WaitForThreads() const {ThreadWaiter.Wait(true, INFINITE);}

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
	enum DBCHECK
	{
		CHECK_NONE = 0,
		CHECK_FILTERS = 0x1,
		CHECK_HIGHLIGHT = 0x2,
		CHECK_SHORTCUTS = 0x4,
		CHECK_PANELMODES = 0x8,
	};
	template<class T> HierarchicalConfigUniquePtr CreateHierarchicalConfig(DBCHECK DbId, const string& dbn, const char *xmln, bool Local=false, bool plugin=false);
	template<class T> T* CreateDatabase(const char *son = nullptr);
	void TryImportDatabase(XmlConfig *p, const char *son = nullptr, bool plugin=false);
	void CheckDatabase(SQLiteDb *pDb);

	std::list<string> m_Problems;
	std::unique_ptr<tinyxml::TiXmlDocument> m_TemplateDoc;
	tinyxml::TiXmlElement *m_TemplateRoot;
	int m_TemplateLoadState;
	char m_ImportExportMode;
	MultiWaiter ThreadWaiter;

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
