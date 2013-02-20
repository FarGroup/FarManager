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

struct VersionInfo;
class SQLiteDb;
class TiXmlElement;
class TiXmlHandle;
class TiXmlDocument;

class XmlConfig {

public:

	virtual ~XmlConfig() {}
	virtual TiXmlElement *Export() { return nullptr; }
	virtual bool Import(const TiXmlHandle &root) { return true; }
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
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value) = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, const void *Value, size_t Size) = 0;
	virtual bool GetValue(const wchar_t *Key, const wchar_t *Name, DWORD *Value, DWORD Default) = 0;
	virtual bool GetValue(const wchar_t *Key, const wchar_t *Name, int *Value, int Default) = 0;
	virtual int GetValue(const wchar_t *Key, const wchar_t *Name, int Default) = 0;
	virtual bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue, const wchar_t *Default) = 0;
	virtual int GetValue(const wchar_t *Key, const wchar_t *Name, void *Value, size_t Size, const void *Default) = 0;
	virtual bool DeleteValue(const wchar_t *Key, const wchar_t *Name) = 0;
	virtual bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, string &strValue) = 0;
	virtual bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, DWORD *Value) = 0;

protected:
	GeneralConfig() {}
};

class HierarchicalConfig: public XmlConfig, public Transactional {

public:

	enum {
		TYPE_INTEGER,
		TYPE_STRING,
		TYPE_BLOB,
		TYPE_UNKNOWN
	};

	virtual ~HierarchicalConfig() {}
	virtual unsigned __int64 CreateKey(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Description=nullptr) = 0;
	virtual unsigned __int64 GetKeyID(unsigned __int64 Root, const wchar_t *Name) = 0;
	virtual bool SetKeyDescription(unsigned __int64 Root, const wchar_t *Description) = 0;
	virtual bool SetValue(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Value) = 0;
	virtual bool SetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(unsigned __int64 Root, const wchar_t *Name, const void *Value, size_t Size) = 0;
	virtual bool GetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 *Value) = 0;
	virtual bool GetValue(unsigned __int64 Root, const wchar_t *Name, string &strValue) = 0;
	virtual int GetValue(unsigned __int64 Root, const wchar_t *Name, void *Value, size_t Size) = 0;
	virtual bool DeleteKeyTree(unsigned __int64 KeyID) = 0;
	virtual bool DeleteValue(unsigned __int64 Root, const wchar_t *Name) = 0;
	virtual bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName) = 0;
	virtual bool EnumValues(unsigned __int64 Root, DWORD Index, string &strName, DWORD *Type) = 0;
	virtual bool Flush() = 0;

protected:
	HierarchicalConfig() {}
};

class ColorsConfig: public XmlConfig, public Transactional {

public:

	virtual ~ColorsConfig() {}
	virtual bool SetValue(const wchar_t *Name, const FarColor& Value) = 0;
	virtual bool GetValue(const wchar_t *Name, FarColor& Value) = 0;

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
	virtual bool SetCommand(unsigned __int64 id, int Type, const wchar_t *Command, bool Enabled) = 0;
	virtual bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2) = 0;
	virtual unsigned __int64 AddType(unsigned __int64 after_id, const wchar_t *Mask, const wchar_t *Description) = 0;
	virtual bool UpdateType(unsigned __int64 id, const wchar_t *Mask, const wchar_t *Description) = 0;
	virtual bool DelType(unsigned __int64 id) = 0;

protected:
	AssociationsConfig() {}
};

class PluginsCacheConfig: public XmlConfig, public Transactional {

public:

	virtual ~PluginsCacheConfig() {}
	virtual unsigned __int64 CreateCache(const wchar_t *CacheName) = 0;
	virtual unsigned __int64 GetCacheID(const wchar_t *CacheName) = 0;
	virtual bool DeleteCache(const wchar_t *CacheName) = 0;
	virtual bool IsPreload(unsigned __int64 id) = 0;
	virtual string GetSignature(unsigned __int64 id) = 0;
	virtual void *GetExport(unsigned __int64 id, const wchar_t *ExportName) = 0;
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
#if defined(MANTIS_0000466)
	virtual string GetMacroFunctions(unsigned __int64 id) = 0;
#endif
	virtual unsigned __int64 GetFlags(unsigned __int64 id) = 0;
	virtual bool SetPreload(unsigned __int64 id, bool Preload) = 0;
	virtual bool SetSignature(unsigned __int64 id, const wchar_t *Signature) = 0;
	virtual bool SetDiskMenuItem(unsigned __int64 id, size_t index, const wchar_t *Text, const wchar_t *Guid) = 0;
	virtual bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const wchar_t *Text, const wchar_t *Guid) = 0;
	virtual bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const wchar_t *Text, const wchar_t *Guid) = 0;
	virtual bool SetCommandPrefix(unsigned __int64 id, const wchar_t *Prefix) = 0;
