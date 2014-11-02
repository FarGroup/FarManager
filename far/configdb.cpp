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
	const tinyxml::TiXmlElement* operator->() const { return m_data; }
	const tinyxml::TiXmlElement& operator*() const { return *m_data; }

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
			value = value->NextSiblingElement(m_name.data()) :
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
public:
	virtual ~iGeneralConfigDb() {};

	virtual bool BeginTransaction() override { return SQLiteDb::BeginTransaction(); }
	virtual bool EndTransaction() override { return SQLiteDb::EndTransaction(); }
	virtual bool RollbackTransaction() override { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));"
		;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtUpdateValue, L"UPDATE general_config SET value=?1 WHERE key=?2 AND name=?3;" },
			{ stmtInsertValue, L"INSERT INTO general_config VALUES (?1,?2,?3);" },
			{ stmtGetValue, L"SELECT value FROM general_config WHERE key=?1 AND name=?2;" },
			{ stmtDelValue, L"DELETE FROM general_config WHERE key=?1 AND name=?2;" },
			{ stmtEnumValues, L"SELECT name, value FROM general_config WHERE key=?1;" },
		};

		return Open(DbName, Local)
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool SetValue(const string& Key, const string& Name, const string& Value) override
	{
		bool b = m_Statements[stmtUpdateValue].Bind(Value, Key, Name).StepAndReset();
		if (!b || Changes() == 0)
			b = m_Statements[stmtInsertValue].Bind(Key, Name, Value).StepAndReset();
		return b;
	}

	virtual bool SetValue(const string& Key, const string& Name, unsigned __int64 Value) override
	{
		bool b = m_Statements[stmtUpdateValue].Bind(Value, Key, Name).StepAndReset();
		if (!b || Changes() == 0)
			b = m_Statements[stmtInsertValue].Bind(Key, Name, Value).StepAndReset();
		return b;
	}

	virtual bool SetValue(const string& Key, const string& Name, const void *Value, size_t Size) override
	{
		bool b = m_Statements[stmtUpdateValue].Bind(blob(Value,Size), Key, Name).StepAndReset();
		if (!b || Changes() == 0)
			b = m_Statements[stmtInsertValue].Bind(Key, Name, blob(Value, Size)).StepAndReset();
		return b;
	}

	virtual bool GetValue(const string& Key, const string& Name, long long *Value, long long Default) override
	{
		bool b = m_Statements[stmtGetValue].Bind(Key, Name).Step();
		if (b)
		{
			if (m_Statements[stmtGetValue].GetColType(0) == TYPE_INTEGER)
			{
				*Value = m_Statements[stmtGetValue].GetColInt64(0);
			}
			else
			{
				// TODO: log
			}
		}
		m_Statements[stmtGetValue].Reset();

		if (b)
		{
			return true;
		}
		*Value = Default;
		return false;
	}

	virtual bool GetValue(const string& Key, const string& Name, string &strValue, const wchar_t *Default) override
	{
		bool b = m_Statements[stmtGetValue].Bind(Key, Name).Step();
		if (b)
		{
			if (m_Statements[stmtGetValue].GetColType(0) == TYPE_STRING)
			{
				strValue = m_Statements[stmtGetValue].GetColText(0);
			}
			else
			{
				// TODO: log
			}
		}
		m_Statements[stmtGetValue].Reset();

		if (b)
		{
			return true;
		}
		strValue = Default;
		return false;
	}

	virtual int GetValue(const string& Key, const string& Name, void *Value, size_t Size, const void *Default) override
	{
		int realsize = 0;
		if (m_Statements[stmtGetValue].Bind(Key, Name).Step())
		{
			if (m_Statements[stmtGetValue].GetColType(0) == TYPE_BLOB)
			{
				const char *blob = m_Statements[stmtGetValue].GetColBlob(0);
				realsize = m_Statements[stmtGetValue].GetColBytes(0);
				if (Value)
					memcpy(Value, blob, std::min(realsize, static_cast<int>(Size)));
			}
			else
			{
				// TODO: log
			}
		}
		m_Statements[stmtGetValue].Reset();

		if (realsize)
		{
			return realsize;
		}
		else if (Default)
		{
			memcpy(Value, Default, Size);
			return static_cast<int>(Size);
		}
		return 0;
	}

	virtual bool DeleteValue(const string& Key, const string& Name) override
	{
		return m_Statements[stmtDelValue].Bind(Key, Name).StepAndReset();
	}

	virtual bool EnumValues(const string& Key, DWORD Index, string &Name, string &Value) override
	{
		if (Index == 0)
			m_Statements[stmtEnumValues].Reset().Bind(Key);

		if (m_Statements[stmtEnumValues].Step())
		{
			Name = m_Statements[stmtEnumValues].GetColText(0);
			Value = m_Statements[stmtEnumValues].GetColText(1);
			return true;
		}

		m_Statements[stmtEnumValues].Reset();
		return false;
	}

	virtual bool EnumValues(const string& Key, DWORD Index, string &Name, DWORD& Value) override
	{
		if (Index == 0)
			m_Statements[stmtEnumValues].Reset().Bind(transient(Key));

		if (m_Statements[stmtEnumValues].Step())
		{
			Name = m_Statements[stmtEnumValues].GetColText(0);
			Value = (DWORD)m_Statements[stmtEnumValues].GetColInt(1);
			return true;
		}

		m_Statements[stmtEnumValues].Reset();
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
		FOR(const auto& e, xml_enum(root.FirstChild(GetKeyName()), "setting"))
		{
			const char *key = e->Attribute("key");
			const char *name = e->Attribute("name");
			const char *type = e->Attribute("type");
			const char *value = e->Attribute("value");

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

protected:
	iGeneralConfigDb(const wchar_t* DbName, bool local)
	{
		Initialize(DbName, local);
	}

private:
	virtual const char* GetKeyName() const = 0;

	enum statement_id
	{
		stmtUpdateValue,
		stmtInsertValue,
		stmtGetValue,
		stmtDelValue,
		stmtEnumValues,
	};
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

class HierarchicalConfigDb: public HierarchicalConfig, public SQLiteDb
{
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
		Global->Db->AddThread(Thread(&Thread::detach, &HierarchicalConfigDb::AsyncDelete, this));
	}

	virtual bool BeginTransaction() override { return SQLiteDb::BeginTransaction(); }
	virtual bool EndTransaction() override { return SQLiteDb::EndTransaction(); }
	virtual bool RollbackTransaction() override { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS table_keys(id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
			"CREATE TABLE IF NOT EXISTS table_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id <> 0));"
			//root key (needs to be before the transaction start)
			"INSERT OR IGNORE INTO table_keys VALUES (0,0,\"\",\"Root - do not edit\");"
		;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtCreateKey, L"INSERT INTO table_keys VALUES (NULL,?1,?2,?3);" },
			{ stmtFindKey, L"SELECT id FROM table_keys WHERE parent_id=?1 AND name=?2 AND id<>0;" },
			{ stmtSetKeyDescription, L"UPDATE table_keys SET description=?1 WHERE id=?2 AND id<>0 AND description<>?1;" },
			{ stmtSetValue, L"INSERT OR REPLACE INTO table_values VALUES (?1,?2,?3);" },
			{ stmtGetValue, L"SELECT value FROM table_values WHERE key_id=?1 AND name=?2;" },
			{ stmtEnumKeys, L"SELECT name FROM table_keys WHERE parent_id=?1 AND id<>0;" },
			{ stmtEnumValues, L"SELECT name, value FROM table_values WHERE key_id=?1;" },
			{ stmtDelValue, L"DELETE FROM table_values WHERE key_id=?1 AND name=?2;" },
			{ stmtDeleteTree, L"DELETE FROM table_keys WHERE id=?1 AND id<>0;" },
		};

		return Open(DbName, Local)
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		    && BeginTransaction()
		;
	}

	virtual bool Flush() override
	{
		bool b = EndTransaction();
		BeginTransaction();
		return b;
	}

	virtual unsigned __int64 CreateKey(unsigned __int64 Root, const string& Name, const string* Description) override
	{
		if (m_Statements[stmtCreateKey].Bind(Root, Name, Description? *Description : string()).StepAndReset())
			return LastInsertRowID();
		unsigned __int64 id = GetKeyID(Root,Name);
		if (id && Description)
			SetKeyDescription(id,Description? *Description : string());
		return id;
	}

	virtual unsigned __int64 GetKeyID(unsigned __int64 Root, const string& Name) override
	{
		unsigned __int64 id = 0;
		if (m_Statements[stmtFindKey].Bind(Root, Name).Step())
			id = m_Statements[stmtFindKey].GetColInt64(0);
		m_Statements[stmtFindKey].Reset();
		return id;
	}

	virtual bool SetKeyDescription(unsigned __int64 Root, const string& Description) override
	{
		return m_Statements[stmtSetKeyDescription].Bind(Description, Root).StepAndReset();
	}

	virtual bool SetValue(unsigned __int64 Root, const string& Name, const string& Value) override
	{
		return m_Statements[stmtSetValue].Bind(Root, Name, Value).StepAndReset();
	}

	virtual bool SetValue(unsigned __int64 Root, const string& Name, unsigned __int64 Value) override
	{
		return m_Statements[stmtSetValue].Bind(Root, Name, Value).StepAndReset();
	}

	virtual bool SetValue(unsigned __int64 Root, const string& Name, const void *Value, size_t Size) override
	{
		return m_Statements[stmtSetValue].Bind(Root, Name, blob(Value, Size)).StepAndReset();
	}

	virtual bool GetValue(unsigned __int64 Root, const string& Name, unsigned __int64 *Value) override
	{
		bool b = m_Statements[stmtGetValue].Bind(Root, Name).Step();
		if (b)
			*Value = m_Statements[stmtGetValue].GetColInt64(0);
		m_Statements[stmtGetValue].Reset();
		return b;
	}

	virtual bool GetValue(unsigned __int64 Root, const string& Name, string &strValue) override
	{
		bool b = m_Statements[stmtGetValue].Bind(Root, Name).Step();
		if (b)
			strValue = m_Statements[stmtGetValue].GetColText(0);
		m_Statements[stmtGetValue].Reset();
		return b;
	}

	virtual int GetValue(unsigned __int64 Root, const string& Name, void *Value, size_t Size) override
	{
		int realsize = 0;
		if (m_Statements[stmtGetValue].Bind(Root, Name).Step())
		{
			const char *blob = m_Statements[stmtGetValue].GetColBlob(0);
			realsize = m_Statements[stmtGetValue].GetColBytes(0);
			if (Value)
				memcpy(Value,blob,std::min(realsize,static_cast<int>(Size)));
		}
		m_Statements[stmtGetValue].Reset();
		return realsize;
	}

	virtual bool DeleteKeyTree(unsigned __int64 KeyID) override
	{
		//All subtree is automatically deleted because of foreign key constraints
		return m_Statements[stmtDeleteTree].Bind(KeyID).StepAndReset();
	}

	virtual bool DeleteValue(unsigned __int64 Root, const string& Name) override
	{
		return m_Statements[stmtDelValue].Bind(Root, Name).StepAndReset();
	}

	virtual bool EnumKeys(unsigned __int64 Root, DWORD Index, string& Name) override
	{
		if (Index == 0)
			m_Statements[stmtEnumKeys].Reset().Bind(Root);

		if (m_Statements[stmtEnumKeys].Step())
		{
			Name = m_Statements[stmtEnumKeys].GetColText(0);
			return true;
		}

		m_Statements[stmtEnumKeys].Reset();
		return false;
	}

	virtual bool EnumValues(unsigned __int64 Root, DWORD Index, string& Name, DWORD *Type) override
	{
		if (Index == 0)
			m_Statements[stmtEnumValues].Reset().Bind(Root);

		if (m_Statements[stmtEnumValues].Step())
		{
			Name = m_Statements[stmtEnumValues].GetColText(0);
			*Type = m_Statements[stmtEnumValues].GetColType(1);
			return true;
		}

		m_Statements[stmtEnumValues].Reset();
		return false;

	}

	virtual void SerializeBlob(const char* Name, const char* Blob, int Size, tinyxml::TiXmlElement& e)
	{
			auto hex = BlobToHexString(Blob, Size);
			e.SetAttribute("type", "hex");
			e.SetAttribute("value", hex.get());
	}

	virtual void Export(unsigned __int64 id, tinyxml::TiXmlElement& key)
	{
		m_Statements[stmtEnumValues].Bind(id);
		while (m_Statements[stmtEnumValues].Step())
		{
			auto& e = CreateChild(key, "value");

			const char* name = m_Statements[stmtEnumValues].GetColTextUTF8(0);
			e.SetAttribute("name", name);

			switch (m_Statements[stmtEnumValues].GetColType(1))
			{
				case TYPE_INTEGER:
					e.SetAttribute("type", "qword");
					e.SetAttribute("value", Int64ToHexString(m_Statements[stmtEnumValues].GetColInt64(1)));
					break;
				case TYPE_STRING:
					e.SetAttribute("type", "text");
					e.SetAttribute("value", m_Statements[stmtEnumValues].GetColTextUTF8(1));
					break;
				default:
					SerializeBlob(name, m_Statements[stmtEnumValues].GetColBlob(1), m_Statements[stmtEnumValues].GetColBytes(1), e);
			}
		}
		m_Statements[stmtEnumValues].Reset();

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

	virtual void Import(unsigned __int64 root, const tinyxml::TiXmlElement& key)
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

		FOR(const auto& e, xml_enum(key, "value"))
		{
			const char *name = e->Attribute("name");
			const char *type = e->Attribute("type");
			const char *value = e->Attribute("value");

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
				int Size = DeserializeBlob(name, type, value, *e, Blob);
				if (Blob)
				{
					SetValue(id, Name, Blob.get(), Size);
				}
			}
		}

		FOR(const auto& e, xml_enum(key, "key"))
		{
			Import(id, *e);
		}

	}

	virtual void Import(const tinyxml::TiXmlHandle &root) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		FOR(const auto& e, xml_enum(root.FirstChild("hierarchicalconfig"), "key"))
		{
			Import(0, *e);
		}
	}

