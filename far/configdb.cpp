/*
configdb.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "configdb.hpp"
#include "strmix.hpp"
#include "pathmix.hpp"
#include "sqlite/sqlite3.h"
#include "config.hpp"

GeneralConfig *GeneralCfg;
AssociationsConfig *AssocConfig;
PluginsCacheConfig *PlCacheCfg;
PluginsHotkeysConfig *PlHotkeyCfg;

void GetDatabasePath(const wchar_t *FileName, string &strOut)
{
	strOut = Opt.ProfilePath;
	AddEndSlash(strOut);
	strOut += FileName;
}

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

	SQLiteStmt& Bind(const wchar_t *Value, bool bStatic=true)
	{
		if (Value)
			sqlite3_bind_text16(pStmt,param++,Value,-1,bStatic?SQLITE_STATIC:SQLITE_TRANSIENT);
		else
			sqlite3_bind_null(pStmt,param++);
		return *this;
	}

	SQLiteStmt& Bind(const char *Value, int Size, bool bStatic=true) { sqlite3_bind_blob(pStmt,param++,Value,Size,bStatic?SQLITE_STATIC:SQLITE_TRANSIENT); return *this; }

	const wchar_t *GetColText(int Col) { return (const wchar_t *)sqlite3_column_text16(pStmt,Col); }

	int GetColBytes(int Col) { return sqlite3_column_bytes(pStmt,Col); }

	int GetColInt(int Col) { return sqlite3_column_int(pStmt,Col); }

	unsigned __int64 GetColInt64(int Col) { return sqlite3_column_int64(pStmt,Col); }

	const char *GetColBlob(int Col) { return (const char *)sqlite3_column_blob(pStmt,Col); }

	int GetColType(int Col) { return sqlite3_column_type(pStmt,Col); }

	friend class SQLiteDb;
};

class SQLiteDb {
	sqlite3 *pDb;

public:

	SQLiteDb() : pDb(nullptr) { };

	~SQLiteDb()	{ Close(); }

	bool Open(const wchar_t *DbFile)
	{
		string strPath;
		GetDatabasePath(DbFile, strPath);
		if (sqlite3_open16(strPath.CPtr(),&pDb) != SQLITE_OK)
		{
			sqlite3_close(pDb);
			pDb = nullptr;
			//if failed, let's open a memory only db so Far can work anyway
			if (sqlite3_open16(L":memory:",&pDb) != SQLITE_OK)
			{
				sqlite3_close(pDb);
				pDb = nullptr;
				return false;
			}
		}
		return true;
	}

	bool Exec(const char *Command) { return sqlite3_exec(pDb, Command, nullptr, nullptr, nullptr) == SQLITE_OK; }

	bool BeginTransaction() { return Exec("BEGIN TRANSACTION;"); }

	bool EndTransaction() { return Exec("END TRANSACTION;"); }

	bool IsOpen() { return pDb != nullptr; }

	bool InitStmt(SQLiteStmt &stmtStmt, const wchar_t *Stmt) { return sqlite3_prepare16_v2(pDb, Stmt, -1, &stmtStmt.pStmt, nullptr) == SQLITE_OK; }

	int Changes() { return sqlite3_changes(pDb); }

	unsigned __int64 LastInsertRowID() { return sqlite3_last_insert_rowid(pDb); }

	bool Close() { return sqlite3_close(pDb) == SQLITE_OK; }
};

class GeneralConfigDb: public GeneralConfig {
	SQLiteDb   db;
	SQLiteStmt stmtUpdateValue;
	SQLiteStmt stmtInsertValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtDelValue;
	SQLiteStmt stmtEnumValues;

public:

	GeneralConfigDb()
	{
		if (!db.Open(L"generalconfig.db"))
			return;

		//schema
		db.Exec("CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));");

		//update value statement
		db.InitStmt(stmtUpdateValue, L"UPDATE general_config SET value=?1 WHERE key=?2 AND name=?3;");

		//insert value statement
		db.InitStmt(stmtInsertValue, L"INSERT INTO general_config VALUES (?1,?2,?3);");

		//get value statement
		db.InitStmt(stmtGetValue, L"SELECT value FROM general_config WHERE key=?1 AND name=?2;");

		//delete value statement
		db.InitStmt(stmtDelValue, L"DELETE FROM general_config WHERE key=?1 AND name=?2;");

		//enum values statement
		db.InitStmt(stmtEnumValues, L"SELECT name, value FROM general_config WHERE key=?1;");
	}

	virtual ~GeneralConfigDb() { }

	void BeginTransaction() { db.BeginTransaction(); }

	void EndTransaction() { db.EndTransaction(); }

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || db.Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || db.Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const char *Value, int Size)
	{
		bool b = stmtUpdateValue.Bind(Value,Size).Bind(Key).Bind(Name).StepAndReset();
		if (!b || db.Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value,Size).StepAndReset();
		return b;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 *Value)
	{
		bool b = stmtGetValue.Bind(Key).Bind(Name).Step();
		if (b)
			*Value = stmtGetValue.GetColInt64(0);
		stmtGetValue.Reset();
		return b;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
	{
		bool b = stmtGetValue.Bind(Key).Bind(Name).Step();
		if (b)
			strValue = stmtGetValue.GetColText(0);
		stmtGetValue.Reset();
		return b;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, char *Value, int Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Key).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,Min(realsize,Size));
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, DWORD *Value, DWORD Default)
	{
		unsigned __int64 v;
		if (GetValue(Key,Name,&v))
		{   *Value = (DWORD)v;
			return true;
		}
		*Value = Default;
		return false;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, int *Value, int Default)
	{
		unsigned __int64 v;
		if (GetValue(Key,Name,&v))
		{   *Value = (int)v;
			return true;
		}
		*Value = Default;
		return false;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, int Default)
	{
		unsigned __int64 v;
		if (GetValue(Key,Name,&v))
			return (int)v;
		return Default;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue, const wchar_t *Default)
	{
		if (GetValue(Key,Name,strValue))
			return true;
		strValue=Default;
		return false;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, char *Value, int Size, const char *Default)
	{
		int s = GetValue(Key,Name,Value,Size);
		if (s)
			return s;
		if (Default)
		{
			memcpy(Value,Default,Size);
			return Size;
		}
		return 0;
	}

	bool DeleteValue(const wchar_t *Key, const wchar_t *Name)
	{
		return stmtDelValue.Bind(Key).Bind(Name).StepAndReset();
	}

	bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, string &strValue)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Key,false);

		if (stmtEnumValues.Step())
		{
			strName = stmtEnumValues.GetColText(0);
			strValue = stmtEnumValues.GetColText(1);
			return true;
		}

		return false;
	}

	bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, DWORD *Value)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Key,false);

		if (stmtEnumValues.Step())
		{
			strName = stmtEnumValues.GetColText(0);
			*Value = (DWORD)stmtEnumValues.GetColInt(1);
			return true;
		}

		return false;
	}
};

class PluginsConfigDb: public PluginsConfig {
	SQLiteDb   db;
	SQLiteStmt stmtCreateKey;
	SQLiteStmt stmtFindKey;
	SQLiteStmt stmtSetKeyDescription;
	SQLiteStmt stmtSetValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtEnumKeys;
	SQLiteStmt stmtEnumValues;
	SQLiteStmt stmtDelValue;

public:

	PluginsConfigDb()
	{
		if (!db.Open(L"pluginsconfig.db"))
			return;

		//schema
		db.Exec(
			"PRAGMA foreign_keys = ON;"
			"CREATE TABLE IF NOT EXISTS plugin_keys(id INTEGER PRIMARY KEY ASC, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES plugin_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
			"CREATE TABLE IF NOT EXISTS plugin_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES plugin_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id > 0));"
		);

		//root key (needs to be before the transaction start)
		db.Exec("INSERT INTO plugin_keys VALUES (0,0,\"\",\"Root - do not edit\");");

		db.BeginTransaction();

		//create key statement
		db.InitStmt(stmtCreateKey, L"INSERT INTO plugin_keys VALUES (NULL,?1,?2,?3);");

		//find key statement
		db.InitStmt(stmtFindKey, L"SELECT id FROM plugin_keys WHERE parent_id=?1 AND name=?2 AND id>0;");

		//set key description statement
		db.InitStmt(stmtSetKeyDescription, L"UPDATE plugin_keys SET description=?1 WHERE id=?2 AND id>0 AND description<>?1;");

		//set value statement
		db.InitStmt(stmtSetValue, L"INSERT OR REPLACE INTO plugin_values VALUES (?1,?2,?3);");

		//get value statement
		db.InitStmt(stmtGetValue, L"SELECT value FROM plugin_values WHERE key_id=?1 AND name=?2;");

		//enum keys statement
		db.InitStmt(stmtEnumKeys, L"SELECT name FROM plugin_keys WHERE parent_id=?1 AND id>0;");

		//enum values statement
		db.InitStmt(stmtEnumValues, L"SELECT name, value FROM plugin_values WHERE key_id=?1;");

		//delete value statement
		db.InitStmt(stmtDelValue, L"DELETE FROM plugin_values WHERE key_id=?1 AND name=?2;");
	}

	virtual ~PluginsConfigDb() { db.EndTransaction(); }

	bool Flush()
	{
		bool b = db.EndTransaction();
		db.BeginTransaction();
		return b;
	}

	unsigned __int64 CreateKey(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Description=nullptr)
	{
		if (stmtCreateKey.Bind(Root).Bind(Name).Bind(Description).StepAndReset())
			return db.LastInsertRowID();
		unsigned __int64 id = GetKeyID(Root,Name);
		if (id && Description)
			SetKeyDescription(id,Description);
		return id;
	}

	unsigned __int64 GetKeyID(unsigned __int64 Root, const wchar_t *Name)
	{
		unsigned __int64 id = 0;
		if (stmtFindKey.Bind(Root).Bind(Name).Step())
			id = stmtFindKey.GetColInt64(0);
		stmtFindKey.Reset();
		return id;
	}

	bool SetKeyDescription(unsigned __int64 Root, const wchar_t *Description)
	{
		return stmtSetKeyDescription.Bind(Description).Bind(Root).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Value)
	{
		if (!Name)
			return SetKeyDescription(Root,Value);
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 Value)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const char *Value, int Size)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value,Size).StepAndReset();
	}

	bool GetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 *Value)
	{
		bool b = stmtGetValue.Bind(Root).Bind(Name).Step();
		if (b)
			*Value = stmtGetValue.GetColInt64(0);
		stmtGetValue.Reset();
		return b;
	}

	bool GetValue(unsigned __int64 Root, const wchar_t *Name, string &strValue)
	{
		bool b = stmtGetValue.Bind(Root).Bind(Name).Step();
		if (b)
			strValue = stmtGetValue.GetColText(0);
		stmtGetValue.Reset();
		return b;
	}

	int GetValue(unsigned __int64 Root, const wchar_t *Name, char *Value, int Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Root).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,Min(realsize,Size));
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool DeleteKeyTree(unsigned __int64 KeyID)
	{
		//All subtree is automatically deleted because of foreign key constraints
		SQLiteStmt stmtDeleteTree;
		db.InitStmt(stmtDeleteTree, L"DELETE FROM plugin_keys WHERE id=?1 AND id>0;");
		return stmtDeleteTree.Bind(KeyID).StepAndReset();
	}

	bool DeleteValue(unsigned __int64 Root, const wchar_t *Name)
	{
		return stmtDelValue.Bind(Root).Bind(Name).StepAndReset();
	}

	bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName)
	{
		if (Index == 0)
			stmtEnumKeys.Reset().Bind(Root);

		if (stmtEnumKeys.Step())
		{
			strName = stmtEnumKeys.GetColText(0);
			return true;
		}

		return false;
	}

	bool EnumValues(unsigned __int64 Root, DWORD Index, string &strName, DWORD *Type)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Root);

		if (stmtEnumValues.Step())
		{
			strName = stmtEnumValues.GetColText(0);
			switch (stmtEnumValues.GetColType(1))
			{
				case SQLITE_INTEGER: *Type = TYPE_INTEGER; break;
				case SQLITE_TEXT: *Type = TYPE_TEXT; break;
				case SQLITE_BLOB: *Type = TYPE_BLOB; break;
				default: *Type = TYPE_UNKNOWN;
			}

			return true;
		}

		return false;

	}
};

class AssociationsConfigDb: public AssociationsConfig {
	SQLiteDb   db;
	SQLiteStmt stmtReorder;
	SQLiteStmt stmtAddType;
	SQLiteStmt stmtGetMask;
	SQLiteStmt stmtGetDescription;
	SQLiteStmt stmtUpdateType;
	SQLiteStmt stmtSetCommand;
	SQLiteStmt stmtGetCommand;
	SQLiteStmt stmtEnumTypes;
	SQLiteStmt stmtEnumMasks;
	SQLiteStmt stmtEnumMasksForType;
	SQLiteStmt stmtDelType;
	SQLiteStmt stmtGetWeight;
	SQLiteStmt stmtSetWeight;

public:

	AssociationsConfigDb()
	{
		if (!db.Open(L"associations.db"))
			return;

		//schema
		db.Exec(
			"PRAGMA foreign_keys = ON;"
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
		);

		//add new type and reorder statements
		db.InitStmt(stmtReorder, L"UPDATE filetypes SET weight=weight+1 WHERE weight>(CASE ?1 WHEN 0 THEN 0 ELSE (SELECT weight FROM filetypes WHERE id=?1) END);");
		db.InitStmt(stmtAddType, L"INSERT INTO filetypes VALUES (NULL,(CASE ?1 WHEN 0 THEN 1 ELSE (SELECT weight FROM filetypes WHERE id=?1)+1 END),?2,?3);");

		//get mask statement
		db.InitStmt(stmtGetMask, L"SELECT mask FROM filetypes WHERE id=?1;");

		//get description statement
		db.InitStmt(stmtGetDescription, L"SELECT description FROM filetypes WHERE id=?1;");

		//update type statement
		db.InitStmt(stmtUpdateType, L"UPDATE filetypes SET mask=?1, description=?2 WHERE id=?3;");

		//set association statement
		db.InitStmt(stmtSetCommand, L"INSERT OR REPLACE INTO commands VALUES (?1,?2,?3,?4);");

		//get association statement
		db.InitStmt(stmtGetCommand, L"SELECT command, enabled FROM commands WHERE ft_id=?1 AND type=?2;");

		//enum types statement
		db.InitStmt(stmtEnumTypes, L"SELECT id, description FROM filetypes ORDER BY weight;");

		//enum masks statement
		db.InitStmt(stmtEnumMasks, L"SELECT id, mask FROM filetypes ORDER BY weight;");

		//enum masks with a specific type on statement
		db.InitStmt(stmtEnumMasksForType, L"SELECT id, mask FROM filetypes, commands WHERE id=ft_id AND type=?1 AND enabled<>0 ORDER BY weight;");

		//delete type statement
		db.InitStmt(stmtDelType, L"DELETE FROM filetypes WHERE id=?1;");

		//get weight and set weight statements
		db.InitStmt(stmtGetWeight, L"SELECT weight FROM filetypes WHERE id=?1;");
		db.InitStmt(stmtSetWeight, L"UPDATE filetypes SET weight=?1 WHERE id=?2;");
	}

	virtual ~AssociationsConfigDb() { }

	void BeginTransaction() { db.BeginTransaction(); }

	void EndTransaction() { db.EndTransaction(); }

	bool EnumMasks(DWORD Index, unsigned __int64 *id, string &strMask)
	{
		if (Index == 0)
			stmtEnumMasks.Reset();

		if (stmtEnumMasks.Step())
		{
			*id = stmtEnumMasks.GetColInt64(0);
			strMask = stmtEnumMasks.GetColText(1);
			return true;
		}

		return false;
	}

	bool EnumMasksForType(int Type, DWORD Index, unsigned __int64 *id, string &strMask)
	{
		if (Index == 0)
			stmtEnumMasksForType.Reset().Bind(Type);

		if (stmtEnumMasksForType.Step())
		{
			*id = stmtEnumMasksForType.GetColInt64(0);
			strMask = stmtEnumMasksForType.GetColText(1);
			return true;
		}

		return false;
	}

	bool GetMask(unsigned __int64 id, string &strMask)
	{
		bool b = stmtGetMask.Bind(id).Step();
		if (b)
			strMask = stmtGetMask.GetColText(0);
		stmtGetMask.Reset();
		return b;
	}

	bool GetDescription(unsigned __int64 id, string &strDescription)
	{
		bool b = stmtGetDescription.Bind(id).Step();
		if (b)
			strDescription = stmtGetDescription.GetColText(0);
		stmtGetDescription.Reset();
		return b;
	}

	bool GetCommand(unsigned __int64 id, int Type, string &strCommand, bool *Enabled=nullptr)
	{
		bool b = stmtGetCommand.Bind(id).Bind(Type).Step();
		if (b)
		{
			strCommand = stmtGetCommand.GetColText(0);
			if (Enabled)
				*Enabled = stmtGetCommand.GetColInt(1) ? true : false;
		}
		stmtGetCommand.Reset();
		return b;
	}

	bool SetCommand(unsigned __int64 id, int Type, const wchar_t *Command, bool Enabled)
	{
		return stmtSetCommand.Bind(id).Bind(Type).Bind(Enabled?1:0).Bind(Command).StepAndReset();
	}

	bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2)
	{
		if (stmtGetWeight.Bind(id1).Step())
		{
			unsigned __int64 weight1 = stmtGetWeight.GetColInt64(0);
			stmtGetWeight.Reset();
			if (stmtGetWeight.Bind(id2).Step())
			{
				unsigned __int64 weight2 = stmtGetWeight.GetColInt64(0);
				stmtGetWeight.Reset();
				return stmtSetWeight.Bind(weight1).Bind(id2).StepAndReset() && stmtSetWeight.Bind(weight2).Bind(id1).StepAndReset();
			}
		}
		stmtGetWeight.Reset();
		return false;
	}

	unsigned __int64 AddType(unsigned __int64 after_id, const wchar_t *Mask, const wchar_t *Description)
	{
		if (stmtReorder.Bind(after_id).StepAndReset() && stmtAddType.Bind(after_id).Bind(Mask).Bind(Description).StepAndReset())
			return db.LastInsertRowID();
		return 0;
	}

	bool UpdateType(unsigned __int64 id, const wchar_t *Mask, const wchar_t *Description)
	{
		return stmtUpdateType.Bind(Mask).Bind(Description).Bind(id).StepAndReset();
	}

	bool DelType(unsigned __int64 id)
	{
		return stmtDelType.Bind(id).StepAndReset();
	}
};

class PluginsCacheConfigDb: public PluginsCacheConfig {
	SQLiteDb   db;
	SQLiteStmt stmtCreateCache;
	SQLiteStmt stmtFindCacheName;
	SQLiteStmt stmtDelCache;
	SQLiteStmt stmtCountCacheNames;
	SQLiteStmt stmtGetPreloadState;
	SQLiteStmt stmtGetSignature;
	SQLiteStmt stmtGetExportState;
	SQLiteStmt stmtGetGuid;
	SQLiteStmt stmtGetTitle;
	SQLiteStmt stmtGetAuthor;
	SQLiteStmt stmtGetPrefix;
	SQLiteStmt stmtGetDescription;
	SQLiteStmt stmtGetFlags;
	SQLiteStmt stmtGetMinFarVersion;
	SQLiteStmt stmtGetVersion;
	SQLiteStmt stmtSetPreloadState;
	SQLiteStmt stmtSetSignature;
	SQLiteStmt stmtSetExportState;
	SQLiteStmt stmtSetGuid;
	SQLiteStmt stmtSetTitle;
	SQLiteStmt stmtSetAuthor;
	SQLiteStmt stmtSetPrefix;
	SQLiteStmt stmtSetDescription;
	SQLiteStmt stmtSetFlags;
	SQLiteStmt stmtSetMinFarVersion;
	SQLiteStmt stmtSetVersion;
	SQLiteStmt stmtEnumCache;
	SQLiteStmt stmtGetMenuItem;
	SQLiteStmt stmtSetMenuItem;

	enum MenuItemTypeEnum {
		PLUGINS_MENU,
		CONFIG_MENU,
		DRIVE_MENU
	};

	bool GetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, int index, string &Text, string &Guid)
	{
		bool b = stmtGetMenuItem.Bind(id).Bind((int)type).Bind(index).Step();
		if (b)
		{
			Text = stmtGetMenuItem.GetColText(0);
			Guid = stmtGetMenuItem.GetColText(1);
		}
		stmtGetMenuItem.Reset();
		return b;
	}

	bool SetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return stmtSetMenuItem.Bind(id).Bind((int)type).Bind(index).Bind(Guid).Bind(Text).StepAndReset();
	}

	string GetTextFromID(SQLiteStmt &stmt, unsigned __int64 id)
	{
		string strText;
		if (stmt.Bind(id).Step())
			strText = stmt.GetColText(0);
		stmt.Reset();
		return strText;
	}

public:

	PluginsCacheConfigDb()
	{
		if (!db.Open(L"plugincache.db"))
			return;

		//schema
		db.Exec(
			"PRAGMA foreign_keys = ON;"
			"CREATE TABLE IF NOT EXISTS cachename(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE);"
			"CREATE TABLE IF NOT EXISTS preload(cid INTEGER NOT NULL PRIMARY KEY, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS signatures(cid INTEGER NOT NULL PRIMARY KEY, signature TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS guids(cid INTEGER NOT NULL PRIMARY KEY, guid TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS titles(cid INTEGER NOT NULL PRIMARY KEY, title TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS authors(cid INTEGER NOT NULL PRIMARY KEY, author TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS descriptions(cid INTEGER NOT NULL PRIMARY KEY, description TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS minfarversions(cid INTEGER NOT NULL PRIMARY KEY, version BLOB NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS pluginversions(cid INTEGER NOT NULL PRIMARY KEY, version BLOB NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS flags(cid INTEGER NOT NULL PRIMARY KEY, bitmask INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS prefixes(cid INTEGER NOT NULL PRIMARY KEY, prefix TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
			"CREATE TABLE IF NOT EXISTS exports(cid INTEGER NOT NULL, export TEXT NOT NULL, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, export));"
			"CREATE TABLE IF NOT EXISTS menuitems(cid INTEGER NOT NULL, type INTEGER NOT NULL, number INTEGER NOT NULL, guid TEXT NOT NULL, name TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, type, number));"
		);

		//get menu item text and guid statement
		db.InitStmt(stmtGetMenuItem, L"SELECT name, guid FROM menuitems WHERE cid=?1 AND type=?2 AND number=?3;");

		//set menu item statement
		db.InitStmt(stmtSetMenuItem, L"INSERT OR REPLACE INTO menuitems VALUES (?1,?2,?3,?4,?5);");

		//add new cache name statement
		db.InitStmt(stmtCreateCache, L"INSERT INTO cachename VALUES (NULL,?1);");

		//get cache id by name statement
		db.InitStmt(stmtFindCacheName, L"SELECT id FROM cachename WHERE name=?1;");

		//del cache by name statement
		db.InitStmt(stmtDelCache, L"DELETE FROM cachename WHERE name=?1;");

		//count cache names statement
		db.InitStmt(stmtCountCacheNames, L"SELECT count(name) FROM cachename");

		//get preload state statement
		db.InitStmt(stmtGetPreloadState, L"SELECT enabled FROM preload WHERE cid=?1;");

		//get signature statement
		db.InitStmt(stmtGetSignature, L"SELECT signature FROM signatures WHERE cid=?1;");

		//get export state statement
		db.InitStmt(stmtGetExportState, L"SELECT enabled FROM exports WHERE cid=?1 and export=?2;");

		//get guid statement
		db.InitStmt(stmtGetGuid, L"SELECT guid FROM guids WHERE cid=?1;");

		//get title statement
		db.InitStmt(stmtGetTitle, L"SELECT title FROM titles WHERE cid=?1;");

		//get author statement
		db.InitStmt(stmtGetAuthor, L"SELECT author FROM authors WHERE cid=?1;");

		//get description statement
		db.InitStmt(stmtGetDescription, L"SELECT description FROM descriptions WHERE cid=?1;");

		//get command prefix statement
		db.InitStmt(stmtGetPrefix, L"SELECT prefix FROM prefixes WHERE cid=?1;");

		//get flags statement
		db.InitStmt(stmtGetFlags, L"SELECT bitmask FROM flags WHERE cid=?1;");

		//get MinFarVersion statement
		db.InitStmt(stmtGetMinFarVersion, L"SELECT version FROM minfarversions WHERE cid=?1;");

		//get plugin version statement
		db.InitStmt(stmtGetVersion, L"SELECT version FROM pluginversions WHERE cid=?1;");

		//set preload state statement
		db.InitStmt(stmtSetPreloadState, L"INSERT OR REPLACE INTO preload VALUES (?1,?2);");

		//set signature statement
		db.InitStmt(stmtSetSignature, L"INSERT OR REPLACE INTO signatures VALUES (?1,?2);");

		//set export state statement
		db.InitStmt(stmtSetExportState, L"INSERT OR REPLACE INTO exports VALUES (?1,?2,?3);");

		//set guid statement
		db.InitStmt(stmtSetGuid, L"INSERT OR REPLACE INTO guids VALUES (?1,?2);");

		//set title statement
		db.InitStmt(stmtSetTitle, L"INSERT OR REPLACE INTO titles VALUES (?1,?2);");

		//set author statement
		db.InitStmt(stmtSetAuthor, L"INSERT OR REPLACE INTO authors VALUES (?1,?2);");

		//set description statement
		db.InitStmt(stmtSetDescription, L"INSERT OR REPLACE INTO descriptions VALUES (?1,?2);");

		//set command prefix statement
		db.InitStmt(stmtSetPrefix, L"INSERT OR REPLACE INTO prefixes VALUES (?1,?2);");

		//set flags statement
		db.InitStmt(stmtSetFlags, L"INSERT OR REPLACE INTO flags VALUES (?1,?2);");

		//set MinFarVersion statement
		db.InitStmt(stmtSetMinFarVersion, L"INSERT OR REPLACE INTO minfarversions VALUES (?1,?2);");

		//set plugin version statement
		db.InitStmt(stmtSetVersion, L"INSERT OR REPLACE INTO pluginversions VALUES (?1,?2);");

		//enum cache names statement
		db.InitStmt(stmtEnumCache, L"SELECT name FROM cachename ORDER BY name;");
	}

	virtual ~PluginsCacheConfigDb() {}

	void BeginTransaction() { db.BeginTransaction(); }

	void EndTransaction() { db.EndTransaction(); }

	unsigned __int64 CreateCache(const wchar_t *CacheName)
	{
		if (stmtCreateCache.Bind(CacheName).StepAndReset())
			return db.LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetCacheID(const wchar_t *CacheName)
	{
		unsigned __int64 id = 0;
		if (stmtFindCacheName.Bind(CacheName).Step())
			id = stmtFindCacheName.GetColInt64(0);
		stmtFindCacheName.Reset();
		return id;
	}

	bool DeleteCache(const wchar_t *CacheName)
	{
		//All related entries are automatically deleted because of foreign key constraints
		return stmtDelCache.Bind(CacheName).StepAndReset();
	}

	bool IsPreload(unsigned __int64 id)
	{
		bool preload = false;
		if (stmtGetPreloadState.Bind(id).Step())
			preload = stmtGetPreloadState.GetColInt(0) ? true : false;
		stmtGetPreloadState.Reset();
		return preload;
	}

	string GetSignature(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetSignature, id);
	}

	void *GetExport(unsigned __int64 id, const wchar_t *ExportName)
	{
		void *enabled = nullptr;
		if (stmtGetExportState.Bind(id).Bind(ExportName).Step())
			if (stmtGetExportState.GetColInt(0) > 0)
				enabled = (void *)1;
		stmtGetExportState.Reset();
		return enabled;
	}

	string GetGuid(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetGuid, id);
	}

	string GetTitle(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetTitle, id);
	}

	string GetAuthor(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetAuthor, id);
	}

	string GetDescription(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetDescription, id);
	}

	bool GetMinFarVersion(unsigned __int64 id, VersionInfo *Version)
	{
		bool b = stmtGetMinFarVersion.Bind(id).Step();
		if (b)
		{
			const char *blob = stmtGetMinFarVersion.GetColBlob(0);
			int realsize = stmtGetMinFarVersion.GetColBytes(0);
			memcpy(Version,blob,Min(realsize,(int)sizeof(VersionInfo)));
		}
		stmtGetMinFarVersion.Reset();
		return b;
	}

	bool GetVersion(unsigned __int64 id, VersionInfo *Version)
	{
		bool b = stmtGetVersion.Bind(id).Step();
		if (b)
		{
			const char *blob = stmtGetVersion.GetColBlob(0);
			int realsize = stmtGetVersion.GetColBytes(0);
			memcpy(Version,blob,Min(realsize,(int)sizeof(VersionInfo)));
		}
		stmtGetVersion.Reset();
		return b;
	}

	bool GetDiskMenuItem(unsigned __int64 id, int index, string &Text, string &Guid)
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool GetPluginsMenuItem(unsigned __int64 id, int index, string &Text, string &Guid)
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool GetPluginsConfigMenuItem(unsigned __int64 id, int index, string &Text, string &Guid)
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	string GetCommandPrefix(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetPrefix, id);
	}

	unsigned __int64 GetFlags(unsigned __int64 id)
	{
		unsigned __int64 flags = 0;
		if (stmtGetFlags.Bind(id).Step())
			flags = stmtGetFlags.GetColInt64(0);
		stmtGetFlags.Reset();
		return flags;
	}

	bool SetPreload(unsigned __int64 id, bool Preload)
	{
		return stmtSetPreloadState.Bind(id).Bind(Preload?1:0).StepAndReset();
	}

	bool SetSignature(unsigned __int64 id, const wchar_t *Signature)
	{
		return stmtSetSignature.Bind(id).Bind(Signature).StepAndReset();
	}

	bool SetDiskMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool SetPluginsMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool SetPluginsConfigMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	bool SetCommandPrefix(unsigned __int64 id, const wchar_t *Prefix)
	{
		return stmtSetPrefix.Bind(id).Bind(Prefix).StepAndReset();
	}

	bool SetFlags(unsigned __int64 id, unsigned __int64 Flags)
	{
		return stmtSetFlags.Bind(id).Bind(Flags).StepAndReset();
	}

	bool SetExport(unsigned __int64 id, const wchar_t *ExportName, bool Exists)
	{
		return stmtSetExportState.Bind(id).Bind(ExportName).Bind(Exists?1:0).StepAndReset();
	}

	bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version)
	{
		return stmtSetMinFarVersion.Bind(id).Bind((const char *)Version,(int)sizeof(VersionInfo)).StepAndReset();
	}

	bool SetVersion(unsigned __int64 id, const VersionInfo *Version)
	{
		return stmtSetVersion.Bind(id).Bind((const char *)Version,(int)sizeof(VersionInfo)).StepAndReset();
	}

	bool SetGuid(unsigned __int64 id, const wchar_t *Guid)
	{
		return stmtSetGuid.Bind(id).Bind(Guid).StepAndReset();
	}

	bool SetTitle(unsigned __int64 id, const wchar_t *Title)
	{
		return stmtSetTitle.Bind(id).Bind(Title).StepAndReset();
	}

	bool SetAuthor(unsigned __int64 id, const wchar_t *Author)
	{
		return stmtSetAuthor.Bind(id).Bind(Author).StepAndReset();
	}

	bool SetDescription(unsigned __int64 id, const wchar_t *Description)
	{
		return stmtSetDescription.Bind(id).Bind(Description).StepAndReset();
	}

	bool EnumPlugins(DWORD index, string &CacheName)
	{
		if (index == 0)
			stmtEnumCache.Reset();

		if (stmtEnumCache.Step())
		{
			CacheName = stmtEnumCache.GetColText(0);
			return true;
		}

		return false;
	}

	bool DiscardCache()
	{
		db.BeginTransaction();
		bool ret = db.Exec("DELETE FROM cachename");
		db.EndTransaction();
		return ret;
	}

	bool IsCacheEmpty()
	{
		int count = 0;
		if (stmtCountCacheNames.Step())
			count = stmtCountCacheNames.GetColInt(0);
		stmtCountCacheNames.Reset();
		return count==0;
	}
};

class PluginsHotkeysConfigDb: public PluginsHotkeysConfig {
	SQLiteDb   db;
	SQLiteStmt stmtGetHotkey;
	SQLiteStmt stmtSetHotkey;
	SQLiteStmt stmtDelHotkey;
	SQLiteStmt stmtCheckForHotkeys;

public:

	PluginsHotkeysConfigDb()
	{
		if (!db.Open(L"pluginhotkeys.db"))
			return;

		//schema
		db.Exec("CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));");

		//get hotkey statement
		db.InitStmt(stmtGetHotkey, L"SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;");

		//set hotkey statement
		db.InitStmt(stmtSetHotkey, L"INSERT OR REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);");

		//delete hotkey statement
		db.InitStmt(stmtDelHotkey, L"DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;");

		//check if exist hotkeys of specific type statement
		db.InitStmt(stmtCheckForHotkeys, L"SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1");
	}

	virtual ~PluginsHotkeysConfigDb() {}

	bool HotkeysPresent(HotKeyTypeEnum HotKeyType)
	{
		int count = 0;
		if (stmtCheckForHotkeys.Bind((int)HotKeyType).Step())
			count = stmtCheckForHotkeys.GetColInt(0);
		stmtCheckForHotkeys.Reset();
		return count!=0;
	}

	string GetHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType)
	{
		string strHotKey;
		if (stmtGetHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).Step())
			strHotKey = stmtGetHotkey.GetColText(0);
		stmtGetHotkey.Reset();
		return strHotKey;
	}

	bool SetHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType, const wchar_t *HotKey)
	{
		return stmtSetHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).Bind(HotKey).StepAndReset();
	}

	bool DelHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType)
	{
		return stmtDelHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).StepAndReset();
	}
};

class PanelModeConfigDb: public PanelModeConfig {
	SQLiteDb   db;
	SQLiteStmt stmtGetMode;
	SQLiteStmt stmtSetMode;

public:

	PanelModeConfigDb()
	{
		if (!db.Open(L"panelmodes.db"))
			return;

		//schema
		db.Exec("CREATE TABLE IF NOT EXISTS panelmodes(mode INTEGER NOT NULL PRIMARY KEY, columntitles TEXT NOT NULL, columnwidths TEXT NOT NULL, statuscolumntitles TEXT NOT NULL, statuscolumnwidths TEXT NOT NULL, flags INTEGER NOT NULL);");

		//get mode statement
		db.InitStmt(stmtGetMode, L"SELECT columntitles, columnwidths, statuscolumntitles, statuscolumnwidths, flags FROM panelmodes WHERE mode=?1;");

		//set mode statement
		db.InitStmt(stmtSetMode, L"INSERT OR REPLACE INTO panelmodes VALUES (?1,?2,?3,?4,?5,?6);");

		db.BeginTransaction();
	}

	virtual ~PanelModeConfigDb() { db.EndTransaction(); }

	bool GetMode(int mode, string &strColumnTitles, string &strColumnWidths, string &strStatusColumnTitles, string &strStatusColumnWidths, DWORD *Flags)
	{
		bool b = stmtGetMode.Bind(mode).Step();
		if (b)
		{
			strColumnTitles = stmtGetMode.GetColText(0);
			strColumnWidths = stmtGetMode.GetColText(1);
			strStatusColumnTitles = stmtGetMode.GetColText(2);
			strStatusColumnWidths = stmtGetMode.GetColText(3);
			*Flags = (DWORD)stmtGetMode.GetColInt(4);
		}
		stmtGetMode.Reset();
		return b;
	}

	bool SetMode(int mode, const wchar_t *ColumnTitles, const wchar_t *ColumnWidths, const wchar_t *StatusColumnTitles, const wchar_t *StatusColumnWidths, DWORD Flags)
	{
		return stmtSetMode.Bind(mode).Bind(ColumnTitles).Bind(ColumnWidths).Bind(StatusColumnTitles).Bind(StatusColumnWidths).Bind((int)Flags).StepAndReset();
	}
};

PluginsConfig *CreatePluginsConfig()
{
	return new PluginsConfigDb();
}

PanelModeConfig *CreatePanelModeConfig()
{
	return new PanelModeConfigDb();
}

void InitDb()
{
	GeneralCfg = new GeneralConfigDb();
	AssocConfig = new AssociationsConfigDb();
	PlCacheCfg = new PluginsCacheConfigDb();
	PlHotkeyCfg = new PluginsHotkeysConfigDb();
}

void ReleaseDb()
{
	delete GeneralCfg;
	delete AssocConfig;
	delete PlCacheCfg;
	delete PlHotkeyCfg;
}