#if defined(MANTIS_0000466)
	virtual bool SetMacroFunctions(unsigned __int64 id, const wchar_t *MacroFunc) = 0;
#endif
	virtual bool SetFlags(unsigned __int64 id, unsigned __int64 Flags) = 0;
	virtual bool SetExport(unsigned __int64 id, const wchar_t *ExportName, bool Exists) = 0;
	virtual bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version) = 0;
	virtual bool SetVersion(unsigned __int64 id, const VersionInfo *Version) = 0;
	virtual bool SetGuid(unsigned __int64 id, const wchar_t *Guid) = 0;
	virtual bool SetTitle(unsigned __int64 id, const wchar_t *Title) = 0;
	virtual bool SetAuthor(unsigned __int64 id, const wchar_t *Author) = 0;
	virtual bool SetDescription(unsigned __int64 id, const wchar_t *Description) = 0;
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
	virtual string GetHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType) = 0;
	virtual bool SetHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType, const wchar_t *HotKey) = 0;
	virtual bool DelHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType) = 0;

protected:
	PluginsHotkeysConfig() {}
};

class HistoryConfig: public XmlConfig, public Transactional {

public:

	virtual ~HistoryConfig() {}

	//command,view,edit,folder,dialog history
	virtual bool Enum(DWORD index, DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 *id, string &strName, int *Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false) = 0;
	virtual bool Delete(unsigned __int64 id) = 0;
	virtual bool DeleteOldUnlocked(DWORD TypeHistory, const wchar_t *HistoryName, int DaysToKeep, int MinimumEntries) = 0;
	virtual bool EnumLargeHistories(DWORD index, int MinimumEntries, DWORD TypeHistory, string &strHistoryName) = 0;
	virtual bool Add(DWORD TypeHistory, const wchar_t *HistoryName, string strName, int Type, bool Lock, string &strGuid, string &strFile, string &strData) = 0;
	virtual bool GetNewest(DWORD TypeHistory, const wchar_t *HistoryName, string &strName) = 0;
	virtual bool Get(unsigned __int64 id, string &strName) = 0;
	virtual bool Get(unsigned __int64 id, string &strName, int *Type, string &strGuid, string &strFile, string &strData) = 0;
	virtual DWORD Count(DWORD TypeHistory, const wchar_t *HistoryName) = 0;
	virtual bool FlipLock(unsigned __int64 id) = 0;
	virtual bool IsLocked(unsigned __int64 id) = 0;
	virtual bool DeleteAllUnlocked(DWORD TypeHistory, const wchar_t *HistoryName) = 0;
	virtual unsigned __int64 GetNext(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName) = 0;
	virtual unsigned __int64 GetPrev(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName) = 0;
	virtual unsigned __int64 CyclicGetPrev(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName) = 0;

	//view,edit file positions and bookmarks history
	virtual unsigned __int64 SetEditorPos(const wchar_t *Name, int Line, int LinePos, int ScreenLine, int LeftPos, uintptr_t CodePage) = 0;
	virtual unsigned __int64 GetEditorPos(const wchar_t *Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, uintptr_t *CodePage) = 0;
	virtual bool SetEditorBookmark(unsigned __int64 id, int i, int Line, int LinePos, int ScreenLine, int LeftPos) = 0;
	virtual bool GetEditorBookmark(unsigned __int64 id, int i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos) = 0;
	virtual unsigned __int64 SetViewerPos(const wchar_t *Name, __int64 FilePos, __int64 LeftPos, int Hex_Wrap, uintptr_t CodePage) = 0;
	virtual unsigned __int64 GetViewerPos(const wchar_t *Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, uintptr_t *CodePage) = 0;
	virtual bool SetViewerBookmark(unsigned __int64 id, int i, __int64 FilePos, __int64 LeftPos) = 0;
	virtual bool GetViewerBookmark(unsigned __int64 id, int i, __int64 *FilePos, __int64 *LeftPos) = 0;
	virtual void DeleteOldPositions(int DaysToKeep, int MinimumEntries) = 0;

protected:
	HistoryConfig() {}
};

