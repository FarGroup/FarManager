﻿/*
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

#include "configdb.hpp"

#include "sqlitedb.hpp"
#include "strmix.hpp"
#include "encoding.hpp"
#include "pathmix.hpp"
#include "config.hpp"
#include "farversion.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "regex_helpers.hpp"
#include "global.hpp"

#include "platform.concurrency.hpp"
#include "platform.fs.hpp"

#include "common/bytes_view.hpp"
#include "common/scope_exit.hpp"

#include "format.hpp"
#include "tinyxml.hpp"

class representation_source
{
public:
	explicit representation_source(const string& File):
		m_Root(nullptr)
	{
		const auto RootName = "farconfig";

		const auto XmlFile = file_ptr(_wfopen(NTPath(File).c_str(), L"rb"));
		if (!XmlFile)
			return;

		if (m_Document.LoadFile(XmlFile.get()) != tinyxml::XML_SUCCESS)
			return;

		const auto root = m_Document.FirstChildElement(RootName);
		SetRoot(*root);
	}

	auto GetRoot() const { return m_Root; }

	void SetRoot(tinyxml::XMLHandle Root) { m_Root = Root; }

	string GetError() const
	{
		return encoding::utf8::get_chars(m_Document.ErrorStr());
	}

private:
	tinyxml::XMLDocument m_Document;
	tinyxml::XMLHandle m_Root;
};

static auto& CreateChild(tinyxml::XMLElement& Parent, const char* Name)
{
	const auto e = Parent.GetDocument()->NewElement(Name);
	Parent.LinkEndChild(e);
	return *e;
}

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

	auto& GetRoot() const { return *m_Root; }

	void SetRoot(tinyxml::XMLElement& Root) { m_Root = &Root; }

	bool Save(const string& File)
	{
		const file_ptr XmlFile(_wfopen(NTPath(File).c_str(), L"w"));
		return XmlFile && m_Document.SaveFile(XmlFile.get()) == tinyxml::XML_SUCCESS;
	}

private:
	tinyxml::XMLDocument m_Document;
	tinyxml::XMLElement* m_Root;
};

namespace
{

class xml_enum: noncopyable, public enumerator<xml_enum, const tinyxml::XMLElement*>
{
	IMPLEMENTS_ENUMERATOR(xml_enum);

public:
	xml_enum(const tinyxml::XMLNode& base, const char* name):
		m_name(name),
		m_base(&base)
	{
	}

	xml_enum(tinyxml::XMLHandle base, const char* name):
		xml_enum(*base.ToNode(), name)
	{
	}

private:
	bool get(bool Reset, value_type& value) const
	{
		value = !Reset? value->NextSiblingElement(m_name) :
		        m_base? m_base->FirstChildElement(m_name) : nullptr;

		return value != nullptr;
	}

	const char* m_name;
	const tinyxml::XMLNode* m_base;
};

class iGeneralConfigDb: public GeneralConfig, public SQLiteDb
{
protected:
	iGeneralConfigDb(string_view const DbName, bool Local):
		SQLiteDb(&iGeneralConfigDb::Initialise, DbName, Local)
	{
	}

private:
	static bool Initialise(const db_initialiser& Db)
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));"
		;

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtSetValue,              "REPLACE INTO general_config VALUES (?1,?2,?3);"sv },
			{ stmtGetValue,              "SELECT value FROM general_config WHERE key=?1 AND name=?2;"sv },
			{ stmtDelValue,              "DELETE FROM general_config WHERE key=?1 AND name=?2;"sv },
			{ stmtEnumValues,            "SELECT name, value FROM general_config WHERE key=?1;"sv },
		};

		return
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	bool SetValue(const string_view Key, const string_view Name, const string_view Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	bool SetValue(const string_view Key, const string_view Name, const unsigned long long Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	bool SetValue(const string_view Key, const string_view Name, const bytes_view& Value) override
	{
		return SetValueT(Key, Name, Value);
	}

	bool GetValue(const string_view Key, const string_view Name, bool& Value, const bool Default) const override
	{
		long long Data = Default;
		const auto Result = GetValue(Key, Name, Data, Data);
		Value = Data != 0;
		return Result;
	}

	bool GetValue(const string_view Key, const string_view Name, long long& Value, const long long Default) const override
	{
		return GetValueT<column_type::integer>(Key, Name, Value, Default, &SQLiteStmt::GetColInt64);
	}

	bool GetValue(const string_view Key, const string_view Name, string& Value, const wchar_t* const Default) const override
	{
		return GetValueT<column_type::string>(Key, Name, Value, Default, &SQLiteStmt::GetColText);
	}

	bool GetValue(const string_view Key, const string_view Name, string& Value, const string& Default) const override
	{
		return GetValueT<column_type::string>(Key, Name, Value, Default, &SQLiteStmt::GetColText);
	}

	bool DeleteValue(const string_view Key, const string_view Name) override
	{
		return ExecuteStatement(stmtDelValue, Key, Name);
	}

	bool EnumValues(const string_view Key, const bool Reset, string &Name, string &Value) const override
	{
		return EnumValuesT(Key, Reset, Name, Value, &SQLiteStmt::GetColText);
	}

	bool EnumValues(const string_view Key, const bool Reset, string &Name, long long& Value) const override
	{
		return EnumValuesT(Key, Reset, Name, Value, &SQLiteStmt::GetColInt64);
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.GetRoot(), GetKeyName());

		auto stmtEnumAllValues = create_stmt("SELECT key, name, value FROM general_config ORDER BY key, name;"sv);

		while (stmtEnumAllValues.Step())
		{
			auto& e = CreateChild(root, "setting");

			e.SetAttribute("key", stmtEnumAllValues.GetColTextUTF8(0).c_str());
			e.SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(1).c_str());

			switch (stmtEnumAllValues.GetColType(2))
			{
			case column_type::integer:
				e.SetAttribute("type", "qword");
				e.SetAttribute("value", to_hex_string(stmtEnumAllValues.GetColInt64(2)).c_str());
				break;

			case column_type::string:
				e.SetAttribute("type", "text");
				e.SetAttribute("value", stmtEnumAllValues.GetColTextUTF8(2).c_str());
				break;

			case column_type::blob:
			case column_type::unknown:
				{
					e.SetAttribute("type", "hex");
					const auto Blob = stmtEnumAllValues.GetColBlob(2);
					e.SetAttribute("value", BlobToHexString(Blob.data(), Blob.size()).c_str());
				}
			}
		}
	}

	void Import(const representation_source& Representation) override
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

			const auto Key = encoding::utf8::get_chars(key);
			const auto Name = encoding::utf8::get_chars(name);

			if (!strcmp(type,"qword"))
			{
				SetValue(Key, Name, strtoull(value, nullptr, 16));
				continue;
			}
			
			if (!strcmp(type,"text"))
			{
				SetValue(Key, Name, encoding::utf8::get_chars(value));
				continue;
			}
			
			if (!strcmp(type,"hex"))
			{
				SetValue(Key, Name, HexStringToBlob(value));
				continue;
			}
		}
	}

	virtual const char* GetKeyName() const = 0;

	template<column_type TypeId, class getter_t, class T, class DT>
	bool GetValueT(const string_view Key, const string_view Name, T& Value, const DT& Default, const getter_t Getter) const
	{
		const auto Stmt = AutoStatement(stmtGetValue);
		if (!Stmt->Bind(Key, Name).Step() || Stmt->GetColType(0) != TypeId)
		{
			Value = Default;
			return false;
		}

		Value = std::invoke(Getter, Stmt, 0);
		return true;
	}

	template<class T>
	bool SetValueT(const string_view Key, const string_view Name, const T Value)
	{
		return ExecuteStatement(stmtSetValue, Key, Name, Value);
	}

	template<class T, class getter_t>
	bool EnumValuesT(const string_view Key, bool Reset, string& Name, T& Value, const getter_t Getter) const
	{
		auto Stmt = AutoStatement(stmtEnumValues);

		if (Reset)
			Stmt->Reset().Bind(transient(Key));

		if (!Stmt->Step())
			return false;

		Name = Stmt->GetColText(0);
		Value = std::invoke(Getter, Stmt, 1);
		Stmt.release();
		return true;
	}

	enum statement_id
	{
		stmtSetValue,
		stmtGetValue,
		stmtDelValue,
		stmtEnumValues,

		stmt_count
	};
};

class GeneralConfigDb: public iGeneralConfigDb
{
public:
	GeneralConfigDb():iGeneralConfigDb(L"generalconfig.db"sv, false) {}

private:
	const char* GetKeyName() const override {return "generalconfig";}
};

class LocalGeneralConfigDb: public iGeneralConfigDb
{
public:
	LocalGeneralConfigDb():iGeneralConfigDb(L"localconfig.db"sv, true) {}

private:
	const char* GetKeyName() const override {return "localconfig";}
};

class async_delete_impl: virtual public async_delete
{
public:
	~async_delete_impl() override
	{
		m_AsyncDone.set();
	}

	void finish() override
	{
		m_AsyncDone.reset();
		// TODO: SEH guard, try/catch, exception_ptr
		ConfigProvider().AsyncCall([this]
		{
			delete this;
		});
	}

protected:
	explicit async_delete_impl(string_view const Name):
		// If a thread with same event name is running, we will open that event here
		m_AsyncDone(os::event::type::manual, os::event::state::signaled, Name)
	{
		// and wait for the signal
		m_AsyncDone.wait();
	}

private:
	os::event m_AsyncDone;
};

class HierarchicalConfigDb: public async_delete_impl, public HierarchicalConfig, public SQLiteDb
{
public:
	explicit HierarchicalConfigDb(string_view const DbName, bool Local):
		async_delete_impl(os::make_name<os::event>(Local? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath, DbName)),
		SQLiteDb(&HierarchicalConfigDb::Initialise, DbName, Local)
	{
		HierarchicalConfigDb::BeginTransaction();
	}

	~HierarchicalConfigDb() override
	{
		HierarchicalConfigDb::EndTransaction();
	}

protected:
	static bool Initialise(const db_initialiser& Db)
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS table_keys(id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
			"CREATE TABLE IF NOT EXISTS table_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id <> 0));"
			//root key (needs to be before the transaction start)
			"INSERT OR IGNORE INTO table_keys VALUES (0,0,\"\",\"Root - do not edit\");"
		;

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtCreateKey,             "INSERT INTO table_keys VALUES (NULL,?1,?2,?3);"sv },
			{ stmtFindKey,               "SELECT id FROM table_keys WHERE parent_id=?1 AND name=?2 AND id<>0;"sv },
			{ stmtSetKeyDescription,     "UPDATE table_keys SET description=?1 WHERE id=?2 AND id<>0 AND description<>?1;"sv },
			{ stmtSetValue,              "REPLACE INTO table_values VALUES (?1,?2,?3);"sv },
			{ stmtGetValue,              "SELECT value FROM table_values WHERE key_id=?1 AND name=?2;"sv },
			{ stmtEnumKeys,              "SELECT name FROM table_keys WHERE parent_id=?1 AND id<>0;"sv },
			{ stmtEnumValues,            "SELECT name, value FROM table_values WHERE key_id=?1;"sv },
			{ stmtDelValue,              "DELETE FROM table_values WHERE key_id=?1 AND name=?2;"sv },
			{ stmtDeleteTree,            "DELETE FROM table_keys WHERE id=?1 AND id<>0;"sv },
		};

		return
			Db.EnableForeignKeysConstraints() &&
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	bool Flush() override
	{
		const bool b = EndTransaction();
		BeginTransaction();
		return b;
	}

	key CreateKey(const key& Root, const string_view Name, const string* const Description) override
	{
		if (ExecuteStatement(stmtCreateKey, Root.get(), Name, Description))
			return key(LastInsertRowID());

		const auto Key = FindByName(Root, Name);
		if (Key.get() && Description)
			SetKeyDescription(Key, *Description);
		return Key;
	}

	key FindByName(const key& Root, const string_view Name) const override
	{
		const auto Stmt = AutoStatement(stmtFindKey);
		if (!Stmt->Bind(Root.get(), Name).Step())
			return root_key();

		return key(Stmt->GetColInt64(0));
	}

	bool SetKeyDescription(const key& Root, const string_view Description) override
	{
		return ExecuteStatement(stmtSetKeyDescription, Description, Root.get());
	}

	bool SetValue(const key& Root, const string_view Name, const string_view Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	bool SetValue(const key& Root, const string_view Name, const unsigned long long Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	bool SetValue(const key& Root, const string_view Name, const bytes_view& Value) override
	{
		return SetValueT(Root, Name, Value);
	}

	bool GetValue(const key& Root, const string_view Name, unsigned long long& Value) const override
	{
		return GetValueT(Root, Name, Value, &SQLiteStmt::GetColInt64);
	}

	bool GetValue(const key& Root, const string_view Name, string& Value) const override
	{
		return GetValueT(Root, Name, Value, &SQLiteStmt::GetColText);
	}

	bool GetValue(const key& Root, const string_view Name, bytes& Value) const override
	{
		return GetValueT(Root, Name, Value, &SQLiteStmt::GetColBlob);
	}

	bool DeleteKeyTree(const key& Key) override
	{
		//All subtree is automatically deleted because of foreign key constraints
		return ExecuteStatement(stmtDeleteTree, Key.get());
	}

	bool DeleteValue(const key& Root, const string_view Name) override
	{
		return ExecuteStatement(stmtDelValue, Root.get(), Name);
	}

	bool EnumKeys(const key& Root, const bool Reset, string& Name) const override
	{
		auto Stmt = AutoStatement(stmtEnumKeys);
	
		if (Reset)
			Stmt->Reset().Bind(Root.get());

		if (!Stmt->Step())
			return false;

		Name = Stmt->GetColText(0);
		Stmt.release();
		return true;
	}

	bool EnumValues(const key& Root, const bool Reset, string& Name, int& Type) const override
	{
		auto Stmt = AutoStatement(stmtEnumValues);

		if (Reset)
			Stmt->Reset().Bind(Root.get());

		if (!Stmt->Step())
			return false;

		Name = Stmt->GetColText(0);
		Type = static_cast<int>(Stmt->GetColType(1));
		Stmt.release();
		return true;
	}

	virtual void SerializeBlob(std::string_view /*Name*/, const bytes_view& Blob, tinyxml::XMLElement& e) const
	{
		e.SetAttribute("type", "hex");
		e.SetAttribute("value", BlobToHexString(Blob).c_str());
	}

	void Export(representation_destination& Representation) const override
	{
		Export(Representation, root_key(), CreateChild(Representation.GetRoot(), "hierarchicalconfig"));
	}

	virtual bytes DeserializeBlob(const char* Type, const char* Value, const tinyxml::XMLElement& e) const
	{
		return HexStringToBlob(Value);
	}

	void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.GetRoot().FirstChildElement("hierarchicalconfig"), "key"))
		{
			Import(root_key(), *e);
		}
	}

	void Export(representation_destination& Representation, const key& Key, tinyxml::XMLElement& XmlKey) const
	{
		{
			const auto Stmt = AutoStatement(stmtEnumValues);
			Stmt->Bind(Key.get());
			while (Stmt->Step())
			{
				auto& e = CreateChild(XmlKey, "value");

				const auto name = Stmt->GetColTextUTF8(0);
				e.SetAttribute("name", name.c_str());

				switch (Stmt->GetColType(1))
				{
				case column_type::integer:
					e.SetAttribute("type", "qword");
					e.SetAttribute("value", to_hex_string(Stmt->GetColInt64(1)).c_str());
					break;

				case column_type::string:
					e.SetAttribute("type", "text");
					e.SetAttribute("value", Stmt->GetColTextUTF8(1).c_str());
					break;

				case column_type::blob:
				case column_type::unknown:
					SerializeBlob(name, Stmt->GetColBlob(1), e);
					break;
				}
			}
		}

		auto stmtEnumSubKeys = create_stmt("SELECT id, name, description FROM table_keys WHERE parent_id=?1 AND id<>0;"sv);
		stmtEnumSubKeys.Bind(Key.get());
		while (stmtEnumSubKeys.Step())
		{
			auto& e = CreateChild(XmlKey, "key");

			e.SetAttribute("name", stmtEnumSubKeys.GetColTextUTF8(1).c_str());
			const auto description = stmtEnumSubKeys.GetColTextUTF8(2);
			if (!description.empty())
				e.SetAttribute("description", description.c_str());

			Export(Representation, key(stmtEnumSubKeys.GetColInt64(0)), e);
		}
	}

	void Import(const key& root, const tinyxml::XMLElement& key)
	{
		const auto key_name = key.Attribute("name");
		if (!key_name)
			return;
		const auto KeyName = encoding::utf8::get_chars(key_name);
		const auto key_description = key.Attribute("description");
		string KeyDescription;
		if (key_description)
		{
			KeyDescription = encoding::utf8::get_chars(key_description);
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

			const auto Name = encoding::utf8::get_chars(name);

			if (value && !strcmp(type, "qword"))
			{
				SetValue(Key, Name, strtoull(value, nullptr, 16));
			}
			else if (value && !strcmp(type, "text"))
			{
				SetValue(Key, Name, encoding::utf8::get_chars(value));
			}
			else if (value && !strcmp(type, "hex"))
			{
				SetValue(Key, Name, HexStringToBlob(value));
			}
			else
			{
				// custom types, value is optional
				SetValue(Key, Name, DeserializeBlob(type, value, *e));
			}
		}

		for (const auto& e: xml_enum(key, "key"))
		{
			Import(Key, *e);
		}
	}

	template<class T, class getter_t>
	bool GetValueT(const key& Root, const string_view Name, T& Value, const getter_t Getter) const
	{
		const auto Stmt = AutoStatement(stmtGetValue);
		if (!Stmt->Bind(Root.get(), Name).Step())
			return false;

		try
		{
			Value = std::invoke(Getter, Stmt, 0);
		}
		catch (const far_exception&)
		{
			// Blob assignment could fail if the size is incorrect
			return false;
		}

		return true;
	}

	template<class T>
	bool SetValueT(const key& Root, const string_view Name, const T& Value)
	{
		return ExecuteStatement(stmtSetValue, Root.get(), Name, Value);
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
};

