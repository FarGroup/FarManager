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
#include "console.hpp"
#include "language.hpp"
#include "message.hpp"
#include "synchro.hpp"
#include "regex_helpers.hpp"

class representation_source
{
public:
	representation_source(const string& File):
		m_File(File),
		m_Root(nullptr)
	{
		const auto RootName = "farconfig";

		if (const auto XmlFile = file_ptr(_wfopen(NTPath(File).data(), L"rb")))
		{
			if (m_Document.LoadFile(XmlFile.get()) == tinyxml::XML_NO_ERROR)
			{
				const auto root = m_Document.FirstChildElement(RootName);
				SetRoot(*root);
			}
		}
	}

	tinyxml::XMLHandle GetRoot() const { return m_Root; }

	void SetRoot(tinyxml::XMLHandle Root) { m_Root = Root; }

	void PrintError() const { m_Document.PrintError(); }

private:
	tinyxml::XMLDocument m_Document;
	string m_File;
	tinyxml::XMLHandle m_Root;
};

class representation_destination
{
public:
	representation_destination():
		m_Root()
	{
		const auto RootName = "farconfig";

		m_Document.SetBOM(true);
		m_Document.LinkEndChild(m_Document.NewDeclaration());
		const auto root = m_Document.NewElement(RootName);
		m_Document.LinkEndChild(root);
		SetRoot(*root);
	}

	tinyxml::XMLElement& GetRoot() { return *m_Root; }

	tinyxml::XMLElement& CreateChild(tinyxml::XMLElement& Parent, const char* Name)
	{
		const auto e = m_Document.NewElement(Name);
		Parent.LinkEndChild(e);
		return *e;
	}

	void SetRoot(tinyxml::XMLElement& Root) { m_Root = &Root; }

	bool Save(const string& File)
	{
		const file_ptr XmlFile(_wfopen(NTPath(File).data(), L"w"));
		return XmlFile && m_Document.SaveFile(XmlFile.get()) == tinyxml::XML_NO_ERROR;
	}

private:
	tinyxml::XMLDocument m_Document;
	tinyxml::XMLElement* m_Root;
};

namespace {

class xml_enum: noncopyable, public enumerator<xml_enum, const tinyxml::XMLElement*>
{
public:
	xml_enum(tinyxml::XMLHandle base, const char* name):
		m_name(name),
		m_base(base.ToNode())
	{}

	xml_enum(const tinyxml::XMLNode& base, const char* name):
		m_name(name),
		m_base(&base)
	{}