protected:
	HierarchicalConfigDb() {}

	virtual ~HierarchicalConfigDb() { EndTransaction(); AsyncDone.Set(); }

private:
	void AsyncDelete()
	{
		delete this;
	}

	enum statement_id
	{
		stmtCreateKey,
		stmtFindKey,
		stmtSetKeyDescription,
		stmtSetValue,
		stmtGetValue,
		stmtEnumKeys,
		stmtEnumValues,
		stmtDelValue,
		stmtDeleteTree,
	};

	Event AsyncDone;
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
public:
	ColorsConfigDb()
	{
		Initialize(L"colors.db");
	}

	virtual ~ColorsConfigDb() { }

	virtual bool BeginTransaction() override { return SQLiteDb::BeginTransaction(); }
	virtual bool EndTransaction() override { return SQLiteDb::EndTransaction(); }
	virtual bool RollbackTransaction() override { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS colors(name TEXT NOT NULL PRIMARY KEY, value BLOB);"
		;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtUpdateValue, L"UPDATE colors SET value=?1 WHERE name=?2;" },
			{ stmtInsertValue, L"INSERT INTO colors VALUES (?1,?2);" },
			{ stmtGetValue, L"SELECT value FROM colors WHERE name=?1;" },
			{ stmtDelValue, L"DELETE FROM colors WHERE name=?1;" },
		};

		return Open(DbName, Local)
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool SetValue(const string& Name, const FarColor& Value) override
	{
		bool b = m_Statements[stmtUpdateValue].Bind(blob(&Value, sizeof(Value)), Name).StepAndReset();
		if (!b || Changes() == 0)
			b = m_Statements[stmtInsertValue].Bind(Name, blob(&Value, sizeof(Value))).StepAndReset();
		return b;
	}

	virtual bool GetValue(const string& Name, FarColor& Value) override
	{
		bool b = m_Statements[stmtGetValue].Bind(Name).Step();
		if (b)
		{
			const void* blob = m_Statements[stmtGetValue].GetColBlob(0);
			Value = *static_cast<const FarColor*>(blob);
		}
		m_Statements[stmtGetValue].Reset();
		return b;
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
		FOR(const auto& e, xml_enum(root.FirstChild("colors"), "object"))
		{
			const char *name = e->Attribute("name");
			const char *background = e->Attribute("background");
			const char *foreground = e->Attribute("foreground");
			const char *flags = e->Attribute("flags");

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
				m_Statements[stmtDelValue].Bind(Name).StepAndReset();
			}
		}
	}

