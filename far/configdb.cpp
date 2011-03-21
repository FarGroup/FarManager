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

class GeneralConfigDb: public GeneralConfig {
	sqlite3 *pDb;
	sqlite3_stmt *pStmtUpdateValue;
	sqlite3_stmt *pStmtInsertValue;
	sqlite3_stmt *pStmtGetValue;
	sqlite3_stmt *pStmtDelValue;
	sqlite3_stmt *pStmtEnumValues;

public:

	GeneralConfigDb() : pDb(nullptr), pStmtUpdateValue(nullptr), pStmtInsertValue(nullptr), pStmtGetValue(nullptr), pStmtDelValue(nullptr), pStmtEnumValues(nullptr)
	{
		string strPath;
		GetDatabasePath(L"generalconfig.db", strPath);
		if (sqlite3_open16(strPath.CPtr(),&pDb) != SQLITE_OK)
		{
			sqlite3_close(pDb);
			pDb = nullptr;
			//if failed, let's open a memory only db so Far can work anyway
			if (sqlite3_open16(L":memory:",&pDb) != SQLITE_OK)
			{
				sqlite3_close(pDb);
				pDb = nullptr;
				return;
			}
		}

		//schema
		sqlite3_exec(pDb,
			"CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, type INTEGER NOT NULL, value BLOB, PRIMARY KEY (key, name));"
			,nullptr,nullptr,nullptr);

		//update value statement
		sqlite3_prepare16_v2(pDb, L"UPDATE general_config SET type=?1, value=?2 WHERE key=?3 AND name=?4;", -1, &pStmtUpdateValue, nullptr);

		//insert value statement
		sqlite3_prepare16_v2(pDb, L"INSERT INTO general_config VALUES (?1,?2,?3,?4);", -1, &pStmtInsertValue, nullptr);

		//get value statement
		sqlite3_prepare16_v2(pDb, L"SELECT value FROM general_config WHERE key=?1 AND name=?2;", -1, &pStmtGetValue, nullptr);

		//delete value statement
		sqlite3_prepare16_v2(pDb, L"DELETE FROM general_config WHERE key=?1 AND name=?2;", -1, &pStmtDelValue, nullptr);

		//enum values statement
		sqlite3_prepare16_v2(pDb, L"SELECT name, value FROM general_config WHERE key=?1;", -1, &pStmtEnumValues, nullptr);
	}

	virtual ~GeneralConfigDb()
	{
		sqlite3_finalize(pStmtUpdateValue);
		sqlite3_finalize(pStmtInsertValue);
		sqlite3_finalize(pStmtGetValue);
		sqlite3_finalize(pStmtDelValue);
		sqlite3_finalize(pStmtEnumValues);

		sqlite3_close(pDb);
	}

	void BeginTransaction()
	{
		sqlite3_exec(pDb,"BEGIN TRANSACTION;",nullptr,nullptr,nullptr);
	}

