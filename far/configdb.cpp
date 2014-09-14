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
#include "console.hpp"
#include "syslog.hpp"
#include "language.hpp"
#include "message.hpp"
#include "synchro.hpp"

static int IntToHex(int h)
{
	if (h >= 10)
		return 'A' + h - 10;
	return '0' + h;
}

static int HexToInt(int h)
{
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;

	if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;

	if (h >= '0' && h <= '9')
		return h - '0';

	return 0;
}

static char_ptr BlobToHexString(const char *Blob, int Size)
{
	char_ptr Hex(Size*2 + Size + 1);
	for (int i=0, j=0; i<Size; i++, j+=3)
	{
		Hex[j] = IntToHex((Blob[i]&0xF0) >> 4);
		Hex[j+1] = IntToHex(Blob[i]&0x0F);
		Hex[j+2] = ',';
	}
	Hex[Size ? Size*2+Size-1 : 0] = 0;
	return Hex;
}

static char_ptr HexStringToBlob(const char *Hex, int *Size)
{
	*Size=0;
	char_ptr Blob(strlen(Hex)/2 + 1);
	if (Blob)
	{
		while (*Hex && *(Hex+1))
		{
			Blob[(*Size)++] = (HexToInt(*Hex)<<4) | HexToInt(*(Hex+1));
			Hex+=2;
			if (!*Hex)
				break;
			Hex++;
		}
	}
	return Blob;
}

static const char *Int64ToHexString(unsigned __int64 X)
{
	static char Bin[16+1];
	for (int i=15; i>=0; i--, X>>=4)
		Bin[i] = IntToHex(X&0xFull);
	return Bin;
}

static const char *IntToHexString(unsigned int X)
{
	static char Bin[8+1];
	for (int i=7; i>=0; i--, X>>=4)
		Bin[i] = IntToHex(X&0xFull);
	return Bin;
}

static unsigned __int64 HexStringToInt64(const char *Hex)
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

static unsigned int HexStringToInt(const char *Hex)
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

static inline void PrintError(const wchar_t *Title, const string& Error, int Row, int Col)
{
	string strResult(Title);
	strResult += L" (" + std::to_wstring(Row) + L"," + std::to_wstring(Col) + L"): " + Error + L'\n';
	Console().Write(strResult);
	Console().Commit();
}

static void PrintError(const wchar_t *Title, const tinyxml::TiXmlDocument &doc)
{
	PrintError(Title, wide(doc.ErrorDesc(), CP_UTF8), doc.ErrorRow(), doc.ErrorCol());
}

class TiXmlElementWrapper
{
public:
	TiXmlElementWrapper(): m_data() {}
	TiXmlElementWrapper(tinyxml::TiXmlElement* rhs) { m_data = rhs; }
	TiXmlElementWrapper& operator=(const tinyxml::TiXmlElement* rhs) { m_data = rhs; return *this; }

	operator bool() const { return m_data != nullptr; }
	operator const tinyxml::TiXmlElement&() const { return *m_data; }
	const tinyxml::TiXmlElement& get() const { return *m_data; }

private:
	const tinyxml::TiXmlElement* m_data;
};

class xml_enum: NonCopyable, public enumerator<TiXmlElementWrapper>
{
public:
	xml_enum(const tinyxml::TiXmlHandle& base, const std::string& name):
		m_name(name),
		m_base(base.ToNode())
	{}

	xml_enum(const tinyxml::TiXmlNode& base, const std::string& name):
		m_name(name),
		m_base(&base)
	{}

	virtual bool get(size_t index, TiXmlElementWrapper& value) override
	{
		return index ?
			value = value.get().NextSiblingElement(m_name.data()) :
			value = m_base? m_base->FirstChildElement(m_name.data()) : nullptr;
	}

private:
	std::string m_name;
	const tinyxml::TiXmlNode* m_base;
};

template<class T>
static inline tinyxml::TiXmlElement& CreateChild(T& Parent, const char* Name)
{
	auto e = new tinyxml::TiXmlElement(Name);
	Parent.LinkEndChild(e);
	return *e;
}

class iGeneralConfigDb: public GeneralConfig, public SQLiteDb
{
protected:
	iGeneralConfigDb(const wchar_t* DbName, bool local)
	{
		Initialize(DbName, local);
	}

public:
	virtual ~iGeneralConfigDb() {};

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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