	bool get(size_t index, value_type& value)
	{
		value = index? value->NextSiblingElement(m_name) :
		        m_base? m_base->FirstChildElement(m_name) : nullptr;

		return value? true : false;
	}

private:
	const char* m_name;
	const tinyxml::XMLNode* m_base;
};

class iGeneralConfigDb: public GeneralConfig, public SQLiteDb
{
public:
	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));"
		;

		static const stmt_init_t Statements[] =
		{
			{ stmtUpdateValue, L"UPDATE general_config SET value=?3 WHERE key=?1 AND name=?2;" },
			{ stmtInsertValue, L"INSERT INTO general_config VALUES (?1,?2,?3);" },
			{ stmtGetValue, L"SELECT value FROM general_config WHERE key=?1 AND name=?2;" },
			{ stmtDelValue, L"DELETE FROM general_config WHERE key=?1 AND name=?2;" },
			{ stmtEnumValues, L"SELECT name, value FROM general_config WHERE key=?1;" },
		};
		CheckStmt<stmt_count>(Statements);

		return Open(DbName, Local)
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool SetValue(const string& Key, const string& Name, const string& Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	virtual bool SetValue(const string& Key, const string& Name, const wchar_t* Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	virtual bool SetValue(const string& Key, const string& Name, unsigned __int64 Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	virtual bool SetValue(const string& Key, const string& Name, const blob& Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	virtual bool GetValue(const string& Key, const string& Name, bool& Value, bool Default) override
	{
		long long Data = Default;
		const auto Result = GetValue(Key, Name, Data, Data);
		Value = Data != 0;
		return Result;
	}

	virtual bool GetValue(const string& Key, const string& Name, long long& Value, long long Default) override
	{
		return GetValueT<TYPE_INTEGER>(Key, Name, Value, Default, &SQLiteStmt::GetColInt64);
	}

	virtual bool GetValue(const string& Key, const string& Name, string& Value, const wchar_t* Default) override
	{
		return GetValueT<TYPE_STRING>(Key, Name, Value, Default, &SQLiteStmt::GetColText);
	}

	virtual bool GetValue(const string& Key, const string& Name, string& Value, const string& Default) override
	{
		return GetValueT<TYPE_STRING>(Key, Name, Value, Default, &SQLiteStmt::GetColText);
	}

	virtual bool DeleteValue(const string& Key, const string& Name) override
	{
		return Statement(stmtDelValue).Bind(Key, Name).StepAndReset();
	}

	virtual bool EnumValues(const string& Key, DWORD Index, string &Name, string &Value) override
	{
		return EnumValuesT(Key, Index, Name, Value, &SQLiteStmt::GetColText);
	}

	virtual bool EnumValues(const string& Key, DWORD Index, string &Name, long long& Value) override
	{
		return EnumValuesT(Key, Index, Name, Value, &SQLiteStmt::GetColInt64);
	}

	virtual void Export(representation_destination& Representation) override
	{
		auto& root = Representation.CreateChild(Representation.GetRoot(), GetKeyName());

		auto stmtEnumAllValues = create_stmt(L"SELECT key, name, value FROM general_config ORDER BY key, name;");

		while (stmtEnumAllValues.Step())
		{
			auto& e = Representation.CreateChild(root, "setting");

			e.SetAttribute("key", stmtEnumAllValues.GetColTextUTF8(0));
			e.SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(1));

			switch (stmtEnumAllValues.GetColType(2))
			{
				case TYPE_INTEGER:
					e.SetAttribute("type", "qword");
					e.SetAttribute("value", to_hex_string(stmtEnumAllValues.GetColInt64(2)).data());
					break;
				case TYPE_STRING:
					e.SetAttribute("type", "text");
					e.SetAttribute("value", stmtEnumAllValues.GetColTextUTF8(2));
					break;
				default:
				{
					e.SetAttribute("type", "hex");
					const auto Blob = stmtEnumAllValues.GetColBlob(2);
					e.SetAttribute("value", BlobToHexString(Blob.data(), Blob.size()).data());
				}
			}
		}

		stmtEnumAllValues.Reset();
	}

	virtual void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for(const auto& e: xml_enum(Representation.GetRoot().FirstChildElement(GetKeyName()), "setting"))
		{
			const auto key = e->Attribute("key");
			const auto name = e->Attribute("name");
			const auto type = e->Attribute("type");
			const auto value = e->Attribute("value");

			if (!key || !name || !type || !value)
				continue;

			const auto Key = wide(key, CP_UTF8);
			const auto Name = wide(name, CP_UTF8);

			if (!strcmp(type,"qword"))
			{
				SetValue(Key, Name, strtoull(value, nullptr, 16));
			}
			else if (!strcmp(type,"text"))
			{
				SetValue(Key, Name, wide(value, CP_UTF8));
			}
			else if (!strcmp(type,"hex"))
			{
				const auto Blob = HexStringToBlob(value);
				SetValue(Key, Name, blob(Blob.data(), Blob.size()));
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

	template<int TypeId, class getter_t, class T, class DT>
	bool GetValueT(const string& Key, const string& Name, T& Value, const DT& Default, getter_t Getter)
	{
		auto& Stmt = Statement(stmtGetValue);
		bool Result = false;
		if (Stmt.Bind(Key, Name).Step())
		{
			if (Stmt.GetColType(0) == TypeId)
			{
				Value = (Stmt.*Getter)(0);
				Result = true;
			}
			else
			{
				// TODO: log
			}
		}
		Stmt.Reset();

		if (!Result)
		{
			Value = Default;
		}
		return Result;
	}

	template<class T>
	bool SetValueT(const string& Key, const string& Name, const T Value)
	{
		const auto StmtStepAndReset = [&](statement_id StmtId) { return Statement(StmtId).Bind(Key, Name, Value).StepAndReset(); };

		bool b = StmtStepAndReset(stmtUpdateValue);
		if (!b || !Changes())
			b = StmtStepAndReset(stmtInsertValue);
		return b;
	}

	template<class T, class getter_t>
	bool EnumValuesT(const string& Key, DWORD Index, string& Name, T& Value, getter_t Getter)
	{
		auto& Stmt = Statement(stmtEnumValues);
		if (Index == 0)
			Stmt.Reset().Bind(transient(Key));

		if (Stmt.Step())
		{
			Name = Stmt.GetColText(0);
			Value = (Stmt.*Getter)(1);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	enum statement_id
	{
		stmtUpdateValue,
		stmtInsertValue,
		stmtGetValue,
		stmtDelValue,
		stmtEnumValues,

		stmt_count
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
	explicit HierarchicalConfigDb(const string& DbName, bool Local = false):
		// If a thread with same event name is running, we will open that event here
		AsyncDone(Event::manual, Event::signaled, make_name<Event>(GetPath(), GetName()).data())
	{
		// and wait for the signal
		AsyncDone.Wait();
		Initialize(DbName, Local);
	}

	virtual ~HierarchicalConfigDb() { EndTransaction(); AsyncDone.Set(); }

	virtual void AsyncFinish() override
	{
		AsyncDone.Reset();
		ConfigProvider().AddThread(Thread(&Thread::detach, &HierarchicalConfigDb::AsyncDelete, this));
	}

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS table_keys(id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
			"CREATE TABLE IF NOT EXISTS table_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id <> 0));"
			//root key (needs to be before the transaction start)
			"INSERT OR IGNORE INTO table_keys VALUES (0,0,\"\",\"Root - do not edit\");"
		;

		static const stmt_init_t Statements[] =
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
		CheckStmt<stmt_count>(Statements);

		return Open(DbName, Local)
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		    && BeginTransaction()
		;
	}

	virtual bool Flush() override
	{
		const bool b = EndTransaction();
		BeginTransaction();
		return b;
	}

	virtual key CreateKey(const key& Root, const string& Name, const string* Description) override
	{
		if (Statement(stmtCreateKey).Bind(Root.get(), Name, Description? Description->data() : nullptr).StepAndReset())
			return make_key(LastInsertRowID());

		const auto Key = FindByName(Root, Name);
		if (Key.get() && Description)
			SetKeyDescription(Key, *Description);
		return Key;
	}

	virtual key FindByName(const key& Root, const string& Name) override
	{
		auto& Stmt = Statement(stmtFindKey);
		uint64_t id = 0;
		if (Stmt.Bind(Root.get(), Name).Step())
			id = Stmt.GetColInt64(0);
		Stmt.Reset();
		return make_key(id);
	}

	virtual bool SetKeyDescription(const key& Root, const string& Description) override
	{
		return Statement(stmtSetKeyDescription).Bind(Description, Root.get()).StepAndReset();
	}

	virtual bool SetValue(const key& Root, const string& Name, const string& Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	virtual bool SetValue(const key& Root, const string& Name, const wchar_t* Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	virtual bool SetValue(const key& Root, const string& Name, unsigned long long Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	virtual bool SetValue(const key& Root, const string& Name, const blob& Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	virtual bool GetValue(const key& Root, const string& Name, unsigned long long& Value) override
	{
		return GetValueT(Root, Name, Value, &SQLiteStmt::GetColInt64);
	}

	virtual bool GetValue(const key& Root, const string& Name, string &Value) override
	{
		return GetValueT(Root, Name, Value, &SQLiteStmt::GetColText);
	}

	virtual bool GetValue(const key& Root, const string& Name, writable_blob& Value) override
	{
		return GetValueT(Root, Name, Value, &SQLiteStmt::GetColBlob);
	}

	virtual bool DeleteKeyTree(const key& Key) override
	{
		//All subtree is automatically deleted because of foreign key constraints
		return Statement(stmtDeleteTree).Bind(Key.get()).StepAndReset();
	}

	virtual bool DeleteValue(const key& Root, const string& Name) override
	{
		return Statement(stmtDelValue).Bind(Root.get(), Name).StepAndReset();
	}


	virtual bool EnumKeys(const key& Root, DWORD Index, string& Name) override
	{
		auto& Stmt = Statement(stmtEnumKeys);
		if (Index == 0)
			Stmt.Reset().Bind(Root.get());

		if (Stmt.Step())
		{
			Name = Stmt.GetColText(0);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	virtual bool EnumValues(const key& Root, DWORD Index, string& Name, DWORD& Type) override
	{
		auto& Stmt = Statement(stmtEnumValues);
		if (Index == 0)
			Stmt.Reset().Bind(Root.get());

		if (Statement(stmtEnumValues).Step())
		{
			Name = Stmt.GetColText(0);
			Type = Stmt.GetColType(1);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	virtual void SerializeBlob(const char* Name, const void* Blob, size_t Size, tinyxml::XMLElement& e)
	{
		e.SetAttribute("type", "hex");
		e.SetAttribute("value", BlobToHexString(Blob, Size).data());
	}

	virtual void Export(representation_destination& Representation) override
	{
		Export(Representation, root_key(), Representation.CreateChild(Representation.GetRoot(), "hierarchicalconfig"));
	}

	virtual std::vector<char> DeserializeBlob(const char* Name, const char* Type, const char* Value, const tinyxml::XMLElement& e)
	{
		return HexStringToBlob(Value);
	}

	virtual void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.GetRoot().FirstChildElement("hierarchicalconfig"), "key"))
		{
			Import(root_key(), *e);
		}
	}

private:
	void Export(representation_destination& Representation, const key& Key, tinyxml::XMLElement& XmlKey)
	{
		auto& Stmt = Statement(stmtEnumValues);
		Stmt.Bind(Key.get());
		while (Stmt.Step())
		{
			auto& e = Representation.CreateChild(XmlKey, "value");

			const auto name = Stmt.GetColTextUTF8(0);
			e.SetAttribute("name", name);

			switch (Stmt.GetColType(1))
			{
			case TYPE_INTEGER:
				e.SetAttribute("type", "qword");
				e.SetAttribute("value", to_hex_string(Stmt.GetColInt64(1)).data());
				break;
			case TYPE_STRING:
				e.SetAttribute("type", "text");
				e.SetAttribute("value", Stmt.GetColTextUTF8(1));
				break;
			default:
			{
				const auto Blob = Stmt.GetColBlob(1);
				SerializeBlob(name, Blob.data(), Blob.size(), e);
			}
			break;
			}
		}
		Stmt.Reset();

		auto stmtEnumSubKeys = create_stmt(L"SELECT id, name, description FROM table_keys WHERE parent_id=?1 AND id<>0;");

		stmtEnumSubKeys.Bind(Key.get());
		while (stmtEnumSubKeys.Step())
		{
			auto& e = Representation.CreateChild(XmlKey, "key");

			e.SetAttribute("name", stmtEnumSubKeys.GetColTextUTF8(1));
			const auto description = stmtEnumSubKeys.GetColTextUTF8(2);
			if (description)
				e.SetAttribute("description", description);

			Export(Representation, make_key(stmtEnumSubKeys.GetColInt64(0)), e);
		}
		stmtEnumSubKeys.Reset();
	}

	void Import(const key& root, const tinyxml::XMLElement& key)
	{
		const auto key_name = key.Attribute("name");
		if (!key_name)
			return;
		const auto KeyName = wide(key_name, CP_UTF8);
		const auto key_description = key.Attribute("description");
		string KeyDescription;
		if (key_description)
		{
			KeyDescription = wide(key_description, CP_UTF8);
		}
		const auto Key = CreateKey(root, KeyName, key_description? &KeyDescription : nullptr);
		if (!Key.get())
			return;

		for (const auto& e: xml_enum(key, "value"))
		{
			const auto name = e->Attribute("name");
			const auto type = e->Attribute("type");
			const auto value = e->Attribute("value");

			if (!name || !type)
				continue;

			string Name = wide(name, CP_UTF8);

			if (value && !strcmp(type, "qword"))
			{
				SetValue(Key, Name, strtoull(value, nullptr, 16));
			}
			else if (value && !strcmp(type, "text"))
			{
				string Value = wide(value, CP_UTF8);
				SetValue(Key, Name, Value);
			}
			else if (value && !strcmp(type, "hex"))
			{
				const auto Blob = HexStringToBlob(value);
				SetValue(Key, Name, blob(Blob.data(), Blob.size()));
			}
			else
			{
				// custom types, value is optional
				const auto Blob = DeserializeBlob(name, type, value, *e);
				SetValue(Key, Name, blob(Blob.data(), Blob.size()));
			}
		}

		for (const auto& e: xml_enum(key, "key"))
		{
			Import(Key, *e);
		}
	}

	void AsyncDelete()
	{
		delete this;
	}

	template<class T, class getter_t>
	bool GetValueT(const key& Root, const string& Name, T& Value, getter_t Getter)
	{
		auto& Stmt = Statement(stmtGetValue);
		const bool b = Stmt.Bind(Root.get(), Name).Step();
		if (b)
			Value = (Stmt.*Getter)(0);
		Stmt.Reset();
		return b;
	}

	template<class T>
	bool SetValueT(const key& Root, const string& Name, const T& Value)
	{
		return Statement(stmtSetValue).Bind(Root.get(), Name, Value).StepAndReset();
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

		stmt_count
	};

	Event AsyncDone;
};

static const std::pair<FARCOLORFLAGS, const wchar_t*> ColorFlagNames[] =
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
	virtual void SerializeBlob(const char* Name, const void* Blob, size_t Size, tinyxml::XMLElement& e) override
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
			auto& Color = *static_cast<const FarColor*>(Blob);
			e.SetAttribute("type", "color");
			e.SetAttribute("background", to_hex_string(Color.BackgroundColor).data());
			e.SetAttribute("foreground", to_hex_string(Color.ForegroundColor).data());
			e.SetAttribute("flags", Utf8String(FlagsToString(Color.Flags, ColorFlagNames)).data());
		}
		else
		{
			return HierarchicalConfigDb::SerializeBlob(Name, Blob, Size, e);
		}
	}

	virtual std::vector<char> DeserializeBlob(const char* Name, const char* Type, const char* Value, const tinyxml::XMLElement& e) override
	{
		if(!strcmp(Type, "color"))
		{
			const auto background = e.Attribute("background");
			const auto foreground = e.Attribute("foreground");
			const auto flags = e.Attribute("flags");

			std::vector<char> Blob;
			if(background && foreground && flags)
			{
				Blob.resize(sizeof(FarColor));
				auto& Color = *reinterpret_cast<FarColor*>(Blob.data());
				Color.BackgroundColor = std::strtoul(background, nullptr, 16);
				Color.ForegroundColor = std::strtoul(foreground, nullptr, 16);
				Color.Flags = StringToFlags(wide(flags, CP_UTF8), ColorFlagNames);
			}
			return Blob;
		}
		else
		{
			return HierarchicalConfigDb::DeserializeBlob(Name, Type, Value, e);
		}
	}
};

class ColorsConfigDb: public ColorsConfig, public SQLiteDb
{
public:
	ColorsConfigDb()
	{
		Initialize(L"colors.db");
	}

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS colors(name TEXT NOT NULL PRIMARY KEY, value BLOB);"
		;

		static const stmt_init_t Statements[] =
		{
			{ stmtUpdateValue, L"UPDATE colors SET value=?2 WHERE name=?1;" },
			{ stmtInsertValue, L"INSERT INTO colors VALUES (?1,?2);" },
			{ stmtGetValue, L"SELECT value FROM colors WHERE name=?1;" },
			{ stmtDelValue, L"DELETE FROM colors WHERE name=?1;" },
		};
		CheckStmt<stmt_count>(Statements);

		return Open(DbName, Local)
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool SetValue(const string& Name, const FarColor& Value) override
	{
		const auto StmtStepAndReset = [&](statement_id StmtId) { return Statement(StmtId).Bind(Name, blob(&Value, sizeof(Value))).StepAndReset(); };

		bool b = StmtStepAndReset(stmtUpdateValue);
		if (!b || !Changes())
			b = StmtStepAndReset(stmtInsertValue);
		return b;
	}

	virtual bool GetValue(const string& Name, FarColor& Value) override
	{
		auto& Stmt = Statement(stmtGetValue);
		const bool b = Stmt.Bind(Name).Step();
		if (b)
		{
			const auto Blob = Stmt.GetColBlob(0);
			if (Blob.size() != sizeof(Value))
				throw std::runtime_error("incorrect blob size");
			Value = *static_cast<const FarColor*>(Blob.data());
		}
		Stmt.Reset();
		return b;
	}

	virtual void Export(representation_destination& Representation) override
	{
		auto& root = Representation.CreateChild(Representation.GetRoot(), "colors");

		auto stmtEnumAllValues = create_stmt(L"SELECT name, value FROM colors ORDER BY name;");

		while (stmtEnumAllValues.Step())
		{
			auto& e = Representation.CreateChild(root, "object");

			e.SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(0));
			const auto Blob = stmtEnumAllValues.GetColBlob(1);
			if (Blob.size() != sizeof(FarColor))
				throw std::runtime_error("incorrect blob size");
			auto& Color = *static_cast<const FarColor*>(Blob.data());
			e.SetAttribute("background", to_hex_string(Color.BackgroundColor).data());
			e.SetAttribute("foreground", to_hex_string(Color.ForegroundColor).data());
			e.SetAttribute("flags", Utf8String(FlagsToString(Color.Flags, ColorFlagNames)).data());
		}

		stmtEnumAllValues.Reset();
	}

	virtual void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.GetRoot().FirstChildElement("colors"), "object"))
		{
			const auto name = e->Attribute("name");
			const auto background = e->Attribute("background");
			const auto foreground = e->Attribute("foreground");
			const auto flags = e->Attribute("flags");

			if (!name)
				continue;

			string Name = wide(name, CP_UTF8);

			if(background && foreground && flags)
			{
				FarColor Color = {};
				Color.BackgroundColor = std::strtoul(background, nullptr, 16);
				Color.ForegroundColor = std::strtoul(foreground, nullptr, 16);
				Color.Flags = StringToFlags(wide(flags, CP_UTF8), ColorFlagNames);
				SetValue(Name, Color);
			}
			else
			{
				Statement(stmtDelValue).Bind(Name).StepAndReset();
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

		stmt_count
	};
};

class AssociationsConfigDb: public AssociationsConfig, public SQLiteDb
{
public:
	AssociationsConfigDb()
	{
		Initialize(L"associations.db");
	}

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
		;

		static const stmt_init_t Statements[] =
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
		CheckStmt<stmt_count>(Statements);

		return Open(DbName, Local)
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool EnumMasks(DWORD Index, unsigned __int64 *id, string &strMask) override
	{
		auto& Stmt = Statement(stmtEnumMasks);
		if (Index == 0)
			Stmt.Reset();

		if (Stmt.Step())
		{
			*id = Stmt.GetColInt64(0);
			strMask = Stmt.GetColText(1);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	virtual bool EnumMasksForType(int Type, DWORD Index, unsigned __int64 *id, string &strMask) override
	{
		auto& Stmt = Statement(stmtEnumMasksForType);
		if (Index == 0)
			Stmt.Reset().Bind(Type);

		if (Stmt.Step())
		{
			*id = Stmt.GetColInt64(0);
			strMask = Stmt.GetColText(1);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	virtual bool GetMask(unsigned __int64 id, string &strMask) override
	{
		auto& Stmt = Statement(stmtGetMask);
		const bool b = Stmt.Bind(id).Step();
		if (b)
			strMask = Stmt.GetColText(0);
		Stmt.Reset();
		return b;
	}

	virtual bool GetDescription(unsigned __int64 id, string &strDescription) override
	{
		auto& Stmt = Statement(stmtGetDescription);
		const bool b = Stmt.Bind(id).Step();
		if (b)
			strDescription = Stmt.GetColText(0);
		Stmt.Reset();
		return b;
	}

	virtual bool GetCommand(unsigned __int64 id, int Type, string &strCommand, bool *Enabled = nullptr) override
	{
		auto& Stmt = Statement(stmtGetCommand);
		const bool b = Stmt.Bind(id, Type).Step();
		if (b)
		{
			strCommand = Stmt.GetColText(0);
			if (Enabled)
				*Enabled = Stmt.GetColInt(1) != 0;
		}
		Stmt.Reset();
		return b;
	}

	virtual bool SetCommand(unsigned __int64 id, int Type, const string& Command, bool Enabled) override
	{
		return Statement(stmtSetCommand).Bind(id, Type, Enabled, Command).StepAndReset();
	}

	virtual bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2) override
	{
		auto& Stmt = Statement(stmtGetWeight);
		if (Stmt.Bind(id1).Step())
		{
			const auto weight1 = Stmt.GetColInt64(0);
			Stmt.Reset();
			if (Stmt.Bind(id2).Step())
			{
				const auto weight2 = Stmt.GetColInt64(0);
				Stmt.Reset();
				return Statement(stmtSetWeight).Bind(weight1, id2).StepAndReset() && Statement(stmtSetWeight).Bind(weight2, id1).StepAndReset();
			}
		}
		Stmt.Reset();
		return false;
	}

	virtual unsigned __int64 AddType(unsigned __int64 after_id, const string& Mask, const string& Description) override
	{
		if (Statement(stmtReorder).Bind(after_id).StepAndReset() && Statement(stmtAddType).Bind(after_id, Mask, Description).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual bool UpdateType(unsigned __int64 id, const string& Mask, const string& Description) override
	{
		return Statement(stmtUpdateType).Bind(Mask, Description, id).StepAndReset();
	}

	virtual bool DelType(unsigned __int64 id) override
	{
		return Statement(stmtDelType).Bind(id).StepAndReset();
	}

	virtual void Export(representation_destination& Representation) override
	{
		auto& root = Representation.CreateChild(Representation.GetRoot(), "associations");

		auto stmtEnumAllTypes = create_stmt(L"SELECT id, mask, description FROM filetypes ORDER BY weight;");
		auto stmtEnumCommandsPerFiletype = create_stmt(L"SELECT type, enabled, command FROM commands WHERE ft_id=?1 ORDER BY type;");

		while (stmtEnumAllTypes.Step())
		{
			auto& e = Representation.CreateChild(root, "filetype");

			e.SetAttribute("mask", stmtEnumAllTypes.GetColTextUTF8(1));
			e.SetAttribute("description", stmtEnumAllTypes.GetColTextUTF8(2));

			stmtEnumCommandsPerFiletype.Bind(stmtEnumAllTypes.GetColInt64(0));
			while (stmtEnumCommandsPerFiletype.Step())
			{
				auto& se = Representation.CreateChild(e, "command");

				se.SetAttribute("type", stmtEnumCommandsPerFiletype.GetColInt(0));
				se.SetAttribute("enabled", stmtEnumCommandsPerFiletype.GetColInt(1));
				se.SetAttribute("command", stmtEnumCommandsPerFiletype.GetColTextUTF8(2));
			}
			stmtEnumCommandsPerFiletype.Reset();
		}

		stmtEnumAllTypes.Reset();
	}

	virtual void Import(const representation_source& Representation) override
	{
		auto base = Representation.GetRoot().FirstChildElement("associations");
		if (!base.ToElement())
			return;

		SCOPED_ACTION(auto)(ScopedTransaction());
		Exec("DELETE FROM filetypes;"); //delete all before importing
		unsigned __int64 id = 0;
		for (const auto& e: xml_enum(base, "filetype"))
		{
			const auto mask = e->Attribute("mask");
			const auto description = e->Attribute("description");

			if (!mask)
				continue;

			string Mask = wide(mask, CP_UTF8);
			string Description = wide(description, CP_UTF8);

			id = AddType(id, Mask, Description);
			if (!id)
				continue;

			for (const auto& se: xml_enum(*e, "command"))
			{
				const auto command = se->Attribute("command");
				if (!command)
					continue;

				int type=0;
				if (se->QueryIntAttribute("type", &type) != tinyxml::XML_NO_ERROR)
					continue;

				int enabled=0;
				if (se->QueryIntAttribute("enabled", &enabled) != tinyxml::XML_NO_ERROR)
					continue;

				SetCommand(id, type, wide(command, CP_UTF8), enabled != 0);
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

		stmt_count
	};
};

#if 1
#if   defined(_M_IA64) || defined(__ia64)|| defined(__ia64__)
#define PLATFORM_SUFFIX L"IA64"
#elif defined(_M_AMD64)|| defined(_M_X64)|| defined(__amd64)|| defined(__amd64__)|| defined(__x86_64)|| defined(__x86_64__)
#define PLATFORM_SUFFIX L"64"
#elif defined(_M_ARM)  || defined(__arm) || defined(__arm__)|| defined(_ARM_)
#define PLATFORM_SUFFIX L"ARM"
#elif defined(_M_IX86) || defined(__i386)|| defined(__i386__)
#define PLATFORM_SUFFIX L"32"
#endif
#else
#define PLATFORM_SUFFIX L""
#endif

class PluginsCacheConfigDb: public PluginsCacheConfig, public SQLiteDb
{
public:
	PluginsCacheConfigDb()
	{
		Initialize(L"plugincache" PLATFORM_SUFFIX L".db", true);
	}

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

		static const stmt_init_t Statements[] =
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
		CheckStmt<stmt_count>(Statements);

		return Open(DbName, Local, true)
		    && SetWALJournalingMode()
		    && EnableForeignKeysConstraints()
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual void Import(const representation_source&) override {}
	virtual void Export(representation_destination&) override {}

	virtual unsigned __int64 CreateCache(const string& CacheName) override
	{
		if (Statement(stmtCreateCache).Bind(CacheName).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual unsigned __int64 GetCacheID(const string& CacheName) const override
	{
		auto& Stmt = Statement(stmtFindCacheName);
		unsigned __int64 id = 0;
		if (Stmt.Bind(CacheName).Step())
			id = Stmt.GetColInt64(0);
		Stmt.Reset();
		return id;
	}

	virtual bool DeleteCache(const string& CacheName) override
	{
		//All related entries are automatically deleted because of foreign key constraints
		return Statement(stmtDelCache).Bind(CacheName).StepAndReset();
	}

	virtual bool IsPreload(unsigned __int64 id) const override
	{
		auto& Stmt = Statement(stmtGetPreloadState);
		bool preload = false;
		if (Stmt.Bind(id).Step())
			preload = Stmt.GetColInt(0) != 0;
		Stmt.Reset();
		return preload;
	}

	virtual string GetSignature(unsigned __int64 id) const override
	{
		return GetTextFromID(Statement(stmtGetSignature), id);
	}

	virtual void *GetExport(unsigned __int64 id, const string& ExportName) const override
	{
		auto& Stmt = Statement(stmtGetExportState);
		void *enabled = nullptr;
		if (Stmt.Bind(id, ExportName).Step())
			if (Stmt.GetColInt(0))
				enabled = ToPtr(1);
		Stmt.Reset();
		return enabled;
	}

	virtual string GetGuid(unsigned __int64 id) const override
	{
		return GetTextFromID(Statement(stmtGetGuid), id);
	}

	virtual string GetTitle(unsigned __int64 id) const override
	{
		return GetTextFromID(Statement(stmtGetTitle), id);
	}

	virtual string GetAuthor(unsigned __int64 id) const override
	{
		return GetTextFromID(Statement(stmtGetAuthor), id);
	}

	virtual string GetDescription(unsigned __int64 id) const override
	{
		return GetTextFromID(Statement(stmtGetDescription), id);
	}

	virtual bool GetMinFarVersion(unsigned __int64 id, VersionInfo *Version) const override
	{
		auto& Stmt = Statement(stmtGetMinFarVersion);
		const bool b = Stmt.Bind(id).Step();
		if (b)
		{
			const auto Blob = Stmt.GetColBlob(0);
			if (Blob.size() != sizeof(*Version))
				throw std::runtime_error("incorrect blob size");
			*Version = *static_cast<const VersionInfo*>(Blob.data());
		}
		Stmt.Reset();
		return b;
	}

	virtual bool GetVersion(unsigned __int64 id, VersionInfo *Version) const override
	{
		auto& Stmt = Statement(stmtGetVersion);
		const bool b = Stmt.Bind(id).Step();
		if (b)
		{
			const auto Blob = Stmt.GetColBlob(0);
			if (Blob.size() != sizeof(*Version))
				throw std::runtime_error("incorrect blob size");
			*Version = *static_cast<const VersionInfo*>(Blob.data());
		}
		Stmt.Reset();
		return b;
	}

	virtual bool GetDiskMenuItem(unsigned __int64 id, size_t index, string &Text, GUID& Guid) const override
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	virtual bool GetPluginsMenuItem(unsigned __int64 id, size_t index, string &Text, GUID& Guid) const override
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	virtual bool GetPluginsConfigMenuItem(unsigned __int64 id, size_t index, string &Text, GUID& Guid) const override
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	virtual string GetCommandPrefix(unsigned __int64 id) const override
	{
		return GetTextFromID(Statement(stmtGetPrefix), id);
	}

	virtual unsigned __int64 GetFlags(unsigned __int64 id) const override
	{
		auto& Stmt = Statement(stmtGetFlags);
		unsigned __int64 flags = 0;
		if (Stmt.Bind(id).Step())
			flags = Stmt.GetColInt64(0);
		Stmt.Reset();
		return flags;
	}

	virtual bool SetPreload(unsigned __int64 id, bool Preload) override
	{
		return Statement(stmtSetPreloadState).Bind(id, Preload).StepAndReset();
	}

	virtual bool SetSignature(unsigned __int64 id, const string& Signature) override
	{
		return Statement(stmtSetSignature).Bind(id, Signature).StepAndReset();
	}

	virtual bool SetDiskMenuItem(unsigned __int64 id, size_t index, const string& Text, const GUID& Guid) override
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	virtual bool SetPluginsMenuItem(unsigned __int64 id, size_t index, const string& Text, const GUID& Guid) override
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	virtual bool SetPluginsConfigMenuItem(unsigned __int64 id, size_t index, const string& Text, const GUID& Guid) override
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	virtual bool SetCommandPrefix(unsigned __int64 id, const string& Prefix) override
	{
		return Statement(stmtSetPrefix).Bind(id, Prefix).StepAndReset();
	}

	virtual bool SetFlags(unsigned __int64 id, unsigned __int64 Flags) override
	{
		return Statement(stmtSetFlags).Bind(id, Flags).StepAndReset();
	}

	virtual bool SetExport(unsigned __int64 id, const string& ExportName, bool Exists) override
	{
		return Statement(stmtSetExportState).Bind(id, ExportName, Exists).StepAndReset();
	}

	virtual bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version) override
	{
		return Statement(stmtSetMinFarVersion).Bind(id, blob(Version, sizeof(*Version))).StepAndReset();
	}

	virtual bool SetVersion(unsigned __int64 id, const VersionInfo *Version) override
	{
		return Statement(stmtSetVersion).Bind(id, blob(Version, sizeof(*Version))).StepAndReset();
	}

	virtual bool SetGuid(unsigned __int64 id, const string& Guid) override
	{
		return Statement(stmtSetGuid).Bind(id, Guid).StepAndReset();
	}

	virtual bool SetTitle(unsigned __int64 id, const string& Title) override
	{
		return Statement(stmtSetTitle).Bind(id, Title).StepAndReset();
	}

	virtual bool SetAuthor(unsigned __int64 id, const string& Author) override
	{
		return Statement(stmtSetAuthor).Bind(id, Author).StepAndReset();
	}

	virtual bool SetDescription(unsigned __int64 id, const string& Description) override
	{
		return Statement(stmtSetDescription).Bind(id, Description).StepAndReset();
	}

	virtual bool EnumPlugins(DWORD index, string &CacheName) const override
	{
		auto& Stmt = Statement(stmtEnumCache);
		if (index == 0)
			Stmt.Reset();

		if (Stmt.Step())
		{
			CacheName = Stmt.GetColText(0);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	virtual bool DiscardCache() override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		bool ret = Exec("DELETE FROM cachename;");
		return ret;
	}

	virtual bool IsCacheEmpty() const override
	{
		auto& Stmt = Statement(stmtCountCacheNames);
		int count = 0;
		if (Stmt.Step())
			count = Stmt.GetColInt(0);
		Stmt.Reset();
		return count==0;
	}

private:
	enum MenuItemTypeEnum
	{
		PLUGINS_MENU,
		CONFIG_MENU,
		DRIVE_MENU
	};

	bool GetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, string &Text, GUID& Guid) const
	{
		auto& Stmt = Statement(stmtGetMenuItem);
		bool b = Stmt.Bind(id, type, index).Step();
		if (b)
		{
			Text = Stmt.GetColText(0);
			b = StrToGuid(Stmt.GetColText(1), Guid);
		}
		Stmt.Reset();
		return b;
	}

	bool SetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, size_t index, const string& Text, const GUID& Guid)
	{
		return Statement(stmtSetMenuItem).Bind(id, type, index, GuidToStr(Guid), Text).StepAndReset();
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

		stmt_count
	};
};

class PluginsHotkeysConfigDb: public PluginsHotkeysConfig, public SQLiteDb
{
public:

	PluginsHotkeysConfigDb()
	{
		Initialize(L"pluginhotkeys.db");
	}

	virtual bool InitializeImpl(const string& DbName, bool Local) override
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));"
			;

		static const stmt_init_t Statements[] =
		{
			{ stmtGetHotkey, L"SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;" },
			{ stmtSetHotkey, L"INSERT OR REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);" },
			{ stmtDelHotkey, L"DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;" },
			{ stmtCheckForHotkeys, L"SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1;" },
		};
		CheckStmt<stmt_count>(Statements);

		return Open(DbName, Local)
		    && Exec(Schema)
		    && PrepareStatements(Statements)
		;
	}

	virtual bool HotkeysPresent(HotKeyTypeEnum HotKeyType) override
	{
		auto& Stmt = Statement(stmtCheckForHotkeys);
		int count = 0;
		if (Stmt.Bind((int)HotKeyType).Step())
			count = Stmt.GetColInt(0);
		Stmt.Reset();
		return count!=0;
	}

	virtual string GetHotkey(const string& PluginKey, const GUID& MenuGuid, HotKeyTypeEnum HotKeyType) override
	{
		auto& Stmt = Statement(stmtGetHotkey);
		string strHotKey;
		if (Stmt.Bind(PluginKey, GuidToStr(MenuGuid), HotKeyType).Step())
			strHotKey = Stmt.GetColText(0);
		Stmt.Reset();
		return strHotKey;
	}

	virtual bool SetHotkey(const string& PluginKey, const GUID& MenuGuid, HotKeyTypeEnum HotKeyType, const string& HotKey) override
	{
		return Statement(stmtSetHotkey).Bind(PluginKey, GuidToStr(MenuGuid), HotKeyType, HotKey).StepAndReset();
	}

	virtual bool DelHotkey(const string& PluginKey, const GUID& MenuGuid, HotKeyTypeEnum HotKeyType) override
	{
		return Statement(stmtDelHotkey).Bind(PluginKey, GuidToStr(MenuGuid), HotKeyType).StepAndReset();
	}

	virtual void Export(representation_destination& Representation) override
	{
		auto& root = Representation.CreateChild(Representation.GetRoot(), "pluginhotkeys");

		auto stmtEnumAllPluginKeys = create_stmt(L"SELECT pluginkey FROM pluginhotkeys GROUP BY pluginkey;");
		auto stmtEnumAllHotkeysPerKey = create_stmt(L"SELECT menuguid, type, hotkey FROM pluginhotkeys WHERE pluginkey=$1;");

		while (stmtEnumAllPluginKeys.Step())
		{
			auto& p = Representation.CreateChild(root, "plugin");

			string Key = stmtEnumAllPluginKeys.GetColText(0);
			p.SetAttribute("key", stmtEnumAllPluginKeys.GetColTextUTF8(0));

			stmtEnumAllHotkeysPerKey.Bind(Key);
			while (stmtEnumAllHotkeysPerKey.Step())
			{
				auto& e = Representation.CreateChild(p, "hotkey");

				const char *type;
				switch (stmtEnumAllHotkeysPerKey.GetColInt(1))
				{
					case DRIVE_MENU: type = "drive"; break;
					case CONFIG_MENU: type = "config"; break;
					default: type = "plugins";
				}
				e.SetAttribute("menu", type);
				e.SetAttribute("guid", stmtEnumAllHotkeysPerKey.GetColTextUTF8(0));
				const auto hotkey = stmtEnumAllHotkeysPerKey.GetColTextUTF8(2);
				e.SetAttribute("hotkey", NullToEmpty(hotkey));
			}
			stmtEnumAllHotkeysPerKey.Reset();
		}

		stmtEnumAllPluginKeys.Reset();
	}

	virtual void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.GetRoot().FirstChildElement("pluginhotkeys"), "plugin"))
		{
			const auto key = e->Attribute("key");

			if (!key)
				continue;

			string Key = wide(key, CP_UTF8);

			for (const auto& se: xml_enum(*e, "hotkey"))
			{
				const auto stype = se->Attribute("menu");
				const auto guid = se->Attribute("guid");
				const auto hotkey = se->Attribute("hotkey");

				GUID Guid;

				if (!stype || !guid || !StrToGuid(wide(guid, CP_UTF8), Guid))
					continue;

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

		stmt_count
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
		StopEvent = Event(Event::automatic, Event::nonsignaled);
		string EventName;
		if (GetPath() != L":memory:")
		{
			EventName = make_name<Event>(GetPath(), GetName());
		}
		AsyncDeleteAddDone = Event(Event::manual, Event::signaled, (EventName + L"_Delete").data());
		AsyncCommitDone = Event(Event::manual, Event::signaled, (EventName + L"_Commit").data());
		AllWaiter.Add(AsyncDeleteAddDone);
		AllWaiter.Add(AsyncCommitDone);
		AsyncWork = Event(Event::automatic, Event::nonsignaled);
		WorkThread = Thread(&Thread::join, &HistoryConfigCustom::ThreadProc, this);
		return true;
	}

	void ThreadProc()
	{
		MultiWaiter Waiter;
		Waiter.Add(AsyncWork);
		Waiter.Add(StopEvent);

		for (;;)
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
		return Statement(stmtAdd).Bind(TypeHistory, HistoryName, Type, Lock, Name, GetCurrentUTCTimeInUI64(), strGuid, strFile, strData).StepAndReset();
	}

	bool DeleteInternal(unsigned __int64 id)
	{
		return Statement(stmtDel).Bind(id).StepAndReset();
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
			"CREATE TABLE IF NOT EXISTS editorposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE COLLATE NOCASE, time INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, codepage INTEGER NOT NULL);"
			"CREATE TABLE IF NOT EXISTS editorbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES editorposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
			"CREATE INDEX IF NOT EXISTS editorposition_history_idx1 ON editorposition_history (time DESC);"
			"CREATE TABLE IF NOT EXISTS viewerposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE COLLATE NOCASE, time INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, hex INTEGER NOT NULL, codepage INTEGER NOT NULL);"
			"CREATE TABLE IF NOT EXISTS viewerbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES viewerposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
			"CREATE INDEX IF NOT EXISTS viewerposition_history_idx1 ON viewerposition_history (time DESC);"
		;

		static const stmt_init_t Statements[] =
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
			{ stmtGetEditorPos, L"SELECT id, line, linepos, screenline, leftpos, codepage FROM editorposition_history WHERE name=?1 COLLATE NOCASE;" },
			{ stmtGetEditorBookmark, L"SELECT line, linepos, screenline, leftpos FROM editorbookmarks_history WHERE pid=?1 AND num=?2;" },
			{ stmtSetViewerPos, L"INSERT OR REPLACE INTO viewerposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6);" },
			{ stmtSetViewerBookmark, L"INSERT OR REPLACE INTO viewerbookmarks_history VALUES (?1,?2,?3,?4);" },
			{ stmtGetViewerPos, L"SELECT id, filepos, leftpos, hex, codepage FROM viewerposition_history WHERE name=?1 COLLATE NOCASE;" },
			{ stmtGetViewerBookmark, L"SELECT filepos, leftpos FROM viewerbookmarks_history WHERE pid=?1 AND num=?2;" },
			{ stmtDeleteOldEditor, L"DELETE FROM editorposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM editorposition_history ORDER BY time DESC LIMIT ?2);" },
			{ stmtDeleteOldViewer, L"DELETE FROM viewerposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM viewerposition_history ORDER BY time DESC LIMIT ?2);" },
		};
		CheckStmt<stmt_count>(Statements);

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
		auto &stmt = Statement(Reverse? stmtEnumDesc : stmtEnum);

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
		const auto older = GetCurrentUTCTimeInUI64() - DaysToUI64(DaysToKeep);
		return Statement(stmtDeleteOldUnlocked).Bind(TypeHistory, HistoryName, older, MinimumEntries).StepAndReset();
	}

	virtual bool EnumLargeHistories(DWORD index, int MinimumEntries, unsigned int TypeHistory, string &strHistoryName) override
	{
		WaitAllAsync();
		auto& Stmt = Statement(stmtEnumLargeHistories);
		if (index == 0)
			Stmt.Reset().Bind(TypeHistory, MinimumEntries);

		if (Stmt.Step())
		{
			strHistoryName = Stmt.GetColText(0);
			return true;
		}

		Stmt.Reset();
		return false;
	}

	virtual bool GetNewest(unsigned int TypeHistory, const string& HistoryName, string& Name) override
	{
		WaitAllAsync();
		auto& Stmt = Statement(stmtGetNewestName);
		const bool b = Stmt.Bind(TypeHistory, HistoryName).Step();
		if (b)
		{
			Name = Stmt.GetColText(0);
		}
		Stmt.Reset();
		return b;
	}

	virtual bool Get(unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		auto& Stmt = Statement(stmtGetName);
		const bool b = Stmt.Bind(id).Step();
		if (b)
		{
			Name = Stmt.GetColText(0);
		}
		Stmt.Reset();
		return b;
	}

	virtual bool Get(unsigned __int64 id, string& Name, history_record_type& Type, string &strGuid, string &strFile, string &strData) override
	{
		WaitAllAsync();
		auto& Stmt = Statement(stmtGetNameAndType);
		const bool b = Stmt.Bind(id).Step();
		if (b)
		{
			Name = Stmt.GetColText(0);
			Type = static_cast<history_record_type>(Stmt.GetColInt(1));
			strGuid = Stmt.GetColText(2);
			strFile = Stmt.GetColText(3);
			strData = Stmt.GetColText(4);
		}
		Stmt.Reset();
		return b;
	}

	virtual DWORD Count(unsigned int TypeHistory, const string& HistoryName) override
	{
		WaitAllAsync();
		auto& Stmt = Statement(stmtCount);
		DWORD c = 0;
		if (Stmt.Bind((int)TypeHistory, HistoryName).Step())
		{
			c = (DWORD)Stmt.GetColInt(0);
		}
		Stmt.Reset();
		return c;
	}

	virtual bool FlipLock(unsigned __int64 id) override
	{
		WaitAllAsync();
		return Statement(stmtSetLock).Bind(!IsLocked(id), id).StepAndReset();
	}

	virtual bool IsLocked(unsigned __int64 id) override
	{
		WaitAllAsync();
		auto& Stmt = Statement(stmtGetLock);
		bool l = false;
		if (Stmt.Bind(id).Step())
		{
			l = Stmt.GetColInt(0) != 0;
		}
		Stmt.Reset();
		return l;
	}

	virtual bool DeleteAllUnlocked(unsigned int TypeHistory, const string& HistoryName) override
	{
		WaitAllAsync();
		return Statement(stmtDelUnlocked).Bind(TypeHistory, HistoryName).StepAndReset();
	}

	virtual unsigned __int64 GetNext(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();

		unsigned __int64 nid = 0;
		if (!id)
			return nid;

		auto& Stmt = Statement(stmtGetNext);
		if (Stmt.Bind(id, TypeHistory, HistoryName).Step())
		{
			nid = Stmt.GetColInt64(0);
			Name = Stmt.GetColText(1);
		}
		Stmt.Reset();
		return nid;
	}

	virtual unsigned __int64 GetPrev(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();

		auto& GetNewestStmt = Statement(stmtGetNewest);
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (GetNewestStmt.Bind(TypeHistory, HistoryName).Step())
			{
				nid = GetNewestStmt.GetColInt64(0);
				Name = GetNewestStmt.GetColText(1);
			}
			GetNewestStmt.Reset();
			return nid;
		}

		auto& GetPrevStmt = Statement(stmtGetPrev);
		if (GetPrevStmt.Bind(id, TypeHistory, HistoryName).Step())
		{
			nid = GetPrevStmt.GetColInt64(0);
			Name = GetPrevStmt.GetColText(1);
		}
		else if (Get(id, Name))
		{
			nid = id;
		}
		Statement(stmtGetPrev).Reset();
		return nid;
	}

	virtual unsigned __int64 CyclicGetPrev(unsigned int TypeHistory, const string& HistoryName, unsigned __int64 id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();

		auto& GetNewestStmt = Statement(stmtGetNewest);

		unsigned __int64 nid = 0;
		if (!id)
		{
			if (GetNewestStmt.Bind(TypeHistory, HistoryName).Step())
			{
				nid = GetNewestStmt.GetColInt64(0);
				Name = GetNewestStmt.GetColText(1);
			}
			GetNewestStmt.Reset();
			return nid;
		}

		auto& GetPrevStmt = Statement(stmtGetPrev);
		if (GetPrevStmt.Bind(id, TypeHistory, HistoryName).Step())
		{
			nid = GetPrevStmt.GetColInt64(0);
			Name = GetPrevStmt.GetColText(1);
		}
		GetPrevStmt.Reset();
		return nid;
	}

	virtual unsigned __int64 SetEditorPos(const string& Name, int Line, int LinePos, int ScreenLine, int LeftPos, uintptr_t CodePage) override
	{
		WaitCommitAsync();
		if (Statement(stmtSetEditorPos).Bind(Name, GetCurrentUTCTimeInUI64(), Line, LinePos, ScreenLine, LeftPos, (int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual unsigned __int64 GetEditorPos(const string& Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, uintptr_t *CodePage) override
	{
		WaitCommitAsync();
		auto& Stmt = Statement(stmtGetEditorPos);
		unsigned __int64 id=0;
		if (Stmt.Bind(Name).Step())
		{
			id = Stmt.GetColInt64(0);
			*Line = Stmt.GetColInt(1);
			*LinePos = Stmt.GetColInt(2);
			*ScreenLine = Stmt.GetColInt(3);
			*LeftPos = Stmt.GetColInt(4);
			*CodePage = Stmt.GetColInt(5);
		}
		Stmt.Reset();
		return id;
	}

	virtual bool SetEditorBookmark(unsigned __int64 id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos) override
	{
		WaitCommitAsync();
		return Statement(stmtSetEditorBookmark).Bind(id, i, Line, LinePos, ScreenLine, LeftPos).StepAndReset();
	}

	virtual bool GetEditorBookmark(unsigned __int64 id, size_t i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos) override
	{
		WaitCommitAsync();
		auto& Stmt = Statement(stmtGetEditorBookmark);
		const bool b = Stmt.Bind(id, i).Step();
		if (b)
		{
			*Line = Stmt.GetColInt(0);
			*LinePos = Stmt.GetColInt(1);
			*ScreenLine = Stmt.GetColInt(2);
			*LeftPos = Stmt.GetColInt(3);
		}
		Stmt.Reset();
		return b;
	}

	virtual unsigned __int64 SetViewerPos(const string& Name, __int64 FilePos, __int64 LeftPos, int Hex_Wrap, uintptr_t CodePage) override
	{
		WaitCommitAsync();
		if (Statement(stmtSetViewerPos).Bind(Name, GetCurrentUTCTimeInUI64(), FilePos, LeftPos, Hex_Wrap, CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	virtual unsigned __int64 GetViewerPos(const string& Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, uintptr_t *CodePage) override
	{
		WaitCommitAsync();
		auto& Stmt = Statement(stmtGetViewerPos);
		unsigned __int64 id=0;
		if (Stmt.Bind(Name).Step())
		{
			id = Stmt.GetColInt64(0);
			*FilePos = Stmt.GetColInt64(1);
			*LeftPos = Stmt.GetColInt64(2);
			*Hex = Stmt.GetColInt(3);
			*CodePage = Stmt.GetColInt(4);
		}
		Stmt.Reset();
		return id;
	}

	virtual bool SetViewerBookmark(unsigned __int64 id, size_t i, __int64 FilePos, __int64 LeftPos) override
	{
		WaitCommitAsync();
		return Statement(stmtSetViewerBookmark).Bind(id, i, FilePos, LeftPos).StepAndReset();
	}

	virtual bool GetViewerBookmark(unsigned __int64 id, size_t i, __int64 *FilePos, __int64 *LeftPos) override
	{
		WaitCommitAsync();
		auto& Stmt = Statement(stmtGetViewerBookmark);
		const bool b = Stmt.Bind(id, i).Step();
		if (b)
		{
			*FilePos = Stmt.GetColInt64(0);
			*LeftPos = Stmt.GetColInt64(1);
		}
		Stmt.Reset();
		return b;
	}

	virtual void DeleteOldPositions(int DaysToKeep, int MinimumEntries) override
	{
		WaitCommitAsync();
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= DaysToUI64(DaysToKeep);
		Statement(stmtDeleteOldEditor).Bind(older, MinimumEntries).StepAndReset();
		Statement(stmtDeleteOldViewer).Bind(older, MinimumEntries).StepAndReset();
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

		stmt_count
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
	virtual void Import(const representation_source&) override {}
	virtual void Export(representation_destination&) override {}
};

class HistoryConfigMemory: public HistoryConfigCustom {
public:
	HistoryConfigMemory()
	{
		Initialize(L":memory:", true);
	}

	virtual void Import(const representation_source&) override {}
	virtual void Export(representation_destination&) override {}
};

static const std::wregex& uuid_regex()
{
	static const std::wregex re(RE_BEGIN RE_ANY_UUID RE_END, std::regex::icase | std::regex::optimize);
	return re;
}

}

void config_provider::CheckDatabase(SQLiteDb *pDb)
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

void config_provider::TryImportDatabase(representable *p, const char *son, bool plugin)
{
	if (!m_TemplateSource && !Global->Opt->TemplateProfilePath.empty())
	{
		m_TemplateSource = std::make_unique<representation_source>(Global->Opt->TemplateProfilePath);
	}

	if (m_TemplateSource && m_TemplateSource->GetRoot().ToNode())
	{
		auto root = m_TemplateSource->GetRoot();

		if (!son)
		{
			p->Import(*m_TemplateSource);
		}
		else if (!plugin)
		{
			m_TemplateSource->SetRoot(root.FirstChildElement(son));
			p->Import(*m_TemplateSource);
		}
		else
		{
			for (const auto& i: xml_enum(root.FirstChildElement("pluginsconfig"), "plugin"))
			{
				const auto guid = i->Attribute("guid");
				if (guid && 0 == strcmp(guid, son))
				{
					m_TemplateSource->SetRoot(&const_cast<tinyxml::XMLElement&>(*i));
					p->Import(*m_TemplateSource);
					break;
				}
			}
		}
		m_TemplateSource->SetRoot(root);
	}
}

template<class T>
std::unique_ptr<T> config_provider::CreateDatabase(const char *son)
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
HierarchicalConfigUniquePtr config_provider::CreateHierarchicalConfig(dbcheck DbId, const string& dbn, const char *xmln, bool Local, bool plugin)
{
	auto cfg = std::make_unique<T>(dbn, Local);
	if (!CheckedDb.Check(DbId))
	{
		CheckDatabase(cfg.get());
		if (m_Mode != import_mode && cfg->IsNew())
		{
			TryImportDatabase(cfg.get(), xmln, plugin);
		}
		CheckedDb.Set(DbId);
	}
	return HierarchicalConfigUniquePtr(cfg.release());
}

enum dbcheck: int
{
	CHECK_NONE       = 0,
	CHECK_FILTERS    = BIT(0),
	CHECK_HIGHLIGHT  = BIT(1),
	CHECK_SHORTCUTS  = BIT(2),
	CHECK_PANELMODES = BIT(3),
};

HierarchicalConfigUniquePtr config_provider::CreatePluginsConfig(const string& guid, bool Local)
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_NONE, L"PluginsData\\" + guid + L".db", Utf8String(guid).data(), Local, true);
}

HierarchicalConfigUniquePtr config_provider::CreateFiltersConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_FILTERS, L"filters.db","filters");
}

HierarchicalConfigUniquePtr config_provider::CreateHighlightConfig()
{
	return CreateHierarchicalConfig<HighlightHierarchicalConfigDb>(CHECK_HIGHLIGHT, L"highlight.db","highlight");
}

HierarchicalConfigUniquePtr config_provider::CreateShortcutsConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_SHORTCUTS, L"shortcuts.db","shortcuts", true);
}

HierarchicalConfigUniquePtr config_provider::CreatePanelModeConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_PANELMODES, L"panelmodes.db","panelmodes");
}

config_provider::config_provider(mode Mode):
	m_LoadResult(SQLiteDb::library_load()),
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

config_provider::~config_provider()
{
	MultiWaiter ThreadWaiter;
	for (const auto& i: m_Threads) { ThreadWaiter.Add(i); }
	ThreadWaiter.Wait();
	SQLiteDb::library_free();
}

bool config_provider::Export(const string& File)
{
	representation_destination Representation;
	auto& root = Representation.GetRoot();
	root.SetAttribute("version", Utf8String(std::to_wstring(FAR_VERSION.Major) + L"." + std::to_wstring(FAR_VERSION.Minor) + L"." + std::to_wstring(FAR_VERSION.Build)).data());

	GeneralCfg()->Export(Representation);
	LocalGeneralCfg()->Export(Representation);
	ColorsCfg()->Export(Representation);
	AssocConfig()->Export(Representation);
	PlHotkeyCfg()->Export(Representation);
	Representation.SetRoot(Representation.CreateChild(root, "filters"));
	CreateFiltersConfig()->Export(Representation);
	Representation.SetRoot(Representation.CreateChild(root, "highlight"));
	CreateHighlightConfig()->Export(Representation);
	Representation.SetRoot(Representation.CreateChild(root, "panelmodes"));
	CreatePanelModeConfig()->Export(Representation);
	Representation.SetRoot(Representation.CreateChild(root, "shortcuts"));
	CreateShortcutsConfig()->Export(Representation);

	{ //TODO: export for local plugin settings
		auto& e = Representation.CreateChild(root, "pluginsconfig");
		os::fs::enum_file ff(Global->Opt->ProfilePath + L"\\PluginsData\\*.db");
		std::for_each(RANGE(ff, i)
		{
			i.strFileName.resize(i.strFileName.size()-3);
			ToUpper(i.strFileName);
			if (std::regex_search(i.strFileName, uuid_regex()))
			{
				auto& PluginRoot = Representation.CreateChild(e, "plugin");
				PluginRoot.SetAttribute("guid", Utf8String(i.strFileName).data());
				Representation.SetRoot(PluginRoot);
				CreatePluginsConfig(i.strFileName)->Export(Representation);
			}
		});
	}

	return Representation.Save(File);
}

bool config_provider::Import(const string& Filename)
{
	representation_source Representation(Filename);

	auto root = Representation.GetRoot();

	if (!root.ToNode())
	{
		Representation.PrintError();
		return false;
	}

	GeneralCfg()->Import(Representation);
	LocalGeneralCfg()->Import(Representation);
	ColorsCfg()->Import(Representation);
	AssocConfig()->Import(Representation);
	PlHotkeyCfg()->Import(Representation);
	Representation.SetRoot(root.FirstChildElement("filters"));
	CreateFiltersConfig()->Import(Representation);
	Representation.SetRoot(root.FirstChildElement("highlight"));
	CreateHighlightConfig()->Import(Representation);
	Representation.SetRoot(root.FirstChildElement("panelmodes"));
	CreatePanelModeConfig()->Import(Representation);
	Representation.SetRoot(root.FirstChildElement("shortcuts"));
	CreateShortcutsConfig()->Import(Representation);

	//TODO: import for local plugin settings
	for (const auto& plugin: xml_enum(root.FirstChildElement("pluginsconfig"), "plugin"))
	{
		const auto guid = plugin->Attribute("guid");
		if (!guid)
			continue;
		auto Guid = wide(guid, CP_UTF8);
		ToUpper(Guid);

		if (std::regex_search(Guid, uuid_regex()))
		{
			Representation.SetRoot(&const_cast<tinyxml::XMLElement&>(*plugin));
			CreatePluginsConfig(Guid)->Import(Representation);
		}
	}

	return true;
}

void config_provider::ClearPluginsCache()
{
	PluginsCacheConfigDb().DiscardCache();
}

int config_provider::ShowProblems()
{
	int rc = 0;
	if (!m_Problems.empty())
	{
		rc = Message(MSG_WARNING, MSG(MProblemDb),
			m_Problems,
			make_vector<string>(MSG(MShowConfigFolders), MSG(MIgnore))
			) == Message::first_button ? +1 : -1;
	}
	return rc;
}

void config_provider::AddThread(Thread&& thread)
{
	m_Threads.emplace_back(std::move(thread));
	m_Threads.erase(std::remove_if(ALL_RANGE(m_Threads), std::mem_fn(&Thread::Signaled)), m_Threads.end());
}

config_provider& ConfigProvider()
{
	return *Global->m_ConfigProvider;
}