private:
	enum statement_id
	{
		stmtUpdateValue,
		stmtInsertValue,
		stmtGetValue,
		stmtDelValue,
	};
};

class AssociationsConfigDb: public AssociationsConfig, public SQLiteDb
{
public:
	AssociationsConfigDb()
	{
		Initialize(L"associations.db");
	}

	virtual ~AssociationsConfigDb() {}

	virtual bool BeginTransaction() override { return SQLiteDb::BeginTransaction(); }
	virtual bool EndTransaction() override { return SQLiteDb::EndTransaction(); }
	virtual bool RollbackTransaction() override { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
		;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtReorder, L"UPDATE filetypes SET weight=weight+1 WHERE weight>(CASE ?1 WHEN 0 THEN 0 ELSE (SELECT weight FROM filetypes WHERE id=?1) END);" },
			{ stmtAddType, L"INSERT INTO filetypes VALUES (NULL,(CASE ?1 WHEN 0 THEN 1 ELSE (SELECT weight FROM filetypes WHERE id=?1)+1 END),?2,?3);" },
			{ stmtGetMask, L"SELECT mask FROM filetypes WHERE id=?1;" },
			{ stmtGetDescription, L"SELECT description FROM filetypes WHERE id=?1;" },
			{ stmtUpdateType, L"UPDATE filetypes SET mask=?1, description=?2 WHERE id=?3;" },
			{ stmtSetCommand, L"INSERT OR REPLACE INTO commands VALUES (?1,?2,?3,?4);" },
			{ stmtGetCommand, L"SELECT command, enabled FROM commands WHERE ft_id=?1 AND type=?2;" },
			{ stmtEnumTypes, L"SELECT id, description FROM filetypes ORDER BY weight;" },
			{ stmtEnumMasks, L"SELECT id, mask FROM filetypes ORDER BY weight;" },
			{ stmtEnumMasksForType, L"SELECT id, mask FROM filetypes, commands WHERE id=ft_id AND type=?1 AND enabled<>0 ORDER BY weight;" },
			{ stmtDelType, L"DELETE FROM filetypes WHERE id=?1;" },
			{ stmtGetWeight, L"SELECT weight FROM filetypes WHERE id=?1;" },
			{ stmtSetWeight, L"UPDATE filetypes SET weight=?1 WHERE id=?2;" },
		};

		return Open(DbName, Local)
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool EnumMasks(DWORD Index, unsigned __int64 *id, string &strMask) override
	{
		if (Index == 0)
			m_Statements[stmtEnumMasks].Reset();

		if (m_Statements[stmtEnumMasks].Step())
		{
			*id = m_Statements[stmtEnumMasks].GetColInt64(0);
			strMask = m_Statements[stmtEnumMasks].GetColText(1);
			return true;
		}

		m_Statements[stmtEnumMasks].Reset();
		return false;
	}

	virtual bool EnumMasksForType(int Type, DWORD Index, unsigned __int64 *id, string &strMask) override
	{
		if (Index == 0)
			m_Statements[stmtEnumMasksForType].Reset().Bind(Type);

		if (m_Statements[stmtEnumMasksForType].Step())
		{
			*id = m_Statements[stmtEnumMasksForType].GetColInt64(0);
			strMask = m_Statements[stmtEnumMasksForType].GetColText(1);
			return true;
		}

		m_Statements[stmtEnumMasksForType].Reset();
		return false;
	}

	virtual bool GetMask(unsigned __int64 id, string &strMask) override
	{
		bool b = m_Statements[stmtGetMask].Bind(id).Step();
		if (b)
			strMask = m_Statements[stmtGetMask].GetColText(0);
		m_Statements[stmtGetMask].Reset();
		return b;
	}

	virtual bool GetDescription(unsigned __int64 id, string &strDescription) override
	{
		bool b = m_Statements[stmtGetDescription].Bind(id).Step();
		if (b)
			strDescription = m_Statements[stmtGetDescription].GetColText(0);
		m_Statements[stmtGetDescription].Reset();
		return b;
	}

	virtual bool GetCommand(unsigned __int64 id, int Type, string &strCommand, bool *Enabled = nullptr) override
	{
		bool b = m_Statements[stmtGetCommand].Bind(id, Type).Step();
		if (b)
		{
			strCommand = m_Statements[stmtGetCommand].GetColText(0);
			if (Enabled)
				*Enabled = m_Statements[stmtGetCommand].GetColInt(1) != 0;
		}
		m_Statements[stmtGetCommand].Reset();
		return b;
	}

	virtual bool SetCommand(unsigned __int64 id, int Type, const string& Command, bool Enabled) override
	{
		return m_Statements[stmtSetCommand].Bind(id, Type, Enabled, Command).StepAndReset();
	}

	virtual bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2) override
	{
		if (m_Statements[stmtGetWeight].Bind(id1).Step())
		{
			unsigned __int64 weight1 = m_Statements[stmtGetWeight].GetColInt64(0);
			m_Statements[stmtGetWeight].Reset();
			if (m_Statements[stmtGetWeight].Bind(id2).Step())
			{
				unsigned __int64 weight2 = m_Statements[stmtGetWeight].GetColInt64(0);
				m_Statements[stmtGetWeight].Reset();
				return m_Statements[stmtSetWeight].Bind(weight1, id2).StepAndReset() && m_Statements[stmtSetWeight].Bind(weight2, id1).StepAndReset();
			}
		}
		m_Statements[stmtGetWeight].Reset();
		return false;
	}

	virtual unsigned __int64 AddType(unsigned __int64 after_id, const string& Mask, const string& Description) override
	{
		if (m_Statements[stmtReorder].Bind(after_id).StepAndReset() && m_Statements[stmtAddType].Bind(after_id, Mask, Description).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual bool UpdateType(unsigned __int64 id, const string& Mask, const string& Description) override
	{
		return m_Statements[stmtUpdateType].Bind(Mask, Description, id).StepAndReset();
	}

	virtual bool DelType(unsigned __int64 id) override
	{
		return m_Statements[stmtDelType].Bind(id).StepAndReset();
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
		FOR(const auto& e, xml_enum(base, "filetype"))
		{
			const char *mask = e->Attribute("mask");
			const char *description = e->Attribute("description");

			if (!mask)
				continue;

			string Mask = wide(mask, CP_UTF8);
			string Description = wide(description, CP_UTF8);

			id = AddType(id, Mask, Description);
			if (!id)
				continue;

			FOR(const auto& se, xml_enum(*e, "command"))
			{
				const char *command = se->Attribute("command");
				int type=0;
				const char *stype = se->Attribute("type", &type);
				int enabled=0;
				const char *senabled = se->Attribute("enabled", &enabled);

				if (!command || !stype || !senabled)
					continue;

				string Command = wide(command, CP_UTF8);
				SetCommand(id, type, Command, enabled != 0);
			}

		}
	}

private:
	enum statement_id
	{
		stmtReorder,
		stmtAddType,
		stmtGetMask,
		stmtGetDescription,
		stmtUpdateType,
		stmtSetCommand,
		stmtGetCommand,
		stmtEnumTypes,
		stmtEnumMasks,
		stmtEnumMasksForType,
		stmtDelType,
		stmtGetWeight,
		stmtSetWeight,
	};
};

class PluginsCacheConfigDb: public PluginsCacheConfig, public SQLiteDb
{
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

	virtual ~PluginsCacheConfigDb() {}

	virtual bool BeginTransaction() override { return SQLiteDb::BeginTransaction(); }
	virtual bool EndTransaction() override { return SQLiteDb::EndTransaction(); }
	virtual bool RollbackTransaction() override { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
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
		;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtCreateCache, L"INSERT INTO cachename VALUES (NULL,?1);," },
			{ stmtFindCacheName, L"SELECT id FROM cachename WHERE name=?1;" },
			{ stmtDelCache, L"DELETE FROM cachename WHERE name=?1;" },
			{ stmtCountCacheNames, L"SELECT count(name) FROM cachename;" },
			{ stmtGetPreloadState, L"SELECT enabled FROM preload WHERE cid=?1;" },
			{ stmtGetSignature, L"SELECT signature FROM signatures WHERE cid=?1;" },
			{ stmtGetExportState, L"SELECT enabled FROM exports WHERE cid=?1 and export=?2;" },
			{ stmtGetGuid, L"SELECT guid FROM guids WHERE cid=?1;" },
			{ stmtGetTitle, L"SELECT title FROM titles WHERE cid=?1;" },
			{ stmtGetAuthor, L"SELECT author FROM authors WHERE cid=?1;" },
			{ stmtGetPrefix, L"SELECT prefix FROM prefixes WHERE cid=?1;" },
			{ stmtGetDescription, L"SELECT description FROM descriptions WHERE cid=?1;" },
			{ stmtGetFlags, L"SELECT bitmask FROM flags WHERE cid=?1;" },
			{ stmtGetMinFarVersion, L"SELECT version FROM minfarversions WHERE cid=?1;" },
			{ stmtGetVersion, L"SELECT version FROM pluginversions WHERE cid=?1;" },
			{ stmtSetPreloadState, L"INSERT OR REPLACE INTO preload VALUES (?1,?2);" },
			{ stmtSetSignature, L"INSERT OR REPLACE INTO signatures VALUES (?1,?2);" },
			{ stmtSetExportState, L"INSERT OR REPLACE INTO exports VALUES (?1,?2,?3);" },
			{ stmtSetGuid, L"INSERT OR REPLACE INTO guids VALUES (?1,?2);" },
			{ stmtSetTitle, L"INSERT OR REPLACE INTO titles VALUES (?1,?2);" },
			{ stmtSetAuthor, L"INSERT OR REPLACE INTO authors VALUES (?1,?2);" },
			{ stmtSetPrefix, L"INSERT OR REPLACE INTO prefixes VALUES (?1,?2);" },
			{ stmtSetDescription, L"INSERT OR REPLACE INTO descriptions VALUES (?1,?2);" },
			{ stmtSetFlags, L"INSERT OR REPLACE INTO flags VALUES (?1,?2);," },
			{ stmtSetMinFarVersion, L"INSERT OR REPLACE INTO minfarversions VALUES (?1,?2);" },
			{ stmtSetVersion, L"INSERT OR REPLACE INTO pluginversions VALUES (?1,?2);" },
			{ stmtEnumCache, L"SELECT name FROM cachename ORDER BY name;" },
			{ stmtGetMenuItem, L"SELECT name, guid FROM menuitems WHERE cid=?1 AND type=?2 AND number=?3;" },
			{ stmtSetMenuItem, L"INSERT OR REPLACE INTO menuitems VALUES (?1,?2,?3,?4,?5);" },
		};

		return Open(DbName, Local, true)
		    && SetWALJournalingMode()
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual void Import(const tinyxml::TiXmlHandle&) override {}
	virtual void Export(tinyxml::TiXmlElement&) override {}

	virtual unsigned __int64 CreateCache(const string& CacheName) override
	{
		if (m_Statements[stmtCreateCache].Bind(CacheName).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual unsigned __int64 GetCacheID(const string& CacheName) override
	{
		unsigned __int64 id = 0;
		if (m_Statements[stmtFindCacheName].Bind(CacheName).Step())
			id = m_Statements[stmtFindCacheName].GetColInt64(0);
		m_Statements[stmtFindCacheName].Reset();
		return id;
	}

	virtual bool DeleteCache(const string& CacheName) override
	{
		//All related entries are automatically deleted because of foreign key constraints
		return m_Statements[stmtDelCache].Bind(CacheName).StepAndReset();
	}

	virtual bool IsPreload(unsigned __int64 id) override
	{
		bool preload = false;
		if (m_Statements[stmtGetPreloadState].Bind(id).Step())
			preload = m_Statements[stmtGetPreloadState].GetColInt(0) != 0;
		m_Statements[stmtGetPreloadState].Reset();
		return preload;
	}

	virtual string GetSignature(unsigned __int64 id) override
	{
		return GetTextFromID(m_Statements[stmtGetSignature], id);
	}

	virtual void *GetExport(unsigned __int64 id, const string& ExportName) override
	{
		void *enabled = nullptr;
		if (m_Statements[stmtGetExportState].Bind(id, ExportName).Step())
			if (m_Statements[stmtGetExportState].GetColInt(0) > 0)
				enabled = ToPtr(1);
		m_Statements[stmtGetExportState].Reset();
		return enabled;
	}

	virtual string GetGuid(unsigned __int64 id) override
	{
		return GetTextFromID(m_Statements[stmtGetGuid], id);
	}

	virtual string GetTitle(unsigned __int64 id) override
	{
		return GetTextFromID(m_Statements[stmtGetTitle], id);
	}

	virtual string GetAuthor(unsigned __int64 id) override
	{
		return GetTextFromID(m_Statements[stmtGetAuthor], id);
	}

	virtual string GetDescription(unsigned __int64 id) override
	{
		return GetTextFromID(m_Statements[stmtGetDescription], id);
	}

	virtual bool GetMinFarVersion(unsigned __int64 id, VersionInfo *Version) override
	{
		bool b = m_Statements[stmtGetMinFarVersion].Bind(id).Step();
		if (b)
		{
			const char *blob = m_Statements[stmtGetMinFarVersion].GetColBlob(0);
			size_t realsize = m_Statements[stmtGetMinFarVersion].GetColBytes(0);
			memcpy(Version, blob, std::min(realsize, sizeof(VersionInfo)));
		}
		m_Statements[stmtGetMinFarVersion].Reset();
		return b;
	}

	virtual bool GetVersion(unsigned __int64 id, VersionInfo *Version) override
	{
		bool b = m_Statements[stmtGetVersion].Bind(id).Step();
		if (b)
		{
			const char *blob = m_Statements[stmtGetVersion].GetColBlob(0);
			size_t realsize = m_Statements[stmtGetVersion].GetColBytes(0);
			memcpy(Version, blob, std::min(realsize, sizeof(VersionInfo)));
		}
		m_Statements[stmtGetVersion].Reset();
		return b;
	}

	virtual bool GetDiskMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid) override
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	virtual bool GetPluginsMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid) override
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	virtual bool GetPluginsConfigMenuItem(unsigned __int64 id, size_t index, string &Text, string &Guid) override
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	virtual string GetCommandPrefix(unsigned __int64 id) override
	{
		return GetTextFromID(m_Statements[stmtGetPrefix], id);
	}

	virtual unsigned __int64 GetFlags(unsigned __int64 id) override
	{
		unsigned __int64 flags = 0;
		if (m_Statements[stmtGetFlags].Bind(id).Step())
			flags = m_Statements[stmtGetFlags].GetColInt64(0);
		m_Statements[stmtGetFlags].Reset();
		return flags;
	}

	virtual bool SetPreload(unsigned __int64 id, bool Preload) override
	{
		return m_Statements[stmtSetPreloadState].Bind(id, Preload).StepAndReset();
	}

	virtual bool SetSignature(unsigned __int64 id, const string& Signature) override
	{
		return m_Statements[stmtSetSignature].Bind(id, Signature).StepAndReset();
	}

	virtual bool SetDiskMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid) override
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	virtual bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid) override
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	virtual bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const string& Text, const string& Guid) override
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	virtual bool SetCommandPrefix(unsigned __int64 id, const string& Prefix) override
	{
		return m_Statements[stmtSetPrefix].Bind(id, Prefix).StepAndReset();
	}

	virtual bool SetFlags(unsigned __int64 id, unsigned __int64 Flags) override
	{
		return m_Statements[stmtSetFlags].Bind(id, Flags).StepAndReset();
	}

	virtual bool SetExport(unsigned __int64 id, const string& ExportName, bool Exists) override
	{
		return m_Statements[stmtSetExportState].Bind(id, ExportName, Exists).StepAndReset();
	}

	virtual bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version) override
	{
		return m_Statements[stmtSetMinFarVersion].Bind(id, blob(Version, sizeof(*Version))).StepAndReset();
	}

	virtual bool SetVersion(unsigned __int64 id, const VersionInfo *Version) override
	{
		return m_Statements[stmtSetVersion].Bind(id, blob(Version, sizeof(*Version))).StepAndReset();
	}

	virtual bool SetGuid(unsigned __int64 id, const string& Guid) override
	{
		return m_Statements[stmtSetGuid].Bind(id, Guid).StepAndReset();
	}

	virtual bool SetTitle(unsigned __int64 id, const string& Title) override
	{
		return m_Statements[stmtSetTitle].Bind(id, Title).StepAndReset();
	}

	virtual bool SetAuthor(unsigned __int64 id, const string& Author) override
	{
		return m_Statements[stmtSetAuthor].Bind(id, Author).StepAndReset();
	}

	virtual bool SetDescription(unsigned __int64 id, const string& Description) override
	{
		return m_Statements[stmtSetDescription].Bind(id, Description).StepAndReset();
	}

	virtual bool EnumPlugins(DWORD index, string &CacheName) override
	{
		if (index == 0)
			m_Statements[stmtEnumCache].Reset();

		if (m_Statements[stmtEnumCache].Step())
		{
			CacheName = m_Statements[stmtEnumCache].GetColText(0);
			return true;
		}

		m_Statements[stmtEnumCache].Reset();
		return false;
	}

	virtual bool DiscardCache() override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		bool ret = Exec("DELETE FROM cachename;");
		return ret;
	}

	virtual bool IsCacheEmpty() override
	{
		int count = 0;
		if (m_Statements[stmtCountCacheNames].Step())
			count = m_Statements[stmtCountCacheNames].GetColInt(0);
		m_Statements[stmtCountCacheNames].Reset();
		return count==0;
	}

