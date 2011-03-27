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

public:

	AssociationsConfigDb()
	{
		if (!db.Open(L"associations.db"))
			return;

		//schema
		db.Exec(
			"PRAGMA foreign_keys = ON;"
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY ASC, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, on INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
		);

		db.BeginTransaction();

		//create type statement
		db.InitStmt(stmtAddType, L"INSERT INTO filetypes VALUES (NULL,?2,?3);");

		//get mask statement
		db.InitStmt(stmtGetMask, L"SELECT mask FROM filetypes WHERE id=?1;");

		//get description statement
		db.InitStmt(stmtGetDescription, L"SELECT description FROM filetypes WHERE id=?1;");

		//update type statement
		db.InitStmt(stmtUpdateType, L"UPDATE filetypes SET mask=?1, description=?2 WHERE id=?3;");

		//set association statement
		db.InitStmt(stmtSetCommand, L"INSERT OR REPLACE INTO commands VALUES (?1,?2,?3,?4);");

		//get association statement
		db.InitStmt(stmtGetCommand, L"SELECT command, on FROM commands WHERE ft_id=?1 AND type=?2;");

		//enum types statement
		db.InitStmt(stmtEnumTypes, L"SELECT id, description FROM filetypes ORDER BY id;");

		//enum masks statement
		db.InitStmt(stmtEnumMasks, L"SELECT id, mask FROM filetypes ORDER BY id;");

		//enum masks with a specific type on statement
		db.InitStmt(stmtEnumMasksForType, L"SELECT id, mask FROM filetypes, commands WHERE id=ft_id AND type=?1 AND on<>0 ORDER BY id;");

		//delete type statement
		db.InitStmt(stmtDelType, L"DELETE FROM filetypes WHERE id=?1;");
	}

	virtual ~AssociationsConfigDb() { db.EndTransaction(); }

	bool EnumMasks(DWORD Index, unsigned __int64 *id, string &strMask)
	{
		return false;
	}

	bool EnumMasksForType(int Type, DWORD Index, unsigned __int64 *id, string &strMask)
	{
		return false;
	}

	bool GetMask(unsigned __int64 id, string &strMask)
	{
		return false;
	}

	bool GetDescription(unsigned __int64 id, string &strDescription)
	{
		return false;
	}

	bool GetCommand(unsigned __int64 id, int Type, string &strCommand, bool *Enabled=nullptr)
	{
		return false;
	}

	bool SetCommand(unsigned __int64 id, int Type, string &strCommand, bool Enabled)
	{
		return false;
	}

	bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2)
	{
		return false;
	}

	unsigned __int64 AddType(const wchar_t *Mask, const wchar_t *Description)
	{
		return 0;
	}

	bool UpdateType(unsigned __int64 id, const wchar_t *Mask, const wchar_t *Description)
	{
		return false;
	}

	bool DelType(unsigned __int64 id)
	{
		return false;
	}
};

PluginsConfig *CreatePluginsConfig()
{
	return new PluginsConfigDb();
}

AssociationsConfig *CreateAssociationsConfig()
{
	return new AssociationsConfigDb();
}

void InitDb()
{
	GeneralCfg = new GeneralConfigDb();
}

void ReleaseDb()
{
	delete GeneralCfg;
}