	void EndTransaction()
	{
		sqlite3_exec(pDb,"END TRANSACTION;",nullptr,nullptr,nullptr);
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value)
	{
		sqlite3_bind_int(pStmtUpdateValue,1,TYPE_TEXT);
		sqlite3_bind_text16(pStmtUpdateValue,2,Value,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtUpdateValue,3,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtUpdateValue,4,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtUpdateValue);
		sqlite3_clear_bindings(pStmtUpdateValue);
		sqlite3_reset(pStmtUpdateValue);
		if (res != SQLITE_DONE || sqlite3_changes(pDb) == 0)
		{
			sqlite3_bind_text16(pStmtInsertValue,1,Key,-1,SQLITE_STATIC);
			sqlite3_bind_text16(pStmtInsertValue,2,Name,-1,SQLITE_STATIC);
			sqlite3_bind_int(pStmtInsertValue,3,TYPE_TEXT);
			sqlite3_bind_text16(pStmtInsertValue,4,Value,-1,SQLITE_STATIC);
			res = sqlite3_step(pStmtInsertValue);
			sqlite3_clear_bindings(pStmtInsertValue);
			sqlite3_reset(pStmtInsertValue);
		}
		return res == SQLITE_DONE;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value)
	{
		sqlite3_bind_int(pStmtUpdateValue,1,TYPE_INTEGER);
		sqlite3_bind_int64(pStmtUpdateValue,2,Value);
		sqlite3_bind_text16(pStmtUpdateValue,3,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtUpdateValue,4,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtUpdateValue);
		sqlite3_clear_bindings(pStmtUpdateValue);
		sqlite3_reset(pStmtUpdateValue);
		if (res != SQLITE_DONE || sqlite3_changes(pDb) == 0)
		{
			sqlite3_bind_text16(pStmtInsertValue,1,Key,-1,SQLITE_STATIC);
			sqlite3_bind_text16(pStmtInsertValue,2,Name,-1,SQLITE_STATIC);
			sqlite3_bind_int(pStmtInsertValue,3,TYPE_INTEGER);
			sqlite3_bind_int64(pStmtInsertValue,4,Value);
			res = sqlite3_step(pStmtInsertValue);
			sqlite3_clear_bindings(pStmtInsertValue);
			sqlite3_reset(pStmtInsertValue);
		}
		return res == SQLITE_DONE;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const void *Value, int Size)
	{
		sqlite3_bind_int(pStmtUpdateValue,1,TYPE_BLOB);
		sqlite3_bind_blob(pStmtUpdateValue,2,Value,Size,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtUpdateValue,3,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtUpdateValue,4,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtUpdateValue);
		sqlite3_clear_bindings(pStmtUpdateValue);
		sqlite3_reset(pStmtUpdateValue);
		if (res != SQLITE_DONE || sqlite3_changes(pDb) == 0)
		{
			sqlite3_bind_text16(pStmtInsertValue,1,Key,-1,SQLITE_STATIC);
			sqlite3_bind_text16(pStmtInsertValue,2,Name,-1,SQLITE_STATIC);
			sqlite3_bind_int(pStmtInsertValue,3,TYPE_BLOB);
			sqlite3_bind_blob(pStmtInsertValue,4,Value,Size,SQLITE_STATIC);
			res = sqlite3_step(pStmtInsertValue);
			sqlite3_clear_bindings(pStmtInsertValue);
			sqlite3_reset(pStmtInsertValue);
		}
		return res == SQLITE_DONE;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 *Value)
	{
		sqlite3_bind_text16(pStmtGetValue,1,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtGetValue);
		if (res == SQLITE_ROW)
		{
			*Value = sqlite3_column_int64(pStmtGetValue,0);
		}
		sqlite3_clear_bindings(pStmtGetValue);
		sqlite3_reset(pStmtGetValue);
		return res == SQLITE_ROW;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
	{
		sqlite3_bind_text16(pStmtGetValue,1,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtGetValue);
		if (res == SQLITE_ROW)
		{
			strValue = (const wchar_t *)sqlite3_column_text16(pStmtGetValue,0);
		}
		sqlite3_clear_bindings(pStmtGetValue);
		sqlite3_reset(pStmtGetValue);
		return res == SQLITE_ROW;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, char *Value, int Size)
	{
		int res = 0;
		sqlite3_bind_text16(pStmtGetValue,1,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		if (sqlite3_step(pStmtGetValue) == SQLITE_ROW)
		{
			const void *blob = sqlite3_column_blob(pStmtGetValue,0);
			res = sqlite3_column_bytes(pStmtGetValue,0);
			if (Value)
				memcpy(Value,blob,Min(res,Size));
		}
		sqlite3_clear_bindings(pStmtGetValue);
		sqlite3_reset(pStmtGetValue);
		return res;
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
		{
			return (int)v;
		}
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
		sqlite3_bind_text16(pStmtDelValue,1,Key,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtDelValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtDelValue);
		sqlite3_clear_bindings(pStmtDelValue);
		sqlite3_reset(pStmtDelValue);
		return res == SQLITE_DONE;
	}

	bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, string &strValue)
	{
		if (Index == 0)
		{
			sqlite3_clear_bindings(pStmtEnumValues);
			sqlite3_reset(pStmtEnumValues);
			sqlite3_bind_text16(pStmtEnumValues,1,Key,-1,SQLITE_TRANSIENT);
		}

		if (sqlite3_step(pStmtEnumValues) == SQLITE_ROW)
		{
			strName = (const wchar_t *)sqlite3_column_text16(pStmtEnumValues,0);
			strValue = (const wchar_t *)sqlite3_column_text16(pStmtEnumValues,1);
			return true;
		}

		return false;
	}

	bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, DWORD *Value)
	{
		if (Index == 0)
		{
			sqlite3_clear_bindings(pStmtEnumValues);
			sqlite3_reset(pStmtEnumValues);
			sqlite3_bind_text16(pStmtEnumValues,1,Key,-1,SQLITE_TRANSIENT);
		}

		if (sqlite3_step(pStmtEnumValues) == SQLITE_ROW)
		{
			strName = (const wchar_t *)sqlite3_column_text16(pStmtEnumValues,0);
			*Value = (DWORD)sqlite3_column_int(pStmtEnumValues,1);
			return true;
		}

		return false;
	}
};