private:
	enum MenuItemTypeEnum
	{
		PLUGINS_MENU,
		CONFIG_MENU,
		DRIVE_MENU
	};

	bool GetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, string &Text, string &Guid)
	{
		bool b = m_Statements[stmtGetMenuItem].Bind(id, type, index).Step();
		if (b)
		{
			Text = m_Statements[stmtGetMenuItem].GetColText(0);
			Guid = m_Statements[stmtGetMenuItem].GetColText(1);
		}
		m_Statements[stmtGetMenuItem].Reset();
		return b;
	}

	bool SetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, const string& Text, const string& Guid)
	{
		return m_Statements[stmtSetMenuItem].Bind(id, type, index, Guid, Text).StepAndReset();
	}

	static string GetTextFromID(SQLiteStmt &stmt, unsigned __int64 id)
	{
		string strText;
		if (stmt.Bind(id).Step())
			strText = stmt.GetColText(0);
		stmt.Reset();
		return strText;
	}

	enum statement_id
	{
		stmtCreateCache,
		stmtFindCacheName,
		stmtDelCache,
		stmtCountCacheNames,
		stmtGetPreloadState,
		stmtGetSignature,
		stmtGetExportState,
		stmtGetGuid,
		stmtGetTitle,
		stmtGetAuthor,
		stmtGetPrefix,
		stmtGetDescription,
		stmtGetFlags,
		stmtGetMinFarVersion,
		stmtGetVersion,
		stmtSetPreloadState,
		stmtSetSignature,
		stmtSetExportState,
		stmtSetGuid,
		stmtSetTitle,
		stmtSetAuthor,
		stmtSetPrefix,
		stmtSetDescription,
		stmtSetFlags,
		stmtSetMinFarVersion,
		stmtSetVersion,
		stmtEnumCache,
		stmtGetMenuItem,
		stmtSetMenuItem,
	};
};