	bool SetValue(const string& Key, const string& Name, const string& Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const string& Key, const string& Name, unsigned __int64 Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const string& Key, const string& Name, const void *Value, size_t Size)
	{
		bool b = stmtUpdateValue.Bind(Value,Size).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value,Size).StepAndReset();
		return b;
	}

	bool GetValue(const string& Key, const string& Name, unsigned __int64 *Value)
	{
		bool b = stmtGetValue.Bind(Key).Bind(Name).Step();
		if (b)
		{
			if (stmtGetValue.GetColType(0) == TYPE_INTEGER)
			{
				*Value = stmtGetValue.GetColInt64(0);
			}
			else
			{
				// TODO: log
			}
		}
		stmtGetValue.Reset();
		return b;
	}

	bool GetValue(const string& Key, const string& Name, string &strValue)
	{
		bool b = stmtGetValue.Bind(Key).Bind(Name).Step();
		if (b)
		{
			if (stmtGetValue.GetColType(0) == TYPE_STRING)
			{
				strValue = stmtGetValue.GetColText(0);
			}
			else
			{
				// TODO: log
			}
		}
		stmtGetValue.Reset();
		return b;
	}

	int GetValue(const string& Key, const string& Name, void *Value, size_t Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Key).Bind(Name).Step())
		{
			if (stmtGetValue.GetColType(0) == TYPE_BLOB)
			{
				const char *blob = stmtGetValue.GetColBlob(0);
				realsize = stmtGetValue.GetColBytes(0);
				if (Value)
					memcpy(Value, blob, std::min(realsize, static_cast<int>(Size)));
			}
			else
			{
				// TODO: log
			}
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool GetValue(const string& Key, const string& Name, long long *Value, long long Default)
	{
		unsigned __int64 v = 0;
		if (GetValue(Key,Name,&v))
		{
			*Value = v;
			return true;
		}
		*Value = Default;
		return false;
	}

	bool GetValue(const string& Key, const string& Name, string &strValue, const wchar_t *Default)
	{
		if (GetValue(Key,Name,strValue))
			return true;
		strValue=Default;
		return false;
	}

	int GetValue(const string& Key, const string& Name, void *Value, size_t Size, const void *Default)
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

	bool DeleteValue(const string& Key, const string& Name)
	{
		return stmtDelValue.Bind(Key).Bind(Name).StepAndReset();
	}

	bool EnumValues(const string& Key, DWORD Index, string &Name, string &Value)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Key,false);

		if (stmtEnumValues.Step())
		{
			Name = stmtEnumValues.GetColText(0);
			Value = stmtEnumValues.GetColText(1);
			return true;
		}

		stmtEnumValues.Reset();
		return false;
	}

	bool EnumValues(const string& Key, DWORD Index, string &Name, DWORD& Value)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Key,false);

		if (stmtEnumValues.Step())
		{
			Name = stmtEnumValues.GetColText(0);
			Value = (DWORD)stmtEnumValues.GetColInt(1);
			return true;
		}

		stmtEnumValues.Reset();
		return false;
	}

	virtual void Export(tinyxml::TiXmlElement& Parent) override
	{
		auto& root = CreateChild(Parent, GetKeyName());

		SQLiteStmt stmtEnumAllValues;
		InitStmt(stmtEnumAllValues, L"SELECT key, name, value FROM general_config ORDER BY key, name;");

		while (stmtEnumAllValues.Step())
		{
			auto& e = CreateChild(root, "setting");

			e.SetAttribute("key", stmtEnumAllValues.GetColTextUTF8(0));
			e.SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(1));

			switch (stmtEnumAllValues.GetColType(2))
			{
				case TYPE_INTEGER:
					e.SetAttribute("type", "qword");
					e.SetAttribute("value", Int64ToHexString(stmtEnumAllValues.GetColInt64(2)));
					break;
				case TYPE_STRING:
					e.SetAttribute("type", "text");
					e.SetAttribute("value", stmtEnumAllValues.GetColTextUTF8(2));
					break;
				default:
				{
					auto hex = BlobToHexString(stmtEnumAllValues.GetColBlob(2),stmtEnumAllValues.GetColBytes(2));
					e.SetAttribute("type", "hex");
					e.SetAttribute("value", hex.get());
				}
			}
		}

		stmtEnumAllValues.Reset();
	}

	virtual void Import(const tinyxml::TiXmlHandle &root) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		FOR(const tinyxml::TiXmlElement& e, xml_enum(root.FirstChild(GetKeyName()), "setting"))
		{
			const char *key = e.Attribute("key");
			const char *name = e.Attribute("name");
			const char *type = e.Attribute("type");
			const char *value = e.Attribute("value");

			if (!key || !name || !type || !value)
				continue;

			string Key = wide(key, CP_UTF8);
			string Name = wide(name, CP_UTF8);

			if (!strcmp(type,"qword"))
			{
				SetValue(Key, Name, HexStringToInt64(value));
			}
			else if (!strcmp(type,"text"))
			{
				string Value = wide(value, CP_UTF8);
				SetValue(Key, Name, Value);
			}
			else if (!strcmp(type,"hex"))
			{
				int Size = 0;
				auto Blob = HexStringToBlob(value, &Size);
				if (Blob)
				{
					SetValue(Key, Name, Blob.get(), Size);
				}
			}
			else
			{
				continue;
			}
		}
	}

private:
	virtual const char* GetKeyName() const = 0;

	SQLiteStmt stmtUpdateValue;
	SQLiteStmt stmtInsertValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtDelValue;
	SQLiteStmt stmtEnumValues;
};

class GeneralConfigDb: public iGeneralConfigDb
{
public:
	GeneralConfigDb():iGeneralConfigDb(L"generalconfig.db", false) {}

private:
	virtual const char* GetKeyName() const override {return "generalconfig";}
};

class LocalGeneralConfigDb: public iGeneralConfigDb
{
public:
	LocalGeneralConfigDb():iGeneralConfigDb(L"localconfig.db", true) {}

private:
	virtual const char* GetKeyName() const override {return "localconfig";}
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

	Event AsyncDone;

	void AsyncDelete()
	{
		delete this;
	}

protected:
	HierarchicalConfigDb() {}

	virtual ~HierarchicalConfigDb() { EndTransaction(); AsyncDone.Set(); }

public:

	explicit HierarchicalConfigDb(const string& DbName, bool Local = false)
	{
		AsyncDone.SetName(strPath, m_Name);
		AsyncDone.Open(true,true); // If a thread with same event name is running, we will open that event here
		AsyncDone.Wait();          // and wait for the signal
		Initialize(DbName, Local);
	}