class PluginsConfigDb: public PluginsConfig {
	sqlite3 *pDb;
	sqlite3_stmt *pStmtCreateKey;
	sqlite3_stmt *pStmtFindKey;
	sqlite3_stmt *pStmtSetKeyDescription;
	sqlite3_stmt *pStmtSetValue;
	sqlite3_stmt *pStmtGetValue;
	sqlite3_stmt *pStmtEnumKeys;
	sqlite3_stmt *pStmtEnumValues;
	sqlite3_stmt *pStmtDelValue;

public:

	PluginsConfigDb() : pDb(nullptr),  pStmtCreateKey(nullptr), pStmtFindKey(nullptr), pStmtSetKeyDescription(nullptr), pStmtSetValue(nullptr), pStmtGetValue(nullptr), pStmtEnumKeys(nullptr), pStmtEnumValues(nullptr), pStmtDelValue(nullptr)
	{
		string strPath;
		GetDatabasePath(L"pluginsconfig.db", strPath);
		if (sqlite3_open16(strPath.CPtr(),&pDb) != SQLITE_OK)
		{
			sqlite3_close(pDb);
			pDb = nullptr;
			//if failed, let's open a memory only db so Far can work anyway
			if (sqlite3_open16(L":memory:",&pDb) != SQLITE_OK)
			{
				sqlite3_close(pDb);
				pDb = nullptr;
				return;
			}
		}

		//schema
		sqlite3_exec(pDb,
			"PRAGMA foreign_keys = ON;"
			"CREATE TABLE IF NOT EXISTS plugin_keys(id INTEGER PRIMARY KEY ASC, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES plugin_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
			"CREATE TABLE IF NOT EXISTS plugin_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, type INTEGER NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES plugin_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id > 0));"
			,nullptr,nullptr,nullptr);

		BeginTransaction();

		//root key
		sqlite3_exec(pDb,"INSERT INTO plugin_keys VALUES (0,0,\"\",\"Root - do not edit\");",nullptr,nullptr,nullptr);

		//create key statement
		sqlite3_prepare16_v2(pDb, L"INSERT INTO plugin_keys VALUES (NULL,?1,?2,?3);", -1, &pStmtCreateKey, nullptr);

		//find key statement
		sqlite3_prepare16_v2(pDb, L"SELECT id FROM plugin_keys WHERE parent_id=?1 AND name=?2 AND id>0;", -1, &pStmtFindKey, nullptr);

		//set key description statement
		sqlite3_prepare16_v2(pDb, L"UPDATE plugin_keys SET description=?1 WHERE id=?2 AND id>0 AND description<>?1;", -1, &pStmtSetKeyDescription, nullptr);

		//set value statement
		sqlite3_prepare16_v2(pDb, L"INSERT OR REPLACE INTO plugin_values VALUES (?1,?2,?3,?4);", -1, &pStmtSetValue, nullptr);

		//get value statement
		sqlite3_prepare16_v2(pDb, L"SELECT value FROM plugin_values WHERE key_id=?1 AND name=?2;", -1, &pStmtGetValue, nullptr);

		//enum keys statement
		sqlite3_prepare16_v2(pDb, L"SELECT name FROM plugin_keys WHERE parent_id=?1 AND id>0;", -1, &pStmtEnumKeys, nullptr);

		//enum values statement
		sqlite3_prepare16_v2(pDb, L"SELECT name, type FROM plugin_values WHERE key_id=?1;", -1, &pStmtEnumValues, nullptr);

		//delete value statement
		sqlite3_prepare16_v2(pDb, L"DELETE FROM plugin_values WHERE key_id=?1 AND name=?2;", -1, &pStmtDelValue, nullptr);
	}

