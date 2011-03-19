/*
plugsettings.hpp

API для хранения плагинами настроек.
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

#include "plugsettings.hpp"
#include "ctrlobj.hpp"
#include "strmix.hpp"
#include "pathmix.hpp"
#include "sqlite/sqlite3.h"

class PluginSettingDb {
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

	enum {
		TYPE_INTEGER,
		TYPE_TEXT,
		TYPE_BLOB
	};

	PluginSettingDb() : pDb(nullptr),  pStmtCreateKey(nullptr), pStmtFindKey(nullptr), pStmtSetKeyDescription(nullptr), pStmtSetValue(nullptr), pStmtGetValue(nullptr), pStmtEnumKeys(nullptr), pStmtEnumValues(nullptr), pStmtDelValue(nullptr)
	{
		string strPath;
		SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,0,strPath.GetBuffer(MAX_PATH));
		strPath.ReleaseBuffer();
		AddEndSlash(strPath);
		strPath += L"Far Manager";
		apiCreateDirectory(strPath, nullptr);
		strPath += L"\\pluginsconfig.db";
		if (sqlite3_open16(strPath.CPtr(),&pDb) != SQLITE_OK)
		{
			sqlite3_close(pDb);
			pDb = nullptr;
			//if failed, let's open a memory only db so Far can work anyway
			if (sqlite3_open16(L":memory:",&pDb) != SQLITE_OK)
				return;
		}

		//schema
		sqlite3_exec(pDb,
			"PRAGMA foreign_keys = ON;"
			"CREATE TABLE IF NOT EXISTS plugin_keys(id INTEGER PRIMARY KEY ASC, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES plugin_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
			"CREATE TABLE IF NOT EXISTS plugin_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, type INTEGER NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES plugin_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id > 0));"
			,NULL,NULL,NULL);

		//root key
		sqlite3_exec(pDb,"INSERT INTO plugin_keys VALUES (0,0,\"\",\"Root - do not edit\");",nullptr,nullptr,nullptr);

		//create key statement
		sqlite3_prepare16_v2(pDb, L"INSERT INTO plugin_keys VALUES (NULL,?1,?2,?3);", -1, &pStmtCreateKey, nullptr);

		//find key statement
		sqlite3_prepare16_v2(pDb, L"SELECT id FROM plugin_keys WHERE parent_id=?1 AND name=?2 AND id>0;", -1, &pStmtFindKey, nullptr);

		//set key description statement
		sqlite3_prepare16_v2(pDb, L"UPDATE plugin_keys SET description=?1 WHERE id=?2 AND id>0;", -1, &pStmtSetKeyDescription, nullptr);

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

	~PluginSettingDb()
	{
		sqlite3_finalize(pStmtCreateKey);
		sqlite3_finalize(pStmtFindKey);
		sqlite3_finalize(pStmtSetKeyDescription);
		sqlite3_finalize(pStmtSetValue);
		sqlite3_finalize(pStmtGetValue);
		sqlite3_finalize(pStmtEnumKeys);
		sqlite3_finalize(pStmtEnumValues);
		sqlite3_finalize(pStmtDelValue);

		sqlite3_close(pDb);
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
		sqlite3_reset(pStmtFindKey);
		return id;
	}

	bool SetKeyDescription(unsigned __int64 Root, const wchar_t *Description)
	{
		sqlite3_bind_text16(pStmtSetKeyDescription,1,Description,-1,SQLITE_STATIC);
		sqlite3_bind_int64(pStmtSetKeyDescription,2,Root);
		int res = sqlite3_step(pStmtSetKeyDescription);
		sqlite3_reset(pStmtSetKeyDescription);
		return res == SQLITE_DONE;
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
		sqlite3_reset(pStmtDelValue);
		return res == SQLITE_DONE;
	}

	bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName)
	{
		if (Index == 0)
		{
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

PluginSettingDb db;

PluginSettings::PluginSettings(const GUID& Guid)
{
	Plugin* pPlugin=CtrlObject?CtrlObject->Plugins.FindPlugin(Guid):nullptr;
	if (pPlugin)
	{
		unsigned __int64& root(*m_Keys.insertItem(0));
		root=db.CreateKey(0,GuidToStr(Guid), pPlugin->GetTitle());
	}
}

PluginSettings::~PluginSettings()
{
	for(size_t ii=0;ii<m_Data.getCount();++ii)
	{
		delete [] *m_Data.getItem(ii);
	}
	for(size_t ii=0;ii<m_Enum.getCount();++ii)
	{
		FarSettingsName* array=m_Enum.getItem(ii)->GetItems();
		for(size_t jj=0;jj<m_Enum.getItem(ii)->GetSize();++jj)
		{
			delete [] array[jj].Name;
		}
	}
}

int PluginSettings::Set(const FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				{
					FarSettingsValue value={Item.Root,Item.Name};
					int key=SubKey(value);
					if (key)
					{
						result=TRUE;
					}
				}
				break;
			case FST_QWORD:
				if (db.SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.Number)) result=TRUE;
				break;
			case FST_STRING:
				if (db.SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.String)) result=TRUE;
				break;
			case FST_DATA:
				if (db.SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.Data.Data,Item.Data.Size)) result=TRUE;
				break;
			default:
				break;
		}
	}
	return result;
}

int PluginSettings::Get(FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				{
					unsigned __int64 value;
					if (db.GetValue(*m_Keys.getItem(Item.Root),Item.Name,&value))
					{
						result=TRUE;
						Item.Number=value;
					}
				}
				break;
			case FST_STRING:
				{
					string data;
					if (db.GetValue(*m_Keys.getItem(Item.Root),Item.Name,data))
					{
						result=TRUE;
						char** item=m_Data.addItem();
						size_t size=(data.GetLength()+1)*sizeof(wchar_t);
						*item=new char[size];
						memcpy(*item,data.CPtr(),size);
						Item.String=(wchar_t*)*item;
					}
				}
				break;
			case FST_DATA:
				{
					int size=db.GetValue(*m_Keys.getItem(Item.Root),Item.Name,nullptr,0);
					if (size)
					{
						char** item=m_Data.addItem();
						*item=new char[size];
						int checkedSize=db.GetValue(*m_Keys.getItem(Item.Root),Item.Name,*item,size);
						if (size==checkedSize)
						{
							result=TRUE;
							Item.Data.Data=*item;
							Item.Data.Size=size;
						}
					}
				}
				break;
			default:
				break;
		}
	}
	return result;
}

static void AddString(Vector<FarSettingsName>& Array, FarSettingsName& Item, string& String)
{
	size_t size=String.GetLength()+1;
	Item.Name=new wchar_t[size];
	wmemcpy((wchar_t*)Item.Name,String.CPtr(),size);
	Array.AddItem(Item);
}

int PluginSettings::Enum(FarSettingsEnum& Enum)
{
	int result=FALSE;
	if(Enum.Root<m_Keys.getCount())
	{
		Vector<FarSettingsName>& array=*m_Enum.addItem();
		FarSettingsName item;
		DWORD Index=0,Type;
		string strName,strValue;

		unsigned __int64 root = *m_Keys.getItem(Enum.Root);
		item.Type=FST_SUBKEY;
		while (db.EnumKeys(root,Index++,strName))
		{
			AddString(array,item,strName);
		}
		Index=0;
		while (db.EnumValues(root,Index++,strName,&Type))
		{
			item.Type=FST_UNKNOWN;
			switch (Type)
			{
				case PluginSettingDb::TYPE_INTEGER:
					item.Type=FST_QWORD;
					break;
				case PluginSettingDb::TYPE_TEXT:
					item.Type=FST_STRING;
					break;
				case PluginSettingDb::TYPE_BLOB:
					item.Type=FST_DATA;
					break;
			}
			if(item.Type!=FST_UNKNOWN)
			{
				AddString(array,item,strName);
			}
		}
		Enum.Count=array.GetSize();
		Enum.Items=array.GetItems();
		result=TRUE;
	}
	return result;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	int result=FALSE;
	if(Value.Root<m_Keys.getCount())
	{
		unsigned __int64 root = db.GetKeyID(*m_Keys.getItem(Value.Root),Value.Value);
		if (root)
		{
			if (db.DeleteKeyTree(root))
				result=TRUE;
		}
		else
		{
			if (db.DeleteValue(*m_Keys.getItem(Value.Root),Value.Value))
				result=TRUE;
		}
	}
	return result;
}

int PluginSettings::SubKey(const FarSettingsValue& Value)
{
	int result=0;
	if(Value.Root<m_Keys.getCount()&&!wcschr(Value.Value,'\\'))
	{
		unsigned __int64 root = db.CreateKey(*m_Keys.getItem(Value.Root),Value.Value);
		if (root)
		{
			result=static_cast<int>(m_Keys.getCount());
			*m_Keys.insertItem(result) = root;
		}
	}
	return result;
}
