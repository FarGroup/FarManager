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

#include "sqlite.h"

struct VersionInfo;
class TiXmlElement;
class TiXmlHandle;

class SQLiteStmt {
	int param;

protected:
	sqlite3_stmt *pStmt;

public:

	SQLiteStmt() : param(1), pStmt(nullptr) { }

	~SQLiteStmt() { sqlite3_finalize(pStmt); }

	SQLiteStmt& Reset() { param=1; sqlite3_clear_bindings(pStmt); sqlite3_reset(pStmt); return *this; }

	bool Step() { return sqlite3_step(pStmt) == SQLITE_ROW; }

	bool StepAndReset() { bool b = sqlite3_step(pStmt) == SQLITE_DONE; Reset(); return b; }

	SQLiteStmt& Bind(int Value) { sqlite3_bind_int(pStmt,param++,Value); return *this; }

	SQLiteStmt& Bind(unsigned __int64 Value) { sqlite3_bind_int64(pStmt,param++,Value); return *this; }

	SQLiteStmt& Bind(__int64 Value) { sqlite3_bind_int64(pStmt,param++,Value); return *this; }

	SQLiteStmt& Bind(const wchar_t *Value, bool bStatic=true)
	{
		if (Value)
			sqlite3_bind_text16(pStmt,param++,Value,-1,bStatic?SQLITE_STATIC:SQLITE_TRANSIENT);
		else
			sqlite3_bind_null(pStmt,param++);
		return *this;
	}

	SQLiteStmt& Bind(const void *Value, size_t Size, bool bStatic=true) { sqlite3_bind_blob(pStmt,param++,Value,static_cast<int>(Size),bStatic?SQLITE_STATIC:SQLITE_TRANSIENT); return *this; }

	const wchar_t *GetColText(int Col) { return (const wchar_t *)sqlite3_column_text16(pStmt,Col); }

	const char *GetColTextUTF8(int Col) { return (const char *)sqlite3_column_text(pStmt,Col); }

	int GetColBytes(int Col) { return sqlite3_column_bytes(pStmt,Col); }

	int GetColInt(int Col) { return sqlite3_column_int(pStmt,Col); }

	unsigned __int64 GetColInt64(int Col) { return sqlite3_column_int64(pStmt,Col); }

	const char *GetColBlob(int Col) { return (const char *)sqlite3_column_blob(pStmt,Col); }

	int GetColType(int Col) { return sqlite3_column_type(pStmt,Col); }

	friend class SQLiteDb;
};

class SQLiteDb {
	sqlite3 *pDb;
	string strPath;

public:

	SQLiteDb() : pDb(nullptr) { };

	virtual ~SQLiteDb() { Close(); }

	bool Open(const wchar_t *DbFile, bool Local = false);

	void Initialize(const wchar_t* DbName);

	bool Exec(const char *Command) { return sqlite3_exec(pDb, Command, nullptr, nullptr, nullptr) == SQLITE_OK; }

	bool BeginTransaction() { return Exec("BEGIN TRANSACTION;"); }

	bool EndTransaction() { return Exec("END TRANSACTION;"); }

	bool IsOpen() { return pDb != nullptr; }

	bool InitStmt(SQLiteStmt &stmtStmt, const wchar_t *Stmt)
	{
		return sqlite3_prepare16_v2(pDb, Stmt, -1, &stmtStmt.pStmt, nullptr) == SQLITE_OK;
	}

	int Changes() { return sqlite3_changes(pDb); }

	unsigned __int64 LastInsertRowID() { return sqlite3_last_insert_rowid(pDb); }

	bool Close()
	{
		bool Result = sqlite3_close(pDb) == SQLITE_OK;
		pDb = nullptr;
		return Result;
	}

	bool SetWALJournalingMode() { return Exec("PRAGMA journal_mode = WAL;"); }

	bool EnableForeignKeysConstraints() { return Exec("PRAGMA foreign_keys = ON;"); }

	virtual bool InitializeImpl(const wchar_t* DbName) = 0;
};

class XmlConfig {

public:

	virtual ~XmlConfig() {}
	virtual TiXmlElement *Export() = 0;
	virtual bool Import(const TiXmlHandle &root) = 0;
};

class GeneralConfig: public XmlConfig, public SQLiteDb {

public:

	enum {
		TYPE_INTEGER,
		TYPE_TEXT,
		TYPE_BLOB,
		TYPE_UNKNOWN
	};

	virtual ~GeneralConfig() {}
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value) = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, const void *Value, size_t Size) = 0;
	virtual bool GetValue(const wchar_t *Key, const wchar_t *Name, DWORD *Value, DWORD Default) = 0;
	virtual bool GetValue(const wchar_t *Key, const wchar_t *Name, int *Value, int Default) = 0;
	virtual int GetValue(const wchar_t *Key, const wchar_t *Name, int Default) = 0;
	virtual bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue, const wchar_t *Default) = 0;
	virtual int GetValue(const wchar_t *Key, const wchar_t *Name, void *Value, size_t Size, const void *Default) = 0;
	virtual	bool DeleteValue(const wchar_t *Key, const wchar_t *Name) = 0;
	virtual bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, string &strValue) = 0;
	virtual bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, DWORD *Value) = 0;
};

class HierarchicalConfig: public XmlConfig, public SQLiteDb {

public:

