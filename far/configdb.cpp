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
#include "sqlitedb.hpp"
#include "strmix.hpp"
#include "pathmix.hpp"
#include "config.hpp"
#include "datetime.hpp"
#include "tinyxml.hpp"
#include "farversion.hpp"
#include "RegExp.hpp"
#include "keyboard.hpp"
#include "macro.hpp"
#include "udlist.hpp"
#include "console.hpp"
#include "syslog.hpp"
#include "array.hpp"
#include "sqlite.h"
#include "language.hpp"
#include "message.hpp"

GeneralConfig *GeneralCfg;
ColorsConfig *ColorsCfg;
AssociationsConfig *AssocConfig;
PluginsCacheConfig *PlCacheCfg;
PluginsHotkeysConfig *PlHotkeyCfg;
HistoryConfig *HistoryCfg;
HistoryConfig *HistoryCfgMem;
MacroConfig *MacroCfg;

int IntToHex(int h)
{
	if (h >= 10)
		return 'A' + h - 10;
	return '0' + h;
}

int HexToInt(int h)
{
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;

	if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;

	if (h >= '0' && h <= '9')
		return h - '0';

	return 0;
}

char *BlobToHexString(const char *Blob, int Size)
{
	char *Hex = (char *)xf_malloc(Size*2+Size+1);
	for (int i=0, j=0; i<Size; i++, j+=3)
	{
		Hex[j] = IntToHex((Blob[i]&0xF0) >> 4);
		Hex[j+1] = IntToHex(Blob[i]&0x0F);
		Hex[j+2] = ',';
	}
	Hex[Size ? Size*2+Size-1 : 0] = 0;
	return Hex;

}

string BlobToHexString(const void *Blob, int Size)
{
	char* Hex = BlobToHexString(static_cast<const char*>(Blob),Size);
	string Str = Hex;
	xf_free(Hex);
	return Str;
}

char *HexStringToBlob(const char *Hex, int *Size)
{
	*Size=0;
	char *Blob = (char *)xf_malloc(strlen(Hex)/2+1);
	if (!Blob)
		return nullptr;

	while (*Hex && *(Hex+1))
	{
		Blob[(*Size)++] = (HexToInt(*Hex)<<4) | HexToInt(*(Hex+1));
		Hex+=2;
		if (!*Hex)
			break;
		Hex++;
	}

	return Blob;
}

const char *Int64ToHexString(unsigned __int64 X)
{
	static char Bin[16+1];
	for (int i=15; i>=0; i--, X>>=4)
		Bin[i] = IntToHex(X&0xFull);
	return Bin;
}

const char *IntToHexString(unsigned int X)
{
	static char Bin[8+1];
	for (int i=7; i>=0; i--, X>>=4)
		Bin[i] = IntToHex(X&0xFull);
	return Bin;
}

unsigned __int64 HexStringToInt64(const char *Hex)
{
	unsigned __int64 x = 0;
	while (*Hex)
	{
		x <<= 4;
		x += HexToInt(*Hex);
		Hex++;
	}
	return x;
}

unsigned int HexStringToInt(const char *Hex)
{
	unsigned int x = 0;
	while (*Hex)
	{
		x <<= 4;
		x += HexToInt(*Hex);
		Hex++;
	}
	return x;
}

const char *KeyToNameChar(unsigned __int64 Key)
{
	static char KeyName[256];
	string strKeyName;
	if(KeyToText(Key, strKeyName))
	{
		strKeyName.GetCharString(KeyName, 256);
		return KeyName;
	}
	return nullptr;
}

void SetCDataIfNeeded(TiXmlText *text, const char *value)
{
	if (strpbrk(value, "\"<>&\r\n"))
		text->SetCDATA(true);
}

static inline void PrintError(const wchar_t *Title, const wchar_t *Error, int Row, int Col)
{
	FormatString strResult;
	strResult<<Title<<" ("<<Row<<L","<<Col<<L"): "<<Error<<L"\n";
	Console.Write(strResult);
	Console.Commit();
}

static void PrintError(const wchar_t *Title, const wchar_t *Error, const TiXmlElement *e)
{
	PrintError(Title, Error, e->Row(), e->Column());
}

static void PrintError(const wchar_t *Title, const TiXmlDocument &doc)
{
	PrintError(Title, string(doc.ErrorDesc(), CP_UTF8), doc.ErrorRow(), doc.ErrorCol());
}

class Utf8String
{
public:
	Utf8String(const wchar_t* Str)
	{
		Init(Str, StrLength(Str));
	}

	Utf8String(const string& Str)
	{
		Init(Str, Str.GetLength());
	}

	~Utf8String()
	{
		delete[] Data;
	}

	operator const char*() const {return Data;}
	size_t size() const {return Size;}


private:
	void Init(const wchar_t* Str, size_t Length)
	{
		Size = WideCharToMultiByte(CP_UTF8, 0, Str, static_cast<int>(Length), nullptr, 0, nullptr, nullptr) + 1;
		Data = new char[Size];
		WideCharToMultiByte(CP_UTF8, 0, Str, static_cast<int>(Length), Data, static_cast<int>(Size-1), nullptr, nullptr);
		Data[Size-1] = 0;
	}

	char* Data;
	size_t Size;
};


class GeneralConfigDb: public GeneralConfig, public SQLiteDb {
	SQLiteStmt stmtUpdateValue;
	SQLiteStmt stmtInsertValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtDelValue;
	SQLiteStmt stmtEnumValues;

public:

	GeneralConfigDb()
	{
		Initialize(L"generalconfig.db");
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local) ||

			//schema
			!Exec("CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));")
		) return false;

		if (
			//update value statement
			InitStmt(stmtUpdateValue, L"UPDATE general_config SET value=?1 WHERE key=?2 AND name=?3;") &&

			//insert value statement
			InitStmt(stmtInsertValue, L"INSERT INTO general_config VALUES (?1,?2,?3);") &&

			//get value statement
			InitStmt(stmtGetValue, L"SELECT value FROM general_config WHERE key=?1 AND name=?2;") &&

			//delete value statement
			InitStmt(stmtDelValue, L"DELETE FROM general_config WHERE key=?1 AND name=?2;") &&

			//enum values statement
			InitStmt(stmtEnumValues, L"SELECT name, value FROM general_config WHERE key=?1;")
		) return true;

		stmtEnumValues.Finalize();
		stmtDelValue.Finalize();
		stmtGetValue.Finalize();
		stmtInsertValue.Finalize();
		stmtUpdateValue.Finalize();
		return false;
	}

	virtual ~GeneralConfigDb() { }

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const void *Value, size_t Size)
	{
		bool b = stmtUpdateValue.Bind(Value,Size).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
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

	int GetValue(const wchar_t *Key, const wchar_t *Name, void *Value, size_t Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Key).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,Min(realsize,static_cast<int>(Size)));
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
		{
			*Value = (int)v;
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

	int GetValue(const wchar_t *Key, const wchar_t *Name, void *Value, size_t Size, const void *Default)
	{
		int s = GetValue(Key,Name,Value,Size);
		if (s)
			return s;
		if (Default)
		{
			memcpy(Value,Default,Size);
			return static_cast<int>(Size);
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

		stmtEnumValues.Reset();
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

		stmtEnumValues.Reset();
		return false;
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("generalconfig");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllValues;
		InitStmt(stmtEnumAllValues, L"SELECT key, name, value FROM general_config ORDER BY key, name;");

		while (stmtEnumAllValues.Step())
		{
			TiXmlElement *e = new TiXmlElement("setting");
			if (!e)
				break;

			e->SetAttribute("key", stmtEnumAllValues.GetColTextUTF8(0));
			e->SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(1));

			switch (stmtEnumAllValues.GetColType(2))
			{
				case SQLITE_INTEGER:
					e->SetAttribute("type", "qword");
					e->SetAttribute("value", Int64ToHexString(stmtEnumAllValues.GetColInt64(2)));
					break;
				case SQLITE_TEXT:
					e->SetAttribute("type", "text");
					e->SetAttribute("value", stmtEnumAllValues.GetColTextUTF8(2));
					break;
				default:
				{
					char *hex = BlobToHexString(stmtEnumAllValues.GetColBlob(2),stmtEnumAllValues.GetColBytes(2));
					e->SetAttribute("type", "hex");
					e->SetAttribute("value", hex);
					xf_free(hex);
				}
			}

			root->LinkEndChild(e);
		}

		stmtEnumAllValues.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("generalconfig").FirstChildElement("setting").Element(); e; e=e->NextSiblingElement("setting"))
		{
			const char *key = e->Attribute("key");
			const char *name = e->Attribute("name");
			const char *type = e->Attribute("type");
			const char *value = e->Attribute("value");

			if (!key || !name || !type || !value)
				continue;

			string Key(key, CP_UTF8);
			string Name(name, CP_UTF8);

			if (!strcmp(type,"qword"))
			{
				SetValue(Key, Name, HexStringToInt64(value));
			}
			else if (!strcmp(type,"text"))
			{
				string Value(value, CP_UTF8);
				SetValue(Key, Name, Value);
			}
			else if (!strcmp(type,"hex"))
			{
				int Size = 0;
				char *Blob = HexStringToBlob(value, &Size);
				if (Blob)
				{
					SetValue(Key, Name, Blob, Size);
					xf_free(Blob);
				}
			}
			else
			{
				continue;
			}
		}
		EndTransaction();

		return true;
	}
};

class HierarchicalConfigDb: public HierarchicalConfig, public SQLiteDb {
	SQLiteStmt stmtCreateKey;
	SQLiteStmt stmtFindKey;
	SQLiteStmt stmtSetKeyDescription;
	SQLiteStmt stmtSetValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtEnumKeys;
	SQLiteStmt stmtEnumValues;
	SQLiteStmt stmtDelValue;
	SQLiteStmt stmtDeleteTree;

	HierarchicalConfigDb() {}

public:

	explicit HierarchicalConfigDb(const wchar_t *DbName, bool Local = false)
	{
		Initialize(DbName, Local);
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local) ||

			//schema
			!EnableForeignKeysConstraints() ||

			!Exec(
				"CREATE TABLE IF NOT EXISTS table_keys(id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
				"CREATE TABLE IF NOT EXISTS table_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id <> 0));"
			) ||