static const std::pair<FARCOLORFLAGS, string_view> ColorFlagNames[] =
{
	{FCF_FG_4BIT,      L"fg4bit"sv    },
	{FCF_BG_4BIT,      L"bg4bit"sv    },
	{FCF_FG_BOLD,      L"bold"sv      },
	{FCF_FG_ITALIC,    L"italic"sv    },
	{FCF_FG_UNDERLINE, L"underline"sv },
};

class HighlightHierarchicalConfigDb: public HierarchicalConfigDb
{
public:
	using HierarchicalConfigDb::HierarchicalConfigDb;

private:
	void SerializeBlob(std::string_view const Name, const bytes_view& Blob, tinyxml::XMLElement& e) const override
	{
		static const std::string_view ColorKeys[] =
		{
			"NormalColor"sv, "SelectedColor"sv,
			"CursorColor"sv, "SelectedCursorColor"sv,
			"MarkCharNormalColor"sv, "MarkCharSelectedColor"sv,
			"MarkCharCursorColor"sv, "MarkCharSelectedCursorColor"sv,
		};

		if (std::find(ALL_CONST_RANGE(ColorKeys), Name) != std::cend(ColorKeys))
		{
			const auto Color = deserialise<FarColor>(Blob);
			e.SetAttribute("type", "color");
			e.SetAttribute("background", to_hex_string(Color.BackgroundColor).c_str());
			e.SetAttribute("foreground", to_hex_string(Color.ForegroundColor).c_str());
			e.SetAttribute("flags", encoding::utf8::get_bytes(FlagsToString(Color.Flags, ColorFlagNames)).c_str());
		}
		else
		{
			return HierarchicalConfigDb::SerializeBlob(Name, Blob, e);
		}
	}