	enum {
		TYPE_INTEGER,
		TYPE_TEXT,
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
};

class AssociationsConfig: public XmlConfig, public SQLiteDb {

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
};

class PluginsCacheConfig: public SQLiteDb  {

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
	virtual bool GetDiskMenuItem(unsigned __int64 id, int index, string &Text, string &Guid) = 0;
	virtual bool GetPluginsMenuItem(unsigned __int64 id, int index, string &Text, string &Guid) = 0;
	virtual bool GetPluginsConfigMenuItem(unsigned __int64 id, int index, string &Text, string &Guid) = 0;
	virtual string GetCommandPrefix(unsigned __int64 id) = 0;
	virtual unsigned __int64 GetFlags(unsigned __int64 id) = 0;
	virtual bool SetPreload(unsigned __int64 id, bool Preload) = 0;
	virtual bool SetSignature(unsigned __int64 id, const wchar_t *Signature) = 0;
	virtual bool SetDiskMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid) = 0;
	virtual bool SetPluginsMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid) = 0;
	virtual bool SetPluginsConfigMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid) = 0;
	virtual bool SetCommandPrefix(unsigned __int64 id, const wchar_t *Prefix) = 0;
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
};

class PluginsHotkeysConfig: public XmlConfig, public SQLiteDb  {

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
};

class HistoryConfig: public SQLiteDb  {

public:

	virtual ~HistoryConfig() {}

	//command,view,edit,folder,dialog history
	virtual bool Enum(DWORD index, DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 *id, string &strName, int *Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false) = 0;
	virtual bool Delete(unsigned __int64 id) = 0;
	virtual bool DeleteOldUnlocked(DWORD TypeHistory, const wchar_t *HistoryName, int DaysToKeep, int MinimunEntries) = 0;
	virtual bool EnumLargeHistories(DWORD index, int MinimunEntries, DWORD TypeHistory, string &strHistoryName) = 0;
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
	virtual unsigned __int64 SetEditorPos(const wchar_t *Name, int Line, int LinePos, int ScreenLine, int LeftPos, UINT CodePage) = 0;
	virtual unsigned __int64 GetEditorPos(const wchar_t *Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, UINT *CodePage) = 0;
	virtual bool SetEditorBookmark(unsigned __int64 id, int i, int Line, int LinePos, int ScreenLine, int LeftPos) = 0;
	virtual bool GetEditorBookmark(unsigned __int64 id, int i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos) = 0;
	virtual unsigned __int64 SetViewerPos(const wchar_t *Name, __int64 FilePos, __int64 LeftPos, int Hex, UINT CodePage) = 0;
	virtual unsigned __int64 GetViewerPos(const wchar_t *Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, UINT *CodePage) = 0;
	virtual bool SetViewerBookmark(unsigned __int64 id, int i, __int64 FilePos, __int64 LeftPos) = 0;
	virtual bool GetViewerBookmark(unsigned __int64 id, int i, __int64 *FilePos, __int64 *LeftPos) = 0;
	virtual void DeleteOldPositions(int DaysToKeep, int MinimunEntries) = 0;
};

class MacroConfig: public XmlConfig, public SQLiteDb {

public:

	virtual ~MacroConfig() {}

	virtual bool EnumConsts(string &strName, string &Value, string &Type) = 0;
	virtual bool GetConstValue(const wchar_t *Name, string &Value, string &Type) = 0;
	virtual unsigned __int64 SetConstValue(const wchar_t *Name, const wchar_t *Value, const wchar_t *type) = 0;
	virtual bool DeleteConst(const wchar_t *Name) = 0;

	virtual bool EnumVars(string &strName, string &Value, string &Type) = 0;
	virtual bool GetVarValue(const wchar_t *Name, string &Value, string &Type) = 0;
	virtual unsigned __int64 SetVarValue(const wchar_t *Name, const wchar_t *Value, const wchar_t *Type) = 0;
	virtual bool DeleteVar(const wchar_t *Name) = 0;

	virtual bool EnumFunctions(string &strGuid, string &strFunctionName, int *nParam, int *oParam, string &Flags, string &strSequence, string &strSyntax, string &strDescription) = 0;
	virtual unsigned __int64 SetFunction(const wchar_t *Guid, const wchar_t *FunctionName, unsigned __int64 nParam, unsigned __int64 oParam, const wchar_t *Flags, const wchar_t *Sequence, const wchar_t *Syntax, const wchar_t *Description) = 0;
	virtual bool DeleteFunction(const wchar_t *Guid, const wchar_t *Name) = 0;

	virtual bool EnumKeyMacros(string &strArea, string &strKey, string &strFlags, string &strSequence, string &strDescription) = 0;
	virtual unsigned __int64 SetKeyMacro(const wchar_t *Area, const wchar_t *Key, const wchar_t *Flags, const wchar_t *Sequence, const wchar_t *Description) = 0;
	virtual bool DeleteKeyMacro(const wchar_t *Area, const wchar_t *Key) = 0;
};

extern GeneralConfig *GeneralCfg;
extern AssociationsConfig *AssocConfig;
extern PluginsCacheConfig *PlCacheCfg;
extern PluginsHotkeysConfig *PlHotkeyCfg;
extern HistoryConfig *HistoryCfg;
extern MacroConfig *MacroCfg;

void InitDb();
void ReleaseDb();
bool ExportImportConfig(bool Export, const wchar_t *XML);
void ClearPluginsCache();

HierarchicalConfig *CreatePluginsConfig(const wchar_t *guid);
HierarchicalConfig *CreateFiltersConfig();
HierarchicalConfig *CreateHighlightConfig();
HierarchicalConfig *CreateShortcutsConfig();
HierarchicalConfig *CreatePanelModeConfig();