			//root key (needs to be before the transaction start)
			!Exec("INSERT OR IGNORE INTO table_keys VALUES (0,0,\"\",\"Root - do not edit\");")
		) return false;

		if (
			//create key statement
			InitStmt(stmtCreateKey, L"INSERT INTO table_keys VALUES (NULL,?1,?2,?3);") &&

			//find key statement
			InitStmt(stmtFindKey, L"SELECT id FROM table_keys WHERE parent_id=?1 AND name=?2 AND id<>0;") &&

			//set key description statement
			InitStmt(stmtSetKeyDescription, L"UPDATE table_keys SET description=?1 WHERE id=?2 AND id<>0 AND description<>?1;") &&

			//set value statement
			InitStmt(stmtSetValue, L"INSERT OR REPLACE INTO table_values VALUES (?1,?2,?3);") &&

			//get value statement
			InitStmt(stmtGetValue, L"SELECT value FROM table_values WHERE key_id=?1 AND name=?2;") &&

			//enum keys statement
			InitStmt(stmtEnumKeys, L"SELECT name FROM table_keys WHERE parent_id=?1 AND id<>0;") &&

			//enum values statement
			InitStmt(stmtEnumValues, L"SELECT name, value FROM table_values WHERE key_id=?1;") &&

			//delete value statement
			InitStmt(stmtDelValue, L"DELETE FROM table_values WHERE key_id=?1 AND name=?2;") &&

			//delete tree statement
			InitStmt(stmtDeleteTree, L"DELETE FROM table_keys WHERE id=?1 AND id<>0;") &&

			BeginTransaction()
		) return true;

		stmtDeleteTree.Finalize();
		stmtDelValue.Finalize();
		stmtEnumValues.Finalize();
		stmtEnumKeys.Finalize();
		stmtGetValue.Finalize();
		stmtSetValue.Finalize();
		stmtSetKeyDescription.Finalize();
		stmtFindKey.Finalize();
		stmtCreateKey.Finalize();
		return false;
	}

	virtual ~HierarchicalConfigDb() { EndTransaction(); }

	bool Flush()
	{
		bool b = EndTransaction();
		BeginTransaction();
		return b;
	}

	unsigned __int64 CreateKey(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Description=nullptr)
	{
		if (stmtCreateKey.Bind(Root).Bind(Name).Bind(Description).StepAndReset())
			return LastInsertRowID();
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

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const void *Value, size_t Size)
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

	int GetValue(unsigned __int64 Root, const wchar_t *Name, void *Value, size_t Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Root).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,Min(realsize,static_cast<int>(Size)));
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool DeleteKeyTree(unsigned __int64 KeyID)
	{
		//All subtree is automatically deleted because of foreign key constraints
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

		stmtEnumKeys.Reset();
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
				case SQLITE_TEXT: *Type = TYPE_STRING; break;
				case SQLITE_BLOB: *Type = TYPE_BLOB; break;
				default: *Type = TYPE_UNKNOWN;
			}

			return true;
		}

		stmtEnumValues.Reset();
		return false;

	}

	virtual void SerializeBlob(const char* Name, const char* Blob, int Size, TiXmlElement *e)
	{
			char* hex = BlobToHexString(Blob, Size);
			e->SetAttribute("type", "hex");
			e->SetAttribute("value", hex);
			xf_free(hex);
	}

	void Export(unsigned __int64 id, TiXmlElement *key)
	{
		stmtEnumValues.Bind(id);
		while (stmtEnumValues.Step())
		{
			TiXmlElement *e = new TiXmlElement("value");
			if (!e)
				break;

			const char* name = stmtEnumValues.GetColTextUTF8(0);
			e->SetAttribute("name", name);

			switch (stmtEnumValues.GetColType(1))
			{
				case SQLITE_INTEGER:
					e->SetAttribute("type", "qword");
					e->SetAttribute("value", Int64ToHexString(stmtEnumValues.GetColInt64(1)));
					break;
				case SQLITE_TEXT:
					e->SetAttribute("type", "text");
					e->SetAttribute("value", stmtEnumValues.GetColTextUTF8(1));
					break;
				default:
					SerializeBlob(name, stmtEnumValues.GetColBlob(1), stmtEnumValues.GetColBytes(1), e);
			}

			key->LinkEndChild(e);
		}
		stmtEnumValues.Reset();

		SQLiteStmt stmtEnumSubKeys;
		InitStmt(stmtEnumSubKeys, L"SELECT id, name, description FROM table_keys WHERE parent_id=?1 AND id<>0;");

		stmtEnumSubKeys.Bind(id);
		while (stmtEnumSubKeys.Step())
		{
			TiXmlElement *e = new TiXmlElement("key");
			if (!e)
				break;

			e->SetAttribute("name", stmtEnumSubKeys.GetColTextUTF8(1));
			const char *description = stmtEnumSubKeys.GetColTextUTF8(2);
			if (description)
				e->SetAttribute("description", description);

			Export(stmtEnumSubKeys.GetColInt64(0), e);

			key->LinkEndChild(e);
		}
		stmtEnumSubKeys.Reset();
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("hierarchicalconfig");
		if (!root)
			return nullptr;

		Export(0, root);

		return root;
	}

	virtual int DeserializeBlob(const char* Name, const char* Type, const char* Value, const TiXmlElement *e, char*& Blob)
	{
		int Size = 0;
		Blob = HexStringToBlob(Value, &Size);
		return Size;
	}

	void Import(unsigned __int64 root, const TiXmlElement *key)
	{
		unsigned __int64 id;
		{
			const char *name = key->Attribute("name");
			const char *description = key->Attribute("description");
			if (!name)
				return;

			string Name(name, CP_UTF8);
			string Description(description, CP_UTF8);
			id = CreateKey(root, Name, description ? Description.CPtr() : nullptr);
			if (!id)
				return;
		}

		for (const TiXmlElement *e = key->FirstChildElement("value"); e; e=e->NextSiblingElement("value"))
		{
			const char *name = e->Attribute("name");
			const char *type = e->Attribute("type");
			const char *value = e->Attribute("value");

			if (!name || !type)
				continue;

			string Name(name, CP_UTF8);

			if (value && !strcmp(type,"qword"))
			{
				SetValue(id, Name, HexStringToInt64(value));
			}
			else if (value && !strcmp(type,"text"))
			{
				string Value(value, CP_UTF8);
				SetValue(id, Name, Value);
			}
			else if (value && !strcmp(type,"hex"))
			{
				int Size = 0;
				char *Blob = HexStringToBlob(value, &Size);
				if (Blob)
				{
					SetValue(id, Name, Blob, Size);
					xf_free(Blob);
				}
			}
			else
			{
				// custom types, value is optional
				char* Blob = nullptr;
				int Size = DeserializeBlob(name, type, value, e, Blob);
				if (Blob)
				{
					SetValue(id, Name, Blob, Size);
					xf_free(Blob);
				}
			}
		}

		for (const TiXmlElement *e = key->FirstChildElement("key"); e; e=e->NextSiblingElement("key"))
		{
			Import(id, e);
		}

	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("hierarchicalconfig").FirstChildElement("key").Element(); e; e=e->NextSiblingElement("key"))
		{
			Import(0, e);
		}
		EndTransaction();
		return true;
	}
};

struct _ColorFlagNames
{
	FARCOLORFLAGS Value;
	const wchar_t* Name;
}
ColorFlagNames[] =
{
	{FCF_FG_4BIT,      L"bg4bit"   },
	{FCF_BG_4BIT,      L"fg4bit"   },
	{FCF_FG_BOLD,      L"bold"     },
	{FCF_FG_ITALIC,    L"italic"   },
	{FCF_FG_UNDERLINE, L"underline"},
};

class HighlightHierarchicalConfigDb: public HierarchicalConfigDb
{
public:
	explicit HighlightHierarchicalConfigDb(const wchar_t *DbName, bool Local = false):HierarchicalConfigDb(DbName, Local) {}

private:
	HighlightHierarchicalConfigDb();

	virtual void SerializeBlob(const char* Name, const char* Blob, int Size, TiXmlElement *e)
	{
		if(!strcmp(Name, "NormalColor") || !strcmp(Name, "SelectedColor") ||
			!strcmp(Name, "CursorColor") || !strcmp(Name, "SelectedCursorColor") ||
			!strcmp(Name, "MarkCharNormalColor") || !strcmp(Name, "MarkCharSelectedColor") ||
			!strcmp(Name, "MarkCharCursorColor") || !strcmp(Name, "MarkCharSelectedCursorColor"))
		{
			const FarColor* Color = reinterpret_cast<const FarColor*>(Blob);
			e->SetAttribute("type", "color");
			e->SetAttribute("background", IntToHexString(Color->BackgroundColor));
			e->SetAttribute("foreground", IntToHexString(Color->ForegroundColor));
			e->SetAttribute("flags", Utf8String(FlagsToString(Color->Flags, ColorFlagNames)));
		}
		else
		{
			return HierarchicalConfigDb::SerializeBlob(Name, Blob, Size, e);
		}
	}

	virtual int DeserializeBlob(const char* Name, const char* Type, const char* Value, const TiXmlElement *e, char*& Blob)
	{
		int Result = 0;
		if(!strcmp(Type, "color"))
		{
			const char *background = e->Attribute("background");
			const char *foreground = e->Attribute("foreground");
			const char *flags = e->Attribute("flags");

			if(background && foreground && flags)
			{
				Result = sizeof(FarColor);
				FarColor* Color = static_cast<FarColor*>(xf_malloc(Result));
				Color->BackgroundColor = HexStringToInt(background);
				Color->ForegroundColor = HexStringToInt(foreground);
				Color->Flags = StringToFlags(string(flags, CP_UTF8), ColorFlagNames);
				Blob = reinterpret_cast<char*>(Color);
			}
		}
		else
		{
			Result = HierarchicalConfigDb::DeserializeBlob(Name, Type, Value, e, Blob);
		}
		return Result;
	}
};

class ColorsConfigDb: public ColorsConfig, public SQLiteDb
{
	SQLiteStmt stmtUpdateValue;
	SQLiteStmt stmtInsertValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtDelValue;

public:

	ColorsConfigDb()
	{
		Initialize(L"colors.db");
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local) ||