	bytes DeserializeBlob(const char* Type, const char* Value, const tinyxml::XMLElement& e) const override
	{
		if(!strcmp(Type, "color"))
		{
			FarColor Color{};

			if (const auto background = e.Attribute("background"))
				Color.BackgroundColor = std::strtoul(background, nullptr, 16);
			if (const auto foreground = e.Attribute("foreground"))
				Color.ForegroundColor = std::strtoul(foreground, nullptr, 16);
			if (const auto flags = e.Attribute("flags"))
				Color.Flags = StringToFlags(encoding::utf8::get_chars(flags), ColorFlagNames);

			return bytes::copy(Color);
		}

		return HierarchicalConfigDb::DeserializeBlob(Type, Value, e);
	}
};

class ColorsConfigDb: public ColorsConfig, public SQLiteDb
{
public:
	ColorsConfigDb():
		SQLiteDb(&ColorsConfigDb::Initialise, L"colors.db"sv)
	{
	}

private:
	static bool Initialise(const db_initialiser& Db)
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS colors(name TEXT NOT NULL PRIMARY KEY, value BLOB);"
		;

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtSetValue,              "REPLACE INTO colors VALUES (?1,?2);"sv },
			{ stmtGetValue,              "SELECT value FROM colors WHERE name=?1;"sv },
			{ stmtDelValue,              "DELETE FROM colors WHERE name=?1;"sv },
		};

		return
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	bool SetValue(const string_view Name, const FarColor& Value) override
	{
		return ExecuteStatement(stmtSetValue, Name, bytes_view(Value));
	}

	bool GetValue(const string_view Name, FarColor& Value) const override
	{
		const auto Stmt = AutoStatement(stmtGetValue);
		if (!Stmt->Bind(Name).Step())
			return false;

		Value = deserialise<FarColor>(Stmt->GetColBlob(0));
		return true;
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.GetRoot(), "colors");

		auto stmtEnumAllValues = create_stmt("SELECT name, value FROM colors ORDER BY name;"sv);

		while (stmtEnumAllValues.Step())
		{
			auto& e = CreateChild(root, "object");

			e.SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(0).c_str());
			const auto Color = deserialise<FarColor>(stmtEnumAllValues.GetColBlob(1));
			e.SetAttribute("background", to_hex_string(Color.BackgroundColor).c_str());
			e.SetAttribute("foreground", to_hex_string(Color.ForegroundColor).c_str());
			e.SetAttribute("flags", encoding::utf8::get_bytes(FlagsToString(Color.Flags, ColorFlagNames)).c_str());
		}
	}

	void Import(const representation_source& Representation) override
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

			const auto Name = encoding::utf8::get_chars(name);

			if(background && foreground && flags)
			{
				FarColor Color = {};
				Color.BackgroundColor = std::strtoul(background, nullptr, 16);
				Color.ForegroundColor = std::strtoul(foreground, nullptr, 16);
				Color.Flags = StringToFlags(encoding::utf8::get_chars(flags), ColorFlagNames);
				SetValue(Name, Color);
			}
			else
			{
				ExecuteStatement(stmtDelValue, Name);
			}
		}
	}

	enum statement_id
	{
		stmtSetValue,
		stmtGetValue,
		stmtDelValue,

		stmt_count
	};
};

