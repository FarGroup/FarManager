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
	sqlite3_stmt *pStmtSetPluginTitle;
	sqlite3_stmt *pStmtDelPluginTree;
	sqlite3_stmt *pStmtSetValue;
	sqlite3_stmt *pStmtGetValue;
	sqlite3_stmt *pStmtEnumValues;
	sqlite3_stmt *pStmtDelValue;

public:

	enum {
		TYPE_INTEGER,
		TYPE_TEXT,
		TYPE_BLOB
	};

	PluginSettingDb() : pDb(nullptr),  pStmtSetPluginTitle(nullptr), pStmtDelPluginTree(nullptr), pStmtSetValue(nullptr), pStmtGetValue(nullptr), pStmtEnumValues(nullptr), pStmtDelValue(nullptr)
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

		//schem
		sqlite3_exec(pDb,
			"CREATE TABLE IF NOT EXISTS plugins_config(guid TEXT NOT NULL, name TEXT NOT NULL, type INTEGER NOT NULL, value BLOB, PRIMARY KEY (guid, name));"
			"CREATE TABLE IF NOT EXISTS plugins_config_titles(guid TEXT NOT NULL PRIMARY KEY, title TEXT);"
			,NULL,NULL,NULL);

		//set plugin title statement
		sqlite3_prepare16_v2(pDb, L"INSERT OR REPLACE INTO plugins_config_titles VALUES (?1, ?2);", -1, &pStmtSetPluginTitle, nullptr);

		//delete all plugin settings statement
		//sqlite3_prepare16_v2(pDb, L"DELETE FROM plugin_config WHERE guid=$1; DELETE FROM plugin_config_titles WHERE guid=$1;", -1, &pStmtDelPluginTree, nullptr);
		sqlite3_prepare16_v2(pDb, L"DELETE FROM plugin_config WHERE guid=$1;", -1, &pStmtDelPluginTree, nullptr);

		//set value statement
		sqlite3_prepare16_v2(pDb, L"INSERT OR REPLACE INTO plugins_config VALUES (?1,?2,?3,?4);", -1, &pStmtSetValue, nullptr);

		//get value statement
		sqlite3_prepare16_v2(pDb, L"SELECT value FROM plugins_config WHERE guid=?1 AND name=?2;", -1, &pStmtGetValue, nullptr);

		//enum values statement
		sqlite3_prepare16_v2(pDb, L"SELECT name, type FROM plugins_config WHERE guid=?1;", -1, &pStmtEnumValues, nullptr);

		//delete value statement
		sqlite3_prepare16_v2(pDb, L"DELETE FROM plugins_config WHERE guid=?1 AND name=?2;", -1, &pStmtDelValue, nullptr);
	}

	~PluginSettingDb()
	{
		sqlite3_finalize(pStmtSetPluginTitle);
		sqlite3_finalize(pStmtDelPluginTree);
		sqlite3_finalize(pStmtSetValue);
		sqlite3_finalize(pStmtGetValue);
		sqlite3_finalize(pStmtEnumValues);
		sqlite3_finalize(pStmtDelValue);

		sqlite3_close(pDb);
	}

	bool SetPluginTitle(const wchar_t *Guid, const wchar_t *Title)
	{
		sqlite3_bind_text16(pStmtSetPluginTitle,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtSetPluginTitle,2,Title,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtSetPluginTitle);
		sqlite3_reset(pStmtSetPluginTitle);
		return res == SQLITE_DONE;
	}

	bool DeletePluginTree(const wchar_t *Guid)
	{
		sqlite3_bind_text16(pStmtDelPluginTree,1,Guid,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtDelPluginTree);
		sqlite3_reset(pStmtDelPluginTree);
		return res == SQLITE_DONE;
	}

	bool SetValue(const wchar_t *Guid, const wchar_t *Name, const wchar_t *Value)
	{
		sqlite3_bind_text16(pStmtSetValue,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtSetValue,2,Name,-1,SQLITE_STATIC);
		sqlite3_bind_int(pStmtSetValue,3,TYPE_TEXT);
		sqlite3_bind_text16(pStmtSetValue,4,Value,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtSetValue);
		sqlite3_reset(pStmtSetValue);
		return res == SQLITE_DONE;
	}

	bool SetValue(const wchar_t *Guid, const wchar_t *Name, unsigned __int64 Value)
	{
		sqlite3_bind_text16(pStmtSetValue,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtSetValue,2,Name,-1,SQLITE_STATIC);
		sqlite3_bind_int(pStmtSetValue,3,TYPE_INTEGER);
		sqlite3_bind_int64(pStmtSetValue,4,Value);
		int res = sqlite3_step(pStmtSetValue);
		sqlite3_reset(pStmtSetValue);
		return res == SQLITE_DONE;
	}

	bool SetValue(const wchar_t *Guid, const wchar_t *Name, const void *Value, int Size)
	{
		sqlite3_bind_text16(pStmtSetValue,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtSetValue,2,Name,-1,SQLITE_STATIC);
		sqlite3_bind_int(pStmtSetValue,3,TYPE_BLOB);
		sqlite3_bind_blob(pStmtSetValue,4,Value,Size,SQLITE_STATIC);
		int res = sqlite3_step(pStmtSetValue);
		sqlite3_reset(pStmtSetValue);
		return res == SQLITE_DONE;
	}

	bool GetValue(const wchar_t *Guid, const wchar_t *Name, unsigned __int64 *Value)
	{
		sqlite3_bind_text16(pStmtGetValue,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtGetValue);
		if (res == SQLITE_ROW)
		{
			*Value = sqlite3_column_int64(pStmtGetValue,0);
		}
		sqlite3_reset(pStmtGetValue);
		return res == SQLITE_ROW;
	}

	bool GetValue(const wchar_t *Guid, const wchar_t *Name, string &strValue)
	{
		sqlite3_bind_text16(pStmtGetValue,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtGetValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtGetValue);
		if (res == SQLITE_ROW)
		{
			strValue = (const wchar_t *)sqlite3_column_text16(pStmtGetValue,0);
		}
		sqlite3_reset(pStmtGetValue);
		return res == SQLITE_ROW;
	}

	int GetValue(const wchar_t *Guid, const wchar_t *Name, char *Value, int Size)
	{
		int res = 0;
		sqlite3_bind_text16(pStmtGetValue,1,Guid,-1,SQLITE_STATIC);
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

	bool DeleteValue(const wchar_t *Guid, const wchar_t *Name)
	{
		sqlite3_bind_text16(pStmtDelValue,1,Guid,-1,SQLITE_STATIC);
		sqlite3_bind_text16(pStmtDelValue,2,Name,-1,SQLITE_STATIC);
		int res = sqlite3_step(pStmtDelValue);
		sqlite3_reset(pStmtDelValue);
		return res == SQLITE_DONE;
	}

	bool EnumValues(const wchar_t *Guid, DWORD Index, string &strName, DWORD *Type)
	{
		if (Index == 0)
		{
			sqlite3_reset(pStmtEnumValues);
			sqlite3_bind_text16(pStmtEnumValues,1,Guid,-1,SQLITE_STATIC);
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
		strGuid = GuidToStr(Guid);
		db.SetPluginTitle(strGuid, pPlugin->GetTitle());
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
	if(Item.Name)
	{
		switch(Item.Type)
		{
			case FST_QWORD:
				if (db.SetValue(strGuid,Item.Name,Item.Number)) result=TRUE;
				break;
			case FST_STRING:
				if (db.SetValue(strGuid,Item.Name,Item.String)) result=TRUE;
				break;
			case FST_DATA:
				if (db.SetValue(strGuid,Item.Name,Item.Data.Data,(int)Item.Data.Size)) result=TRUE;
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
	if(Item.Name)
	{
		switch(Item.Type)
		{
			case FST_QWORD:
				{
					unsigned __int64 value;
					if (db.GetValue(strGuid,Item.Name,&value))
					{
						result=TRUE;
						Item.Number=value;
					}
				}
				break;
			case FST_STRING:
				{
					string data;
					if (db.GetValue(strGuid,Item.Name,data))
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
					int size=db.GetValue(strGuid,Item.Name,nullptr,0);
					if (size)
					{
						char** item=m_Data.addItem();
						*item=new char[size];
						int checkedSize=db.GetValue(strGuid,Item.Name,*item,size);
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
	Vector<FarSettingsName>& array=*m_Enum.addItem();
	FarSettingsName item;
	DWORD Index=0,Type;
	string strName;

	while (db.EnumValues(strGuid,Index++,strName,&Type))
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
	return TRUE;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	if(Value.Value)
	{
		if (db.DeleteValue(strGuid,Value.Value))
			return TRUE;
	}
	return FALSE;
}

int PluginSettings::DeleteAll()
{
	if (db.DeletePluginTree(strGuid))
		return TRUE;
	return FALSE;
}