			//schema
			!Exec("CREATE TABLE IF NOT EXISTS colors(name TEXT NOT NULL PRIMARY KEY, value BLOB);")
		) return false;

		if (
			//update value statement
			InitStmt(stmtUpdateValue, L"UPDATE colors SET value=?1 WHERE name=?2;") &&

			//insert value statement
			InitStmt(stmtInsertValue, L"INSERT INTO colors VALUES (?1,?2);") &&

			//get value statement
			InitStmt(stmtGetValue, L"SELECT value FROM colors WHERE name=?1;") &&

			//delete value statement
			InitStmt(stmtDelValue, L"DELETE FROM colors WHERE name=?1;")
		) return true;

		stmtDelValue.Finalize();
		stmtGetValue.Finalize();
		stmtInsertValue.Finalize();
		stmtUpdateValue.Finalize();
		return false;
	}

	virtual ~ColorsConfigDb() { }

	bool SetValue(const wchar_t *Name, const FarColor& Value)
	{
		bool b = stmtUpdateValue.Bind(&Value, sizeof(Value)).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Name).Bind(&Value, sizeof(Value)).StepAndReset();
		return b;
	}

	bool GetValue(const wchar_t *Name, FarColor& Value)
	{
		bool b = stmtGetValue.Bind(Name).Step();
		if (b)
		{
			const void* blob = stmtGetValue.GetColBlob(0);
			Value = *static_cast<const FarColor*>(blob);
		}
		stmtGetValue.Reset();
		return b;
	}

	bool DeleteValue(const wchar_t *Name)
	{
		return stmtDelValue.Bind(Name).StepAndReset();
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("colors");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllValues;
		InitStmt(stmtEnumAllValues, L"SELECT name, value FROM colors ORDER BY name;");

		while (stmtEnumAllValues.Step())
		{
			TiXmlElement *e = new TiXmlElement("object");
			if (!e)
				break;

			e->SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(0));
			const FarColor* Color = reinterpret_cast<const FarColor*>(stmtEnumAllValues.GetColBlob(1));
			e->SetAttribute("background", IntToHexString(Color->BackgroundColor));
			e->SetAttribute("foreground", IntToHexString(Color->ForegroundColor));
			e->SetAttribute("flags", Utf8String(FlagsToString(Color->Flags, ColorFlagNames)));
			root->LinkEndChild(e);
		}

		stmtEnumAllValues.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("colors").FirstChildElement("object").Element(); e; e=e->NextSiblingElement("object"))
		{
			const char *name = e->Attribute("name");
			const char *background = e->Attribute("background");
			const char *foreground = e->Attribute("foreground");
			const char *flags = e->Attribute("flags");

			if (!name)
				continue;

			string Name(name, CP_UTF8);

			if(background && foreground && flags)
			{
				FarColor Color = {};
				Color.BackgroundColor = HexStringToInt(background);
				Color.ForegroundColor = HexStringToInt(foreground);
				Color.Flags = StringToFlags(string(flags, CP_UTF8), ColorFlagNames);
				SetValue(Name, Color);
			}
			else
			{
				DeleteValue(Name);
			}
		}
		EndTransaction();
		return true;
	}

};

class AssociationsConfigDb: public AssociationsConfig, public SQLiteDb {
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
		Initialize(L"associations.db");
	}

	virtual ~AssociationsConfigDb() {}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local) ||

			//schema
			!EnableForeignKeysConstraints() ||
			!Exec(
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
			)
	   ) return false;

		if (
			//add new type and reorder statements
			InitStmt(stmtReorder, L"UPDATE filetypes SET weight=weight+1 WHERE weight>(CASE ?1 WHEN 0 THEN 0 ELSE (SELECT weight FROM filetypes WHERE id=?1) END);") &&
			InitStmt(stmtAddType, L"INSERT INTO filetypes VALUES (NULL,(CASE ?1 WHEN 0 THEN 1 ELSE (SELECT weight FROM filetypes WHERE id=?1)+1 END),?2,?3);") &&

			//get mask statement
			InitStmt(stmtGetMask, L"SELECT mask FROM filetypes WHERE id=?1;") &&

			//get description statement
			InitStmt(stmtGetDescription, L"SELECT description FROM filetypes WHERE id=?1;") &&

			//update type statement
			InitStmt(stmtUpdateType, L"UPDATE filetypes SET mask=?1, description=?2 WHERE id=?3;") &&

			//set association statement
			InitStmt(stmtSetCommand, L"INSERT OR REPLACE INTO commands VALUES (?1,?2,?3,?4);") &&

			//get association statement
			InitStmt(stmtGetCommand, L"SELECT command, enabled FROM commands WHERE ft_id=?1 AND type=?2;") &&

			//enum types statement
			InitStmt(stmtEnumTypes, L"SELECT id, description FROM filetypes ORDER BY weight;") &&

			//enum masks statement
			InitStmt(stmtEnumMasks, L"SELECT id, mask FROM filetypes ORDER BY weight;") &&

			//enum masks with a specific type on statement
			InitStmt(stmtEnumMasksForType, L"SELECT id, mask FROM filetypes, commands WHERE id=ft_id AND type=?1 AND enabled<>0 ORDER BY weight;") &&

			//delete type statement
			InitStmt(stmtDelType, L"DELETE FROM filetypes WHERE id=?1;") &&

			//get weight and set weight statements
			InitStmt(stmtGetWeight, L"SELECT weight FROM filetypes WHERE id=?1;") &&
			InitStmt(stmtSetWeight, L"UPDATE filetypes SET weight=?1 WHERE id=?2;")
		) return true;

		stmtSetWeight.Finalize();
      stmtGetWeight.Finalize();
		stmtDelType.Finalize();
		stmtEnumMasksForType.Finalize();
		stmtEnumMasks.Finalize();
		stmtEnumTypes.Finalize();
		stmtGetCommand.Finalize();
		stmtSetCommand.Finalize();
		stmtUpdateType.Finalize();
		stmtGetDescription.Finalize();
		stmtGetMask.Finalize();
		stmtAddType.Finalize();
		stmtReorder.Finalize();
		return false;
	}

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

		stmtEnumMasks.Reset();
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

		stmtEnumMasksForType.Reset();
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
			return LastInsertRowID();
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

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("associations");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllTypes;
		InitStmt(stmtEnumAllTypes, L"SELECT id, mask, description FROM filetypes ORDER BY weight;");
		SQLiteStmt stmtEnumCommandsPerFiletype;
		InitStmt(stmtEnumCommandsPerFiletype, L"SELECT type, enabled, command FROM commands WHERE ft_id=?1 ORDER BY type;");

		while (stmtEnumAllTypes.Step())
		{
			TiXmlElement *e = new TiXmlElement("filetype");
			if (!e)
				break;

			e->SetAttribute("mask", stmtEnumAllTypes.GetColTextUTF8(1));
			e->SetAttribute("description", stmtEnumAllTypes.GetColTextUTF8(2));

			stmtEnumCommandsPerFiletype.Bind(stmtEnumAllTypes.GetColInt64(0));
			while (stmtEnumCommandsPerFiletype.Step())
			{
				TiXmlElement *se = new TiXmlElement("command");
				if (!se)
					break;

				se->SetAttribute("type", stmtEnumCommandsPerFiletype.GetColInt(0));
				se->SetAttribute("enabled", stmtEnumCommandsPerFiletype.GetColInt(1));
				se->SetAttribute("command", stmtEnumCommandsPerFiletype.GetColTextUTF8(2));
				e->LinkEndChild(se);
			}
			stmtEnumCommandsPerFiletype.Reset();

			root->LinkEndChild(e);
		}

		stmtEnumAllTypes.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		const TiXmlHandle base = root.FirstChild("associations");
		if (!base.ToElement())
			return false;

		BeginTransaction();
		Exec("DELETE FROM filetypes;"); //delete all before importing
		unsigned __int64 id = 0;
		for (const TiXmlElement *e = base.FirstChildElement("filetype").Element(); e; e=e->NextSiblingElement("filetype"))
		{
			const char *mask = e->Attribute("mask");
			const char *description = e->Attribute("description");

			if (!mask)
				continue;

			string Mask(mask, CP_UTF8);
			string Description(description, CP_UTF8);

			id = AddType(id, Mask, Description);
			if (!id)
				continue;

			for (const TiXmlElement *se = e->FirstChildElement("command"); se; se=se->NextSiblingElement("command"))
			{
				const char *command = se->Attribute("command");
				int type=0;
				const char *stype = se->Attribute("type", &type);
				int enabled=0;
				const char *senabled = se->Attribute("enabled", &enabled);

				if (!command || !stype || !senabled)
					continue;

				string Command(command, CP_UTF8);
				SetCommand(id, type, Command, enabled ? true : false);
			}

		}
		EndTransaction();

		return true;
	}
};

class PluginsCacheConfigDb: public PluginsCacheConfig, public SQLiteDb {
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
#if defined(MANTIS_0000466)
	SQLiteStmt stmtGetMacroFuncs;
#endif
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
#if defined(MANTIS_0000466)
	SQLiteStmt stmtSetMacroFuncs;
#endif
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

	bool GetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, string &Text, string &Guid)
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

	bool SetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, const wchar_t *Text, const wchar_t *Guid)
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
		string namedb(L"plugincache"
#if 1
#if   defined(_M_IA64) || defined(__ia64)|| defined(__ia64__)
			L"IA64"
#elif defined(_M_AMD64)|| defined(_M_X64)|| defined(__amd64)|| defined(__amd64__)|| defined(__x86_64)|| defined(__x86_64__)
			L"64"
#elif defined(_M_ARM)  || defined(__arm) || defined(__arm__)|| defined(_ARM_)
			L"ARM"
#elif defined(_M_IX86) || defined(__i386)|| defined(__i386__)
			L"32"
#endif
#endif
			L".db");
		Initialize(namedb.CPtr(), true);
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (!Open(DbName, Local, true))
			return false;

		if (!SetWALJournalingMode())
			return false;

		if (!EnableForeignKeysConstraints())
			return false;

		if (!Exec(
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
#if defined(MANTIS_0000466)
				"CREATE TABLE IF NOT EXISTS macrofuncs(cid INTEGER NOT NULL PRIMARY KEY, func TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
#endif
				"CREATE TABLE IF NOT EXISTS exports(cid INTEGER NOT NULL, export TEXT NOT NULL, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, export));"
				"CREATE TABLE IF NOT EXISTS menuitems(cid INTEGER NOT NULL, type INTEGER NOT NULL, number INTEGER NOT NULL, guid TEXT NOT NULL, name TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, type, number));"
			)
		) return false;

		
		//get menu item text and guid statement
		bool ok = InitStmt(stmtGetMenuItem, L"SELECT name, guid FROM menuitems WHERE cid=?1 AND type=?2 AND number=?3;");

		//set menu item statement
		ok = ok && InitStmt(stmtSetMenuItem, L"INSERT OR REPLACE INTO menuitems VALUES (?1,?2,?3,?4,?5);");

		//add new cache name statement
		ok = ok && InitStmt(stmtCreateCache, L"INSERT INTO cachename VALUES (NULL,?1);");

		//get cache id by name statement
		ok = ok && InitStmt(stmtFindCacheName, L"SELECT id FROM cachename WHERE name=?1;");

		//del cache by name statement
		ok = ok && InitStmt(stmtDelCache, L"DELETE FROM cachename WHERE name=?1;");

		//count cache names statement
		ok = ok && InitStmt(stmtCountCacheNames, L"SELECT count(name) FROM cachename");

		//get preload state statement
		ok = ok && InitStmt(stmtGetPreloadState, L"SELECT enabled FROM preload WHERE cid=?1;");

		//get signature statement
		ok = ok && InitStmt(stmtGetSignature, L"SELECT signature FROM signatures WHERE cid=?1;");

		//get export state statement
		ok = ok && InitStmt(stmtGetExportState, L"SELECT enabled FROM exports WHERE cid=?1 and export=?2;");

		//get guid statement
		ok = ok && InitStmt(stmtGetGuid, L"SELECT guid FROM guids WHERE cid=?1;");

		//get title statement
		ok = ok && InitStmt(stmtGetTitle, L"SELECT title FROM titles WHERE cid=?1;");

		//get author statement
		ok = ok && InitStmt(stmtGetAuthor, L"SELECT author FROM authors WHERE cid=?1;");

		//get description statement
		ok = ok && InitStmt(stmtGetDescription, L"SELECT description FROM descriptions WHERE cid=?1;");

		//get command prefix statement
		ok = ok && InitStmt(stmtGetPrefix, L"SELECT prefix FROM prefixes WHERE cid=?1;");