class AssociationsConfigDb: public AssociationsConfig, public SQLiteDb
{
public:
	AssociationsConfigDb():
		SQLiteDb(&AssociationsConfigDb::Initialise, L"associations.db"sv)
	{
	}

private:
	static bool Initialise(const db_initialiser& Db)
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
		;

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtReorder,               "UPDATE filetypes SET weight=weight+1 WHERE weight>(CASE ?1 WHEN 0 THEN 0 ELSE (SELECT weight FROM filetypes WHERE id=?1) END);"sv },
			{ stmtAddType,               "INSERT INTO filetypes VALUES (NULL,(CASE ?1 WHEN 0 THEN 1 ELSE (SELECT weight FROM filetypes WHERE id=?1)+1 END),?2,?3);"sv },
			{ stmtGetMask,               "SELECT mask FROM filetypes WHERE id=?1;"sv },
			{ stmtGetDescription,        "SELECT description FROM filetypes WHERE id=?1;"sv },
			{ stmtUpdateType,            "UPDATE filetypes SET mask=?1, description=?2 WHERE id=?3;"sv },
			{ stmtSetCommand,            "REPLACE INTO commands VALUES (?1,?2,?3,?4);"sv },
			{ stmtGetCommand,            "SELECT command, enabled FROM commands WHERE ft_id=?1 AND type=?2;"sv },
			{ stmtEnumTypes,             "SELECT id, description FROM filetypes ORDER BY weight;"sv },
			{ stmtEnumMasks,             "SELECT id, mask FROM filetypes ORDER BY weight;"sv },
			{ stmtEnumMasksForType,      "SELECT id, mask FROM filetypes, commands WHERE id=ft_id AND type=?1 AND enabled<>0 ORDER BY weight;"sv },
			{ stmtDelType,               "DELETE FROM filetypes WHERE id=?1;"sv },
			{ stmtGetWeight,             "SELECT weight FROM filetypes WHERE id=?1;"sv },
			{ stmtSetWeight,             "UPDATE filetypes SET weight=?1 WHERE id=?2;"sv },
		};

		return
			Db.EnableForeignKeysConstraints() &&
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	bool EnumMasks(const bool Reset, unsigned long long* const id, string& strMask) override
	{
		auto Stmt = AutoStatement(stmtEnumMasks);

		if (Reset)
			Stmt->Reset();

		if (!Stmt->Step())
			return false;

		*id = Stmt->GetColInt64(0);
		strMask = Stmt->GetColText(1);
		Stmt.release();
		return true;
	}

	bool EnumMasksForType(const bool Reset, const int Type, unsigned long long* const id, string& strMask) override
	{
		auto Stmt = AutoStatement(stmtEnumMasksForType);

		if (Reset)
			Stmt->Reset().Bind(Type);

		if (!Stmt->Step())
			return false;

		*id = Stmt->GetColInt64(0);
		strMask = Stmt->GetColText(1);
		Stmt.release();
		return true;
	}

	bool GetMask(unsigned long long id, string &strMask) override
	{
		const auto Stmt = AutoStatement(stmtGetMask);
		if (!Stmt->Bind(id).Step())
			return false;

		strMask = Stmt->GetColText(0);
		return true;
	}

	bool GetDescription(unsigned long long id, string &strDescription) override
	{
		const auto Stmt = AutoStatement(stmtGetDescription);
		if (!Stmt->Bind(id).Step())
			return false;

		strDescription = Stmt->GetColText(0);
		return true;
	}

	bool GetCommand(unsigned long long id, int Type, string &strCommand, bool *Enabled) override
	{
		const auto Stmt = AutoStatement(stmtGetCommand);
		if (!Stmt->Bind(id, Type).Step())
			return false;

		strCommand = Stmt->GetColText(0);
		if (Enabled)
			*Enabled = Stmt->GetColInt(1) != 0;
		return true;
	}

	bool SetCommand(const unsigned long long id, const int Type, const string_view Command, const bool Enabled) override
	{
		return ExecuteStatement(stmtSetCommand, id, Type, Enabled, Command);
	}

	bool SwapPositions(unsigned long long id1, unsigned long long id2) override
	{
		const auto Stmt = AutoStatement(stmtGetWeight);
		if (!Stmt->Bind(id1).Step())
			return false;

		const auto weight1 = Stmt->GetColInt64(0);
		Stmt->Reset();
		if (!Stmt->Bind(id2).Step())
			return false;

		const auto weight2 = Stmt->GetColInt64(0);
		Stmt->Reset();
		return ExecuteStatement(stmtSetWeight, weight1, id2) && ExecuteStatement(stmtSetWeight, weight2, id1);
	}

	unsigned long long AddType(const unsigned long long after_id, const string_view Mask, const string_view Description) override
	{
		return ExecuteStatement(stmtReorder, after_id) && ExecuteStatement(stmtAddType, after_id, Mask, Description)? LastInsertRowID() : 0;
	}

	bool UpdateType(const unsigned long long id, const string_view Mask, const string_view Description) override
	{
		return ExecuteStatement(stmtUpdateType, Mask, Description, id);
	}

	bool DelType(unsigned long long id) override
	{
		return ExecuteStatement(stmtDelType, id);
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.GetRoot(), "associations");

		auto stmtEnumAllTypes = create_stmt("SELECT id, mask, description FROM filetypes ORDER BY weight;"sv);
		auto stmtEnumCommandsPerFiletype = create_stmt("SELECT type, enabled, command FROM commands WHERE ft_id=?1 ORDER BY type;"sv);

		while (stmtEnumAllTypes.Step())
		{
			auto& e = CreateChild(root, "filetype");

			e.SetAttribute("mask", stmtEnumAllTypes.GetColTextUTF8(1).c_str());
			e.SetAttribute("description", stmtEnumAllTypes.GetColTextUTF8(2).c_str());

			stmtEnumCommandsPerFiletype.Bind(stmtEnumAllTypes.GetColInt64(0));
			while (stmtEnumCommandsPerFiletype.Step())
			{
				auto& se = CreateChild(e, "command");

				se.SetAttribute("type", stmtEnumCommandsPerFiletype.GetColInt(0));
				se.SetAttribute("enabled", stmtEnumCommandsPerFiletype.GetColInt(1));
				se.SetAttribute("command", stmtEnumCommandsPerFiletype.GetColTextUTF8(2).c_str());
			}
			stmtEnumCommandsPerFiletype.Reset();
		}
	}

	void Import(const representation_source& Representation) override
	{
		auto base = Representation.GetRoot().FirstChildElement("associations");
		if (!base.ToElement())
			return;

		SCOPED_ACTION(auto)(ScopedTransaction());
		Exec("DELETE FROM filetypes;"); //delete all before importing
		unsigned long long id = 0;
		for (const auto& e: xml_enum(base, "filetype"))
		{
			const auto mask = e->Attribute("mask");
			const auto description = e->Attribute("description");

			if (!mask)
				continue;

			const auto Mask = encoding::utf8::get_chars(mask);
			const auto Description = encoding::utf8::get_chars(NullToEmpty(description));

			id = AddType(id, Mask, Description);
			if (!id)
				continue;

			for (const auto& se: xml_enum(*e, "command"))
			{
				const auto command = se->Attribute("command");
				if (!command)
					continue;

				int type=0;
				if (se->QueryIntAttribute("type", &type) != tinyxml::XML_SUCCESS)
					continue;

				int enabled=0;
				if (se->QueryIntAttribute("enabled", &enabled) != tinyxml::XML_SUCCESS)
					continue;

				SetCommand(id, type, encoding::utf8::get_chars(command), enabled != 0);
			}

		}
	}

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
#if defined(_M_IA64)
#define PLATFORM_SUFFIX L"IA64"
#elif defined(_M_AMD64)
#define PLATFORM_SUFFIX L"64"
#elif defined(_M_ARM)
#define PLATFORM_SUFFIX L"ARM"
#elif defined(_M_IX86)
#define PLATFORM_SUFFIX L"32"
#endif
#else
#define PLATFORM_SUFFIX L""
#endif

class PluginsCacheConfigDb: public PluginsCacheConfig, public SQLiteDb
{
public:
	PluginsCacheConfigDb():
		SQLiteDb(&PluginsCacheConfigDb::Initialise, L"plugincache" PLATFORM_SUFFIX L".db"sv, true, true)
	{
	}
#undef PLATFORM_SUFFIX

	bool DiscardCache() override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		return Exec("DELETE FROM cachename;");
	}