class Database
{
public:
	Database(bool ImportExportMode=false);
	~Database();
	bool Import(const wchar_t *File);
	bool Export(const wchar_t *File);
	int ShowProblems();

	static void ClearPluginsCache();

	GeneralConfig *GeneralCfg() const { return m_GeneralCfg; }
	GeneralConfig *LocalGeneralCfg() const { return m_LocalGeneralCfg; }
	ColorsConfig *ColorsCfg() const { return m_ColorsCfg; }
	AssociationsConfig *AssocConfig() const { return m_AssocConfig; }
	PluginsCacheConfig *PlCacheCfg() const { return m_PlCacheCfg; }
	PluginsHotkeysConfig *PlHotkeyCfg() const { return m_PlHotkeyCfg; }
	HistoryConfig *HistoryCfg() const { return m_HistoryCfg; }
	HistoryConfig *HistoryCfgMem() const { return m_HistoryCfgMem; }


	HierarchicalConfig *CreatePluginsConfig(const wchar_t *guid, bool Local=false);
	HierarchicalConfig *CreateFiltersConfig();
	HierarchicalConfig *CreateHighlightConfig();
	HierarchicalConfig *CreateShortcutsConfig();
	HierarchicalConfig *CreatePanelModeConfig();

private:
	enum DBCHECK
	{
		CHECK_NONE = 0,
		CHECK_FILTERS = 0x1,
		CHECK_HIGHLIGHT = 0x2,
		CHECK_SHORTCUTS = 0x4,
		CHECK_PANELMODES = 0x8,
	};
	template<class T> HierarchicalConfig *CreateHierarchicalConfig(DBCHECK DbId, const wchar_t *dbn, const char *xmln, bool Local=false, bool plugin=false);
	template<class T> T* CreateDatabase(const char *son = nullptr);
	void TryImportDatabase(XmlConfig *p, const char *son = nullptr, bool plugin=false);
	void CheckDatabase(SQLiteDb *pDb);

	TiXmlDocument *m_TemplateDoc;
	TiXmlElement *m_TemplateRoot;
	int m_TemplateLoadState;
	bool m_ImportExportMode;

	GeneralConfig *m_GeneralCfg;
	GeneralConfig *m_LocalGeneralCfg;
	ColorsConfig *m_ColorsCfg;
	AssociationsConfig *m_AssocConfig;
	PluginsCacheConfig *m_PlCacheCfg;
	PluginsHotkeysConfig *m_PlHotkeyCfg;
	HistoryConfig *m_HistoryCfg;
	HistoryConfig *m_HistoryCfgMem;

	BitFlags CheckedDb;
	std::list<string> m_Problems;
};

template<class T, class Y>
const wchar_t* GetNameOfValue(T Value, const Y& From)
{
	for(size_t i = 0; i < ARRAYSIZE(From); ++i)
	{
		if(From[i].Value == Value)
		{
			return From[i].Name;
		}
	}
	return L"";
}

template<class T>
auto GetValueOfVame(const wchar_t* Name, const T& From) -> decltype(From->Value)
{
	for(size_t i = 0; i < ARRAYSIZE(From); ++i)
	{
		if(!StrCmpI(From[i].Name, Name))
		{
			return From[i].Value;
		}
	}
	return 0;
}

template<class T, class Y>
const string FlagsToString(T Flags, const Y& From)
{
	string strFlags;
	for(size_t i = 0; T(1) << i <= Flags; ++i)
	{
		if(Flags&(T(1)<<i))
		{
			if(!strFlags.IsEmpty())
			{
				strFlags += L"|";
			}
			strFlags+=GetNameOfValue(Flags&(T(1)<<i), From);
		}
	}
	return strFlags;
}

template<class T>
auto StringToFlags(const string& strFlags, const T& From) -> decltype(From->Value)
{
	decltype(From->Value) Flags = 0;
	if(!strFlags.IsEmpty())
	{
		auto FlagList(StringToList(strFlags, STLF_UNIQUE, L"|"));
		std::for_each(CONST_RANGE(FlagList, i)
		{
			Flags |= GetValueOfVame(i, From);
		});
	}
	return Flags;
}