#if defined(MANTIS_0000466)
		//get macro func statement
		ok = ok && InitStmt(stmtGetMacroFuncs, L"SELECT func FROM macrofuncs WHERE cid=?1;");
#endif
		//get flags statement
		ok = ok && InitStmt(stmtGetFlags, L"SELECT bitmask FROM flags WHERE cid=?1;");

		//get MinFarVersion statement
		ok = ok && InitStmt(stmtGetMinFarVersion, L"SELECT version FROM minfarversions WHERE cid=?1;");

		//get plugin version statement
		ok = ok && InitStmt(stmtGetVersion, L"SELECT version FROM pluginversions WHERE cid=?1;");

		//set preload state statement
		ok = ok && InitStmt(stmtSetPreloadState, L"INSERT OR REPLACE INTO preload VALUES (?1,?2);");

		//set signature statement
		ok = ok && InitStmt(stmtSetSignature, L"INSERT OR REPLACE INTO signatures VALUES (?1,?2);");

		//set export state statement
		ok = ok && InitStmt(stmtSetExportState, L"INSERT OR REPLACE INTO exports VALUES (?1,?2,?3);");

		//set guid statement
		ok = ok && InitStmt(stmtSetGuid, L"INSERT OR REPLACE INTO guids VALUES (?1,?2);");

		//set title statement
		ok = ok && InitStmt(stmtSetTitle, L"INSERT OR REPLACE INTO titles VALUES (?1,?2);");

		//set author statement
		ok = ok && InitStmt(stmtSetAuthor, L"INSERT OR REPLACE INTO authors VALUES (?1,?2);");

		//set description statement
		ok = ok && InitStmt(stmtSetDescription, L"INSERT OR REPLACE INTO descriptions VALUES (?1,?2);");

		//set command prefix statement
		ok = ok && InitStmt(stmtSetPrefix, L"INSERT OR REPLACE INTO prefixes VALUES (?1,?2);");

#if defined(MANTIS_0000466)
		//set macro function statement
		ok = ok && InitStmt(stmtSetMacroFuncs, L"INSERT OR REPLACE INTO macrofuncs VALUES (?1,?2);");
#endif
	   //set flags statement
		ok = ok && InitStmt(stmtSetFlags, L"INSERT OR REPLACE INTO flags VALUES (?1,?2);");

		//set MinFarVersion statement
		ok = ok && InitStmt(stmtSetMinFarVersion, L"INSERT OR REPLACE INTO minfarversions VALUES (?1,?2);");

		//set plugin version statement
		ok = ok && InitStmt(stmtSetVersion, L"INSERT OR REPLACE INTO pluginversions VALUES (?1,?2);");

		//enum cache names statement
		ok = ok && InitStmt(stmtEnumCache, L"SELECT name FROM cachename ORDER BY name;");
		
		if (ok)
			return true;

		stmtEnumCache.Finalize();
		stmtSetVersion.Finalize();
		stmtSetMinFarVersion.Finalize();
		stmtSetFlags.Finalize();
#if defined(MANTIS_0000466)
		stmtSetMacroFuncs.Finalize();
#endif
		stmtSetPrefix.Finalize();
		stmtSetDescription.Finalize();
		stmtSetAuthor.Finalize();
		stmtSetTitle.Finalize();
		stmtSetGuid.Finalize();
		stmtSetExportState.Finalize();
		stmtSetSignature.Finalize();
		stmtSetPreloadState.Finalize();
		stmtGetVersion.Finalize();
		stmtGetMinFarVersion.Finalize();
		stmtGetFlags.Finalize();
#if defined(MANTIS_0000466)
		stmtGetMacroFuncs.Finalize();
#endif
		stmtGetPrefix.Finalize();
		stmtGetDescription.Finalize();
		stmtGetAuthor.Finalize();
		stmtGetTitle.Finalize();
		stmtGetGuid.Finalize();
		stmtGetExportState.Finalize();
		stmtGetSignature.Finalize();
		stmtGetPreloadState.Finalize();
		stmtCountCacheNames.Finalize();
		stmtDelCache.Finalize();
		stmtFindCacheName.Finalize();
		stmtCreateCache.Finalize();
		stmtSetMenuItem.Finalize();
		stmtGetMenuItem.Finalize();
		return false;
	}

	virtual ~PluginsCacheConfigDb() {}

	unsigned __int64 CreateCache(const wchar_t *CacheName)
	{
		if (stmtCreateCache.Bind(CacheName).StepAndReset())
			return LastInsertRowID();
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
				enabled = ToPtr(1);
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

	bool GetDiskMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid)
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool GetPluginsMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid)
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool GetPluginsConfigMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid)
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	string GetCommandPrefix(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetPrefix, id);
	}

#if defined(MANTIS_0000466)
	string GetMacroFunctions(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetMacroFuncs, id);
	}
#endif

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

	bool SetDiskMenuItem(unsigned __int64 id, size_t index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	bool SetCommandPrefix(unsigned __int64 id, const wchar_t *Prefix)
	{
		return stmtSetPrefix.Bind(id).Bind(Prefix).StepAndReset();
	}

#if defined(MANTIS_0000466)
	bool SetMacroFunctions(unsigned __int64 id, const wchar_t *MacroFunc)
	{
		return stmtSetMacroFuncs.Bind(id).Bind(MacroFunc).StepAndReset();
	}
#endif
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
		return stmtSetMinFarVersion.Bind(id).Bind(Version, sizeof(VersionInfo)).StepAndReset();
	}

	bool SetVersion(unsigned __int64 id, const VersionInfo *Version)
	{
		return stmtSetVersion.Bind(id).Bind(Version,sizeof(VersionInfo)).StepAndReset();
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

		stmtEnumCache.Reset();
		return false;
	}

	bool DiscardCache()
	{
		BeginTransaction();
		bool ret = Exec("DELETE FROM cachename");
		EndTransaction();
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

class PluginsHotkeysConfigDb: public PluginsHotkeysConfig, public SQLiteDb {
	SQLiteStmt stmtGetHotkey;
	SQLiteStmt stmtSetHotkey;
	SQLiteStmt stmtDelHotkey;
	SQLiteStmt stmtCheckForHotkeys;

public:

	PluginsHotkeysConfigDb()
	{
		Initialize(L"pluginhotkeys.db");
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local) ||

			//schema
			!Exec("CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));")
		) return false;

		if (
			//get hotkey statement
			InitStmt(stmtGetHotkey, L"SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;") &&

			//set hotkey statement
			InitStmt(stmtSetHotkey, L"INSERT OR REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);") &&

			//delete hotkey statement
			InitStmt(stmtDelHotkey, L"DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;") &&

			//check if exist hotkeys of specific type statement
			InitStmt(stmtCheckForHotkeys, L"SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1")
		) return true;

      stmtCheckForHotkeys.Finalize();
		stmtDelHotkey.Finalize();
		stmtSetHotkey.Finalize();
		stmtGetHotkey.Finalize();
		return false;
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

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("pluginhotkeys");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllPluginKeys;
		InitStmt(stmtEnumAllPluginKeys, L"SELECT pluginkey FROM pluginhotkeys GROUP BY pluginkey;");
		SQLiteStmt stmtEnumAllHotkeysPerKey;
		InitStmt(stmtEnumAllHotkeysPerKey, L"SELECT menuguid, type, hotkey FROM pluginhotkeys WHERE pluginkey=$1;");

		while (stmtEnumAllPluginKeys.Step())
		{
			TiXmlElement *p = new TiXmlElement("plugin");
			if (!p)
				break;

			string Key = stmtEnumAllPluginKeys.GetColText(0);
			p->SetAttribute("key", stmtEnumAllPluginKeys.GetColTextUTF8(0));

			stmtEnumAllHotkeysPerKey.Bind(Key);
			while (stmtEnumAllHotkeysPerKey.Step())
			{
				TiXmlElement *e = new TiXmlElement("hotkey");
				if (!e)
					break;

				const char *type;
				switch (stmtEnumAllHotkeysPerKey.GetColInt(1))
				{
					case DRIVE_MENU: type = "drive"; break;
					case CONFIG_MENU: type = "config"; break;
					default: type = "plugins";
				}
				e->SetAttribute("menu", type);
				e->SetAttribute("guid", stmtEnumAllHotkeysPerKey.GetColTextUTF8(0));
				const char *hotkey = stmtEnumAllHotkeysPerKey.GetColTextUTF8(2);
				e->SetAttribute("hotkey", hotkey ? hotkey : "");
				p->LinkEndChild(e);
			}
			stmtEnumAllHotkeysPerKey.Reset();

			root->LinkEndChild(p);
		}

		stmtEnumAllPluginKeys.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("pluginhotkeys").FirstChildElement("plugin").Element(); e; e=e->NextSiblingElement("plugin"))
		{
			const char *key = e->Attribute("key");

			if (!key)
				continue;

			string Key(key, CP_UTF8);

			for (const TiXmlElement *se = e->FirstChildElement("hotkey"); se; se=se->NextSiblingElement("hotkey"))
			{
				const char *stype = se->Attribute("menu");
				const char *guid = se->Attribute("guid");
				const char *hotkey = se->Attribute("hotkey");

				if (!guid || !stype)
					continue;

				string Guid(guid, CP_UTF8);
				string Hotkey(hotkey, CP_UTF8);
				HotKeyTypeEnum type;
				if (!strcmp(stype,"drive"))
					type = DRIVE_MENU;
				else if (!strcmp(stype,"config"))
					type = CONFIG_MENU;
				else
					type = PLUGINS_MENU;
				SetHotkey(Key, Guid, type, Hotkey);
			}

		}
		EndTransaction();

		return true;
	}
};