private:
	static bool Initialise(const db_initialiser& Db)
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

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtCreateCache,           "INSERT INTO cachename VALUES (NULL,?1);,"sv },
			{ stmtFindCacheName,         "SELECT id FROM cachename WHERE name=?1;"sv },
			{ stmtDelCache,              "DELETE FROM cachename WHERE name=?1;"sv },
			{ stmtCountCacheNames,       "SELECT count(name) FROM cachename;"sv },
			{ stmtGetPreloadState,       "SELECT enabled FROM preload WHERE cid=?1;"sv },
			{ stmtGetSignature,          "SELECT signature FROM signatures WHERE cid=?1;"sv },
			{ stmtGetExportState,        "SELECT enabled FROM exports WHERE cid=?1 and export=?2;"sv },
			{ stmtGetGuid,               "SELECT guid FROM guids WHERE cid=?1;"sv },
			{ stmtGetTitle,              "SELECT title FROM titles WHERE cid=?1;"sv },
			{ stmtGetAuthor,             "SELECT author FROM authors WHERE cid=?1;"sv },
			{ stmtGetPrefix,             "SELECT prefix FROM prefixes WHERE cid=?1;"sv },
			{ stmtGetDescription,        "SELECT description FROM descriptions WHERE cid=?1;"sv },
			{ stmtGetFlags,              "SELECT bitmask FROM flags WHERE cid=?1;"sv },
			{ stmtGetMinFarVersion,      "SELECT version FROM minfarversions WHERE cid=?1;"sv },
			{ stmtGetVersion,            "SELECT version FROM pluginversions WHERE cid=?1;"sv },
			{ stmtSetPreloadState,       "REPLACE INTO preload VALUES (?1,?2);"sv },
			{ stmtSetSignature,          "REPLACE INTO signatures VALUES (?1,?2);"sv },
			{ stmtSetExportState,        "REPLACE INTO exports VALUES (?1,?2,?3);"sv },
			{ stmtSetGuid,               "REPLACE INTO guids VALUES (?1,?2);"sv },
			{ stmtSetTitle,              "REPLACE INTO titles VALUES (?1,?2);"sv },
			{ stmtSetAuthor,             "REPLACE INTO authors VALUES (?1,?2);"sv },
			{ stmtSetPrefix,             "REPLACE INTO prefixes VALUES (?1,?2);"sv },
			{ stmtSetDescription,        "REPLACE INTO descriptions VALUES (?1,?2);"sv },
			{ stmtSetFlags,              "REPLACE INTO flags VALUES (?1,?2);,"sv },
			{ stmtSetMinFarVersion,      "REPLACE INTO minfarversions VALUES (?1,?2);"sv },
			{ stmtSetVersion,            "REPLACE INTO pluginversions VALUES (?1,?2);"sv },
			{ stmtEnumCache,             "SELECT name FROM cachename ORDER BY name;"sv },
			{ stmtGetMenuItem,           "SELECT name, guid FROM menuitems WHERE cid=?1 AND type=?2 AND number=?3;"sv },
			{ stmtSetMenuItem,           "REPLACE INTO menuitems VALUES (?1,?2,?3,?4,?5);"sv },
		};

		return
			Db.SetWALJournalingMode() &&
			Db.EnableForeignKeysConstraints() &&
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	void Import(const representation_source&) override {}
	void Export(representation_destination&) const override {}

	unsigned long long CreateCache(const string_view CacheName) override
	{
		return ExecuteStatement(stmtCreateCache, CacheName)? LastInsertRowID() : 0;
	}

	unsigned long long GetCacheID(const string_view CacheName) const override
	{
		const auto Stmt = AutoStatement(stmtFindCacheName);
		return Stmt->Bind(CacheName).Step()?
		       Stmt->GetColInt64(0) :
		       0;
	}

	bool DeleteCache(const string_view CacheName) override
	{
		//All related entries are automatically deleted because of foreign key constraints
		return ExecuteStatement(stmtDelCache, CacheName);
	}

	bool IsPreload(unsigned long long id) const override
	{
		const auto Stmt = AutoStatement(stmtGetPreloadState);
		return Stmt->Bind(id).Step() && Stmt->GetColInt(0) != 0;
	}

	string GetSignature(unsigned long long id) const override
	{
		return GetTextFromID(stmtGetSignature, id);
	}

	bool GetExportState(const unsigned long long id, const string_view ExportName) const override
	{
		if (ExportName.empty())
			return false;

		const auto Stmt = AutoStatement(stmtGetExportState);
		return Stmt->Bind(id, ExportName).Step() && Stmt->GetColInt(0);
	}

	string GetGuid(unsigned long long id) const override
	{
		return GetTextFromID(stmtGetGuid, id);
	}

	string GetTitle(unsigned long long id) const override
	{
		return GetTextFromID(stmtGetTitle, id);
	}

	string GetAuthor(unsigned long long id) const override
	{
		return GetTextFromID(stmtGetAuthor, id);
	}

	string GetDescription(unsigned long long id) const override
	{
		return GetTextFromID(stmtGetDescription, id);
	}

	bool GetMinFarVersion(unsigned long long id, VersionInfo *Version) const override
	{
		return GetVersionImpl(stmtGetMinFarVersion, id, Version);
	}

	bool GetVersion(unsigned long long id, VersionInfo *Version) const override
	{
		return GetVersionImpl(stmtGetVersion, id, Version);
	}

	bool GetDiskMenuItem(unsigned long long id, size_t index, string &Text, GUID& Guid) const override
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool GetPluginsMenuItem(unsigned long long id, size_t index, string &Text, GUID& Guid) const override
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool GetPluginsConfigMenuItem(unsigned long long id, size_t index, string &Text, GUID& Guid) const override
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	string GetCommandPrefix(unsigned long long id) const override
	{
		return GetTextFromID(stmtGetPrefix, id);
	}

	unsigned long long GetFlags(unsigned long long id) const override
	{
		const auto Stmt = AutoStatement(stmtGetFlags);
		return Stmt->Bind(id).Step()? Stmt->GetColInt64(0) : 0;
	}

	bool SetPreload(unsigned long long id, bool Preload) override
	{
		return ExecuteStatement(stmtSetPreloadState, id, Preload);
	}

	bool SetSignature(const unsigned long long id, const string_view Signature) override
	{
		return ExecuteStatement(stmtSetSignature, id, Signature);
	}

	bool SetDiskMenuItem(const unsigned long long id, const size_t index, const string_view Text, const GUID& Guid) override
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool SetPluginsMenuItem(const unsigned long long id, const size_t index, const string_view Text, const GUID& Guid) override
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool SetPluginsConfigMenuItem(const unsigned long long id, const size_t index, const string_view Text, const GUID& Guid) override
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	bool SetCommandPrefix(const unsigned long long id, const string_view Prefix) override
	{
		return ExecuteStatement(stmtSetPrefix, id, Prefix);
	}

	bool SetFlags(unsigned long long id, unsigned long long Flags) override
	{
		return ExecuteStatement(stmtSetFlags, id, Flags);
	}

	bool SetExportState(const unsigned long long id, const string_view ExportName, const bool Exists) override
	{
		return !ExportName.empty() && ExecuteStatement(stmtSetExportState, id, ExportName, Exists);
	}

	bool SetMinFarVersion(unsigned long long id, const VersionInfo *Version) override
	{
		return ExecuteStatement(stmtSetMinFarVersion, id, bytes_view(*Version));
	}

	bool SetVersion(unsigned long long id, const VersionInfo *Version) override
	{
		return ExecuteStatement(stmtSetVersion, id, bytes_view(*Version));
	}

	bool SetGuid(const unsigned long long id, const string_view Guid) override
	{
		return ExecuteStatement(stmtSetGuid, id, Guid);
	}

	bool SetTitle(const unsigned long long id, const string_view Title) override
	{
		return ExecuteStatement(stmtSetTitle, id, Title);
	}

	bool SetAuthor(const unsigned long long id, const string_view Author) override
	{
		return ExecuteStatement(stmtSetAuthor, id, Author);
	}

	bool SetDescription(const unsigned long long id, const string_view Description) override
	{
		return ExecuteStatement(stmtSetDescription, id, Description);
	}

	bool EnumPlugins(DWORD index, string &CacheName) const override
	{
		auto Stmt = AutoStatement(stmtEnumCache);
		if (index == 0)
			Stmt->Reset();

		if (!Stmt->Step())
			return false;

		CacheName = Stmt->GetColText(0);
		Stmt.release();
		return true;
	}

	bool IsCacheEmpty() const override
	{
		const auto Stmt = AutoStatement(stmtCountCacheNames);
		return Stmt->Step() && Stmt->GetColInt(0) == 0;
	}

	enum MenuItemTypeEnum
	{
		PLUGINS_MENU,
		CONFIG_MENU,
		DRIVE_MENU
	};

	bool GetMenuItem(unsigned long long id, MenuItemTypeEnum type, size_t index, string &Text, GUID& Guid) const
	{
		const auto Stmt = AutoStatement(stmtGetMenuItem);
		if (!Stmt->Bind(id, type, index).Step())
			return false;

		Text = Stmt->GetColText(0);
		return StrToGuid(Stmt->GetColText(1), Guid);
	}

	bool SetMenuItem(const unsigned long long id, const MenuItemTypeEnum type, const size_t index, const string_view Text, const GUID& Guid) const
	{
		return ExecuteStatement(stmtSetMenuItem, id, type, index, GuidToStr(Guid), Text);
	}

	string GetTextFromID(size_t StatementIndex, unsigned long long id) const
	{
		auto Stmt = AutoStatement(StatementIndex);
		return Stmt->Bind(id).Step()? Stmt->GetColText(0) : string{};
	}

	bool GetVersionImpl(size_t StatementIndex, unsigned long long id, VersionInfo *Version) const
	{
		const auto Stmt = AutoStatement(StatementIndex);
		if (!Stmt->Bind(id).Step())
			return false;

		*Version = deserialise<VersionInfo>(Stmt->GetColBlob(0));
		return true;
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
	PluginsHotkeysConfigDb():
		SQLiteDb(&PluginsHotkeysConfigDb::Initialise, L"pluginhotkeys.db"sv)
	{
	}

private:
	static bool Initialise(const db_initialiser& Db)
	{
		static const auto Schema =
			"CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));"
			;

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtGetHotkey,             "SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;"sv },
			{ stmtSetHotkey,             "REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);"sv },
			{ stmtDelHotkey,             "DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;"sv },
			{ stmtCheckForHotkeys,       "SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1;"sv },
		};

		return
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	bool HotkeysPresent(hotkey_type HotKeyType) override
	{
		const auto Stmt = AutoStatement(stmtCheckForHotkeys);
		return Stmt->Bind(as_underlying_type(HotKeyType)).Step() && Stmt->GetColInt(0);
	}

	string GetHotkey(const string_view PluginKey, const GUID& MenuGuid, const hotkey_type HotKeyType) override
	{
		const auto Stmt = AutoStatement(stmtGetHotkey);
		if (!Stmt->Bind(PluginKey, GuidToStr(MenuGuid), as_underlying_type(HotKeyType)).Step())
			return {};

		return Stmt->GetColText(0);
	}

	bool SetHotkey(const string_view PluginKey, const GUID& MenuGuid, const hotkey_type HotKeyType, const string_view HotKey) override
	{
		return ExecuteStatement(stmtSetHotkey, PluginKey, GuidToStr(MenuGuid), as_underlying_type(HotKeyType), HotKey);
	}

	bool DelHotkey(const string_view PluginKey, const GUID& MenuGuid, const hotkey_type HotKeyType) override
	{
		return ExecuteStatement(stmtDelHotkey, PluginKey, GuidToStr(MenuGuid), as_underlying_type(HotKeyType));
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.GetRoot(), "pluginhotkeys");

		auto stmtEnumAllPluginKeys = create_stmt("SELECT pluginkey FROM pluginhotkeys GROUP BY pluginkey;"sv);
		auto stmtEnumAllHotkeysPerKey = create_stmt("SELECT menuguid, type, hotkey FROM pluginhotkeys WHERE pluginkey=$1;"sv);

		while (stmtEnumAllPluginKeys.Step())
		{
			auto& p = CreateChild(root, "plugin");

			p.SetAttribute("key", stmtEnumAllPluginKeys.GetColTextUTF8(0).c_str());

			stmtEnumAllHotkeysPerKey.Bind(stmtEnumAllPluginKeys.GetColText(0));
			while (stmtEnumAllHotkeysPerKey.Step())
			{
				const char *type = nullptr;
				switch (static_cast<hotkey_type>(stmtEnumAllHotkeysPerKey.GetColInt(1)))
				{
				case hotkey_type::drive_menu: type = "drive"; break;
				case hotkey_type::config_menu: type = "config"; break;
				case hotkey_type::plugins_menu: type = "plugins"; break;
				}

				if (!type)
				{
					// TODO: log
					continue;
				}

				auto& e = CreateChild(p, "hotkey");
				e.SetAttribute("menu", type);
				e.SetAttribute("guid", stmtEnumAllHotkeysPerKey.GetColTextUTF8(0).c_str());
				e.SetAttribute("hotkey", stmtEnumAllHotkeysPerKey.GetColTextUTF8(2).c_str());
			}
			stmtEnumAllHotkeysPerKey.Reset();
		}
	}

	void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.GetRoot().FirstChildElement("pluginhotkeys"), "plugin"))
		{
			const auto key = e->Attribute("key");

			if (!key)
				continue;

			const auto Key = encoding::utf8::get_chars(key);

			for (const auto& se: xml_enum(*e, "hotkey"))
			{
				const auto stype = se->Attribute("menu");
				const auto guid = se->Attribute("guid");
				const auto hotkey = se->Attribute("hotkey");

				GUID Guid;

				if (!stype || !guid || !StrToGuid(encoding::utf8::get_chars(guid), Guid))
					continue;

				const auto Hotkey = encoding::utf8::get_chars(hotkey);

				if (!strcmp(stype,"drive"))
					SetHotkey(Key, Guid, hotkey_type::drive_menu, Hotkey);
				else if (!strcmp(stype,"config"))
					SetHotkey(Key, Guid, hotkey_type::config_menu, Hotkey);
				else if (!strcmp(stype, "plugins"))
					SetHotkey(Key, Guid, hotkey_type::plugins_menu, Hotkey);
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
public:
	HistoryConfigCustom(string_view const DbName, bool Local):
		SQLiteDb(&HistoryConfigCustom::Initialise, DbName, Local, true)
	{
		StartThread();
	}

	~HistoryConfigCustom() override
	{
		WaitAllAsync();
		StopEvent.set();
	}

private:
	os::thread WorkThread;
	os::event StopEvent;
	os::event AsyncDeleteAddDone;
	os::event AsyncCommitDone;
	os::event AsyncWork;
	os::multi_waiter AllWaiter;

	struct AsyncWorkItem
	{
		unsigned long long DeleteId;
		unsigned int TypeHistory;
		string HistoryName;
		string strName;
		int Type;
		bool Lock;
		string strGuid;
		string strFile;
		string strData;
	};

	os::synced_queue<std::unique_ptr<AsyncWorkItem>> WorkQueue;

	void WaitAllAsync() const { AllWaiter.wait(); }
	void WaitCommitAsync() const { AsyncCommitDone.wait(); }

	bool StartThread()
	{
		StopEvent = os::event(os::event::type::automatic, os::event::state::nonsignaled);
		string EventName;
		if (GetPath() != L":memory:"sv)
		{
			EventName = os::make_name<os::event>(GetPath(), GetName());
		}
		AsyncDeleteAddDone = os::event(os::event::type::manual, os::event::state::signaled, EventName + L"_Delete"sv);
		AsyncCommitDone = os::event(os::event::type::manual, os::event::state::signaled, EventName + L"_Commit"sv);
		AllWaiter.add(AsyncDeleteAddDone);
		AllWaiter.add(AsyncCommitDone);
		AsyncWork = os::event(os::event::type::automatic, os::event::state::nonsignaled);
		WorkThread = os::thread(&os::thread::join, &HistoryConfigCustom::ThreadProc, this);
		return true;
	}

	void ThreadProc()
	{
		// TODO: SEH guard, try/catch, exception_ptr
		os::multi_waiter Waiter;
		Waiter.add(AsyncWork);
		Waiter.add(StopEvent);

		for (;;)
		{
			const auto wait = Waiter.wait(os::multi_waiter::mode::any);

			if (wait != WAIT_OBJECT_0)
				break;

			bool bAddDelete=false, bCommit=false;

			{
				SCOPED_ACTION(auto)(WorkQueue.scoped_lock());

				decltype(WorkQueue)::value_type item;
				while (WorkQueue.try_pop(item))
				{
					SCOPE_EXIT{ SQLiteDb::EndTransaction(); };
					if (item) //DeleteAndAddAsync
					{
						SQLiteDb::BeginTransaction();
						if (item->DeleteId)
							DeleteInternal(item->DeleteId);
						AddInternal(item->TypeHistory, item->HistoryName, item->strName, item->Type, item->Lock, item->strGuid, item->strFile, item->strData);
						bAddDelete = true;
					}
					else // EndTransaction
					{
						bCommit = true;
					}
				}
			}
			if (bAddDelete)
				AsyncDeleteAddDone.set();
			if (bCommit)
				AsyncCommitDone.set();
		}
	}

	bool AddInternal(unsigned int TypeHistory, const string& HistoryName, const string &Name, int Type, bool Lock, const string &strGuid, const string &strFile, const string &strData) const
	{
		return ExecuteStatement(stmtAdd, TypeHistory, HistoryName, Type, Lock, Name, os::chrono::nt_clock::now().time_since_epoch().count(), strGuid, strFile, strData);
	}

	bool DeleteInternal(unsigned long long id) const
	{
		return ExecuteStatement(stmtDel, id);
	}

	unsigned long long GetPrevImpl(const unsigned int TypeHistory, const string_view HistoryName, const unsigned long long id, string& Name, const std::function<unsigned long long()>& Fallback) const
	{
		WaitAllAsync();
		Name.clear();

		if (!id)
		{
			const auto GetNewestStmt = AutoStatement(stmtGetNewest);
			if (!GetNewestStmt->Bind(TypeHistory, HistoryName).Step())
				return 0;

			Name = GetNewestStmt->GetColText(1);
			return GetNewestStmt->GetColInt64(0);
		}

		const auto GetPrevStmt = AutoStatement(stmtGetPrev);
		if (!GetPrevStmt->Bind(id, TypeHistory, HistoryName).Step())
			return Fallback();

		Name = GetPrevStmt->GetColText(1);
		return GetPrevStmt->GetColInt64(0);
	}

	bool BeginTransaction() override { WaitAllAsync(); return SQLiteDb::BeginTransaction(); }

	bool EndTransaction() override
	{
		WorkQueue.emplace(nullptr);
		WaitAllAsync();
		AsyncCommitDone.reset();
		AsyncWork.set();
		return true;
	}

	bool RollbackTransaction() override { WaitAllAsync(); return SQLiteDb::RollbackTransaction(); }

	static bool Initialise(const db_initialiser& Db)
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

		static const stmt_init<statement_id> Statements[] =
		{
			{ stmtEnum,                  "SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY time;"sv },
			{ stmtEnumDesc,              "SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC;"sv },
			{ stmtDel,                   "DELETE FROM history WHERE id=?1;"sv },
			{ stmtDeleteOldUnlocked,     "DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0 AND time<?3 AND id NOT IN (SELECT id FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT ?4);"sv },
			{ stmtEnumLargeHistories,    "SELECT key FROM (SELECT key, num FROM (SELECT key, count(id) as num FROM history WHERE kind=?1 GROUP BY key)) WHERE num > ?2;"sv },
			{ stmtAdd,                   "INSERT INTO history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9);"sv },
			{ stmtGetName,               "SELECT name FROM history WHERE id=?1;"sv },
			{ stmtGetNameAndType,        "SELECT name, type, guid, file, data FROM history WHERE id=?1;"sv },
			{ stmtGetNewestName,         "SELECT name FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT 1;"sv },
			{ stmtCount,                 "SELECT count(id) FROM history WHERE kind=?1 AND key=?2;"sv },
			{ stmtDelUnlocked,           "DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0;"sv },
			{ stmtGetLock,               "SELECT lock FROM history WHERE id=?1;"sv },
			{ stmtSetLock,               "UPDATE history SET lock=?1 WHERE id=?2"sv },
			{ stmtGetNext,               "SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time>b.time ORDER BY a.time LIMIT 1;"sv },
			{ stmtGetPrev,               "SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time<b.time ORDER BY a.time DESC LIMIT 1;"sv },
			{ stmtGetNewest,             "SELECT id, name FROM history WHERE kind=?1 AND key=?2 ORDER BY time DESC LIMIT 1;"sv },
			{ stmtSetEditorPos,          "REPLACE INTO editorposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7);"sv },
			{ stmtSetEditorBookmark,     "REPLACE INTO editorbookmarks_history VALUES (?1,?2,?3,?4,?5,?6);"sv },
			{ stmtGetEditorPos,          "SELECT id, line, linepos, screenline, leftpos, codepage FROM editorposition_history WHERE name=?1 COLLATE NOCASE;"sv },
			{ stmtGetEditorBookmark,     "SELECT line, linepos, screenline, leftpos FROM editorbookmarks_history WHERE pid=?1 AND num=?2;"sv },
			{ stmtSetViewerPos,          "REPLACE INTO viewerposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6);"sv },
			{ stmtSetViewerBookmark,     "REPLACE INTO viewerbookmarks_history VALUES (?1,?2,?3,?4);"sv },
			{ stmtGetViewerPos,          "SELECT id, filepos, leftpos, hex, codepage FROM viewerposition_history WHERE name=?1 COLLATE NOCASE;"sv },
			{ stmtGetViewerBookmark,     "SELECT filepos, leftpos FROM viewerbookmarks_history WHERE pid=?1 AND num=?2;"sv },
			{ stmtDeleteOldEditor,       "DELETE FROM editorposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM editorposition_history ORDER BY time DESC LIMIT ?2);"sv },
			{ stmtDeleteOldViewer,       "DELETE FROM viewerposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM viewerposition_history ORDER BY time DESC LIMIT ?2);"sv },
		};

		return
			Db.SetWALJournalingMode() &&
			Db.EnableForeignKeysConstraints() &&
			Db.Exec(Schema) &&
			Db.PrepareStatements(Statements)
		;
	}

	bool Delete(unsigned long long id) override
	{
		WaitAllAsync();
		return DeleteInternal(id);
	}

	bool Enum(const bool Reset, const unsigned int TypeHistory, const string_view HistoryName, unsigned long long& id, string& Name, history_record_type& Type, bool& Lock, os::chrono::time_point& Time, string& strGuid, string& strFile, string& strData, const bool Reverse) override
	{
		WaitAllAsync();
		auto Stmt = AutoStatement(Reverse? stmtEnumDesc : stmtEnum);

		if (Reset)
			Stmt->Reset().Bind(TypeHistory, transient(HistoryName));

		if (!Stmt->Step())
			return false;

		id = Stmt->GetColInt64(0);
		Name = Stmt->GetColText(1);
		Type = static_cast<history_record_type>(Stmt->GetColInt(2));
		Lock = Stmt->GetColInt(3) != 0;
		Time = os::chrono::time_point(os::chrono::duration(Stmt->GetColInt64(4)));
		strGuid = Stmt->GetColText(5);
		strFile = Stmt->GetColText(6);
		strData = Stmt->GetColText(7);
		Stmt.release();
		return true;
	}

	bool DeleteAndAddAsync(unsigned long long const DeleteId, unsigned int const TypeHistory, string_view const HistoryName, string_view const Name, int const Type, bool const Lock, string_view const Guid, string_view const File, string_view const Data) override
	{
		auto item = std::make_unique<AsyncWorkItem>();
		item->DeleteId=DeleteId;
		item->TypeHistory=TypeHistory;
		item->HistoryName = string(HistoryName);
		item->strName = string(Name);
		item->Type=Type;
		item->Lock=Lock;
		item->strGuid = string(Guid);
		item->strFile = string(File);
		item->strData = string(Data);

		WorkQueue.emplace(std::move(item));

		WaitAllAsync();
		AsyncDeleteAddDone.reset();
		AsyncWork.set();
		return true;
	}

	bool DeleteOldUnlocked(const unsigned int TypeHistory, const string_view HistoryName, const int DaysToKeep, const int MinimumEntries) override
	{
		WaitAllAsync();

		const auto older = (os::chrono::nt_clock::now() - chrono::days(DaysToKeep)).time_since_epoch().count();
		return ExecuteStatement(stmtDeleteOldUnlocked, TypeHistory, HistoryName, older, MinimumEntries);
	}

	bool EnumLargeHistories(const bool Reset, const unsigned int TypeHistory, const int MinimumEntries, string& strHistoryName) override
	{
		WaitAllAsync();
		auto Stmt = AutoStatement(stmtEnumLargeHistories);

		if (Reset)
			Stmt->Reset().Bind(TypeHistory, MinimumEntries);

		if (!Stmt->Step())
			return false;

		strHistoryName = Stmt->GetColText(0);
		Stmt.release();
		return true;
	}

	bool GetNewest(const unsigned int TypeHistory, const string_view HistoryName, string& Name) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtGetNewestName);
		if (!Stmt->Bind(TypeHistory, HistoryName).Step())
			return false;

		Name = Stmt->GetColText(0);
		return true;
	}

	bool Get(unsigned long long id, string& Name) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtGetName);
		if (!Stmt->Bind(id).Step())
			return false;

		Name = Stmt->GetColText(0);
		return true;
	}

	bool Get(unsigned long long id, string& Name, history_record_type& Type, string &strGuid, string &strFile, string &strData) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtGetNameAndType);
		if (!Stmt->Bind(id).Step())
			return false;

		Name = Stmt->GetColText(0);
		Type = static_cast<history_record_type>(Stmt->GetColInt(1));
		strGuid = Stmt->GetColText(2);
		strFile = Stmt->GetColText(3);
		strData = Stmt->GetColText(4);
		return true;
	}

	DWORD Count(const unsigned int TypeHistory, const string_view HistoryName) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtCount);
		return Stmt->Bind(TypeHistory, HistoryName).Step()? static_cast<DWORD>(Stmt-> GetColInt(0)) : 0;
	}

	bool FlipLock(unsigned long long id) override
	{
		WaitAllAsync();
		return ExecuteStatement(stmtSetLock, !IsLocked(id), id);
	}

	bool IsLocked(unsigned long long id) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtGetLock);
		return Stmt->Bind(id).Step() && Stmt->GetColInt(0) != 0;
	}

	bool DeleteAllUnlocked(const unsigned int TypeHistory, const string_view HistoryName) override
	{
		WaitAllAsync();
		return ExecuteStatement(stmtDelUnlocked, TypeHistory, HistoryName);
	}

	unsigned long long GetNext(const unsigned int TypeHistory, const string_view HistoryName, const unsigned long long id, string& Name) override
	{
		WaitAllAsync();
		Name.clear();

		if (!id)
			return 0;

		const auto Stmt = AutoStatement(stmtGetNext);
		if (!Stmt->Bind(id, TypeHistory, HistoryName).Step())
			return 0;

		Name = Stmt->GetColText(1);
		return Stmt->GetColInt64(0);
	}

	unsigned long long GetPrev(const unsigned int TypeHistory, const string_view HistoryName, const unsigned long long id, string& Name) override
	{
		return GetPrevImpl(TypeHistory, HistoryName, id, Name, [&]() { return Get(id, Name)? id : 0; });
	}

	unsigned long long CyclicGetPrev(const unsigned int TypeHistory, const string_view HistoryName, const unsigned long long id, string& Name) override
	{
		return GetPrevImpl(TypeHistory, HistoryName, id, Name, [&]() { return 0; });
	}

	unsigned long long SetEditorPos(const string_view Name, const int Line, const int LinePos, const int ScreenLine, const int LeftPos, const uintptr_t CodePage) override
	{
		WaitCommitAsync();
		return ExecuteStatement(stmtSetEditorPos, Name, os::chrono::nt_clock::now().time_since_epoch().count(), Line, LinePos, ScreenLine, LeftPos, CodePage)? LastInsertRowID() : 0;
	}

	unsigned long long GetEditorPos(const string_view Name, int* const Line, int* const LinePos, int* const ScreenLine, int* const LeftPos, uintptr_t* const CodePage) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetEditorPos);
		if (!Stmt->Bind(Name).Step())
			return 0;

		*Line = Stmt->GetColInt(1);
		*LinePos = Stmt->GetColInt(2);
		*ScreenLine = Stmt->GetColInt(3);
		*LeftPos = Stmt->GetColInt(4);
		*CodePage = Stmt->GetColInt(5);
		return Stmt->GetColInt64(0);
	}

	bool SetEditorBookmark(unsigned long long id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos) override
	{
		WaitCommitAsync();
		return ExecuteStatement(stmtSetEditorBookmark, id, i, Line, LinePos, ScreenLine, LeftPos);
	}

	bool GetEditorBookmark(unsigned long long id, size_t i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetEditorBookmark);
		if (!Stmt->Bind(id, i).Step())
			return false;

		*Line = Stmt->GetColInt(0);
		*LinePos = Stmt->GetColInt(1);
		*ScreenLine = Stmt->GetColInt(2);
		*LeftPos = Stmt->GetColInt(3);
		return true;
	}

	unsigned long long SetViewerPos(const string_view Name, const long long FilePos, const long long LeftPos, const int Hex_Wrap, uintptr_t const CodePage) override
	{
		WaitCommitAsync();
		return ExecuteStatement(stmtSetViewerPos, Name, os::chrono::nt_clock::now().time_since_epoch().count(), FilePos, LeftPos, Hex_Wrap, CodePage)? LastInsertRowID() : 0;
	}

	unsigned long long GetViewerPos(const string_view Name, long long* const FilePos, long long* const LeftPos, int* const Hex, uintptr_t* const CodePage) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetViewerPos);

		if (!Stmt->Bind(Name).Step())
			return 0;

		*FilePos = Stmt->GetColInt64(1);
		*LeftPos = Stmt->GetColInt64(2);
		*Hex = Stmt->GetColInt(3);
		*CodePage = Stmt->GetColInt(4);
		return Stmt->GetColInt64(0);
	}

	bool SetViewerBookmark(unsigned long long id, size_t i, long long FilePos, long long LeftPos) override
	{
		WaitCommitAsync();
		return ExecuteStatement(stmtSetViewerBookmark, id, i, FilePos, LeftPos);
	}

	bool GetViewerBookmark(unsigned long long id, size_t i, long long *FilePos, long long *LeftPos) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetViewerBookmark);
		if (!Stmt->Bind(id, i).Step())
			return false;

		*FilePos = Stmt->GetColInt64(0);
		*LeftPos = Stmt->GetColInt64(1);
		return true;
	}

	void DeleteOldPositions(int DaysToKeep, int MinimumEntries) override
	{
		WaitCommitAsync();
		const auto older = (os::chrono::nt_clock::now() - chrono::days(DaysToKeep)).time_since_epoch().count();
		ExecuteStatement(stmtDeleteOldEditor, older, MinimumEntries);
		ExecuteStatement(stmtDeleteOldViewer, older, MinimumEntries);
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

class HistoryConfigDb: public HistoryConfigCustom
{
public:
	HistoryConfigDb():
		HistoryConfigCustom(L"history.db"sv, true)
	{
	}

private:
	// TODO: log
	// TODO: implementation
	void Import(const representation_source&) override {}
	void Export(representation_destination&) const override {}
};

class HistoryConfigMemory: public HistoryConfigCustom
{
public:
	HistoryConfigMemory():
		HistoryConfigCustom(L":memory:"sv, true)
	{
	}

private:
	void Import(const representation_source&) override {}
	void Export(representation_destination&) const override {}
};

static const std::wregex& uuid_regex()
{
	static const std::wregex re(RE_BEGIN RE_ANY_UUID RE_END, std::regex::icase | std::regex::optimize);
	return re;
}

}