	virtual ~PluginsConfigDb()
	{
		sqlite3_finalize(pStmtCreateKey);
		sqlite3_finalize(pStmtFindKey);
		sqlite3_finalize(pStmtSetKeyDescription);
		sqlite3_finalize(pStmtSetValue);
		sqlite3_finalize(pStmtGetValue);
		sqlite3_finalize(pStmtEnumKeys);
		sqlite3_finalize(pStmtEnumValues);
		sqlite3_finalize(pStmtDelValue);

		EndTransaction();

		sqlite3_close(pDb);
	}

	bool BeginTransaction()
	{
		return sqlite3_exec(pDb, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	bool EndTransaction()
	{
		return sqlite3_exec(pDb, "END TRANSACTION;", nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	bool Flush()
	{
		bool res = EndTransaction();
		BeginTransaction();
		return res;
	}

	unsigned __int64 CreateKey(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Description=nullptr)
	{
		sqlite3_bind_int64(pStmtCreateKey,1,Root);
		sqlite3_bind_text16(pStmtCreateKey,2,Name,-1,SQLITE_STATIC);
		if (Description)
			sqlite3_bind_text16(pStmtCreateKey,3,Description,-1,SQLITE_STATIC);
		else
		   sqlite3_bind_null(pStmtCreateKey,3);
		int res = sqlite3_step(pStmtCreateKey);
		sqlite3_clear_bindings(pStmtCreateKey);
		sqlite3_reset(pStmtCreateKey);
		if (res == SQLITE_DONE)
			return sqlite3_last_insert_rowid(pDb);
		unsigned __int64 id = GetKeyID(Root,Name);
		if (id && Description)
			SetKeyDescription(id,Description);
		return id;
	}

	unsigned __int64 GetKeyID(unsigned __int64 Root, const wchar_t *Name)
	{
		sqlite3_bind_int64(pStmtFindKey,1,Root);
		sqlite3_bind_text16(pStmtFindKey,2,Name,-1,SQLITE_STATIC);
		unsigned __int64 id = 0;
		if (sqlite3_step(pStmtFindKey) == SQLITE_ROW)
		{
			id = sqlite3_column_int64(pStmtFindKey,0);
		}
		sqlite3_clear_bindings(pStmtFindKey);
		sqlite3_reset(pStmtFindKey);
		return id;
	}

	bool SetKeyDescription(unsigned __int64 Root, const wchar_t *Description)
	{
		sqlite3_bind_text16(pStmtSetKeyDescription,1,Description,-1,SQLITE_STATIC);
		sqlite3_bind_int64(pStmtSetKeyDescription,2,Root);
		int res = sqlite3_step(pStmtSetKeyDescription);
		sqlite3_clear_bindings(pStmtSetKeyDescription);
		sqlite3_reset(pStmtSetKeyDescription);
		return (res == SQLITE_DONE);
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Value)
	{
		if (!Name)
			return SetKeyDescription(Root,Value);
		sqlite3_bind_int64(pStmtSetValue,1,Root);
		sqlite3_bind_text16(pStmtSetValue,2,Name,-1,SQLITE_STATIC);
		sqlite3_bind_int(pStmtSetValue,3,TYPE_TEXT);
		sqlite3_bind_text16(pStmtSetValue,4,Value,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtSetValue);
		sqlite3_clear_bindings(pStmtSetValue);
		sqlite3_reset(pStmtSetValue);
		return res == SQLITE_DONE;
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 Value)
	{
		sqlite3_bind_int64(pStmtSetValue,1,Root);
		sqlite3_bind_text16(pStmtSetValue,2,Name,-1,SQLITE_STATIC);
		sqlite3_bind_int(pStmtSetValue,3,TYPE_INTEGER);
		sqlite3_bind_int64(pStmtSetValue,4,Value);
		int res = sqlite3_step(pStmtSetValue);
		sqlite3_clear_bindings(pStmtSetValue);
		sqlite3_reset(pStmtSetValue);
		return res == SQLITE_DONE;
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const void *Value, int Size)
	{
		sqlite3_bind_int64(pStmtSetValue,1,Root);
		sqlite3_bind_text16(pStmtSetValue,2,Name,-1,SQLITE_STATIC);
		sqlite3_bind_int(pStmtSetValue,3,TYPE_BLOB);
		sqlite3_bind_blob(pStmtSetValue,4,Value,Size,SQLITE_STATIC);
		int res = sqlite3_step(pStmtSetValue);
		sqlite3_clear_bindings(pStmtSetValue);
		sqlite3_reset(pStmtSetValue);
		return res == SQLITE_DONE;
	}

	bool GetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 *Value)
	{
		sqlite3_bind_int64(pStmtGetValue,1,Root);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtGetValue);
		if (res == SQLITE_ROW)
		{
			*Value = sqlite3_column_int64(pStmtGetValue,0);
		}
		sqlite3_clear_bindings(pStmtGetValue);
		sqlite3_reset(pStmtGetValue);
		return res == SQLITE_ROW;
	}

	bool GetValue(unsigned __int64 Root, const wchar_t *Name, string &strValue)
	{
		sqlite3_bind_int64(pStmtGetValue,1,Root);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtGetValue);
		if (res == SQLITE_ROW)
		{
			strValue = (const wchar_t *)sqlite3_column_text16(pStmtGetValue,0);
		}
		sqlite3_clear_bindings(pStmtGetValue);
		sqlite3_reset(pStmtGetValue);
		return res == SQLITE_ROW;
	}

	int GetValue(unsigned __int64 Root, const wchar_t *Name, char *Value, int Size)
	{
		int res = 0;
		sqlite3_bind_int64(pStmtGetValue,1,Root);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		if (sqlite3_step(pStmtGetValue) == SQLITE_ROW)
		{
			const void *blob = sqlite3_column_blob(pStmtGetValue,0);
			res = sqlite3_column_bytes(pStmtGetValue,0);
			if (Value)
				memcpy(Value,blob,Min(res,Size));
		}
		sqlite3_clear_bindings(pStmtGetValue);
		sqlite3_reset(pStmtGetValue);
		return res;
	}

	bool DeleteKeyTree(unsigned __int64 KeyID)
	{
		//All subtree is automatically deleted because of foreign key constraints
		char *zSQL = sqlite3_mprintf("DELETE FROM plugin_keys WHERE id=%lld AND id>0;", KeyID);
		int res = sqlite3_exec(pDb, zSQL, nullptr,nullptr,nullptr);
		sqlite3_free(zSQL);
		return res == SQLITE_OK;
	}

	bool DeleteValue(unsigned __int64 Root, const wchar_t *Name)
	{
		sqlite3_bind_int64(pStmtDelValue,1,Root);
		sqlite3_bind_text16(pStmtDelValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtDelValue);
		sqlite3_clear_bindings(pStmtDelValue);
		sqlite3_reset(pStmtDelValue);
		return res == SQLITE_DONE;
	}

	bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName)
	{
		if (Index == 0)
		{
			sqlite3_clear_bindings(pStmtEnumKeys);
			sqlite3_reset(pStmtEnumKeys);
			sqlite3_bind_int64(pStmtEnumKeys,1,Root);
		}

		if (sqlite3_step(pStmtEnumKeys) == SQLITE_ROW)
		{
			strName = (const wchar_t *)sqlite3_column_text16(pStmtEnumKeys,0);
			return true;
		}

		return false;
	}

	bool EnumValues(unsigned __int64 Root, DWORD Index, string &strName, DWORD *Type)
	{
		if (Index == 0)
		{
			sqlite3_clear_bindings(pStmtEnumValues);
			sqlite3_reset(pStmtEnumValues);
			sqlite3_bind_int64(pStmtEnumValues,1,Root);
		}

		if (sqlite3_step(pStmtEnumValues) == SQLITE_ROW)
		{
			strName = (const wchar_t *)sqlite3_column_text16(pStmtEnumValues,0);
			*Type = sqlite3_column_int(pStmtEnumValues,1);
			return true;
		}

		return false;
	}
};

PluginsConfig *CreatePluginsConfig()
{
	return new PluginsConfigDb();
}

void InitDb()
{
	GeneralCfg = new GeneralConfigDb();
}

void ReleaseDb()
{
	delete GeneralCfg;
}