	void AsyncFinish()
	{
		AsyncDone.Reset();
		Global->Db->AddThread(Thread(&HierarchicalConfigDb::AsyncDelete, this));
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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

	bool Flush()
	{
		bool b = EndTransaction();
		BeginTransaction();
		return b;
	}

	unsigned __int64 CreateKey(unsigned __int64 Root, const string& Name, const string* Description)
	{
		if (stmtCreateKey.Bind(Root).Bind(Name).Bind(Description? *Description : string()).StepAndReset())
			return LastInsertRowID();
		unsigned __int64 id = GetKeyID(Root,Name);
		if (id && Description)
			SetKeyDescription(id,Description? *Description : string());
		return id;
	}

	unsigned __int64 GetKeyID(unsigned __int64 Root, const string& Name)
	{
		unsigned __int64 id = 0;
		if (stmtFindKey.Bind(Root).Bind(Name).Step())
			id = stmtFindKey.GetColInt64(0);
		stmtFindKey.Reset();
		return id;
	}

	bool SetKeyDescription(unsigned __int64 Root, const string& Description)
	{
		return stmtSetKeyDescription.Bind(Description).Bind(Root).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const string& Name, const string& Value)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const string& Name, unsigned __int64 Value)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const string& Name, const void *Value, size_t Size)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value,Size).StepAndReset();
	}

	bool GetValue(unsigned __int64 Root, const string& Name, unsigned __int64 *Value)
	{
		bool b = stmtGetValue.Bind(Root).Bind(Name).Step();
		if (b)
			*Value = stmtGetValue.GetColInt64(0);
		stmtGetValue.Reset();
		return b;
	}

	bool GetValue(unsigned __int64 Root, const string& Name, string &strValue)
	{
		bool b = stmtGetValue.Bind(Root).Bind(Name).Step();
		if (b)
			strValue = stmtGetValue.GetColText(0);
		stmtGetValue.Reset();
		return b;
	}

	int GetValue(unsigned __int64 Root, const string& Name, void *Value, size_t Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Root).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,std::min(realsize,static_cast<int>(Size)));
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool DeleteKeyTree(unsigned __int64 KeyID)
	{
		//All subtree is automatically deleted because of foreign key constraints
		return stmtDeleteTree.Bind(KeyID).StepAndReset();
	}

	bool DeleteValue(unsigned __int64 Root, const string& Name)
	{
		return stmtDelValue.Bind(Root).Bind(Name).StepAndReset();
	}

	bool EnumKeys(unsigned __int64 Root, DWORD Index, string& Name)
	{
		if (Index == 0)
			stmtEnumKeys.Reset().Bind(Root);

		if (stmtEnumKeys.Step())
		{
			Name = stmtEnumKeys.GetColText(0);
			return true;
		}

		stmtEnumKeys.Reset();
		return false;
	}

	bool EnumValues(unsigned __int64 Root, DWORD Index, string& Name, DWORD *Type)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Root);

		if (stmtEnumValues.Step())
		{
			Name = stmtEnumValues.GetColText(0);
			*Type = stmtEnumValues.GetColType(1);
			return true;
		}

		stmtEnumValues.Reset();
		return false;

	}

	virtual void SerializeBlob(const char* Name, const char* Blob, int Size, tinyxml::TiXmlElement& e)
	{
			auto hex = BlobToHexString(Blob, Size);
			e.SetAttribute("type", "hex");
			e.SetAttribute("value", hex.get());
	}

	void Export(unsigned __int64 id, tinyxml::TiXmlElement& key)
	{
		stmtEnumValues.Bind(id);
		while (stmtEnumValues.Step())
		{
			auto& e = CreateChild(key, "value");

			const char* name = stmtEnumValues.GetColTextUTF8(0);
			e.SetAttribute("name", name);

			switch (stmtEnumValues.GetColType(1))
			{
				case TYPE_INTEGER:
					e.SetAttribute("type", "qword");
					e.SetAttribute("value", Int64ToHexString(stmtEnumValues.GetColInt64(1)));
					break;
				case TYPE_STRING:
					e.SetAttribute("type", "text");
					e.SetAttribute("value", stmtEnumValues.GetColTextUTF8(1));
					break;
				default:
					SerializeBlob(name, stmtEnumValues.GetColBlob(1), stmtEnumValues.GetColBytes(1), e);
			}
		}
		stmtEnumValues.Reset();

		SQLiteStmt stmtEnumSubKeys;
		InitStmt(stmtEnumSubKeys, L"SELECT id, name, description FROM table_keys WHERE parent_id=?1 AND id<>0;");

		stmtEnumSubKeys.Bind(id);
		while (stmtEnumSubKeys.Step())
		{
			auto& e = CreateChild(key, "key");

			e.SetAttribute("name", stmtEnumSubKeys.GetColTextUTF8(1));
			const char *description = stmtEnumSubKeys.GetColTextUTF8(2);
			if (description)
				e.SetAttribute("description", description);

			Export(stmtEnumSubKeys.GetColInt64(0), e);
		}
		stmtEnumSubKeys.Reset();
	}

	virtual void Export(tinyxml::TiXmlElement& Parent) override
	{
		Export(0, CreateChild(Parent, "hierarchicalconfig"));
	}

	virtual int DeserializeBlob(const char* Name, const char* Type, const char* Value, const tinyxml::TiXmlElement& e, char_ptr& Blob)
	{
		int Size = 0;
		Blob = HexStringToBlob(Value, &Size);
		return Size;
	}

	void Import(unsigned __int64 root, const tinyxml::TiXmlElement& key)
	{
		unsigned __int64 id;
		{
			const char *name = key.Attribute("name");
			const char *description = key.Attribute("description");
			if (!name)
				return;

			string Name = wide(name, CP_UTF8);
			string Description = wide(description, CP_UTF8);
			id = CreateKey(root, Name, &Description);
			if (!id)
				return;
		}

		FOR(const tinyxml::TiXmlElement& e, xml_enum(key, "value"))
		{
			const char *name = e.Attribute("name");
			const char *type = e.Attribute("type");
			const char *value = e.Attribute("value");

			if (!name || !type)
				continue;

			string Name = wide(name, CP_UTF8);

			if (value && !strcmp(type,"qword"))
			{
				SetValue(id, Name, HexStringToInt64(value));
			}
			else if (value && !strcmp(type,"text"))
			{
				string Value = wide(value, CP_UTF8);
				SetValue(id, Name, Value);
			}
			else if (value && !strcmp(type,"hex"))
			{
				int Size = 0;
				auto Blob = HexStringToBlob(value, &Size);
				if (Blob)
				{
					SetValue(id, Name, Blob.get(), Size);
				}
			}
			else
			{
				// custom types, value is optional
				char_ptr Blob;
				int Size = DeserializeBlob(name, type, value, e, Blob);
				if (Blob)
				{
					SetValue(id, Name, Blob.get(), Size);
				}
			}
		}

		FOR(const auto& e, xml_enum(key, "key"))
		{
			Import(id, e);
		}

	}

	virtual void Import(const tinyxml::TiXmlHandle &root) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		FOR(const auto& e, xml_enum(root.FirstChild("hierarchicalconfig"), "key"))
		{
			Import(0, e);
		}
	}
};

static const simple_pair<FARCOLORFLAGS, const wchar_t*> ColorFlagNames[] =
{
	{FCF_FG_4BIT,      L"fg4bit"   },
	{FCF_BG_4BIT,      L"bg4bit"   },
	{FCF_FG_BOLD,      L"bold"     },
	{FCF_FG_ITALIC,    L"italic"   },
	{FCF_FG_UNDERLINE, L"underline"},
};

class HighlightHierarchicalConfigDb: public HierarchicalConfigDb
{
public:
	explicit HighlightHierarchicalConfigDb(const string& DbName, bool Local = false):HierarchicalConfigDb(DbName, Local) {}

private:
	HighlightHierarchicalConfigDb();
	virtual ~HighlightHierarchicalConfigDb() {}

	virtual void SerializeBlob(const char* Name, const char* Blob, int Size, tinyxml::TiXmlElement& e) override
	{
		static const char* ColorKeys[] =
		{
			"NormalColor", "SelectedColor",
			"CursorColor", "SelectedCursorColor",
			"MarkCharNormalColor", "MarkCharSelectedColor",
			"MarkCharCursorColor", "MarkCharSelectedCursorColor",
		};

		if (std::any_of(CONST_RANGE(ColorKeys, i) { return !strcmp(Name, i); }))
		{
			auto Color = reinterpret_cast<const FarColor*>(Blob);
			e.SetAttribute("type", "color");
			e.SetAttribute("background", IntToHexString(Color->BackgroundColor));
			e.SetAttribute("foreground", IntToHexString(Color->ForegroundColor));
			e.SetAttribute("flags", Utf8String(FlagsToString(Color->Flags, ColorFlagNames)).data());
		}
		else
		{
			return HierarchicalConfigDb::SerializeBlob(Name, Blob, Size, e);
		}
	}