void config_provider::CheckDatabase(const SQLiteDb* const pDb)
{
	string pname;
	const auto Status = pDb->GetInitStatus(pname, m_Mode != mode::m_default);
	if (!Status)
		return;

	if (m_Mode != mode::m_default)
	{
		std::wcerr << format(L"Problem with {0}:\n  {1}", pname, SQLiteDb::GetErrorMessage(Status)) << std::endl;
	}
	else
	{
		m_Problems.emplace_back(pname);
	}
}

void config_provider::TryImportDatabase(representable* p, const char* NodeName, bool IsPlugin)
{
	if (!m_TemplateSource && !Global->Opt->TemplateProfilePath.empty())
	{
		m_TemplateSource = std::make_unique<representation_source>(Global->Opt->TemplateProfilePath);
	}

	if (m_TemplateSource && m_TemplateSource->GetRoot().ToNode())
	{
		auto root = m_TemplateSource->GetRoot();

		if (!NodeName)
		{
			p->Import(*m_TemplateSource);
		}
		else if (!IsPlugin)
		{
			m_TemplateSource->SetRoot(root.FirstChildElement(NodeName));
			p->Import(*m_TemplateSource);
		}
		else
		{
			for (const auto& i: xml_enum(root.FirstChildElement("pluginsconfig"), "plugin"))
			{
				const auto guid = i->Attribute("guid");
				if (guid && 0 == strcmp(guid, NodeName))
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
void config_provider::CheckAndImportDatabase(T* Database, const char* ImportNodeName, bool IsPlugin)
{
	CheckDatabase(Database);
	if (m_Mode != mode::m_import && Database->IsNew())
	{
		TryImportDatabase(Database, ImportNodeName, IsPlugin);
	}
}

template<class T>
std::unique_ptr<T> config_provider::CreateDatabase()
{
	auto Database = std::make_unique<T>();
	CheckAndImportDatabase(Database.get(), nullptr, false);
	return Database;
}

template<class T>
HierarchicalConfigUniquePtr config_provider::CreateHierarchicalConfig(dbcheck DbId, string_view const DbName, const char* ImportNodeName, bool IsLocal, bool IsPlugin)
{
	auto Database = std::make_unique<T>(DbName, IsLocal);
	if (!m_CheckedDb.Check(DbId))
	{
		CheckAndImportDatabase(Database.get(), ImportNodeName, IsPlugin);
		m_CheckedDb.Set(DbId);
	}
	return HierarchicalConfigUniquePtr(Database.release());
}

enum dbcheck: int
{
	CHECK_NONE       = 0,
	CHECK_FILTERS    = bit(0),
	CHECK_HIGHLIGHT  = bit(1),
	CHECK_SHORTCUTS  = bit(2),
	CHECK_PANELMODES = bit(3),
};

HierarchicalConfigUniquePtr config_provider::CreatePluginsConfig(const string_view guid, const bool Local)
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_NONE, path::join(L"PluginsData"sv, guid) + L".db"sv, encoding::utf8::get_bytes(guid).c_str(), Local, true);
}

HierarchicalConfigUniquePtr config_provider::CreateFiltersConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_FILTERS, L"filters.db"sv, "filters");
}