class HistoryConfigCustom: public HistoryConfig, public SQLiteDb {
	SQLiteStmt stmtEnum;
	SQLiteStmt stmtEnumDesc;
	SQLiteStmt stmtDel;
	SQLiteStmt stmtDeleteOldUnlocked;
	SQLiteStmt stmtEnumLargeHistories;
	SQLiteStmt stmtAdd;
	SQLiteStmt stmtGetName;
	SQLiteStmt stmtGetNameAndType;
	SQLiteStmt stmtGetNewestName;
	SQLiteStmt stmtCount;
	SQLiteStmt stmtDelUnlocked;
	SQLiteStmt stmtGetLock;
	SQLiteStmt stmtSetLock;
	SQLiteStmt stmtGetNext;
	SQLiteStmt stmtGetPrev;
	SQLiteStmt stmtGetNewest;
	SQLiteStmt stmtGetLastEmpty;
	SQLiteStmt stmtSetLastEmpty;
	SQLiteStmt stmtSetEditorPos;
	SQLiteStmt stmtSetEditorBookmark;
	SQLiteStmt stmtGetEditorPos;
	SQLiteStmt stmtGetEditorBookmark;
	SQLiteStmt stmtSetViewerPos;
	SQLiteStmt stmtSetViewerBookmark;
	SQLiteStmt stmtGetViewerPos;
	SQLiteStmt stmtGetViewerBookmark;
	SQLiteStmt stmtDeleteOldEditor;
	SQLiteStmt stmtDeleteOldViewer;