	virtual int DeserializeBlob(const char* Name, const char* Type, const char* Value, const tinyxml::TiXmlElement& e, char_ptr& Blob) override
	{
		int Result = 0;
		if(!strcmp(Type, "color"))
		{
			const char *background = e.Attribute("background");
			const char *foreground = e.Attribute("foreground");
			const char *flags = e.Attribute("flags");

			if(background && foreground && flags)
			{
				Result = sizeof(FarColor);
				Blob.reset(Result, true);
				auto Color = reinterpret_cast<FarColor*>(Blob.get());
				Color->BackgroundColor = HexStringToInt(background);
				Color->ForegroundColor = HexStringToInt(foreground);
				Color->Flags = StringToFlags(wide(flags, CP_UTF8), ColorFlagNames);
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

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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

	bool SetValue(const string& Name, const FarColor& Value)
	{
		bool b = stmtUpdateValue.Bind(&Value, sizeof(Value)).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Name).Bind(&Value, sizeof(Value)).StepAndReset();
		return b;
	}

	bool GetValue(const string& Name, FarColor& Value)
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

	bool DeleteValue(const string& Name)
	{
		return stmtDelValue.Bind(Name).StepAndReset();
	}

	virtual void Export(tinyxml::TiXmlElement& Parent) override
	{
		auto& root = CreateChild(Parent, "colors");

		SQLiteStmt stmtEnumAllValues;
		InitStmt(stmtEnumAllValues, L"SELECT name, value FROM colors ORDER BY name;");

		while (stmtEnumAllValues.Step())
		{
			auto& e = CreateChild(root, "object");

			e.SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(0));
			auto Color = reinterpret_cast<const FarColor*>(stmtEnumAllValues.GetColBlob(1));
			e.SetAttribute("background", IntToHexString(Color->BackgroundColor));
			e.SetAttribute("foreground", IntToHexString(Color->ForegroundColor));
			e.SetAttribute("flags", Utf8String(FlagsToString(Color->Flags, ColorFlagNames)).data());
		}

		stmtEnumAllValues.Reset();
	}

	virtual void Import(const tinyxml::TiXmlHandle &root) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		FOR(const tinyxml::TiXmlElement& e, xml_enum(root.FirstChild("colors"), "object"))
		{
			const char *name = e.Attribute("name");
			const char *background = e.Attribute("background");
			const char *foreground = e.Attribute("foreground");
			const char *flags = e.Attribute("flags");

			if (!name)
				continue;

			string Name = wide(name, CP_UTF8);

			if(background && foreground && flags)
			{
				FarColor Color = {};
				Color.BackgroundColor = HexStringToInt(background);
				Color.ForegroundColor = HexStringToInt(foreground);
				Color.Flags = StringToFlags(wide(flags, CP_UTF8), ColorFlagNames);
				SetValue(Name, Color);
			}
			else
			{
				DeleteValue(Name);
			}
		}
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

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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
				*Enabled = stmtGetCommand.GetColInt(1) != 0;
		}
		stmtGetCommand.Reset();
		return b;
	}

	bool SetCommand(unsigned __int64 id, int Type, const string& Command, bool Enabled)
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

	unsigned __int64 AddType(unsigned __int64 after_id, const string& Mask, const string& Description)
	{
		if (stmtReorder.Bind(after_id).StepAndReset() && stmtAddType.Bind(after_id).Bind(Mask).Bind(Description).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	bool UpdateType(unsigned __int64 id, const string& Mask, const string& Description)
	{
		return stmtUpdateType.Bind(Mask).Bind(Description).Bind(id).StepAndReset();
	}

	bool DelType(unsigned __int64 id)
	{
		return stmtDelType.Bind(id).StepAndReset();
	}

	virtual void Export(tinyxml::TiXmlElement& Parent) override
	{
		auto& root = CreateChild(Parent, "associations");

		SQLiteStmt stmtEnumAllTypes;
		InitStmt(stmtEnumAllTypes, L"SELECT id, mask, description FROM filetypes ORDER BY weight;");
		SQLiteStmt stmtEnumCommandsPerFiletype;
		InitStmt(stmtEnumCommandsPerFiletype, L"SELECT type, enabled, command FROM commands WHERE ft_id=?1 ORDER BY type;");

		while (stmtEnumAllTypes.Step())
		{
			auto& e = CreateChild(root, "filetype");

			e.SetAttribute("mask", stmtEnumAllTypes.GetColTextUTF8(1));
			e.SetAttribute("description", stmtEnumAllTypes.GetColTextUTF8(2));

			stmtEnumCommandsPerFiletype.Bind(stmtEnumAllTypes.GetColInt64(0));
			while (stmtEnumCommandsPerFiletype.Step())
			{
				auto& se = CreateChild(e, "command");

				se.SetAttribute("type", stmtEnumCommandsPerFiletype.GetColInt(0));
				se.SetAttribute("enabled", stmtEnumCommandsPerFiletype.GetColInt(1));
				se.SetAttribute("command", stmtEnumCommandsPerFiletype.GetColTextUTF8(2));
			}
			stmtEnumCommandsPerFiletype.Reset();
		}

		stmtEnumAllTypes.Reset();
	}

	virtual void Import(const tinyxml::TiXmlHandle &root) override
	{
		const auto base = root.FirstChild("associations");
		if (!base.ToElement())
			return;

		SCOPED_ACTION(auto)(ScopedTransaction());
		Exec("DELETE FROM filetypes;"); //delete all before importing
		unsigned __int64 id = 0;
		FOR(const tinyxml::TiXmlElement& e, xml_enum(base, "filetype"))
		{
			const char *mask = e.Attribute("mask");
			const char *description = e.Attribute("description");

			if (!mask)
				continue;

			string Mask = wide(mask, CP_UTF8);
			string Description = wide(description, CP_UTF8);

			id = AddType(id, Mask, Description);
			if (!id)
				continue;

			FOR(const tinyxml::TiXmlElement& se, xml_enum(e, "command"))
			{
				const char *command = se.Attribute("command");
				int type=0;
				const char *stype = se.Attribute("type", &type);
				int enabled=0;
				const char *senabled = se.Attribute("enabled", &enabled);

				if (!command || !stype || !senabled)
					continue;

				string Command = wide(command, CP_UTF8);
				SetCommand(id, type, Command, enabled != 0);
			}

		}
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

	bool SetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, const string& Text, const string& Guid)
	{
		return stmtSetMenuItem.Bind(id).Bind((int)type).Bind(index).Bind(Guid).Bind(Text).StepAndReset();
	}

	static string GetTextFromID(SQLiteStmt &stmt, unsigned __int64 id)
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
		Initialize(namedb, true);
	}

	bool BeginTransaction() { return SQLiteDb::BeginTransaction(); }
	bool EndTransaction() { return SQLiteDb::EndTransaction(); }
	bool RollbackTransaction() { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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

	virtual void Import(const tinyxml::TiXmlHandle&) override {}
	virtual void Export(tinyxml::TiXmlElement&) override {}

	unsigned __int64 CreateCache(const string& CacheName)
	{
		if (stmtCreateCache.Bind(CacheName).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetCacheID(const string& CacheName)
	{
		unsigned __int64 id = 0;
		if (stmtFindCacheName.Bind(CacheName).Step())
			id = stmtFindCacheName.GetColInt64(0);
		stmtFindCacheName.Reset();
		return id;
	}

	bool DeleteCache(const string& CacheName)
	{
		//All related entries are automatically deleted because of foreign key constraints
		return stmtDelCache.Bind(CacheName).StepAndReset();
	}

	bool IsPreload(unsigned __int64 id)
	{
		bool preload = false;
		if (stmtGetPreloadState.Bind(id).Step())
			preload = stmtGetPreloadState.GetColInt(0) != 0;
		stmtGetPreloadState.Reset();
		return preload;
	}

	string GetSignature(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetSignature, id);
	}

	void *GetExport(unsigned __int64 id, const string& ExportName)
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
			memcpy(Version,blob,std::min(realsize,(int)sizeof(VersionInfo)));
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
			memcpy(Version,blob,std::min(realsize,(int)sizeof(VersionInfo)));
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

	bool SetSignature(unsigned __int64 id, const string& Signature)
	{
		return stmtSetSignature.Bind(id).Bind(Signature).StepAndReset();
	}

	bool SetDiskMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid)
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid)
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid)
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	bool SetCommandPrefix(unsigned __int64 id, const string& Prefix)
	{
		return stmtSetPrefix.Bind(id).Bind(Prefix).StepAndReset();
	}

	bool SetFlags(unsigned __int64 id, unsigned __int64 Flags)
	{
		return stmtSetFlags.Bind(id).Bind(Flags).StepAndReset();
	}

	bool SetExport(unsigned __int64 id, const string& ExportName, bool Exists)
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

	bool SetGuid(unsigned __int64 id, const string& Guid)
	{
		return stmtSetGuid.Bind(id).Bind(Guid).StepAndReset();
	}

	bool SetTitle(unsigned __int64 id, const string& Title)
	{
		return stmtSetTitle.Bind(id).Bind(Title).StepAndReset();
	}

	bool SetAuthor(unsigned __int64 id, const string& Author)
	{
		return stmtSetAuthor.Bind(id).Bind(Author).StepAndReset();
	}

	bool SetDescription(unsigned __int64 id, const string& Description)
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
		SCOPED_ACTION(auto)(ScopedTransaction());
		bool ret = Exec("DELETE FROM cachename");
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

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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

	string GetHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType)
	{
		string strHotKey;
		if (stmtGetHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).Step())
			strHotKey = stmtGetHotkey.GetColText(0);
		stmtGetHotkey.Reset();
		return strHotKey;
	}

	bool SetHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType, const string& HotKey)
	{
		return stmtSetHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).Bind(HotKey).StepAndReset();
	}

	bool DelHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType)
	{
		return stmtDelHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).StepAndReset();
	}

	virtual void Export(tinyxml::TiXmlElement& Parent) override
	{
		auto& root = CreateChild(Parent, "pluginhotkeys");

		SQLiteStmt stmtEnumAllPluginKeys;
		InitStmt(stmtEnumAllPluginKeys, L"SELECT pluginkey FROM pluginhotkeys GROUP BY pluginkey;");
		SQLiteStmt stmtEnumAllHotkeysPerKey;
		InitStmt(stmtEnumAllHotkeysPerKey, L"SELECT menuguid, type, hotkey FROM pluginhotkeys WHERE pluginkey=$1;");

		while (stmtEnumAllPluginKeys.Step())
		{
			auto& p = CreateChild(root, "plugin");

			string Key = stmtEnumAllPluginKeys.GetColText(0);
			p.SetAttribute("key", stmtEnumAllPluginKeys.GetColTextUTF8(0));

			stmtEnumAllHotkeysPerKey.Bind(Key);
			while (stmtEnumAllHotkeysPerKey.Step())
			{
				auto& e = CreateChild(p, "hotkey");

				const char *type;
				switch (stmtEnumAllHotkeysPerKey.GetColInt(1))
				{
					case DRIVE_MENU: type = "drive"; break;
					case CONFIG_MENU: type = "config"; break;
					default: type = "plugins";
				}
				e.SetAttribute("menu", type);
				e.SetAttribute("guid", stmtEnumAllHotkeysPerKey.GetColTextUTF8(0));
				const char *hotkey = stmtEnumAllHotkeysPerKey.GetColTextUTF8(2);
				e.SetAttribute("hotkey", NullToEmpty(hotkey));
			}
			stmtEnumAllHotkeysPerKey.Reset();
		}

		stmtEnumAllPluginKeys.Reset();
	}

	virtual void Import(const tinyxml::TiXmlHandle &root) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		FOR(const tinyxml::TiXmlElement& e, xml_enum(root.FirstChild("pluginhotkeys"), "plugin"))
		{
			const char *key = e.Attribute("key");

			if (!key)
				continue;

			string Key = wide(key, CP_UTF8);

			FOR(const tinyxml::TiXmlElement& se, xml_enum(e, "hotkey"))
			{
				const char *stype = se.Attribute("menu");
				const char *guid = se.Attribute("guid");
				const char *hotkey = se.Attribute("hotkey");

				if (!guid || !stype)
					continue;

				string Guid = wide(guid, CP_UTF8);
				string Hotkey = wide(hotkey, CP_UTF8);
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

	static inline uint64_t DaysToUI64(int Days)
	{
		return Days * 24ull * 60ull * 60ull * 10000000ull;
	}

	Thread WorkThread;
	Event StopEvent;
	Event AsyncDeleteAddDone;
	Event AsyncCommitDone;
	Event AsyncWork;
	MultiWaiter AllWaiter;

	struct AsyncWorkItem
	{
		unsigned __int64 DeleteId;
		DWORD TypeHistory;
		string HistoryName;
		string strName;
		int Type;
		bool Lock;
		string strGuid;
		string strFile;
		string strData;
	};

	SyncedQueue<std::unique_ptr<AsyncWorkItem>> WorkQueue;

	void WaitAllAsync() const { AllWaiter.Wait(); }
	void WaitCommitAsync() const { AsyncCommitDone.Wait(); }

	void StartThread()
	{
		StopEvent.Open();
		if (strPath != L":memory:")
		{
			AsyncDeleteAddDone.SetName(strPath, m_Name);
			AsyncCommitDone.SetName(strPath, m_Name);
		}
		AsyncDeleteAddDone.Open(true,true);
		AsyncCommitDone.Open(true,true);
		AllWaiter.Add(AsyncDeleteAddDone);
		AllWaiter.Add(AsyncCommitDone);
		AsyncWork.Open();
		WorkThread = Thread(&HistoryConfigCustom::ThreadProc, this);
	}

	void StopThread()
	{
		WaitAllAsync();
		StopEvent.Set();
		if (WorkThread.joinable())
		{
			WorkThread.join();
		}
	}

	void ThreadProc()
	{
		MultiWaiter Waiter;
		Waiter.Add(AsyncWork);
		Waiter.Add(StopEvent);

		while (true)
		{
			DWORD wait = Waiter.Wait(MultiWaiter::wait_any);

			if (wait != WAIT_OBJECT_0)
				break;

			bool bAddDelete=false, bCommit=false;

			decltype(WorkQueue)::value_type item;
			while (WorkQueue.PopIfNotEmpty(item))
			{
				if (item) //DeleteAndAddAsync
				{
					SQLiteDb::BeginTransaction();
					if (item->DeleteId)
						DeleteInternal(item->DeleteId);
					AddInternal(item->TypeHistory,item->HistoryName,item->strName,item->Type,item->Lock,item->strGuid,item->strFile,item->strData);
					SQLiteDb::EndTransaction();
					bAddDelete = true;
				}
				else // EndTransaction
				{
					SQLiteDb::EndTransaction();
					bCommit = true;
				}
			}

			if (bAddDelete)
				AsyncDeleteAddDone.Set();
			if (bCommit)
				AsyncCommitDone.Set();
		}
	}

	bool AddInternal(DWORD TypeHistory, const string& HistoryName, const string &Name, int Type, bool Lock, const string &strGuid, const string &strFile, const string &strData)
	{
		return stmtAdd.Bind((int) TypeHistory).Bind(HistoryName).Bind(Type).Bind(Lock ? 1 : 0).Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(strGuid).Bind(strFile).Bind(strData).StepAndReset();
	}

	bool DeleteInternal(unsigned __int64 id)
	{
		return stmtDel.Bind(id).StepAndReset();
	}

public:

	bool BeginTransaction() { WaitAllAsync(); return SQLiteDb::BeginTransaction(); }

	bool EndTransaction()
	{
		WorkQueue.Push(nullptr);
		WaitAllAsync();
		AsyncCommitDone.Reset();
		AsyncWork.Set();
		return true;
	}

	bool RollbackTransaction() { WaitAllAsync(); return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
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
		) { StartThread(); return true; }

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

	virtual ~HistoryConfigCustom() { StopThread(); }

	bool Delete(unsigned __int64 id)
	{
		WaitAllAsync();
		return DeleteInternal(id);
	}

	bool Enum(DWORD index, DWORD TypeHistory, const string& HistoryName, unsigned __int64 *id, string &Name, history_record_type* Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false)
	{
		WaitAllAsync();
		SQLiteStmt &stmt = Reverse ? stmtEnumDesc : stmtEnum;

		if (index == 0)
			stmt.Reset().Bind((int)TypeHistory).Bind(HistoryName,false);

		if (stmt.Step())
		{
			*id = stmt.GetColInt64(0);
			Name = stmt.GetColText(1);
			*Type = static_cast<history_record_type>(stmt.GetColInt(2));
			*Lock = stmt.GetColInt(3) != 0;
			*Time = stmt.GetColInt64(4);
			strGuid = stmt.GetColText(5);
			strFile = stmt.GetColText(6);
			strData = stmt.GetColText(7);
			return true;
		}

		stmt.Reset();
		return false;
	}

	bool DeleteAndAddAsync(unsigned __int64 DeleteId, DWORD TypeHistory, const string& HistoryName, string Name, int Type, bool Lock, string &strGuid, string &strFile, string &strData)
	{
		auto item = std::make_unique<AsyncWorkItem>();
		item->DeleteId=DeleteId;
		item->TypeHistory=TypeHistory;
		item->HistoryName=HistoryName;
		item->strName=Name;
		item->Type=Type;
		item->Lock=Lock;
		item->strGuid=strGuid;
		item->strFile=strFile;
		item->strData=strData;

		WorkQueue.Push(std::move(item));

		WaitAllAsync();
		AsyncDeleteAddDone.Reset();
		AsyncWork.Set();
		return true;
	}

	bool DeleteOldUnlocked(DWORD TypeHistory, const string& HistoryName, int DaysToKeep, int MinimumEntries)
	{
		WaitAllAsync();
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= DaysToUI64(DaysToKeep);
		return stmtDeleteOldUnlocked.Bind((int)TypeHistory).Bind(HistoryName).Bind(older).Bind(MinimumEntries).StepAndReset();
	}

	bool EnumLargeHistories(DWORD index, int MinimumEntries, DWORD TypeHistory, string &strHistoryName)
	{
		WaitAllAsync();
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

	bool GetNewest(DWORD TypeHistory, const string& HistoryName, string& Name)
	{
		WaitAllAsync();
		bool b = stmtGetNewestName.Bind((int)TypeHistory).Bind(HistoryName).Step();
		if (b)
		{
			Name = stmtGetNewestName.GetColText(0);
		}
		stmtGetNewestName.Reset();
		return b;
	}

	bool Get(unsigned __int64 id, string& Name)
	{
		WaitAllAsync();
		bool b = stmtGetName.Bind(id).Step();
		if (b)
		{
			Name = stmtGetName.GetColText(0);
		}
		stmtGetName.Reset();
		return b;
	}

	bool Get(unsigned __int64 id, string& Name, history_record_type* Type, string &strGuid, string &strFile, string &strData)
	{
		WaitAllAsync();
		bool b = stmtGetNameAndType.Bind(id).Step();
		if (b)
		{
			Name = stmtGetNameAndType.GetColText(0);
			*Type = static_cast<history_record_type>(stmtGetNameAndType.GetColInt(1));
			strGuid = stmtGetNameAndType.GetColText(2);
			strFile = stmtGetNameAndType.GetColText(3);
			strData = stmtGetNameAndType.GetColText(4);
		}
		stmtGetNameAndType.Reset();
		return b;
	}

	DWORD Count(DWORD TypeHistory, const string& HistoryName)
	{
		WaitAllAsync();
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
		WaitAllAsync();
		return stmtSetLock.Bind(IsLocked(id)?0:1).Bind(id).StepAndReset();
	}

	bool IsLocked(unsigned __int64 id)
	{
		WaitAllAsync();
		bool l = false;
		if (stmtGetLock.Bind(id).Step())
		{
			 l = stmtGetLock.GetColInt(0) != 0;
		}
		stmtGetLock.Reset();
		return l;
	}

	bool DeleteAllUnlocked(DWORD TypeHistory, const string& HistoryName)
	{
		WaitAllAsync();
		return stmtDelUnlocked.Bind((int)TypeHistory).Bind(HistoryName).StepAndReset();
	}

	unsigned __int64 GetNext(DWORD TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name)
	{
		WaitAllAsync();
		Name.clear();
		unsigned __int64 nid = 0;
		if (!id)
			return nid;
		if (stmtGetNext.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetNext.GetColInt64(0);
			Name = stmtGetNext.GetColText(1);
		}
		stmtGetNext.Reset();
		return nid;
	}

	unsigned __int64 GetPrev(DWORD TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name)
	{
		WaitAllAsync();
		Name.clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (stmtGetNewest.Bind((int)TypeHistory).Bind(HistoryName).Step())
			{
				nid = stmtGetNewest.GetColInt64(0);
				Name = stmtGetNewest.GetColText(1);
			}
			stmtGetNewest.Reset();
			return nid;
		}
		if (stmtGetPrev.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetPrev.GetColInt64(0);
			Name = stmtGetPrev.GetColText(1);
		}
		else if (Get(id, Name))
		{
			nid = id;
		}
		stmtGetPrev.Reset();
		return nid;
	}

	unsigned __int64 CyclicGetPrev(DWORD TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name)
	{
		WaitAllAsync();
		Name.clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (stmtGetNewest.Bind((int)TypeHistory).Bind(HistoryName).Step())
			{
				nid = stmtGetNewest.GetColInt64(0);
				Name = stmtGetNewest.GetColText(1);
			}
			stmtGetNewest.Reset();
			return nid;
		}
		if (stmtGetPrev.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetPrev.GetColInt64(0);
			Name = stmtGetPrev.GetColText(1);
		}
		stmtGetPrev.Reset();
		return nid;
	}

	unsigned __int64 SetEditorPos(const string& Name, int Line, int LinePos, int ScreenLine, int LeftPos, uintptr_t CodePage)
	{
		WaitCommitAsync();
		if (stmtSetEditorPos.Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(Line).Bind(LinePos).Bind(ScreenLine).Bind(LeftPos).Bind((int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetEditorPos(const string& Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, uintptr_t *CodePage)
	{
		WaitCommitAsync();
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

	bool SetEditorBookmark(unsigned __int64 id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos)
	{
		WaitCommitAsync();
		return stmtSetEditorBookmark.Bind(id).Bind(i).Bind(Line).Bind(LinePos).Bind(ScreenLine).Bind(LeftPos).StepAndReset();
	}

	bool GetEditorBookmark(unsigned __int64 id, size_t i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos)
	{
		WaitCommitAsync();
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

	unsigned __int64 SetViewerPos(const string& Name, __int64 FilePos, __int64 LeftPos, int Hex_Wrap, uintptr_t CodePage)
	{
		WaitCommitAsync();
		if (stmtSetViewerPos.Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(FilePos).Bind(LeftPos).Bind(Hex_Wrap).Bind((int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetViewerPos(const string& Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, uintptr_t *CodePage)
	{
		WaitCommitAsync();
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

	bool SetViewerBookmark(unsigned __int64 id, size_t i, __int64 FilePos, __int64 LeftPos)
	{
		WaitCommitAsync();
		return stmtSetViewerBookmark.Bind(id).Bind(i).Bind(FilePos).Bind(LeftPos).StepAndReset();
	}

	bool GetViewerBookmark(unsigned __int64 id, size_t i, __int64 *FilePos, __int64 *LeftPos)
	{
		WaitCommitAsync();
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
		WaitCommitAsync();
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= DaysToUI64(DaysToKeep);
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

	// TODO: log
	// TODO: implementation
	virtual void Import(const tinyxml::TiXmlHandle&) override {}
	virtual void Export(tinyxml::TiXmlElement&) override {}
};

class HistoryConfigMemory: public HistoryConfigCustom {
public:
	HistoryConfigMemory()
	{
		Initialize(L":memory:", true);
	}

	virtual void Import(const tinyxml::TiXmlHandle&) override {}
	virtual void Export(tinyxml::TiXmlElement&) override {}
};

void Database::CheckDatabase(SQLiteDb *pDb)
{
	string pname;
	int rc = pDb->InitStatus(pname, m_Mode != default_mode);
	if ( rc > 0 )
	{
		if (m_Mode != default_mode)
		{
			Console().Write(L"problem with " + pname + (rc <= 1 ? L":\r\n  database file is renamed to *.bad and new one is created\r\n" : L":\r\n  database is opened in memory\r\n"));
			Console().Commit();
		}
		else
		{
			m_Problems.emplace_back(pname);
		}
	}
}

void Database::TryImportDatabase(XmlConfig *p, const char *son, bool plugin)
{
	if (m_TemplateLoadState != 0)
	{
		if (m_TemplateLoadState < 0 && !Global->Opt->TemplateProfilePath.empty())
		{
			m_TemplateLoadState = 0;
			string def_config = Global->Opt->TemplateProfilePath;
			FILE* XmlFile = _wfopen(NTPath(def_config).data(), L"rb");
			if (XmlFile)
			{
				m_TemplateDoc = std::make_unique<tinyxml::TiXmlDocument>();
				if (m_TemplateDoc->LoadFile(XmlFile))
				{
					if (nullptr != (m_TemplateRoot = m_TemplateDoc->FirstChildElement("farconfig")))
						m_TemplateLoadState = +1;
				}
				fclose(XmlFile);
			}
		}

		if (m_TemplateLoadState > 0)
		{
			const tinyxml::TiXmlHandle root(m_TemplateRoot);
			if (!son)
				p->Import(root);
			else if (!plugin)
				p->Import(root.FirstChildElement(son));
			else
			{
				FOR(const tinyxml::TiXmlElement& i, xml_enum(root.FirstChild("pluginsconfig"), "plugin"))
				{
					const char *guid = i.Attribute("guid");
					if (guid && 0 == strcmp(guid, son))
					{
						p->Import(tinyxml::TiXmlHandle(&const_cast<tinyxml::TiXmlElement&>(i)));
						break;
					}
				}
			}
		}
	}
}

template<class T>
std::unique_ptr<T> Database::CreateDatabase(const char *son)
{
	auto cfg = std::make_unique<T>();
	CheckDatabase(cfg.get());
	if (m_Mode != import_mode && cfg->IsNew())
	{
		TryImportDatabase(cfg.get(), son);
	}
	return cfg;
}

template<class T>
HierarchicalConfigUniquePtr Database::CreateHierarchicalConfig(dbcheck DbId, const string& dbn, const char *xmln, bool Local, bool plugin)
{
	T *cfg = new T(dbn, Local);
	bool first = !CheckedDb.Check(DbId);

	if (first)
		CheckDatabase(cfg);

	if (m_Mode != import_mode && cfg->IsNew() && first)
		TryImportDatabase(cfg, xmln, plugin);

	CheckedDb.Set(DbId);
	return HierarchicalConfigUniquePtr(cfg);
}

ENUM(Database::dbcheck)
{
	CHECK_NONE = 0,
	CHECK_FILTERS = BIT(0),
	CHECK_HIGHLIGHT = BIT(1),
	CHECK_SHORTCUTS = BIT(2),
	CHECK_PANELMODES = BIT(3),
};

HierarchicalConfigUniquePtr Database::CreatePluginsConfig(const string& guid, bool Local)
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_NONE, L"PluginsData\\" + guid + L".db", Utf8String(guid).data(), Local, true);
}

HierarchicalConfigUniquePtr Database::CreateFiltersConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_FILTERS, L"filters.db","filters");
}

HierarchicalConfigUniquePtr Database::CreateHighlightConfig()
{
	return CreateHierarchicalConfig<HighlightHierarchicalConfigDb>(CHECK_HIGHLIGHT, L"highlight.db","highlight");
}

HierarchicalConfigUniquePtr Database::CreateShortcutsConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_SHORTCUTS, L"shortcuts.db","shortcuts", true);
}

HierarchicalConfigUniquePtr Database::CreatePanelModeConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_PANELMODES, L"panelmodes.db","panelmodes");
}

Database::Database(mode Mode):
	m_TemplateDoc(nullptr),
	m_TemplateRoot(nullptr),
	m_TemplateLoadState(-1),
	m_Mode(Mode),
	m_GeneralCfg(CreateDatabase<GeneralConfigDb>()),
	m_LocalGeneralCfg(CreateDatabase<LocalGeneralConfigDb>()),
	m_ColorsCfg(CreateDatabase<ColorsConfigDb>()),
	m_AssocConfig(CreateDatabase<AssociationsConfigDb>()),
	m_PlCacheCfg(CreateDatabase<PluginsCacheConfigDb>()),
	m_PlHotkeyCfg(CreateDatabase<PluginsHotkeysConfigDb>()),
	m_HistoryCfg(CreateDatabase<HistoryConfigDb>()),
	m_HistoryCfgMem(CreateDatabase<HistoryConfigMemory>())
{
}

Database::~Database()
{
	MultiWaiter ThreadWaiter;
	FOR(const auto& i, m_Threads) { ThreadWaiter.Add(i); }
	ThreadWaiter.Wait();
	FOR(auto& i, m_Threads) { i.detach(); }
}

RegExp& GetRE()
{
	static RegExp re;
	re.Compile(L"/^[0-9A-F]{8}-([0-9A-F]{4}-){3}[0-9A-F]{12}$/", OP_PERLSTYLE | OP_OPTIMIZE);
	return re;
}

bool Database::Export(const string& File)
{
	FILE* XmlFile = _wfopen(NTPath(File).data(), L"w");
	if(!XmlFile)
		return false;

	SCOPE_EXIT
	{
		fclose(XmlFile);
	};

	tinyxml::TiXmlDocument doc;
	doc.LinkEndChild(new tinyxml::TiXmlDeclaration("1.0", "UTF-8", ""));

	auto& root = CreateChild(doc, "farconfig");
	root.SetAttribute("version", Utf8String(std::to_wstring(FAR_VERSION.Major) + L"." + std::to_wstring(FAR_VERSION.Minor) + L"." + std::to_wstring(FAR_VERSION.Build)).data());

	GeneralCfg()->Export(root);
	LocalGeneralCfg()->Export(root);
	ColorsCfg()->Export(root);
	AssocConfig()->Export(root);
	PlHotkeyCfg()->Export(root);
	CreateFiltersConfig()->Export(CreateChild(root, "filters"));
	CreateHighlightConfig()->Export(CreateChild(root, "highlight"));
	CreatePanelModeConfig()->Export(CreateChild(root, "panelmodes"));
	CreateShortcutsConfig()->Export(CreateChild(root, "shortcuts"));

	{ //TODO: export for local plugin settings
		auto& e = CreateChild(root, "pluginsconfig");
		api::enum_file ff(Global->Opt->ProfilePath + L"\\PluginsData\\*.db");
		std::for_each(RANGE(ff, i)
		{
			i.strFileName.resize(i.strFileName.size()-3);
			Upper(i.strFileName);
			RegExpMatch m[2];
			intptr_t mc = ARRAYSIZE(m);
			if (GetRE().Match(i.strFileName.data(), i.strFileName.data() + i.strFileName.size(), m, mc))
			{
				auto& plugin = CreateChild(e, "plugin");
				plugin.SetAttribute("guid", Utf8String(i.strFileName).data());
				CreatePluginsConfig(i.strFileName)->Export(plugin);
			}
		});
	}

	return doc.SaveFile(XmlFile);
}

bool Database::Import(const string& File)
{
	FILE* XmlFile = _wfopen(NTPath(File).data(), L"rb");
	if(!XmlFile)
		return false;

	SCOPE_EXIT
	{
		fclose(XmlFile);
	};

	bool ret = false;

	tinyxml::TiXmlDocument doc;

	if (doc.LoadFile(XmlFile))
	{
		auto farconfig = doc.FirstChildElement("farconfig");
		if (farconfig)
		{
			const tinyxml::TiXmlHandle root(farconfig);

			GeneralCfg()->Import(root);

			LocalGeneralCfg()->Import(root);

			ColorsCfg()->Import(root);

			AssocConfig()->Import(root);

			PlHotkeyCfg()->Import(root);

			CreateFiltersConfig()->Import(root.FirstChildElement("filters"));

			CreateHighlightConfig()->Import(root.FirstChildElement("highlight"));

			CreatePanelModeConfig()->Import(root.FirstChildElement("panelmodes"));

			CreateShortcutsConfig()->Import(root.FirstChildElement("shortcuts"));

			//TODO: import for local plugin settings
			RegExpMatch m[2];

			FOR(const tinyxml::TiXmlElement& plugin, xml_enum(root.FirstChild("pluginsconfig"), "plugin"))
			{
				auto guid = plugin.Attribute("guid");
				if (!guid)
					continue;
				string Guid = wide(guid, CP_UTF8);
				Upper(Guid);

				intptr_t mc = ARRAYSIZE(m);
				if (GetRE().Match(Guid.data(), Guid.data() + Guid.size(), m, mc))
				{
					CreatePluginsConfig(Guid)->Import(tinyxml::TiXmlHandle(&const_cast<tinyxml::TiXmlElement&>(plugin)));
				}
			}

			ret = true;
		}
	}

	if (doc.Error())
		PrintError(L"XML Error", doc);

	return ret;
}

void Database::ClearPluginsCache()
{
	PluginsCacheConfigDb().DiscardCache();
}

int Database::ShowProblems()
{
	int rc = 0;
	if (!m_Problems.empty())
	{
		std::vector<string> msgs(m_Problems.cbegin(), m_Problems.cend());
		msgs.emplace_back(MSG(MShowConfigFolders));
		msgs.emplace_back(MSG(MIgnore));
		rc = Message(MSG_WARNING, 2, MSG(MProblemDb), msgs) == 0 ? +1 : -1;
	}
	return rc;
}