HierarchicalConfigUniquePtr config_provider::CreateHighlightConfig()
{
	return CreateHierarchicalConfig<HighlightHierarchicalConfigDb>(CHECK_HIGHLIGHT, L"highlight.db"sv, "highlight");
}

HierarchicalConfigUniquePtr config_provider::CreateShortcutsConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_SHORTCUTS, L"shortcuts.db"sv, "shortcuts", true);
}

HierarchicalConfigUniquePtr config_provider::CreatePanelModesConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_PANELMODES, L"panelmodes.db"sv, "panelmodes");
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
	// Make sure all threads are joined before freeing the library
	m_Threads.clear();
	SQLiteDb::library_free();
}

bool config_provider::Export(const string& File)
{
	representation_destination Representation;
	auto& root = Representation.GetRoot();
	root.SetAttribute("version", format("{0}.{1}.{2}", FAR_VERSION.Major, FAR_VERSION.Minor, FAR_VERSION.Build).c_str());

	GeneralCfg()->Export(Representation);
	LocalGeneralCfg()->Export(Representation);
	ColorsCfg()->Export(Representation);
	AssocConfig()->Export(Representation);
	PlHotkeyCfg()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "filters"));
	CreateFiltersConfig()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "highlight"));
	CreateHighlightConfig()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "panelmodes"));
	CreatePanelModesConfig()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "shortcuts"));
	CreateShortcutsConfig()->Export(Representation);

	{
		//TODO: export local plugin settings
		auto& e = CreateChild(root, "pluginsconfig");
		for(auto& i: os::fs::enum_files(path::join(Global->Opt->ProfilePath, L"PluginsData"sv, L"*.db"sv)))
		{
			i.FileName.resize(i.FileName.size()-3);
			inplace::upper(i.FileName);
			if (std::regex_search(i.FileName, uuid_regex()))
			{
				auto& PluginRoot = CreateChild(e, "plugin");
				PluginRoot.SetAttribute("guid", encoding::utf8::get_bytes(i.FileName).c_str());
				Representation.SetRoot(PluginRoot);
				CreatePluginsConfig(i.FileName)->Export(Representation);
			}
		}
	}

	return Representation.Save(File);
}