	unsigned __int64 CalcDays(int Days)
	{
		return ((unsigned __int64)Days) * 24ull * 60ull * 60ull * 10000000ull;
	}

public:

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local, true) ||

			//schema
			!SetWALJournalingMode() ||

			!EnableForeignKeysConstraints() ||
			//command,view,edit,folder,dialog history
			!Exec(
				"CREATE TABLE IF NOT EXISTS history(id INTEGER PRIMARY KEY, kind INTEGER NOT NULL, key TEXT NOT NULL, type INTEGER NOT NULL, lock INTEGER NOT NULL, name TEXT NOT NULL, time INTEGER NOT NULL, guid TEXT NOT NULL, file TEXT NOT NULL, data TEXT NOT NULL);"
				"CREATE INDEX IF NOT EXISTS history_idx1 ON history (kind, key);"
				"CREATE INDEX IF NOT EXISTS history_idx2 ON history (kind, key, time);"
				"CREATE INDEX IF NOT EXISTS history_idx3 ON history (kind, key, lock DESC, time DESC);"
				"CREATE INDEX IF NOT EXISTS history_idx4 ON history (kind, key, time DESC);"
			) ||
			//view,edit file positions and bookmarks history
			!Exec(
				"CREATE TABLE IF NOT EXISTS editorposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, time INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, codepage INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS editorbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES editorposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
				"CREATE INDEX IF NOT EXISTS editorposition_history_idx1 ON editorposition_history (time DESC);"
				"CREATE TABLE IF NOT EXISTS viewerposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, time INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, hex INTEGER NOT NULL, codepage INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS viewerbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES viewerposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
				"CREATE INDEX IF NOT EXISTS viewerposition_history_idx1 ON viewerposition_history (time DESC);"
			)
		) return false;

		if (
			//enum items order by time statement
			InitStmt(stmtEnum, L"SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY time;") &&

			//enum items order by time DESC and lock DESC statement
			InitStmt(stmtEnumDesc, L"SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC;") &&

			//delete item statement
			InitStmt(stmtDel, L"DELETE FROM history WHERE id=?1;") &&

			//delete old unlocked items statement
			InitStmt(stmtDeleteOldUnlocked, L"DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0 AND time<?3 AND id NOT IN (SELECT id FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT ?4);") &&

			//enum histories with more than X entries statement
			InitStmt(stmtEnumLargeHistories, L"SELECT key FROM (SELECT key, num FROM (SELECT key, count(id) as num FROM history WHERE kind=?1 GROUP BY key)) WHERE num > ?2;") &&

			//add item statement
			InitStmt(stmtAdd, L"INSERT INTO history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9);") &&

			//get item name statement
			InitStmt(stmtGetName, L"SELECT name FROM history WHERE id=?1;") &&

			//get item name and type statement
			InitStmt(stmtGetNameAndType, L"SELECT name, type, guid, file, data FROM history WHERE id=?1;") &&

			//get newest item (locked items go first) name statement
			InitStmt(stmtGetNewestName, L"SELECT name FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT 1;") &&

			//count items statement
			InitStmt(stmtCount, L"SELECT count(id) FROM history WHERE kind=?1 AND key=?2;") &&

			//delete unlocked items statement
			InitStmt(stmtDelUnlocked, L"DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0;") &&

			//get item lock statement
			InitStmt(stmtGetLock, L"SELECT lock FROM history WHERE id=?1;") &&

			//set item lock statement
			InitStmt(stmtSetLock, L"UPDATE history SET lock=?1 WHERE id=?2") &&

			//get next (newer than current) item statement
			InitStmt(stmtGetNext, L"SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time>b.time ORDER BY a.time LIMIT 1;") &&

			//get prev (older than current) item statement
			InitStmt(stmtGetPrev, L"SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time<b.time ORDER BY a.time DESC LIMIT 1;") &&

			//get newest item name statement
			InitStmt(stmtGetNewest, L"SELECT id, name FROM history WHERE kind=?1 AND key=?2 ORDER BY time DESC LIMIT 1;") &&

			//set editor position statement
			InitStmt(stmtSetEditorPos, L"INSERT OR REPLACE INTO editorposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7);") &&

			//set editor bookmark statement
			InitStmt(stmtSetEditorBookmark, L"INSERT OR REPLACE INTO editorbookmarks_history VALUES (?1,?2,?3,?4,?5,?6);") &&

			//get editor position statement
			InitStmt(stmtGetEditorPos, L"SELECT id, line, linepos, screenline, leftpos, codepage FROM editorposition_history WHERE name=?1;") &&

			//get editor bookmark statement
			InitStmt(stmtGetEditorBookmark, L"SELECT line, linepos, screenline, leftpos FROM editorbookmarks_history WHERE pid=?1 AND num=?2;") &&

			//set viewer position statement
			InitStmt(stmtSetViewerPos, L"INSERT OR REPLACE INTO viewerposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6);") &&

			//set viewer bookmark statement
			InitStmt(stmtSetViewerBookmark, L"INSERT OR REPLACE INTO viewerbookmarks_history VALUES (?1,?2,?3,?4);") &&

			//get viewer position statement
			InitStmt(stmtGetViewerPos, L"SELECT id, filepos, leftpos, hex, codepage FROM viewerposition_history WHERE name=?1;") &&

			//get viewer bookmark statement
			InitStmt(stmtGetViewerBookmark, L"SELECT filepos, leftpos FROM viewerbookmarks_history WHERE pid=?1 AND num=?2;") &&

			//delete old editor positions statement
			InitStmt(stmtDeleteOldEditor, L"DELETE FROM editorposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM editorposition_history ORDER BY time DESC LIMIT ?2);") &&

			//delete old viewer positions statement
			InitStmt(stmtDeleteOldViewer, L"DELETE FROM viewerposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM viewerposition_history ORDER BY time DESC LIMIT ?2);")
		) return true;

		stmtDeleteOldViewer.Finalize();
		stmtDeleteOldEditor.Finalize();
		stmtGetViewerBookmark.Finalize();
		stmtGetViewerPos.Finalize();
		stmtSetViewerBookmark.Finalize();
		stmtSetViewerPos.Finalize();
		stmtGetEditorBookmark.Finalize();
		stmtGetEditorPos.Finalize();
		stmtSetEditorBookmark.Finalize();
		stmtSetEditorPos.Finalize();
		stmtGetNewest.Finalize();
		stmtGetPrev.Finalize();
		stmtGetNext.Finalize();
		stmtSetLock.Finalize();
		stmtGetLock.Finalize();
		stmtDelUnlocked.Finalize();
		stmtCount.Finalize();
		stmtGetNewestName.Finalize();
		stmtGetNameAndType.Finalize();
		stmtGetName.Finalize();
		stmtAdd.Finalize();
		stmtEnumLargeHistories.Finalize();
		stmtDeleteOldUnlocked.Finalize();
		stmtDel.Finalize();
		stmtEnumDesc.Finalize();
		stmtEnum.Finalize();
		return false;
	}

	virtual ~HistoryConfigCustom() {}

	bool Enum(DWORD index, DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 *id, string &strName, int *Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false)
	{
		SQLiteStmt &stmt = Reverse ? stmtEnumDesc : stmtEnum;

		if (index == 0)
			stmt.Reset().Bind((int)TypeHistory).Bind(HistoryName,false);

		if (stmt.Step())
		{
			*id = stmt.GetColInt64(0);
			strName = stmt.GetColText(1);
			*Type = stmt.GetColInt(2);
			*Lock = stmt.GetColInt(3) ? true : false;
			*Time = stmt.GetColInt64(4);
			strGuid = stmt.GetColText(5);
			strFile = stmt.GetColText(6);
			strData = stmt.GetColText(7);
			return true;
		}

		stmt.Reset();
		return false;
	}

	bool Delete(unsigned __int64 id)
	{
		return stmtDel.Bind(id).StepAndReset();
	}

	bool DeleteOldUnlocked(DWORD TypeHistory, const wchar_t *HistoryName, int DaysToKeep, int MinimumEntries)
	{
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= CalcDays(DaysToKeep);
		return stmtDeleteOldUnlocked.Bind((int)TypeHistory).Bind(HistoryName).Bind(older).Bind(MinimumEntries).StepAndReset();
	}

	bool EnumLargeHistories(DWORD index, int MinimumEntries, DWORD TypeHistory, string &strHistoryName)
	{
		if (index == 0)
			stmtEnumLargeHistories.Reset().Bind((int)TypeHistory).Bind(MinimumEntries);

		if (stmtEnumLargeHistories.Step())
		{
			strHistoryName = stmtEnumLargeHistories.GetColText(0);
			return true;
		}

		stmtEnumLargeHistories.Reset();
		return false;
	}

	bool Add(DWORD TypeHistory, const wchar_t *HistoryName, string strName, int Type, bool Lock, string &strGuid, string &strFile, string &strData)
	{
		return stmtAdd.Bind((int)TypeHistory).Bind(HistoryName).Bind(Type).Bind(Lock?1:0).Bind(strName).Bind(GetCurrentUTCTimeInUI64()).Bind(strGuid).Bind(strFile).Bind(strData).StepAndReset();
	}

	bool GetNewest(DWORD TypeHistory, const wchar_t *HistoryName, string &strName)
	{
		bool b = stmtGetNewestName.Bind((int)TypeHistory).Bind(HistoryName).Step();
		if (b)
		{
			strName = stmtGetNewestName.GetColText(0);
		}
		stmtGetNewestName.Reset();
		return b;
	}

	bool Get(unsigned __int64 id, string &strName)
	{
		bool b = stmtGetName.Bind(id).Step();
		if (b)
		{
			strName = stmtGetName.GetColText(0);
		}
		stmtGetName.Reset();
		return b;
	}

	bool Get(unsigned __int64 id, string &strName, int *Type, string &strGuid, string &strFile, string &strData)
	{
		bool b = stmtGetNameAndType.Bind(id).Step();
		if (b)
		{
			strName = stmtGetNameAndType.GetColText(0);
			*Type = stmtGetNameAndType.GetColInt(1);
			strGuid = stmtGetNameAndType.GetColText(2);
			strFile = stmtGetNameAndType.GetColText(3);
			strData = stmtGetNameAndType.GetColText(4);
		}
		stmtGetNameAndType.Reset();
		return b;
	}

	DWORD Count(DWORD TypeHistory, const wchar_t *HistoryName)
	{
		DWORD c = 0;
		if (stmtCount.Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			 c = (DWORD) stmtCount.GetColInt(0);
		}
		stmtCount.Reset();
		return c;
	}

	bool FlipLock(unsigned __int64 id)
	{
		return stmtSetLock.Bind(IsLocked(id)?0:1).Bind(id).StepAndReset();
	}

	bool IsLocked(unsigned __int64 id)
	{
		bool l = false;
		if (stmtGetLock.Bind(id).Step())
		{
			 l = stmtGetLock.GetColInt(0) ? true : false;
		}
		stmtGetLock.Reset();
		return l;
	}

	bool DeleteAllUnlocked(DWORD TypeHistory, const wchar_t *HistoryName)
	{
		return stmtDelUnlocked.Bind((int)TypeHistory).Bind(HistoryName).StepAndReset();
	}

	unsigned __int64 GetNext(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName)
	{
		strName.Clear();
		unsigned __int64 nid = 0;
		if (!id)
			return nid;
		if (stmtGetNext.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetNext.GetColInt64(0);
			strName = stmtGetNext.GetColText(1);
		}
		stmtGetNext.Reset();
		return nid;
	}

	unsigned __int64 GetPrev(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName)
	{
		strName.Clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (stmtGetNewest.Bind((int)TypeHistory).Bind(HistoryName).Step())
			{
				nid = stmtGetNewest.GetColInt64(0);
				strName = stmtGetNewest.GetColText(1);
			}
			stmtGetNewest.Reset();
			return nid;
		}
		if (stmtGetPrev.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetPrev.GetColInt64(0);
			strName = stmtGetPrev.GetColText(1);
		}
		else if (Get(id, strName))
		{
			nid = id;
		}
		stmtGetPrev.Reset();
		return nid;
	}

	unsigned __int64 CyclicGetPrev(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName)
	{
		strName.Clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (stmtGetNewest.Bind((int)TypeHistory).Bind(HistoryName).Step())
			{
				nid = stmtGetNewest.GetColInt64(0);
				strName = stmtGetNewest.GetColText(1);
			}
			stmtGetNewest.Reset();
			return nid;
		}
		if (stmtGetPrev.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetPrev.GetColInt64(0);
			strName = stmtGetPrev.GetColText(1);
		}
		stmtGetPrev.Reset();
		return nid;
	}

	unsigned __int64 SetEditorPos(const wchar_t *Name, int Line, int LinePos, int ScreenLine, int LeftPos, UINT CodePage)
	{
		if (stmtSetEditorPos.Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(Line).Bind(LinePos).Bind(ScreenLine).Bind(LeftPos).Bind((int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetEditorPos(const wchar_t *Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, UINT *CodePage)
	{
		unsigned __int64 id=0;
		if (stmtGetEditorPos.Bind(Name).Step())
		{
			id = stmtGetEditorPos.GetColInt64(0);
			*Line = stmtGetEditorPos.GetColInt(1);
			*LinePos = stmtGetEditorPos.GetColInt(2);
			*ScreenLine = stmtGetEditorPos.GetColInt(3);
			*LeftPos = stmtGetEditorPos.GetColInt(4);
			*CodePage = stmtGetEditorPos.GetColInt(5);
		}
		stmtGetEditorPos.Reset();
		return id;
	}

	bool SetEditorBookmark(unsigned __int64 id, int i, int Line, int LinePos, int ScreenLine, int LeftPos)
	{
		return stmtSetEditorBookmark.Bind(id).Bind(i).Bind(Line).Bind(LinePos).Bind(ScreenLine).Bind(LeftPos).StepAndReset();
	}

	bool GetEditorBookmark(unsigned __int64 id, int i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos)
	{
		bool b = stmtGetEditorBookmark.Bind(id).Bind(i).Step();
		if (b)
		{
			*Line = stmtGetEditorBookmark.GetColInt(0);
			*LinePos = stmtGetEditorBookmark.GetColInt(1);
			*ScreenLine = stmtGetEditorBookmark.GetColInt(2);
			*LeftPos = stmtGetEditorBookmark.GetColInt(3);
		}
		stmtGetEditorBookmark.Reset();
		return b;
	}

	unsigned __int64 SetViewerPos(const wchar_t *Name, __int64 FilePos, __int64 LeftPos, int Hex_Wrap, UINT CodePage)
	{
		if (stmtSetViewerPos.Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(FilePos).Bind(LeftPos).Bind(Hex_Wrap).Bind((int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetViewerPos(const wchar_t *Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, UINT *CodePage)
	{
		unsigned __int64 id=0;
		if (stmtGetViewerPos.Bind(Name).Step())
		{
			id = stmtGetViewerPos.GetColInt64(0);
			*FilePos = stmtGetViewerPos.GetColInt64(1);
			*LeftPos = stmtGetViewerPos.GetColInt64(2);
			*Hex = stmtGetViewerPos.GetColInt(3);
			*CodePage = stmtGetViewerPos.GetColInt(4);
		}
		stmtGetViewerPos.Reset();
		return id;
	}

	bool SetViewerBookmark(unsigned __int64 id, int i, __int64 FilePos, __int64 LeftPos)
	{
		return stmtSetViewerBookmark.Bind(id).Bind(i).Bind(FilePos).Bind(LeftPos).StepAndReset();
	}

	bool GetViewerBookmark(unsigned __int64 id, int i, __int64 *FilePos, __int64 *LeftPos)
	{
		bool b = stmtGetViewerBookmark.Bind(id).Bind(i).Step();
		if (b)
		{
			*FilePos = stmtGetViewerBookmark.GetColInt64(0);
			*LeftPos = stmtGetViewerBookmark.GetColInt64(1);
		}
		stmtGetViewerBookmark.Reset();
		return b;
	}

	void DeleteOldPositions(int DaysToKeep, int MinimumEntries)
	{
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= CalcDays(DaysToKeep);
		stmtDeleteOldEditor.Bind(older).Bind(MinimumEntries).StepAndReset();
		stmtDeleteOldViewer.Bind(older).Bind(MinimumEntries).StepAndReset();
	}

};

class HistoryConfigDb: public HistoryConfigCustom {
public:
	HistoryConfigDb()
	{
		Initialize(L"history.db", true);
	}
};

class HistoryConfigMemory: public HistoryConfigCustom {
public:
	HistoryConfigMemory()
	{
		Initialize(L":memory:", true);
	}
};

class MacroConfigDb: public MacroConfig, public SQLiteDb {
	SQLiteStmt stmtConstsEnum;
	SQLiteStmt stmtGetConstValue;
	SQLiteStmt stmtSetConstValue;
	SQLiteStmt stmtDelConst;

	SQLiteStmt stmtVarsEnum;
	SQLiteStmt stmtGetVarValue;
	SQLiteStmt stmtSetVarValue;
	SQLiteStmt stmtDelVar;

	SQLiteStmt stmtFunctionsEnum;
	SQLiteStmt stmtSetFunction;
	SQLiteStmt stmtDelFunction;

	SQLiteStmt stmtKeyMacrosEnum;
	SQLiteStmt stmtSetKeyMacro;
	SQLiteStmt stmtDelKeyMacro;

public:

	MacroConfigDb()
	{
		Initialize(L"macros.db");
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	bool InitializeImpl(const wchar_t* DbName, bool Local)
	{
		if (
			!Open(DbName, Local) ||
			//schema
			!Exec(
				"CREATE TABLE IF NOT EXISTS constants(name TEXT NOT NULL, value TEXT, type TEXT NOT NULL, PRIMARY KEY (name));"
				"CREATE TABLE IF NOT EXISTS variables(name TEXT NOT NULL, value TEXT, type TEXT NOT NULL, PRIMARY KEY (name));"
				"CREATE TABLE IF NOT EXISTS functions(guid TEXT NOT NULL, name TEXT NOT NULL, flags TEXT, sequence TEXT, syntax TEXT NOT NULL, description TEXT, PRIMARY KEY (guid, name));"
				"CREATE TABLE IF NOT EXISTS key_macros(area TEXT NOT NULL, key TEXT NOT NULL, flags TEXT, sequence TEXT, description TEXT, PRIMARY KEY (area, key));"
			)
		) return false;

		if (
			InitStmt(stmtConstsEnum, L"SELECT name, value, type FROM constants ORDER BY name;") &&
			InitStmt(stmtGetConstValue, L"SELECT value, type FROM constants WHERE name=?1;") &&
			InitStmt(stmtSetConstValue, L"INSERT OR REPLACE INTO constants VALUES (?1,?2,?3);") &&
			InitStmt(stmtDelConst, L"DELETE FROM constants WHERE name=?1;") &&

			InitStmt(stmtVarsEnum, L"SELECT name, value, type FROM variables ORDER BY name;") &&
			InitStmt(stmtGetVarValue, L"SELECT value,type FROM variables WHERE name=?1;") &&
			InitStmt(stmtSetVarValue, L"INSERT OR REPLACE INTO variables VALUES (?1,?2,?3);") &&
			InitStmt(stmtDelVar, L"DELETE FROM variables WHERE name=?1;") &&

			InitStmt(stmtFunctionsEnum, L"SELECT guid, name, flags, sequence, syntax, description FROM functions ORDER BY guid, name;") &&
			InitStmt(stmtSetFunction, L"INSERT OR REPLACE INTO functions VALUES (?1,?2,?3,?4,?5,?6);") &&
			InitStmt(stmtDelFunction, L"DELETE FROM functions WHERE guid=?1 AND name=?2;") &&

			InitStmt(stmtKeyMacrosEnum, L"SELECT key, flags, sequence, description FROM key_macros WHERE area=?1 ORDER BY key;") &&
			InitStmt(stmtSetKeyMacro, L"INSERT OR REPLACE INTO key_macros VALUES (?1,?2,?3,?4,?5);") &&
			InitStmt(stmtDelKeyMacro, L"DELETE FROM key_macros WHERE area=?1 AND key=?2;")
		) return true;

		stmtDelKeyMacro.Finalize();
		stmtSetKeyMacro.Finalize();
		stmtKeyMacrosEnum.Finalize();

		stmtDelFunction.Finalize();
		stmtSetFunction.Finalize();
		stmtFunctionsEnum.Finalize();

		stmtDelVar.Finalize();
		stmtSetVarValue.Finalize();
		stmtGetVarValue.Finalize();
		stmtVarsEnum.Finalize();

		stmtDelConst.Finalize();
		stmtSetConstValue.Finalize();
		stmtGetConstValue.Finalize();
		stmtConstsEnum.Finalize();

		return false;
	}

	virtual ~MacroConfigDb() { }

	/* *************** */
	bool EnumConsts(string &strName, string &Value, string &Type)
	{
		if (stmtConstsEnum.Step())
		{
			strName = stmtConstsEnum.GetColText(0);
			Value = stmtConstsEnum.GetColText(1);
			Type = stmtConstsEnum.GetColText(2);
			return true;
		}

		stmtConstsEnum.Reset();
		return false;
	}

	bool GetConstValue(const wchar_t *Name, string &Value, string &Type)
	{
		bool b = stmtGetConstValue.Bind(Name).Step();
		if (b)
		{
			Value = stmtGetConstValue.GetColText(0);
			Type  = stmtGetConstValue.GetColText(1);
		}
		stmtGetConstValue.Reset();
		return b;
	}

	unsigned __int64 SetConstValue(const wchar_t *Name, const wchar_t *Value, const wchar_t *Type)
	{
		if (stmtSetConstValue.Bind(Name).Bind(Value).Bind(Type).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	bool DeleteConst(const wchar_t *Name)
	{
		return stmtDelConst.Bind(Name).StepAndReset();
	}

	/* *************** */
	bool EnumVars(string &strName, string &Value, string &Type)
	{
		if (stmtVarsEnum.Step())
		{
			strName = stmtVarsEnum.GetColText(0);
			Value = stmtVarsEnum.GetColText(1);
			Type = stmtVarsEnum.GetColText(2);
			return true;
		}

		stmtVarsEnum.Reset();
		return false;
	}

	bool GetVarValue(const wchar_t *Name, string &Value, string &Type)
	{
		bool b = stmtGetVarValue.Bind(Name).Step();
		if (b)
		{
			Value = stmtGetVarValue.GetColText(0);
			Type  = stmtGetVarValue.GetColText(1);
		}
		stmtGetVarValue.Reset();
		return b;
	}

	unsigned __int64 SetVarValue(const wchar_t *Name, const wchar_t *Value, const wchar_t *Type)
	{
		if (stmtSetVarValue.Bind(Name).Bind(Value).Bind(Type).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	bool DeleteVar(const wchar_t *Name)
	{
		return stmtDelVar.Bind(Name).StepAndReset();
	}

	/* *************** */
	bool EnumFunctions(string &strGuid, string &strFunctionName, string &strFlags, string &strSequence, string &strSyntax, string &strDescription)
	{
		if (stmtFunctionsEnum.Step())
		{
			strGuid = stmtFunctionsEnum.GetColText(0);
			strFunctionName = stmtFunctionsEnum.GetColText(1);
			strFlags = stmtFunctionsEnum.GetColText(2);
			strSequence = stmtFunctionsEnum.GetColText(3);
			strSyntax = stmtFunctionsEnum.GetColText(4);
			strDescription = stmtFunctionsEnum.GetColText(5);
			return true;
		}

		stmtKeyMacrosEnum.Reset();
		return false;
	}

	unsigned __int64 SetFunction(const wchar_t *Guid, const wchar_t *FunctionName, const wchar_t *Flags, const wchar_t *Sequence, const wchar_t *Syntax, const wchar_t *Description)
	{
		if (stmtSetFunction.Bind(Guid).Bind(FunctionName).Bind(Flags).Bind(Sequence).Bind(Syntax).Bind(Description).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	bool DeleteFunction(const wchar_t *Guid, const wchar_t *Name)
	{
		return stmtDelFunction.Bind(Guid).Bind(Name).StepAndReset();
	}

	/* *************** */
	bool EnumKeyMacros(string &strArea, string &strKey, string &strFlags, string &strSequence, string &strDescription)
	{
		stmtKeyMacrosEnum.Bind(strArea);
		if (stmtKeyMacrosEnum.Step())
		{
			strKey = stmtKeyMacrosEnum.GetColText(0);
			strFlags = stmtKeyMacrosEnum.GetColText(1);
			strSequence = stmtKeyMacrosEnum.GetColText(2);
			strDescription = stmtKeyMacrosEnum.GetColText(3);
			return true;
		}

		stmtKeyMacrosEnum.Reset();
		return false;
	}

	unsigned __int64 SetKeyMacro(const wchar_t *Area, const wchar_t *Key, const wchar_t *Flags, const wchar_t *Sequence, const wchar_t *Description)
	{
		if (stmtSetKeyMacro.Bind(Area).Bind(Key).Bind(Flags).Bind(Sequence).Bind(Description).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	bool DeleteKeyMacro(const wchar_t *Area, const wchar_t *Key)
	{
		return stmtDelKeyMacro.Bind(Area).Bind(Key).StepAndReset();
	}

	/* *************** */
	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("macros");
		if (!root)
			return nullptr;

		TiXmlElement *e;
		TiXmlElement *se;

		SQLiteStmt stmtEnumAllConsts;
		InitStmt(stmtEnumAllConsts, L"SELECT name, value, type FROM constants ORDER BY name;");

		SQLiteStmt stmtEnumAllVars;
		InitStmt(stmtEnumAllVars, L"SELECT name, value, type FROM variables ORDER BY name;");

		SQLiteStmt stmtEnumAllFunctions;
		InitStmt(stmtEnumAllFunctions, L"SELECT guid, name, flags, sequence, syntax, description FROM functions ORDER BY guid, name;");

		SQLiteStmt stmtEnumAllKeyMacros;
		InitStmt(stmtEnumAllKeyMacros, L"SELECT area, key, flags, sequence, description FROM key_macros ORDER BY area, key;");

		// --------------------------------------------------
		e = new TiXmlElement("constants");
		if (!e)
			return nullptr;

		while (stmtEnumAllConsts.Step())
		{
			se = new TiXmlElement("constant");
			if (!se)
				break;

			se->SetAttribute("name", stmtEnumAllConsts.GetColTextUTF8(0));
			se->SetAttribute("type", stmtEnumAllConsts.GetColTextUTF8(2));
			const char* value = stmtEnumAllConsts.GetColTextUTF8(1);
			TiXmlText* vtext = new TiXmlText(value);
			SetCDataIfNeeded(vtext, value);
			TiXmlElement *text = new TiXmlElement("text");
			text->LinkEndChild(vtext);
			se->LinkEndChild(text);
			e->LinkEndChild(se);
		}
		stmtEnumAllConsts.Reset();

		root->LinkEndChild(e);

		// --------------------------------------------------
		e = new TiXmlElement("variables");
		if (!e)
			return root;

		while (stmtEnumAllVars.Step())
		{
			se = new TiXmlElement("variable");
			if (!se)
				break;

			se->SetAttribute("name", stmtEnumAllVars.GetColTextUTF8(0));
			se->SetAttribute("type", stmtEnumAllVars.GetColTextUTF8(2));
			const char* value = stmtEnumAllVars.GetColTextUTF8(1);
			TiXmlText* vtext = new TiXmlText(value);
			SetCDataIfNeeded(vtext, value);
			TiXmlElement *text = new TiXmlElement("text");
			text->LinkEndChild(vtext);
			se->LinkEndChild(text);
			e->LinkEndChild(se);
		}
		stmtEnumAllVars.Reset();

		root->LinkEndChild(e);

		// --------------------------------------------------
		e = new TiXmlElement("functions");
		if (!e)
			return root;

		while (stmtEnumAllFunctions.Step())
		{
			se = new TiXmlElement("function");
			if (!se)
				break;

			se->SetAttribute("guid", stmtEnumAllFunctions.GetColTextUTF8(0));
			se->SetAttribute("name", stmtEnumAllFunctions.GetColTextUTF8(1));
			se->SetAttribute("flags", stmtEnumAllFunctions.GetColTextUTF8(2));
			se->SetAttribute("sequence", stmtEnumAllFunctions.GetColTextUTF8(3));
			se->SetAttribute("syntax", stmtEnumAllFunctions.GetColTextUTF8(4));
			se->SetAttribute("description", stmtEnumAllFunctions.GetColTextUTF8(5));
			e->LinkEndChild(se);
		}
		stmtEnumAllFunctions.Reset();

		root->LinkEndChild(e);

		// --------------------------------------------------
		e = new TiXmlElement("keymacros");
		if (!e)
			return root;

		while (stmtEnumAllKeyMacros.Step())
		{
			se = new TiXmlElement("macro");
			if (!se)
				break;

			se->SetAttribute("area", stmtEnumAllKeyMacros.GetColTextUTF8(0));
			se->SetAttribute("key", stmtEnumAllKeyMacros.GetColTextUTF8(1));
			string strFlags = stmtEnumAllKeyMacros.GetColTextUTF8(2);
			if(!strFlags.IsEmpty())
			{
				se->SetAttribute("flags", Utf8String(strFlags));
			}
			const char* Description = stmtEnumAllKeyMacros.GetColTextUTF8(4);
			if(Description && *Description)
			{
				se->SetAttribute("description", Description);
			}
			const char* sequence = stmtEnumAllKeyMacros.GetColTextUTF8(3);
			TiXmlText* stext = new TiXmlText(sequence);
			SetCDataIfNeeded(stext, sequence);
			TiXmlElement *text = new TiXmlElement("text");
			text->LinkEndChild(stext);
			se->LinkEndChild(text);
			e->LinkEndChild(se);
		}
		stmtEnumAllKeyMacros.Reset();

		root->LinkEndChild(e);

		return root;
	}

	/* *************** */
	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		size_t ErrCount=0;

		for (const TiXmlElement *e = root.FirstChild("macros").FirstChild("constants").FirstChildElement("constant").Element(); e; e=e->NextSiblingElement("constant"))
		{
			const char* name = e->Attribute("name");
			const char* type = e->Attribute("type"); // optional

			if(name && *name)
			{
				const TiXmlElement *text = e->FirstChildElement("text");
				if (text)
				{
					const char* value = text->GetText();
					SetConstValue(string(name, CP_UTF8), string(value, CP_UTF8), string(type, CP_UTF8));
				}
				else
				{
					DeleteConst(string(name, CP_UTF8));
				}
			}
			else
			{
				PrintError(L"Constant", L"<name> is empty or not found", e);
				ErrCount++;
			}
		}

		for (const TiXmlElement *e = root.FirstChild("macros").FirstChild("variables").FirstChildElement("variable").Element(); e; e=e->NextSiblingElement("variable"))
		{
			const char* name = e->Attribute("name");
			const char* type = e->Attribute("type"); // optional

			if(name && *name)
			{
				const TiXmlElement *text = e->FirstChildElement("text");
				if (text)
				{
					const char* value = text->GetText();
					SetVarValue(string(name, CP_UTF8), string(value, CP_UTF8), string(type, CP_UTF8));
				}
				else
				{
					DeleteVar(string(name, CP_UTF8));
				}
			}
			else
			{
				PrintError(L"Variable", L"<name> is empty or not found", e);
				ErrCount++;
			}
		}

		// TODO: к function вернуться, когда будет нужный функционал :-)
		for (const TiXmlElement *e = root.FirstChild("macros").FirstChild("functions").FirstChildElement("function").Element(); e; e=e->NextSiblingElement("function"))
		{
			const char* guid = e->Attribute("guid");
			const char* fname = e->Attribute("name");
			const char* flags = e->Attribute("flags");
			const char* sequence = e->Attribute("sequence");
			const char* syntax = e->Attribute("syntax");
			const char* description = e->Attribute("description");

			// BUGBUG, params can be optional
			if(guid && fname && sequence && syntax)
			{
				SetFunction(string(guid, CP_UTF8), string(fname, CP_UTF8), string(flags, CP_UTF8), string(sequence, CP_UTF8), string(syntax, CP_UTF8), string(description, CP_UTF8));
			}
		}

		for (const TiXmlElement *e = root.FirstChild("macros").FirstChild("keymacros").FirstChildElement("macro").Element(); e; e=e->NextSiblingElement("macro"))
		{
			const char* area = e->Attribute("area");
			const char* key = e->Attribute("key");
			const char* flags = e->Attribute("flags"); // optional
			const char* description = e->Attribute("description"); // optional

			if (area && *area && key && *key)
			{
				const TiXmlElement *text = e->FirstChildElement("text");
				if (text) // delete macro if sequence is absent
				{
					const char *sequence = text->GetText();
					string strFlags(flags, CP_UTF8);
					SetKeyMacro(string(area, CP_UTF8), string(key, CP_UTF8), RemoveExternalSpaces(strFlags), string(sequence, CP_UTF8), string(description, CP_UTF8));
				}
				else
				{
					DeleteKeyMacro(string(area, CP_UTF8), string(key, CP_UTF8));
				}
			}
			else
			{
				PrintError(L"Macro", L"<area> or <key> is empty or not found", e);
				ErrCount++;
			}
		}

		if (!ErrCount)
			EndTransaction();
		else
			RollbackTransaction();

		return !ErrCount;
	}
};

HierarchicalConfig *CreatePluginsConfig(const wchar_t *guid, bool Local)
{
	string strDbName = L"PluginsData\\";
	strDbName += guid;
	strDbName += L".db";
	return new HierarchicalConfigDb(strDbName, Local);
}

HierarchicalConfig *CreateFiltersConfig()
{
	return new HierarchicalConfigDb(L"filters.db");
}

HierarchicalConfig *CreateHighlightConfig()
{
	return new HighlightHierarchicalConfigDb(L"highlight.db");
}

HierarchicalConfig *CreateShortcutsConfig()
{
	return new HierarchicalConfigDb(L"shortcuts.db", true);
}

HierarchicalConfig *CreatePanelModeConfig()
{
	return new HierarchicalConfigDb(L"panelmodes.db");
}

static int nProblem = 0;
static const wchar_t* sProblem[10];

int ShowProblemDb()
{
   int rc = 0;
	if (nProblem > 0)
	{
		const wchar_t* msgs[ARRAYSIZE(sProblem)+2];
      memcpy(msgs, sProblem, nProblem*sizeof(sProblem[0]));
		msgs[nProblem] = MSG(MShowConfigFolders);
		msgs[nProblem+1] = MSG(MIgnore);
		rc = Message(MSG_WARNING, 2, MSG(MProblemDb), msgs, nProblem+2) == 0 ? +1 : -1;
	}
	return rc;
}

static void check_db( SQLiteDb *pDb, bool err_report )
{
	const wchar_t* pname = nullptr;
	int rc = pDb->InitStatus(pname, err_report);
	if ( rc > 0 )
	{
		if ( err_report )
		{
			Console.Write(L"problem\r\n  ");
			Console.Write(pname);
			pname = rc <= 1 ? L"\r\n  renamed/reopened\r\n" : L"\r\n  opened in memory\r\n";
			Console.Write(pname);
			Console.Commit();
		}
		else if ( nProblem < (int)ARRAYSIZE(sProblem) ) {
			sProblem[nProblem++] = pname;
		}
	}
}

template<class T>
T* new_db(bool err_report)
{
	T* p = new T();
	check_db(p, err_report);
	return p;
}

void InitDb( bool err_report )
{
	nProblem = 0;
	GeneralCfg = new_db<GeneralConfigDb>(err_report);
	ColorsCfg = new_db<ColorsConfigDb>(err_report);
	AssocConfig = new_db<AssociationsConfigDb>(err_report);
	PlCacheCfg = new_db<PluginsCacheConfigDb>(err_report);
	PlHotkeyCfg = new_db<PluginsHotkeysConfigDb>(err_report);
	HistoryCfg = new_db<HistoryConfigDb>(err_report);
	HistoryCfgMem = new_db<HistoryConfigMemory>(err_report);
	MacroCfg = new_db<MacroConfigDb>(err_report);
}

void ReleaseDb()
{
	nProblem = 0;
	delete MacroCfg;
	delete HistoryCfgMem;
	delete HistoryCfg;
	delete PlHotkeyCfg;
	delete PlCacheCfg;
	delete AssocConfig;
	delete ColorsCfg;
	delete GeneralCfg;
}

bool ExportImportConfig(bool Export, const wchar_t *XML)
{
	FILE* XmlFile = _wfopen(NTPath(XML), Export?L"w":L"rb");
	if(!XmlFile)
		return false;

	bool ret = false;

	int mc;
	SMatch m[2];
	RegExp re;
	re.Compile(L"/^[0-9A-F]{8}-([0-9A-F]{4}-){3}[0-9A-F]{12}$/", OP_PERLSTYLE|OP_OPTIMIZE);

	if (Export)
	{
		TiXmlDocument doc;
		doc.LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", ""));

		FormatString strVer;
		strVer << FAR_VERSION.Major << L"." << FAR_VERSION.Minor << L"." << FAR_VERSION.Build;
		char ver[50];
		strVer.GetCharString(ver, ARRAYSIZE(ver));
		TiXmlElement *root = new TiXmlElement("farconfig");
		root->SetAttribute("version", ver);

		root->LinkEndChild(GeneralCfg->Export());

		root->LinkEndChild(ColorsCfg->Export());

		root->LinkEndChild(AssocConfig->Export());

		root->LinkEndChild(PlHotkeyCfg->Export());

		HierarchicalConfig *cfg = CreateFiltersConfig();
		TiXmlElement *e = new TiXmlElement("filters");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		cfg = CreateHighlightConfig();
		e = new TiXmlElement("highlight");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		cfg = CreatePanelModeConfig();
		e = new TiXmlElement("panelmodes");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		cfg = CreateShortcutsConfig();
		e = new TiXmlElement("shortcuts");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		{ //TODO: export for local plugin settings
			string strPlugins = Opt.ProfilePath;
			strPlugins += L"\\PluginsData\\*.db";
			FAR_FIND_DATA_EX fd;
			FindFile ff(strPlugins);
			e = new TiXmlElement("pluginsconfig");
			while (ff.Get(fd))
			{
				fd.strFileName.SetLength(fd.strFileName.GetLength()-3);
				fd.strFileName.Upper();
				mc=2;
				if (re.Match(fd.strFileName, fd.strFileName.CPtr() + fd.strFileName.GetLength(), m, mc))
				{
					char guid[37];
					for (size_t i=0; i<ARRAYSIZE(guid); i++)
						guid[i] = fd.strFileName[i]&0xFF;

					TiXmlElement *plugin = new TiXmlElement("plugin");
					plugin->SetAttribute("guid", guid);
					cfg = CreatePluginsConfig(fd.strFileName, false);
					plugin->LinkEndChild(cfg->Export());
					e->LinkEndChild(plugin);
					delete cfg;
				}
			}
			root->LinkEndChild(e);
		}

		root->LinkEndChild(MacroCfg->Export());

		doc.LinkEndChild(root);
		ret = doc.SaveFile(XmlFile);
	}
	else // Import
	{
		TiXmlDocument doc;

		if (doc.LoadFile(XmlFile))
		{
			TiXmlElement *farconfig = doc.FirstChildElement("farconfig");
			if (farconfig)
			{
				const TiXmlHandle root(farconfig);

				GeneralCfg->Import(root);

				ColorsCfg->Import(root);

				AssocConfig->Import(root);

				PlHotkeyCfg->Import(root);

				HierarchicalConfig *cfg = CreateFiltersConfig();
				cfg->Import(root.FirstChildElement("filters"));
				delete cfg;

				cfg = CreateHighlightConfig();
				cfg->Import(root.FirstChildElement("highlight"));
				delete cfg;

				cfg = CreatePanelModeConfig();
				cfg->Import(root.FirstChildElement("panelmodes"));
				delete cfg;

				cfg = CreateShortcutsConfig();
				cfg->Import(root.FirstChildElement("shortcuts"));
				delete cfg;

				//TODO: import for local plugin settings
				for (TiXmlElement *plugin=root.FirstChild("pluginsconfig").FirstChildElement("plugin").Element(); plugin; plugin=plugin->NextSiblingElement("plugin"))
				{
					const char *guid = plugin->Attribute("guid");
					if (!guid)
						continue;
					string Guid(guid, CP_UTF8);
					Guid.Upper();

					mc=2;
					if (re.Match(Guid, Guid.CPtr() + Guid.GetLength(), m, mc))
					{
						cfg = CreatePluginsConfig(Guid, false);
						const TiXmlHandle h(plugin);
						cfg->Import(h);
						delete cfg;
					}
				}

				MacroCfg->Import(root);

				ret = true;
			}
		}

		if (doc.Error())
			PrintError(L"XML Error", doc);
	}

	fclose(XmlFile);
	return ret;
}

void ClearPluginsCache()
{
	PluginsCacheConfigDb *p = new PluginsCacheConfigDb();
	p->DiscardCache();
	delete p;
}