class PluginsHotkeysConfigDb: public PluginsHotkeysConfig, public SQLiteDb
{
public:

	PluginsHotkeysConfigDb()
	{
		Initialize(L"pluginhotkeys.db");
	}

	virtual bool BeginTransaction() override { return SQLiteDb::BeginTransaction(); }
	virtual bool EndTransaction() override { return SQLiteDb::EndTransaction(); }
	virtual bool RollbackTransaction() override { return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));"
			;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtGetHotkey, L"SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;" },
			{ stmtSetHotkey, L"INSERT OR REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);" },
			{ stmtDelHotkey, L"DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;" },
			{ stmtCheckForHotkeys, L"SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1;" },
		};

		return Open(DbName, Local)
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual ~PluginsHotkeysConfigDb() {}

	virtual bool HotkeysPresent(HotKeyTypeEnum HotKeyType) override
	{
		int count = 0;
		if (m_Statements[stmtCheckForHotkeys].Bind((int)HotKeyType).Step())
			count = m_Statements[stmtCheckForHotkeys].GetColInt(0);
		m_Statements[stmtCheckForHotkeys].Reset();
		return count!=0;
	}

	virtual string GetHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType) override
	{
		string strHotKey;
		if (m_Statements[stmtGetHotkey].Bind(PluginKey, MenuGuid, HotKeyType).Step())
			strHotKey = m_Statements[stmtGetHotkey].GetColText(0);
		m_Statements[stmtGetHotkey].Reset();
		return strHotKey;
	}

	virtual bool SetHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType, const string& HotKey) override
	{
		return m_Statements[stmtSetHotkey].Bind(PluginKey, MenuGuid, HotKeyType, HotKey).StepAndReset();
	}

	virtual bool DelHotkey(const string& PluginKey, const string& MenuGuid, HotKeyTypeEnum HotKeyType) override
	{
		return m_Statements[stmtDelHotkey].Bind(PluginKey, MenuGuid, HotKeyType).StepAndReset();
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
		FOR(const auto& e, xml_enum(root.FirstChild("pluginhotkeys"), "plugin"))
		{
			const char *key = e->Attribute("key");

			if (!key)
				continue;

			string Key = wide(key, CP_UTF8);

			FOR(const auto& se, xml_enum(*e, "hotkey"))
			{
				const char *stype = se->Attribute("menu");
				const char *guid = se->Attribute("guid");
				const char *hotkey = se->Attribute("hotkey");

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

	enum statement_id
	{
		stmtGetHotkey,
		stmtSetHotkey,
		stmtDelHotkey,
		stmtCheckForHotkeys,
	};
};

class HistoryConfigCustom: public HistoryConfig, public SQLiteDb
{
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
		unsigned int TypeHistory;
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

	bool StartThread()
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
		WorkThread = Thread(&Thread::join, &HistoryConfigCustom::ThreadProc, this);
		return true;
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

	bool AddInternal(unsigned int TypeHistory, const string& HistoryName, const string &Name, int Type, bool Lock, const string &strGuid, const string &strFile, const string &strData)
	{
		return m_Statements[stmtAdd].Bind(TypeHistory, HistoryName, Type, Lock, Name, GetCurrentUTCTimeInUI64(), strGuid, strFile, strData).StepAndReset();
	}

	bool DeleteInternal(unsigned __int64 id)
	{
		return m_Statements[stmtDel].Bind(id).StepAndReset();
	}

public:

	virtual bool BeginTransaction() override { WaitAllAsync(); return SQLiteDb::BeginTransaction(); }

	virtual bool EndTransaction() override
	{
		WorkQueue.Push(nullptr);
		WaitAllAsync();
		AsyncCommitDone.Reset();
		AsyncWork.Set();
		return true;
	}

	virtual bool RollbackTransaction() override { WaitAllAsync(); return SQLiteDb::RollbackTransaction(); }

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			//command,view,edit,folder,dialog history
			"CREATE TABLE IF NOT EXISTS history(id INTEGER PRIMARY KEY, kind INTEGER NOT NULL, key TEXT NOT NULL, type INTEGER NOT NULL, lock INTEGER NOT NULL, name TEXT NOT NULL, time INTEGER NOT NULL, guid TEXT NOT NULL, file TEXT NOT NULL, data TEXT NOT NULL);"
			"CREATE INDEX IF NOT EXISTS history_idx1 ON history (kind, key);"
			"CREATE INDEX IF NOT EXISTS history_idx2 ON history (kind, key, time);"
			"CREATE INDEX IF NOT EXISTS history_idx3 ON history (kind, key, lock DESC, time DESC);"
			"CREATE INDEX IF NOT EXISTS history_idx4 ON history (kind, key, time DESC);"
			//view,edit file positions and bookmarks history
			"CREATE TABLE IF NOT EXISTS editorposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, time INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, codepage INTEGER NOT NULL);"
			"CREATE TABLE IF NOT EXISTS editorbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES editorposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
			"CREATE INDEX IF NOT EXISTS editorposition_history_idx1 ON editorposition_history (time DESC);"
			"CREATE TABLE IF NOT EXISTS viewerposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, time INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, hex INTEGER NOT NULL, codepage INTEGER NOT NULL);"
			"CREATE TABLE IF NOT EXISTS viewerbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES viewerposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
			"CREATE INDEX IF NOT EXISTS viewerposition_history_idx1 ON viewerposition_history (time DESC);"
		;

		static const simple_pair<int, const wchar_t*> Statements[] =
		{
			{ stmtEnum, L"SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY time;" },
			{ stmtEnumDesc, L"SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC;" },
			{ stmtDel, L"DELETE FROM history WHERE id=?1;" },
			{ stmtDeleteOldUnlocked, L"DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0 AND time<?3 AND id NOT IN (SELECT id FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT ?4);" },
			{ stmtEnumLargeHistories, L"SELECT key FROM (SELECT key, num FROM (SELECT key, count(id) as num FROM history WHERE kind=?1 GROUP BY key)) WHERE num > ?2;" },
			{ stmtAdd, L"INSERT INTO history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9);" },
			{ stmtGetName, L"SELECT name FROM history WHERE id=?1;" },
			{ stmtGetNameAndType, L"SELECT name, type, guid, file, data FROM history WHERE id=?1;" },
			{ stmtGetNewestName, L"SELECT name FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT 1;" },
			{ stmtCount, L"SELECT count(id) FROM history WHERE kind=?1 AND key=?2;" },
			{ stmtDelUnlocked, L"DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0;" },
			{ stmtGetLock, L"SELECT lock FROM history WHERE id=?1;" },
			{ stmtSetLock, L"UPDATE history SET lock=?1 WHERE id=?2" },
			{ stmtGetNext, L"SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time>b.time ORDER BY a.time LIMIT 1;" },
			{ stmtGetPrev, L"SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time<b.time ORDER BY a.time DESC LIMIT 1;" },
			{ stmtGetNewest, L"SELECT id, name FROM history WHERE kind=?1 AND key=?2 ORDER BY time DESC LIMIT 1;" },
			{ stmtSetEditorPos, L"INSERT OR REPLACE INTO editorposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7);" },
			{ stmtSetEditorBookmark, L"INSERT OR REPLACE INTO editorbookmarks_history VALUES (?1,?2,?3,?4,?5,?6);" },
			{ stmtGetEditorPos, L"SELECT id, line, linepos, screenline, leftpos, codepage FROM editorposition_history WHERE name=?1;" },
			{ stmtGetEditorBookmark, L"SELECT line, linepos, screenline, leftpos FROM editorbookmarks_history WHERE pid=?1 AND num=?2;" },
			{ stmtSetViewerPos, L"INSERT OR REPLACE INTO viewerposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6);" },
			{ stmtSetViewerBookmark, L"INSERT OR REPLACE INTO viewerbookmarks_history VALUES (?1,?2,?3,?4);" },
			{ stmtGetViewerPos, L"SELECT id, filepos, leftpos, hex, codepage FROM viewerposition_history WHERE name=?1;" },
			{ stmtGetViewerBookmark, L"SELECT filepos, leftpos FROM viewerbookmarks_history WHERE pid=?1 AND num=?2;" },
			{ stmtDeleteOldEditor, L"DELETE FROM editorposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM editorposition_history ORDER BY time DESC LIMIT ?2);" },
			{ stmtDeleteOldViewer, L"DELETE FROM viewerposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM viewerposition_history ORDER BY time DESC LIMIT ?2);" },
		};

		return Open(DbName, Local, true)
		    && SetWALJournalingMode()
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		    && StartThread()
		;
	}

	virtual ~HistoryConfigCustom()
	{
		WaitAllAsync();
		StopEvent.Set();
	}

	virtual bool Delete(unsigned __int64 id) override
	{
		WaitAllAsync();
		return DeleteInternal(id);
	}

	virtual bool Enum(DWORD index, unsigned int TypeHistory, const string& HistoryName, unsigned __int64 *id, string &Name, history_record_type* Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse = false) override
	{
		WaitAllAsync();
		auto &stmt = m_Statements[Reverse? stmtEnumDesc : stmtEnum];

		if (index == 0)
			stmt.Reset().Bind(TypeHistory, transient(HistoryName));

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

	virtual bool DeleteAndAddAsync(unsigned __int64 DeleteId, unsigned int TypeHistory, const string& HistoryName, string Name, int Type, bool Lock, string &strGuid, string &strFile, string &strData) override
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

	virtual bool DeleteOldUnlocked(unsigned int TypeHistory, const string& HistoryName, int DaysToKeep, int MinimumEntries) override
	{
		WaitAllAsync();
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= DaysToUI64(DaysToKeep);
		return m_Statements[stmtDeleteOldUnlocked].Bind(TypeHistory, HistoryName, older, MinimumEntries).StepAndReset();
	}

	virtual bool EnumLargeHistories(DWORD index, int MinimumEntries, unsigned int TypeHistory, string &strHistoryName) override
	{
		WaitAllAsync();
		if (index == 0)
			m_Statements[stmtEnumLargeHistories].Reset().Bind(TypeHistory, MinimumEntries);

		if (m_Statements[stmtEnumLargeHistories].Step())
		{
			strHistoryName = m_Statements[stmtEnumLargeHistories].GetColText(0);
			return true;
		}

		m_Statements[stmtEnumLargeHistories].Reset();
		return false;
	}

	virtual bool GetNewest(unsigned int TypeHistory, const string& HistoryName, string& Name) override
	{
		WaitAllAsync();
		bool b = m_Statements[stmtGetNewestName].Bind(TypeHistory, HistoryName).Step();
		if (b)
		{
			Name = m_Statements[stmtGetNewestName].GetColText(0);
		}
		m_Statements[stmtGetNewestName].Reset();
		return b;
	}

	virtual bool Get(unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		bool b = m_Statements[stmtGetName].Bind(id).Step();
		if (b)
		{
			Name = m_Statements[stmtGetName].GetColText(0);
		}
		m_Statements[stmtGetName].Reset();
		return b;
	}

	virtual bool Get(unsigned __int64 id, string& Name, history_record_type* Type, string &strGuid, string &strFile, string &strData) override
	{
		WaitAllAsync();
		bool b = m_Statements[stmtGetNameAndType].Bind(id).Step();
		if (b)
		{
			Name = m_Statements[stmtGetNameAndType].GetColText(0);
			*Type = static_cast<history_record_type>(m_Statements[stmtGetNameAndType].GetColInt(1));
			strGuid = m_Statements[stmtGetNameAndType].GetColText(2);
			strFile = m_Statements[stmtGetNameAndType].GetColText(3);
			strData = m_Statements[stmtGetNameAndType].GetColText(4);
		}
		m_Statements[stmtGetNameAndType].Reset();
		return b;
	}

	virtual DWORD Count(unsigned int TypeHistory, const string& HistoryName) override
	{
		WaitAllAsync();
		DWORD c = 0;
		if (m_Statements[stmtCount].Bind((int)TypeHistory, HistoryName).Step())
		{
			 c = (DWORD) m_Statements[stmtCount].GetColInt(0);
		}
		m_Statements[stmtCount].Reset();
		return c;
	}

	virtual bool FlipLock(unsigned __int64 id) override
	{
		WaitAllAsync();
		return m_Statements[stmtSetLock].Bind(!IsLocked(id), id).StepAndReset();
	}

	virtual bool IsLocked(unsigned __int64 id) override
	{
		WaitAllAsync();
		bool l = false;
		if (m_Statements[stmtGetLock].Bind(id).Step())
		{
			 l = m_Statements[stmtGetLock].GetColInt(0) != 0;
		}
		m_Statements[stmtGetLock].Reset();
		return l;
	}

	virtual bool DeleteAllUnlocked(unsigned int TypeHistory, const string& HistoryName) override
	{
		WaitAllAsync();
		return m_Statements[stmtDelUnlocked].Bind(TypeHistory, HistoryName).StepAndReset();
	}

	virtual unsigned __int64 GetNext(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();
		unsigned __int64 nid = 0;
		if (!id)
			return nid;
		if (m_Statements[stmtGetNext].Bind(id, TypeHistory, HistoryName).Step())
		{
			nid = m_Statements[stmtGetNext].GetColInt64(0);
			Name = m_Statements[stmtGetNext].GetColText(1);
		}
		m_Statements[stmtGetNext].Reset();
		return nid;
	}

	virtual unsigned __int64 GetPrev(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (m_Statements[stmtGetNewest].Bind(TypeHistory, HistoryName).Step())
			{
				nid = m_Statements[stmtGetNewest].GetColInt64(0);
				Name = m_Statements[stmtGetNewest].GetColText(1);
			}
			m_Statements[stmtGetNewest].Reset();
			return nid;
		}
		if (m_Statements[stmtGetPrev].Bind(id, TypeHistory, HistoryName).Step())
		{
			nid = m_Statements[stmtGetPrev].GetColInt64(0);
			Name = m_Statements[stmtGetPrev].GetColText(1);
		}
		else if (Get(id, Name))
		{
			nid = id;
		}
		m_Statements[stmtGetPrev].Reset();
		return nid;
	}

	virtual unsigned __int64 CyclicGetPrev(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (m_Statements[stmtGetNewest].Bind(TypeHistory, HistoryName).Step())
			{
				nid = m_Statements[stmtGetNewest].GetColInt64(0);
				Name = m_Statements[stmtGetNewest].GetColText(1);
			}
			m_Statements[stmtGetNewest].Reset();
			return nid;
		}
		if (m_Statements[stmtGetPrev].Bind(id, TypeHistory, HistoryName).Step())
		{
			nid = m_Statements[stmtGetPrev].GetColInt64(0);
			Name = m_Statements[stmtGetPrev].GetColText(1);
		}
		m_Statements[stmtGetPrev].Reset();
		return nid;
	}

	virtual unsigned __int64 SetEditorPos(const string& Name, int Line, int LinePos, int ScreenLine, int LeftPos, uintptr_t CodePage) override
	{
		WaitCommitAsync();
		if (m_Statements[stmtSetEditorPos].Bind(Name, GetCurrentUTCTimeInUI64(), Line, LinePos, ScreenLine, LeftPos, (int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual unsigned __int64 GetEditorPos(const string& Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, uintptr_t *CodePage) override
	{
		WaitCommitAsync();
		unsigned __int64 id=0;
		if (m_Statements[stmtGetEditorPos].Bind(Name).Step())
		{
			id = m_Statements[stmtGetEditorPos].GetColInt64(0);
			*Line = m_Statements[stmtGetEditorPos].GetColInt(1);
			*LinePos = m_Statements[stmtGetEditorPos].GetColInt(2);
			*ScreenLine = m_Statements[stmtGetEditorPos].GetColInt(3);
			*LeftPos = m_Statements[stmtGetEditorPos].GetColInt(4);
			*CodePage = m_Statements[stmtGetEditorPos].GetColInt(5);
		}
		m_Statements[stmtGetEditorPos].Reset();
		return id;
	}

	virtual bool SetEditorBookmark(unsigned __int64 id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos) override
	{
		WaitCommitAsync();
		return m_Statements[stmtSetEditorBookmark].Bind(id, i, Line, LinePos, ScreenLine, LeftPos).StepAndReset();
	}

	virtual bool GetEditorBookmark(unsigned __int64 id, size_t i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos) override
	{
		WaitCommitAsync();
		bool b = m_Statements[stmtGetEditorBookmark].Bind(id, i).Step();
		if (b)
		{
			*Line = m_Statements[stmtGetEditorBookmark].GetColInt(0);
			*LinePos = m_Statements[stmtGetEditorBookmark].GetColInt(1);
			*ScreenLine = m_Statements[stmtGetEditorBookmark].GetColInt(2);
			*LeftPos = m_Statements[stmtGetEditorBookmark].GetColInt(3);
		}
		m_Statements[stmtGetEditorBookmark].Reset();
		return b;
	}

	virtual unsigned __int64 SetViewerPos(const string& Name, __int64 FilePos, __int64 LeftPos, int Hex_Wrap, uintptr_t CodePage) override
	{
		WaitCommitAsync();
		if (m_Statements[stmtSetViewerPos].Bind(Name, GetCurrentUTCTimeInUI64(), FilePos, LeftPos, Hex_Wrap, CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual unsigned __int64 GetViewerPos(const string& Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, uintptr_t *CodePage) override
	{
		WaitCommitAsync();
		unsigned __int64 id=0;
		if (m_Statements[stmtGetViewerPos].Bind(Name).Step())
		{
			id = m_Statements[stmtGetViewerPos].GetColInt64(0);
			*FilePos = m_Statements[stmtGetViewerPos].GetColInt64(1);
			*LeftPos = m_Statements[stmtGetViewerPos].GetColInt64(2);
			*Hex = m_Statements[stmtGetViewerPos].GetColInt(3);
			*CodePage = m_Statements[stmtGetViewerPos].GetColInt(4);
		}
		m_Statements[stmtGetViewerPos].Reset();
		return id;
	}

	virtual bool SetViewerBookmark(unsigned __int64 id, size_t i, __int64 FilePos, __int64 LeftPos) override
	{
		WaitCommitAsync();
		return m_Statements[stmtSetViewerBookmark].Bind(id, i, FilePos, LeftPos).StepAndReset();
	}

	virtual bool GetViewerBookmark(unsigned __int64 id, size_t i, __int64 *FilePos, __int64 *LeftPos) override
	{
		WaitCommitAsync();
		bool b = m_Statements[stmtGetViewerBookmark].Bind(id, i).Step();
		if (b)
		{
			*FilePos = m_Statements[stmtGetViewerBookmark].GetColInt64(0);
			*LeftPos = m_Statements[stmtGetViewerBookmark].GetColInt64(1);
		}
		m_Statements[stmtGetViewerBookmark].Reset();
		return b;
	}

	virtual void DeleteOldPositions(int DaysToKeep, int MinimumEntries) override
	{
		WaitCommitAsync();
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= DaysToUI64(DaysToKeep);
		m_Statements[stmtDeleteOldEditor].Bind(older, MinimumEntries).StepAndReset();
		m_Statements[stmtDeleteOldViewer].Bind(older, MinimumEntries).StepAndReset();
	}

	enum statement_id
	{
		stmtEnum,
		stmtEnumDesc,
		stmtDel,
		stmtDeleteOldUnlocked,
		stmtEnumLargeHistories,
		stmtAdd,
		stmtGetName,
		stmtGetNameAndType,
		stmtGetNewestName,
		stmtCount,
		stmtDelUnlocked,
		stmtGetLock,
		stmtSetLock,
		stmtGetNext,
		stmtGetPrev,
		stmtGetNewest,
		stmtSetEditorPos,
		stmtSetEditorBookmark,
		stmtGetEditorPos,
		stmtGetEditorBookmark,
		stmtSetViewerPos,
		stmtSetViewerBookmark,
		stmtGetViewerPos,
		stmtGetViewerBookmark,
		stmtDeleteOldEditor,
		stmtDeleteOldViewer,
	};
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
			file_ptr XmlFile(_wfopen(NTPath(def_config).data(), L"rb"));
			if (XmlFile)
			{
				m_TemplateDoc = std::make_unique<tinyxml::TiXmlDocument>();
				if (m_TemplateDoc->LoadFile(XmlFile.get()))
				{
					if (nullptr != (m_TemplateRoot = m_TemplateDoc->FirstChildElement("farconfig")))
						m_TemplateLoadState = +1;
				}
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
				FOR(const auto& i, xml_enum(root.FirstChild("pluginsconfig"), "plugin"))
				{
					const char *guid = i->Attribute("guid");
					if (guid && 0 == strcmp(guid, son))
					{
						p->Import(tinyxml::TiXmlHandle(&const_cast<tinyxml::TiXmlElement&>(*i)));
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
}

RegExp& GetRE()
{
	static RegExp re;
	re.Compile(L"/^[0-9A-F]{8}-([0-9A-F]{4}-){3}[0-9A-F]{12}$/", OP_PERLSTYLE | OP_OPTIMIZE);
	return re;
}


bool Database::Export(const string& File)
{
	file_ptr XmlFile(_wfopen(NTPath(File).data(), L"w"));

	if(!XmlFile)
		return false;

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

	return doc.SaveFile(XmlFile.get());
}

bool Database::Import(const string& File)
{
	file_ptr XmlFile(_wfopen(NTPath(File).data(), L"rb"));

	if(!XmlFile)
		return false;

	bool ret = false;

	tinyxml::TiXmlDocument doc;

	if (doc.LoadFile(XmlFile.get()))
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

			FOR(const auto& plugin, xml_enum(root.FirstChild("pluginsconfig"), "plugin"))
			{
				auto guid = plugin->Attribute("guid");
				if (!guid)
					continue;
				string Guid = wide(guid, CP_UTF8);
				Upper(Guid);

				intptr_t mc = ARRAYSIZE(m);
				if (GetRE().Match(Guid.data(), Guid.data() + Guid.size(), m, mc))
				{
					CreatePluginsConfig(Guid)->Import(tinyxml::TiXmlHandle(&const_cast<tinyxml::TiXmlElement&>(*plugin)));
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
		rc = Message(MSG_WARNING, MSG(MProblemDb),
			m_Problems,
			make_vector<string>(MSG(MShowConfigFolders), MSG(MIgnore))
			) == 0 ? +1 : -1;
	}
	return rc;
}

void Database::AddThread(Thread&& thread)
{
	m_Threads.emplace_back(std::move(thread));
	m_Threads.erase(std::remove_if(ALL_RANGE(m_Threads), std::mem_fn(&Thread::Signaled)), m_Threads.end());
}