bool config_provider::ServiceMode(const string& Filename)
{
	return m_Mode == mode::m_import? Import(Filename) : m_Mode == mode::m_export? Export(Filename) : throw MAKE_FAR_EXCEPTION(L"Unexpected service mode"sv);
}

bool config_provider::Import(const string& Filename)
{
	representation_source Representation(Filename);

	auto root = Representation.GetRoot();

	if (!root.ToNode())
	{
		std::wcerr << format(L"Error importing {0}:\n {1}", Filename, Representation.GetError()) << std::endl;
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
	CreatePanelModesConfig()->Import(Representation);
	Representation.SetRoot(root.FirstChildElement("shortcuts"));
	CreateShortcutsConfig()->Import(Representation);

	//TODO: import local plugin settings
	for (const auto& plugin: xml_enum(root.FirstChildElement("pluginsconfig"), "plugin"))
	{
		const auto guid = plugin->Attribute("guid");
		if (!guid)
			continue;
		const auto Guid = upper(encoding::utf8::get_chars(guid));

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

bool config_provider::ShowProblems() const
{
	if (m_Problems.empty())
		return false;
	return Message(MSG_WARNING,
		msg(lng::MProblemDb),
		m_Problems,
		{ lng::MShowConfigFolders, lng::MIgnore }) == Message::first_button;
}

void config_provider::AsyncCall(const std::function<void()>& Routine)
{
	m_Threads.erase(std::remove_if(ALL_RANGE(m_Threads), std::mem_fn(&os::thread::is_signaled)), m_Threads.end());
	m_Threads.emplace_back(&os::thread::join, Routine);
}

config_provider& ConfigProvider()
{
	return *Global->m_ConfigProvider;
}
