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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "configdb.hpp"

// Internal:
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
#include "stddlg.hpp"
#include "log.hpp"
#include "colormix.hpp"

// Platform:
#include "platform.hpp"
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/base64.hpp"
#include "common/bytes_view.hpp"
#include "common/function_ref.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"

// External:
#include "format.hpp"
#include "tinyxml.hpp"

#include <share.h>

//----------------------------------------------------------------------------

static const auto XmlDocumentRootName = "farconfig";

class representation_source
{
public:
	explicit representation_source(string_view const File)
	{
		const file_ptr XmlFile(_wfsopen(nt_path(File).c_str(), L"rb", _SH_DENYWR));
		if (!XmlFile)
			throw far_known_exception(far::format(L"Error opening file \"{}\": {}"sv, File, os::format_errno(errno)));

		if (const auto LoadResult = m_Document.LoadFile(XmlFile.get()); LoadResult != tinyxml::XML_SUCCESS)
			throw far_known_exception(far::format(L"Error loading document from \"{}\": {}"sv, File, encoding::utf8::get_chars(m_Document.ErrorIDToName(LoadResult))));

		const auto root = m_Document.FirstChildElement(XmlDocumentRootName);
		SetRoot(root);
	}

	auto Root() const { return m_Root; }

	void SetRoot(tinyxml::XMLHandle Root) { m_Root = Root; }
	void SetRoot(tinyxml::XMLElement* const Root) { m_Root = tinyxml::XMLHandle{ Root }; }

	string GetError() const
	{
		return encoding::utf8::get_chars(m_Document.ErrorStr());
	}

private:
	tinyxml::XMLDocument m_Document;
	tinyxml::XMLHandle m_Root{ nullptr };
};

static auto& CreateChild(tinyxml::XMLElement& Parent, const char* Name)
{
	const auto e = Parent.GetDocument()->NewElement(Name);
	Parent.LinkEndChild(e);
	return *e;
}

template<typename T>
static void SetAttribute(tinyxml::XMLElement& Element, const char* Name, T const& Value)
{
	if constexpr (std::convertible_to<T, std::string_view>)
		Element.SetAttribute(Name, null_terminated_t<char>(Value).c_str());
	else
		Element.SetAttribute(Name, Value);
}

class representation_destination
{
public:
	representation_destination()
	{
		m_Document.SetBOM(true);
		m_Document.LinkEndChild(m_Document.NewDeclaration());
		const auto root = m_Document.NewElement(XmlDocumentRootName);
		m_Document.LinkEndChild(root);
		SetRoot(*root);
	}

	auto& Root() const { return *m_Root; }

	void SetRoot(tinyxml::XMLElement& Root) { m_Root = &Root; }

	void Save(string_view const File)
	{
		const file_ptr XmlFile(_wfsopen(nt_path(File).c_str(), L"w", _SH_DENYWR));
		if (!XmlFile)
			throw far_known_exception(far::format(L"Error opening file \"{}\": {}"sv, File, os::format_errno(errno)));

		if (const auto SaveResult = m_Document.SaveFile(XmlFile.get()); SaveResult != tinyxml::XML_SUCCESS)
			throw far_known_exception(far::format(L"Error saving document to \"{}\": {}"sv, File, encoding::utf8::get_chars(m_Document.ErrorIDToName(SaveResult))));
	}

private:
	tinyxml::XMLDocument m_Document;
	tinyxml::XMLElement* m_Root{};
};


class async_delete_impl: virtual public async_delete
{
protected:
	explicit async_delete_impl(string_view const Name):
		// If a thread with same event name is running, we will open that event here
		m_AsyncDone(os::event::type::manual, os::event::state::signaled, Name)
	{
		// and wait for the signal
		m_AsyncDone.wait();
	}

	~async_delete_impl() override
	{
		m_AsyncDone.set();
	}

	void finish() override
	{
		m_AsyncDone.reset();
		// TODO: SEH guard, try/catch, exception_ptr
		ConfigProvider().AsyncCall(config_provider::async_key{}, [this]
		{
			delete this;
		});
	}

private:
	os::event m_AsyncDone;
};


namespace
{

class [[nodiscard]] xml_enum: noncopyable, public enumerator<xml_enum, const tinyxml::XMLElement*, true>
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
	[[nodiscard, maybe_unused]]
	bool get(bool Reset, value_type& value) const
	{
		value =
			!Reset? value->NextSiblingElement(m_name) :
			m_base? m_base->FirstChildElement(m_name) :
			nullptr;

		return value != nullptr;
	}

	const char* m_name;
	const tinyxml::XMLNode* m_base;
};

void serialise_integer(tinyxml::XMLElement& e, long long const Value)
{
	SetAttribute(e, "type", "qword"sv);
	SetAttribute(e, "value", encoding::utf8::get_bytes(to_hex_wstring(Value)));
}

void serialise_string(tinyxml::XMLElement& e, std::string const& Value)
{
	SetAttribute(e, "type", "text"sv);
	SetAttribute(e, "value", Value);
}

void serialise_blob(tinyxml::XMLElement& e, bytes_view const Value)
{
	SetAttribute(e, "type", "base64"sv);
	SetAttribute(e, "value", base64::encode(Value));
}

bool deserialise_value(char const* Type, char const* Value, auto const& Setter)
{
	if (!strcmp(Type, "qword"))
	{
		if (Value)
			Setter(strtoull(Value, nullptr, 16));
		return true;
	}

	if (!strcmp(Type, "text"))
	{
		if (Value)
			Setter(encoding::utf8::get_chars(Value));
		return true;
	}

	if (!strcmp(Type, "base64"))
	{
		if (Value)
			Setter(base64::decode(Value));
		return true;
	}

	if (!strcmp(Type, "hex"))
	{
		if (Value)
			Setter(HexStringToBlob(encoding::utf8::get_chars(Value)));
		return true;
	}

	return false;
}

int sqlite_busy_handler(void* Param, int Retries) noexcept
{
	try
	{
		if (Retries < 10)
		{
			// Let's retry silently first:
			os::chrono::sleep_for(500ms);
			return 1;
		}

		const auto& Db = *static_cast<const SQLiteDb*>(Param);
		return RetryAbort({ Db.GetPath(), L"Database is busy"s });
	}
	catch (...)
	{
		return 0;
	}
}

class sqlite_boilerplate : public SQLiteDb
{
protected:
	explicit sqlite_boilerplate(auto&&... Args) :
		SQLiteDb(sqlite_busy_handler, FWD(Args)...)
	{
	}
};

class iGeneralConfigDb: public GeneralConfig, public sqlite_boilerplate
{
protected:
	explicit iGeneralConfigDb(string_view const DbName):
		sqlite_boilerplate(&iGeneralConfigDb::Initialise, DbName)
	{
	}

private:
	static void Initialise(const db_initialiser& Db)
	{
		static const std::string_view Schema[]
		{
			"CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));"sv,
		};

		Db.Exec(Schema);

		static const stmt_init<statement_id> Statements[]
		{
			{ stmtSetValue,              "REPLACE INTO general_config VALUES (?1,?2,?3);"sv },
			{ stmtGetValue,              "SELECT value FROM general_config WHERE key=?1 AND name=?2;"sv },
			{ stmtDelValue,              "DELETE FROM general_config WHERE key=?1 AND name=?2;"sv },
			{ stmtEnumValues,            "SELECT name, value FROM general_config WHERE key=?1;"sv },
		};

		Db.PrepareStatements(Statements);
	}

	void SetValue(const string_view Key, const string_view Name, const string_view Value) override
	{
		SetValueT(Key, Name, Value);
	}

	void SetValue(const string_view Key, const string_view Name, const unsigned long long Value) override
	{
		SetValueT(Key, Name, Value);
	}

	void SetValue(const string_view Key, const string_view Name, bytes_view const Value) override
	{
		SetValueT(Key, Name, Value);
	}

	bool GetValue(const string_view Key, const string_view Name, bool& Value) const override
	{
		long long Data;
		if (!GetValue(Key, Name, Data))
			return false;

		Value = Data != 0;
		return true;
	}

	bool GetValue(const string_view Key, const string_view Name, long long& Value) const override
	{
		return GetValueT<column_type::integer>(Key, Name, Value, &SQLiteStmt::GetColInt64);
	}

	bool GetValue(const string_view Key, const string_view Name, string& Value) const override
	{
		return GetValueT<column_type::string>(Key, Name, Value, &SQLiteStmt::GetColText);
	}

	void DeleteValue(const string_view Key, const string_view Name) override
	{
		ExecuteStatement(stmtDelValue, Key, Name);
	}

	bool EnumValues(const string_view Key, const bool Reset, string &Name, string &Value) const override
	{
		return EnumValuesT(Key, Reset, Name, Value, &SQLiteStmt::GetColText);
	}

	bool EnumValues(const string_view Key, const bool Reset, string &Name, long long& Value) const override
	{
		return EnumValuesT(Key, Reset, Name, Value, &SQLiteStmt::GetColInt64);
	}

	auto EnumValuesStmt() const
	{
		return AutoStatement(stmtEnumValues);
	}

	void CloseEnum() const override
	{
		(void)EnumValuesStmt();
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.Root(), GetKeyName());

		const auto stmtEnumAllValues = create_stmt("SELECT key, name, value FROM general_config ORDER BY key, name;"sv);

		while (stmtEnumAllValues.Step())
		{
			auto& e = CreateChild(root, "setting");

			SetAttribute(e, "key", stmtEnumAllValues.GetColTextUTF8(0));
			SetAttribute(e, "name", stmtEnumAllValues.GetColTextUTF8(1));

			switch (stmtEnumAllValues.GetColType(2))
			{
			case column_type::integer:
				serialise_integer(e, stmtEnumAllValues.GetColInt64(2));
				break;

			case column_type::string:
				serialise_string(e, stmtEnumAllValues.GetColTextUTF8(2));
				break;

			case column_type::blob:
			case column_type::unknown:
				serialise_blob(e, stmtEnumAllValues.GetColBlob(2));
				break;
			}
		}
	}

	void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for(const auto& e: xml_enum(Representation.Root().FirstChildElement(GetKeyName()), "setting"))
		{
			const auto key = e.Attribute("key");
			const auto name = e.Attribute("name");
			const auto type = e.Attribute("type");
			const auto value = e.Attribute("value");

			if (!key || !name || !type || !value)
				continue;

			const auto Key = encoding::utf8::get_chars(key);
			const auto Name = encoding::utf8::get_chars(name);
			deserialise_value(type, value, [&](auto const& Value){ SetValue(Key, Name, Value); });
		}
	}

	virtual const char* GetKeyName() const = 0;

	template<column_type TypeId>
	bool GetValueT(const string_view Key, const string_view Name, auto& Value, const auto Getter) const
	{
		const auto Stmt = AutoStatement(stmtGetValue);
		if (!Stmt->Bind(Key, Name).Step() || Stmt->GetColType(0) != TypeId)
			return false;

		Value = std::invoke(Getter, Stmt, 0);
		return true;
	}

	void SetValueT(const string_view Key, const string_view Name, const auto Value)
	{
		ExecuteStatement(stmtSetValue, Key, Name, Value);
	}

	bool EnumValuesT(const string_view Key, bool Reset, string& Name, auto& Value, const auto Getter) const
	{
		auto Stmt = EnumValuesStmt();

		if (Reset)
			Stmt->Reset().Bind(Key);

		if (!Stmt->Step())
			return false;

		Name = Stmt->GetColText(0);
		Value = std::invoke(Getter, Stmt, 1);
		KeepStatement(Stmt);
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

class GeneralConfigDb final: public iGeneralConfigDb
{
public:
	explicit GeneralConfigDb(string_view const Name):
		iGeneralConfigDb(Name)
	{
	}

private:
	const char* GetKeyName() const override {return "generalconfig";}
};

class LocalGeneralConfigDb final: public iGeneralConfigDb
{
public:
	explicit LocalGeneralConfigDb(string_view const Name):
		iGeneralConfigDb(Name)
	{
	}

private:
	const char* GetKeyName() const override {return "localconfig";}
};

class HierarchicalConfigDb: public async_delete_impl, public HierarchicalConfig, public sqlite_boilerplate
{
public:
	explicit HierarchicalConfigDb(string_view const DbName):
		async_delete_impl(os::make_name<os::event>(DbName, PointToName(DbName))),
		sqlite_boilerplate(&HierarchicalConfigDb::Initialise, DbName)
	{
	}

protected:
	virtual void SerializeBlob(std::string_view /*Name*/, bytes_view const Blob, tinyxml::XMLElement& e) const
	{
		serialise_blob(e, Blob);
	}

	virtual bytes DeserializeBlob(const char* Type, const char* Value, const tinyxml::XMLElement& e) const
	{
		return Value? HexStringToBlob(encoding::utf8::get_chars(Value)) : bytes{};
	}

private:
	static void Initialise(const db_initialiser& Db)
	{
		Db.EnableForeignKeysConstraints();

		Db.add_numeric_collation();

		static const std::string_view Schema[]
		{
			"CREATE TABLE IF NOT EXISTS table_keys(id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"sv,
			"CREATE TABLE IF NOT EXISTS table_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id <> 0));"sv,
			//root key (needs to be before the transaction start)
			"INSERT OR IGNORE INTO table_keys VALUES (0,0,'','Root - do not edit');"sv,
		};

		Db.Exec(Schema);

		static const stmt_init<statement_id> Statements[]
		{
			{ stmtCreateKey,             "INSERT OR IGNORE INTO table_keys VALUES (NULL,?1,?2,NULL);"sv },
			{ stmtFindKey,               "SELECT id FROM table_keys WHERE parent_id=?1 AND name=?2 AND id<>0;"sv },
			{ stmtGetKeyName,            "SELECT name from table_keys WHERE parent_id=?1 AND id=?2 AND id<>0 ;"sv },
			{ stmtSetKeyDescription,     "UPDATE table_keys SET description=?1 WHERE id=?2 AND id<>0 AND (description IS NULL OR description<>?1);"sv },
			{ stmtSetValue,              "REPLACE INTO table_values VALUES (?1,?2,?3);"sv },
			{ stmtGetValue,              "SELECT value FROM table_values WHERE key_id=?1 AND name=?2;"sv },
			{ stmtEnumKeys,              "SELECT id FROM table_keys WHERE parent_id=?1 AND id<>0;"sv },
			{ stmtEnumKeysLike,          "SELECT id FROM table_keys WHERE parent_id=?1 AND id<>0 AND name LIKE ?2 ORDER BY name COLLATE numeric;"sv },
			{ stmtEnumValues,            "SELECT name, value FROM table_values WHERE key_id=?1;"sv },
			{ stmtEnumValuesLike,        "SELECT name, value FROM table_values WHERE key_id=?1 AND name LIKE ?2 ORDER BY name COLLATE numeric;"sv },
			{ stmtDelValue,              "DELETE FROM table_values WHERE key_id=?1 AND name=?2;"sv },
			{ stmtDeleteTree,            "DELETE FROM table_keys WHERE id=?1 AND id<>0;"sv },
		};

		Db.PrepareStatements(Statements);
	}

	void Flush() override
	{
		EndTransaction();
		BeginTransaction();
	}

	const string& GetName() const override
	{
		return GetPath();
	}

	key CreateKey(const key& Root, const string_view Name) override
	{
		if (const auto Key = FindByName(Root, Name))
			return Key;

		ExecuteStatement(stmtCreateKey, Root.get(), Name);
		return key(LastInsertRowID());
	}

	key FindByName(const key& Root, const string_view Name) const override
	{
		const auto Stmt = AutoStatement(stmtFindKey);
		if (!Stmt->Bind(Root.get(), Name).Step())
			return root_key;

		return key(Stmt->GetColInt64(0));
	}

	bool GetKeyName(const key& Root, const key& Key, string& Name) const override
	{
		const auto Stmt = AutoStatement(stmtGetKeyName);
		if (!Stmt->Bind(Root.get(), Key.get()).Step())
			return false;

		Name = Stmt->GetColText(0);
		return true;
	}

	void SetKeyDescription(const key& Root, const string_view Description) override
	{
		ExecuteStatement(stmtSetKeyDescription, Description, Root.get());
	}

	void SetValue(const key& Root, const string_view Name, const string_view Value) override
	{
		SetValueT(Root, Name, Value);
	}

	void SetValue(const key& Root, const string_view Name, const unsigned long long Value) override
	{
		SetValueT(Root, Name, Value);
	}

	void SetValue(const key& Root, const string_view Name, bytes_view const Value) override
	{
		SetValueT(Root, Name, Value);
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

	void DeleteKeyTree(const key& Key) override
	{
		// Whole subtree is automatically deleted because of foreign key constraints
		ExecuteStatement(stmtDeleteTree, Key.get());
	}

	void DeleteValue(const key& Root, const string_view Name) override
	{
		ExecuteStatement(stmtDelValue, Root.get(), Name);
	}

	auto EnumKeysStmt(string_view const Pattern) const
	{
		return AutoStatement(Pattern.empty()? stmtEnumKeys : stmtEnumKeysLike);
	}

	bool EnumKeys(const key& Root, const bool Reset, key& Key, string_view const Pattern) const override
	{
		auto Stmt = EnumKeysStmt(Pattern);

		if (Reset)
		{
			Stmt->Reset().Bind(Root.get());
			if (!Pattern.empty())
				Stmt->Bind(Pattern + L"%"sv);
		}

		if (!Stmt->Step())
			return false;

		Key = key(Stmt->GetColInt64(0));
		KeepStatement(Stmt);
		return true;
	}

	void CloseEnumKeys(string_view const Pattern) const override
	{
		(void)EnumKeysStmt(Pattern);
	}

	auto EnumValuesStmt(string_view const Pattern) const
	{
		return AutoStatement(Pattern.empty()? stmtEnumValues : stmtEnumValuesLike);
	}

	bool EnumValues(const key& Root, const bool Reset, string& Name, int& Type, string_view const Pattern) const override
	{
		auto Stmt = EnumValuesStmt(Pattern);

		if (Reset)
		{
			Stmt->Reset().Bind(Root.get());
			if (!Pattern.empty())
				Stmt->Bind(Pattern + L"%"sv);
		}

		if (!Stmt->Step())
			return false;

		Name = Stmt->GetColText(0);
		Type = static_cast<int>(Stmt->GetColType(1));
		KeepStatement(Stmt);
		return true;
	}

	void CloseEnumValues(string_view const Pattern) const override
	{
		(void)EnumValuesStmt(Pattern);
	}

	void Export(representation_destination& Representation) const override
	{
		Export(Representation, root_key, CreateChild(Representation.Root(), "hierarchicalconfig"));
	}

	void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.Root().FirstChildElement("hierarchicalconfig"), "key"))
		{
			Import(root_key, e);
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
				SetAttribute(e, "name", name);

				switch (Stmt->GetColType(1))
				{
				case column_type::integer:
					serialise_integer(e, Stmt->GetColInt64(1));
					break;

				case column_type::string:
					serialise_string(e, Stmt->GetColTextUTF8(1));
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

			SetAttribute(e, "name", stmtEnumSubKeys.GetColTextUTF8(1));
			const auto description = stmtEnumSubKeys.GetColTextUTF8(2);
			if (!description.empty())
				SetAttribute(e, "description", description);

			Export(Representation, key(stmtEnumSubKeys.GetColInt64(0)), e);
		}
	}

	void Import(const key& root, const tinyxml::XMLElement& key)
	{
		const auto KeyName = key.Attribute("name");
		if (!KeyName)
			return;

		const auto Key = CreateKey(root, encoding::utf8::get_chars(KeyName));

		if (const auto KeyDescription = key.Attribute("description"))
			SetKeyDescription(Key, encoding::utf8::get_chars(KeyDescription));

		for (const auto& e: xml_enum(key, "value"))
		{
			const auto name = e.Attribute("name");
			const auto type = e.Attribute("type");
			const auto value = e.Attribute("value");

			if (!name || !type)
				continue;

			const auto Name = encoding::utf8::get_chars(name);

			if (!deserialise_value(type, value, [&](auto const& Value){ SetValue(Key, Name, Value); }))
			{
				// custom types, value is optional
				SetValue(Key, Name, DeserializeBlob(type, value, e));
			}
		}

		for (const auto& e: xml_enum(key, "key"))
		{
			Import(Key, e);
		}
	}

	bool GetValueT(const key& Root, const string_view Name, auto& Value, const auto Getter) const
	{
		const auto Stmt = AutoStatement(stmtGetValue);
		if (!Stmt->Bind(Root.get(), Name).Step())
			return false;

		Value = std::invoke(Getter, Stmt, 0);
		return true;
	}

	void SetValueT(const key& Root, const string_view Name, const auto& Value)
	{
		ExecuteStatement(stmtSetValue, Root.get(), Name, Value);
	}

	enum statement_id
	{
		stmtCreateKey,
		stmtFindKey,
		stmtGetKeyName,
		stmtSetKeyDescription,
		stmtSetValue,
		stmtGetValue,
		stmtEnumKeys,
		stmtEnumKeysLike,
		stmtEnumValues,
		stmtEnumValuesLike,
		stmtDelValue,
		stmtDeleteTree,

		stmt_count
	};
};

const std::pair<FARCOLORFLAGS, string_view> LegacyColorFlagNames[]
{
	{ FCF_FG_INDEX, L"fg4bit"sv },
	{ FCF_BG_INDEX, L"bg4bit"sv },
};

void color_to_xml(bytes_view const Blob, tinyxml::XMLElement& e)
{
	const auto process_color = [&](const char* const Name, COLORREF const Color)
	{
		if (Color)
			SetAttribute(e, Name, encoding::utf8::get_bytes(to_hex_wstring(Color)));
	};

	FarColor Color;
	if (!deserialise(Blob, Color))
		return;

	process_color("background", Color.BackgroundColor);
	process_color("foreground", Color.ForegroundColor);
	process_color("underline", Color.UnderlineColor);

	if (Color.Flags)
	{
		if (const auto StrFlags = encoding::utf8::get_bytes(colors::ColorFlagsToString(Color.Flags)); !StrFlags.empty())
			SetAttribute(e, "flags", StrFlags);
	}
}

FarColor color_from_xml(tinyxml::XMLElement const& e)
{
	const auto process_color = [&](const char* const Name, COLORREF& Color)
	{
		if (const auto Value = e.Attribute(Name))
			Color = std::strtoul(Value, nullptr, 16);
	};

	FarColor Color{};

	process_color("background", Color.BackgroundColor);
	process_color("foreground", Color.ForegroundColor);
	process_color("underline", Color.UnderlineColor);

	if (const auto flags = e.Attribute("flags"))
	{
		const auto FlagsStr = encoding::utf8::get_chars(flags);
		Color.Flags = colors::ColorStringToFlags(FlagsStr) | StringToFlags(FlagsStr, LegacyColorFlagNames);
	}

	return Color;
}

class HighlightHierarchicalConfigDb final: public HierarchicalConfigDb
{
public:
	using HierarchicalConfigDb::HierarchicalConfigDb;

private:
	void SerializeBlob(std::string_view const Name, bytes_view const Blob, tinyxml::XMLElement& e) const override
	{
		static const std::string_view ColorKeys[]
		{
			"NormalColor"sv,
			"SelectedColor"sv,
			"CursorColor"sv,
			"SelectedCursorColor"sv,
			"MarkCharNormalColor"sv,
			"MarkCharSelectedColor"sv,
			"MarkCharCursorColor"sv,
			"MarkCharSelectedCursorColor"sv,
		};

		if (contains(ColorKeys, Name))
		{
			SetAttribute(e, "type", "color"sv);
			color_to_xml(Blob, e);
			return;
		}

		return HierarchicalConfigDb::SerializeBlob(Name, Blob, e);
	}

	bytes DeserializeBlob(const char* Type, const char* Value, const tinyxml::XMLElement& e) const override
	{
		if(Type == "color"sv)
			return bytes(view_bytes(color_from_xml(e)));

		return HierarchicalConfigDb::DeserializeBlob(Type, Value, e);
	}
};

class ColorsConfigDb final: public ColorsConfig, public sqlite_boilerplate
{
public:
	explicit ColorsConfigDb(string_view const Name):
		sqlite_boilerplate(&ColorsConfigDb::Initialise, Name)
	{
	}

private:
	static void Initialise(const db_initialiser& Db)
	{
		Db.add_numeric_collation();

		static const std::string_view Schema[]
		{
			"CREATE TABLE IF NOT EXISTS colors(name TEXT NOT NULL PRIMARY KEY, value BLOB);"sv,
		};

		Db.Exec(Schema);

		static const stmt_init<statement_id> Statements[]
		{
			{ stmtSetValue,              "REPLACE INTO colors VALUES (?1,?2);"sv },
			{ stmtGetValue,              "SELECT value FROM colors WHERE name=?1;"sv },
			{ stmtDelValue,              "DELETE FROM colors WHERE name=?1;"sv },
		};

		Db.PrepareStatements(Statements);
	}

	void SetValue(const string_view Name, const FarColor& Value) override
	{
		ExecuteStatement(stmtSetValue, Name, view_bytes(Value));
	}

	bool GetValue(const string_view Name, FarColor& Value) const override
	{
		const auto Stmt = AutoStatement(stmtGetValue);
		if (!Stmt->Bind(Name).Step())
			return false;

		return deserialise(Stmt->GetColBlob(0), Value);
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.Root(), "colors");

		const auto stmtEnumAllValues = create_stmt("SELECT name, value FROM colors ORDER BY name COLLATE numeric;"sv);

		while (stmtEnumAllValues.Step())
		{
			auto& e = CreateChild(root, "object");
			SetAttribute(e, "name", stmtEnumAllValues.GetColTextUTF8(0));
			color_to_xml(stmtEnumAllValues.GetColBlob(1), e);
		}
	}

	void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.Root().FirstChildElement("colors"), "object"))
		{
			const auto name = e.Attribute("name");

			if (!name)
				continue;

			const auto Name = encoding::utf8::get_chars(name);

			if (const auto Color = color_from_xml(e); Color != FarColor{})
			{
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

class AssociationsConfigDb final: public AssociationsConfig, public sqlite_boilerplate
{
public:
	explicit AssociationsConfigDb(string_view const Name):
		sqlite_boilerplate(&AssociationsConfigDb::Initialise, Name)
	{
	}

private:
	static void Initialise(const db_initialiser& Db)
	{
		Db.EnableForeignKeysConstraints();

		static const std::string_view Schema[]
		{
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"sv,
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"sv,
		};

		Db.Exec(Schema);

		static const stmt_init<statement_id> Statements[]
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

		Db.PrepareStatements(Statements);
	}

	auto EnumMasksStmt() const
	{
		return AutoStatement(stmtEnumMasks);
	}

	bool EnumMasks(const bool Reset, primary_key& id, string& strMask) const override
	{
		auto Stmt = EnumMasksStmt();

		if (Reset)
			Stmt->Reset();

		if (!Stmt->Step())
			return false;

		id = Stmt->GetColInt64(0);
		strMask = Stmt->GetColText(1);
		KeepStatement(Stmt);
		return true;
	}

	void CloseEnumMasks() const override
	{
		(void)EnumMasksStmt();
	}

	auto EnumMasksForTypeStmt() const
	{
		return AutoStatement(stmtEnumMasksForType);
	}

	bool EnumMasksForType(const bool Reset, const int Type, primary_key& id, string& strMask) const override
	{
		auto Stmt = EnumMasksForTypeStmt();

		if (Reset)
			Stmt->Reset().Bind(Type);

		if (!Stmt->Step())
			return false;

		id = Stmt->GetColInt64(0);
		strMask = Stmt->GetColText(1);
		KeepStatement(Stmt);
		return true;
	}

	void CloseEnumMasksForType() const override
	{
		(void)EnumMasksForTypeStmt();
	}

	bool GetMask(primary_key const id, string &strMask) override
	{
		const auto Stmt = AutoStatement(stmtGetMask);
		if (!Stmt->Bind(id).Step())
			return false;

		strMask = Stmt->GetColText(0);
		return true;
	}

	bool GetDescription(primary_key const id, string &strDescription) override
	{
		const auto Stmt = AutoStatement(stmtGetDescription);
		if (!Stmt->Bind(id).Step())
			return false;

		strDescription = Stmt->GetColText(0);
		return true;
	}

	bool GetCommand(primary_key const id, int Type, string &strCommand, bool *Enabled) override
	{
		const auto Stmt = AutoStatement(stmtGetCommand);
		if (!Stmt->Bind(id, Type).Step())
			return false;

		strCommand = Stmt->GetColText(0);
		if (Enabled)
			*Enabled = Stmt->GetColInt(1) != 0;
		return true;
	}

	void SetCommand(primary_key const id, const int Type, const string_view Command, const bool Enabled) override
	{
		ExecuteStatement(stmtSetCommand, id, Type, Enabled, Command);
	}

	bool SwapPositions(primary_key const id1, primary_key const id2) override
	{
		const auto Stmt = AutoStatement(stmtGetWeight);
		if (!Stmt->Bind(id1).Step())
			return false;
		const auto weight1 = Stmt->GetColInt64(0);

		Stmt->Reset();
		if (!Stmt->Bind(id2).Step())
			return false;
		const auto weight2 = Stmt->GetColInt64(0);

		ExecuteStatement(stmtSetWeight, weight1, id2);
		ExecuteStatement(stmtSetWeight, weight2, id1);

		return true;
	}

	primary_key AddType(primary_key after_id, const string_view Mask, const string_view Description) override
	{
		ExecuteStatement(stmtReorder, after_id);
		ExecuteStatement(stmtAddType, after_id, Mask, Description);
		return LastInsertRowID();
	}

	void UpdateType(primary_key const id, const string_view Mask, const string_view Description) override
	{
		ExecuteStatement(stmtUpdateType, Mask, Description, id);
	}

	void DelType(primary_key const id) override
	{
		ExecuteStatement(stmtDelType, id);
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.Root(), "associations");

		const auto stmtEnumAllTypes = create_stmt("SELECT id, mask, description FROM filetypes ORDER BY weight;"sv);
		auto stmtEnumCommandsPerFiletype = create_stmt("SELECT type, enabled, command FROM commands WHERE ft_id=?1 ORDER BY type;"sv);

		while (stmtEnumAllTypes.Step())
		{
			auto& e = CreateChild(root, "filetype");

			SetAttribute(e, "mask", stmtEnumAllTypes.GetColTextUTF8(1));
			SetAttribute(e, "description", stmtEnumAllTypes.GetColTextUTF8(2));

			stmtEnumCommandsPerFiletype.Bind(stmtEnumAllTypes.GetColInt64(0));
			while (stmtEnumCommandsPerFiletype.Step())
			{
				auto& se = CreateChild(e, "command");

				SetAttribute(se, "type", stmtEnumCommandsPerFiletype.GetColInt(0));
				SetAttribute(se, "enabled", stmtEnumCommandsPerFiletype.GetColInt(1));
				SetAttribute(se, "command", stmtEnumCommandsPerFiletype.GetColTextUTF8(2));
			}
			stmtEnumCommandsPerFiletype.Reset();
		}
	}

	void Import(const representation_source& Representation) override
	{
		auto base = Representation.Root().FirstChildElement("associations");
		if (!base.ToElement())
			return;

		SCOPED_ACTION(auto)(ScopedTransaction());
		Exec("DELETE FROM filetypes;"sv); // delete all before importing
		primary_key id{};
		for (const auto& e: xml_enum(base, "filetype"))
		{
			const auto mask = e.Attribute("mask");
			const auto description = e.Attribute("description");

			if (!mask)
				continue;

			const auto Mask = encoding::utf8::get_chars(mask);
			const auto Description = encoding::utf8::get_chars(NullToEmpty(description));

			id = AddType(id, Mask, Description);

			for (const auto& se: xml_enum(e, "command"))
			{
				const auto command = se.Attribute("command");
				if (!command)
					continue;

				int type=0;
				if (se.QueryIntAttribute("type", &type) != tinyxml::XML_SUCCESS)
					continue;

				int enabled=0;
				if (se.QueryIntAttribute("enabled", &enabled) != tinyxml::XML_SUCCESS)
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

class PluginsCacheConfigDb final: public PluginsCacheConfig, public sqlite_boilerplate
{
public:
	explicit PluginsCacheConfigDb(string_view const Name):
		sqlite_boilerplate(&PluginsCacheConfigDb::Initialise, Name, true)
	{
	}

	void DiscardCache() override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		Exec("DELETE FROM cachename;"sv);
	}

private:
	static void Initialise(const db_initialiser& Db)
	{
		Db.SetWALJournalingMode();
		Db.EnableForeignKeysConstraints();

		static const std::string_view Schema[]
		{
			"CREATE TABLE IF NOT EXISTS cachename(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE);"sv,
			"CREATE TABLE IF NOT EXISTS preload(cid INTEGER NOT NULL PRIMARY KEY, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS signatures(cid INTEGER NOT NULL PRIMARY KEY, signature TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS guids(cid INTEGER NOT NULL PRIMARY KEY, guid TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS titles(cid INTEGER NOT NULL PRIMARY KEY, title TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS authors(cid INTEGER NOT NULL PRIMARY KEY, author TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS descriptions(cid INTEGER NOT NULL PRIMARY KEY, description TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS minfarversions(cid INTEGER NOT NULL PRIMARY KEY, version BLOB NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS pluginversions(cid INTEGER NOT NULL PRIMARY KEY, version BLOB NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS flags(cid INTEGER NOT NULL PRIMARY KEY, bitmask INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS prefixes(cid INTEGER NOT NULL PRIMARY KEY, prefix TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"sv,
			"CREATE TABLE IF NOT EXISTS exports(cid INTEGER NOT NULL, export TEXT NOT NULL, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, export));"sv,
			"CREATE TABLE IF NOT EXISTS menuitems(cid INTEGER NOT NULL, type INTEGER NOT NULL, number INTEGER NOT NULL, guid TEXT NOT NULL, name TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, type, number));"sv,
		};

		Db.Exec(Schema);

		static const stmt_init<statement_id> Statements[]
		{
			{ stmtCreateCache,           "INSERT INTO cachename VALUES (NULL,?1);"sv },
			{ stmtFindCacheName,         "SELECT id FROM cachename WHERE name=?1;"sv },
			{ stmtDelCache,              "DELETE FROM cachename WHERE name=?1;"sv },
			{ stmtCountCacheNames,       "SELECT count(name) FROM cachename;"sv },
			{ stmtGetPreloadState,       "SELECT enabled FROM preload WHERE cid=?1;"sv },
			{ stmtGetSignature,          "SELECT signature FROM signatures WHERE cid=?1;"sv },
			{ stmtGetExportState,        "SELECT enabled FROM exports WHERE cid=?1 and export=?2;"sv },
			{ stmtGetUuid,               "SELECT guid FROM guids WHERE cid=?1;"sv },
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
			{ stmtSetUuid,               "REPLACE INTO guids VALUES (?1,?2);"sv },
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

		Db.PrepareStatements(Statements);
	}

	void Import(const representation_source&) override {}
	void Export(representation_destination&) const override {}

	primary_key CreateCache(const string_view CacheName) override
	{
		//All related entries are automatically deleted because of foreign key constraints
		ExecuteStatement(stmtDelCache, CacheName);
		ExecuteStatement(stmtCreateCache, CacheName);
		return LastInsertRowID();
	}

	primary_key GetCacheID(const string_view CacheName) const override
	{
		const auto Stmt = AutoStatement(stmtFindCacheName);
		return Stmt->Bind(CacheName).Step()?
		       Stmt->GetColInt64(0) :
		       0;
	}

	bool IsPreload(primary_key const id) const override
	{
		const auto Stmt = AutoStatement(stmtGetPreloadState);
		return Stmt->Bind(id).Step() && Stmt->GetColInt(0) != 0;
	}

	string GetSignature(primary_key const id) const override
	{
		return GetTextFromID(stmtGetSignature, id);
	}

	bool GetExportState(primary_key const id, const string_view ExportName) const override
	{
		if (ExportName.empty())
			return false;

		const auto Stmt = AutoStatement(stmtGetExportState);
		return Stmt->Bind(id, ExportName).Step() && Stmt->GetColInt(0);
	}

	string GetUuid(primary_key const id) const override
	{
		return GetTextFromID(stmtGetUuid, id);
	}

	string GetTitle(primary_key const id) const override
	{
		return GetTextFromID(stmtGetTitle, id);
	}

	string GetAuthor(primary_key const id) const override
	{
		return GetTextFromID(stmtGetAuthor, id);
	}

	string GetDescription(primary_key const id) const override
	{
		return GetTextFromID(stmtGetDescription, id);
	}

	bool GetMinFarVersion(primary_key const id, VersionInfo& Version) const override
	{
		return GetVersionImpl(stmtGetMinFarVersion, id, Version);
	}

	bool GetVersion(primary_key const id, VersionInfo& Version) const override
	{
		return GetVersionImpl(stmtGetVersion, id, Version);
	}

	bool GetDiskMenuItem(primary_key const id, size_t index, string &Text, UUID& Uuid) const override
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Uuid);
	}

	bool GetPluginsMenuItem(primary_key const id, size_t index, string &Text, UUID& Uuid) const override
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Uuid);
	}

	bool GetPluginsConfigMenuItem(primary_key const id, size_t index, string &Text, UUID& Uuid) const override
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Uuid);
	}

	string GetCommandPrefix(primary_key const id) const override
	{
		return GetTextFromID(stmtGetPrefix, id);
	}

	unsigned long long GetFlags(primary_key const id) const override
	{
		const auto Stmt = AutoStatement(stmtGetFlags);
		return Stmt->Bind(id).Step()? Stmt->GetColInt64(0) : 0;
	}

	void SetPreload(primary_key const id, bool Preload) override
	{
		ExecuteStatement(stmtSetPreloadState, id, Preload);
	}

	void SetSignature(primary_key const id, const string_view Signature) override
	{
		ExecuteStatement(stmtSetSignature, id, Signature);
	}

	void SetDiskMenuItem(primary_key const id, const size_t index, const string_view Text, const UUID& Uuid) override
	{
		SetMenuItem(id, DRIVE_MENU, index, Text, Uuid);
	}

	void SetPluginsMenuItem(primary_key const id, const size_t index, const string_view Text, const UUID& Uuid) override
	{
		SetMenuItem(id, PLUGINS_MENU, index, Text, Uuid);
	}

	void SetPluginsConfigMenuItem(primary_key const id, const size_t index, const string_view Text, const UUID& Uuid) override
	{
		SetMenuItem(id, CONFIG_MENU, index, Text, Uuid);
	}

	void SetCommandPrefix(primary_key const id, const string_view Prefix) override
	{
		ExecuteStatement(stmtSetPrefix, id, Prefix);
	}

	void SetFlags(primary_key const id, unsigned long long Flags) override
	{
		ExecuteStatement(stmtSetFlags, id, Flags);
	}

	void SetExportState(primary_key const id, const string_view ExportName, const bool Exists) override
	{
		if (!ExportName.empty())
			ExecuteStatement(stmtSetExportState, id, ExportName, Exists);
	}

	void SetMinFarVersion(primary_key const id, const VersionInfo& Version) override
	{
		ExecuteStatement(stmtSetMinFarVersion, id, view_bytes(Version));
	}

	void SetVersion(primary_key const id, const VersionInfo& Version) override
	{
		ExecuteStatement(stmtSetVersion, id, view_bytes(Version));
	}

	void SetUuid(primary_key const id, const string_view Uuid) override
	{
		ExecuteStatement(stmtSetUuid, id, Uuid);
	}

	void SetTitle(primary_key const id, const string_view Title) override
	{
		ExecuteStatement(stmtSetTitle, id, Title);
	}

	void SetAuthor(primary_key const id, const string_view Author) override
	{
		ExecuteStatement(stmtSetAuthor, id, Author);
	}

	void SetDescription(primary_key const id, const string_view Description) override
	{
		ExecuteStatement(stmtSetDescription, id, Description);
	}

	bool EnumPlugins(size_t const Index, string &CacheName) const override
	{
		auto Stmt = AutoStatement(stmtEnumCache);
		if (Index == 0)
			Stmt->Reset();

		if (!Stmt->Step())
			return false;

		CacheName = Stmt->GetColText(0);
		KeepStatement(Stmt);
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

	bool GetMenuItem(primary_key const id, MenuItemTypeEnum type, size_t index, string &Text, UUID& Uuid) const
	{
		const auto Stmt = AutoStatement(stmtGetMenuItem);
		if (!Stmt->Bind(id, type, index).Step())
			return false;

		Text = Stmt->GetColText(0);

		const auto UuidOpt = uuid::try_parse(Stmt->GetColText(1));
		if (!UuidOpt)
			return false;

		Uuid = *UuidOpt;
		return true;
	}

	void SetMenuItem(primary_key const id, const MenuItemTypeEnum type, const size_t index, const string_view Text, const UUID& Uuid) const
	{
		ExecuteStatement(stmtSetMenuItem, id, type, index, uuid::str(Uuid), Text);
	}

	string GetTextFromID(size_t StatementIndex, primary_key const id) const
	{
		const auto Stmt = AutoStatement(StatementIndex);
		return Stmt->Bind(id).Step()? Stmt->GetColText(0) : string{};
	}

	bool GetVersionImpl(size_t StatementIndex, primary_key const id, VersionInfo& Version) const
	{
		const auto Stmt = AutoStatement(StatementIndex);
		if (!Stmt->Bind(id).Step())
			return false;

		return deserialise(Stmt->GetColBlob(0), Version);
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
		stmtGetUuid,
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
		stmtSetUuid,
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

class PluginsHotkeysConfigDb final: public PluginsHotkeysConfig, public sqlite_boilerplate
{
public:
	explicit PluginsHotkeysConfigDb(string_view const Name):
		sqlite_boilerplate(&PluginsHotkeysConfigDb::Initialise, Name)
	{
	}

private:
	static void Initialise(const db_initialiser& Db)
	{
		static const std::string_view Schema[]
		{
			"CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));"sv,
		};

		Db.Exec(Schema);

		static const stmt_init<statement_id> Statements[]
		{
			{ stmtGetHotkey,             "SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;"sv },
			{ stmtSetHotkey,             "REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);"sv },
			{ stmtDelHotkey,             "DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;"sv },
			{ stmtCheckForHotkeys,       "SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1;"sv },
		};

		Db.PrepareStatements(Statements);
	}

	bool HotkeysPresent(hotkey_type HotKeyType) override
	{
		const auto Stmt = AutoStatement(stmtCheckForHotkeys);
		return Stmt->Bind(std::to_underlying(HotKeyType)).Step() && Stmt->GetColInt(0);
	}

	string GetHotkey(const string_view PluginKey, const UUID& MenuUuid, const hotkey_type HotKeyType) override
	{
		const auto Stmt = AutoStatement(stmtGetHotkey);
		if (!Stmt->Bind(PluginKey, uuid::str(MenuUuid), std::to_underlying(HotKeyType)).Step())
			return {};

		return Stmt->GetColText(0);
	}

	void SetHotkey(const string_view PluginKey, const UUID& MenuUuid, const hotkey_type HotKeyType, const string_view HotKey) override
	{
		ExecuteStatement(stmtSetHotkey, PluginKey, uuid::str(MenuUuid), std::to_underlying(HotKeyType), HotKey);
	}

	void DelHotkey(const string_view PluginKey, const UUID& MenuUuid, const hotkey_type HotKeyType) override
	{
		ExecuteStatement(stmtDelHotkey, PluginKey, uuid::str(MenuUuid), std::to_underlying(HotKeyType));
	}

	void Export(representation_destination& Representation) const override
	{
		auto& root = CreateChild(Representation.Root(), "pluginhotkeys");

		const auto stmtEnumAllPluginKeys = create_stmt("SELECT pluginkey FROM pluginhotkeys GROUP BY pluginkey;"sv);
		auto stmtEnumAllHotkeysPerKey = create_stmt("SELECT menuguid, type, hotkey FROM pluginhotkeys WHERE pluginkey=$1;"sv);

		while (stmtEnumAllPluginKeys.Step())
		{
			auto& p = CreateChild(root, "plugin");

			SetAttribute(p, "key", stmtEnumAllPluginKeys.GetColTextUTF8(0));

			stmtEnumAllHotkeysPerKey.Bind(stmtEnumAllPluginKeys.GetColText(0));
			while (stmtEnumAllHotkeysPerKey.Step())
			{
				const char *type = nullptr;
				switch (const auto TypeValue = stmtEnumAllHotkeysPerKey.GetColInt(1); static_cast<hotkey_type>(TypeValue))
				{
				case hotkey_type::drive_menu: type = "drive"; break;
				case hotkey_type::config_menu: type = "config"; break;
				case hotkey_type::plugins_menu: type = "plugins"; break;
				default:
					LOGWARNING(L"Unknown hotkey_type: {}"sv, TypeValue);
					continue;
				}

				auto& e = CreateChild(p, "hotkey");
				SetAttribute(e, "menu", type);
				SetAttribute(e, "guid", stmtEnumAllHotkeysPerKey.GetColTextUTF8(0));
				SetAttribute(e, "hotkey", stmtEnumAllHotkeysPerKey.GetColTextUTF8(2));
			}
			stmtEnumAllHotkeysPerKey.Reset();
		}
	}

	void Import(const representation_source& Representation) override
	{
		SCOPED_ACTION(auto)(ScopedTransaction());
		for (const auto& e: xml_enum(Representation.Root().FirstChildElement("pluginhotkeys"), "plugin"))
		{
			const auto key = e.Attribute("key");

			if (!key)
				continue;

			const auto Key = encoding::utf8::get_chars(key);

			for (const auto& se: xml_enum(e, "hotkey"))
			{
				const auto stype = se.Attribute("menu");
				if (!stype)
					continue;

				const auto UuidStr = se.Attribute("guid");
				if (!UuidStr)
					continue;

				const auto Uuid = uuid::try_parse(encoding::utf8::get_chars(UuidStr));
				if (!Uuid)
					continue;

				const auto hotkey = se.Attribute("hotkey");

				const auto ProcessHotkey = [&](hotkey_type const Type)
				{
					if (hotkey && *hotkey)
						SetHotkey(Key, *Uuid, Type, encoding::utf8::get_chars(hotkey));
					else
						DelHotkey(Key, *Uuid, Type);
				};

				if (!strcmp(stype, "drive"))
					ProcessHotkey(hotkey_type::drive_menu);
				else if (!strcmp(stype, "config"))
					ProcessHotkey(hotkey_type::config_menu);
				else if (!strcmp(stype, "plugins"))
					ProcessHotkey(hotkey_type::plugins_menu);
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

class HistoryConfigCustom: public HistoryConfig, public sqlite_boilerplate
{
public:
	explicit HistoryConfigCustom(string_view const DbName):
		sqlite_boilerplate(&HistoryConfigCustom::Initialise, DbName, true)
	{
	}

	~HistoryConfigCustom() override
	{
		WaitAllAsync();
		StopEvent.set();
	}

private:
	os::event StopEvent{os::event::type::automatic, os::event::state::nonsignaled};
	os::event AsyncDeleteAddDone{os::event::type::manual, os::event::state::signaled};
	os::event AsyncCommitDone{os::event::type::manual, os::event::state::signaled};
	os::event AsyncWork{os::event::type::automatic, os::event::state::nonsignaled};
	[[maybe_unused]] os::thread WorkThread{&HistoryConfigCustom::ThreadProc, this};

	struct AsyncWorkItem
	{
		primary_key DeleteId;
		unsigned int TypeHistory;
		string HistoryName;
		int Type;
		bool Lock;
		string strName;
		os::chrono::time_point Time;
		string strUuid;
		string strFile;
		string strData;
	};

	os::synced_queue<std::unique_ptr<AsyncWorkItem>> WorkQueue;

	void WaitAllAsync() const
	{
		os::handle::wait_all({ AsyncDeleteAddDone.native_handle(), AsyncCommitDone.native_handle() });
	}

	void WaitCommitAsync() const
	{
		AsyncCommitDone.wait();
	}

	void ThreadProc()
	{
		os::debug::set_thread_name(L"Async history config");

		// TODO: SEH guard, try/catch, exception_ptr

		while (os::handle::wait_any({ AsyncWork.native_handle(), StopEvent.native_handle() }) != 1)
		{
			bool bAddDelete=false, bCommit=false;

			for (auto Messages = WorkQueue.pop_all(); !Messages.empty(); Messages.pop())
			{
				SCOPE_EXIT{ SQLiteDb::EndTransaction(); };

				if (const auto& item = Messages.front()) //DeleteAndAddAsync
				{
					SQLiteDb::BeginTransaction();
					if (item->DeleteId)
						DeleteInternal(item->DeleteId);
					AddInternal(item->TypeHistory, item->HistoryName, item->Type, item->Lock, item->strName, item->Time, item->strUuid, item->strFile, item->strData);
					bAddDelete = true;
				}
				else // EndTransaction
				{
					bCommit = true;
				}
			}

			if (bAddDelete)
				AsyncDeleteAddDone.set();

			if (bCommit)
				AsyncCommitDone.set();
		}
	}

	void AddInternal(unsigned int const TypeHistory, string_view const HistoryName, int const Type, bool const Lock, string_view const Name, os::chrono::time_point const Time, string_view const Uuid, string_view const File, string_view const Data) const
	{
		ExecuteStatement(stmtAdd, TypeHistory, HistoryName, Type, Lock, Name, os::chrono::nt_clock::to_hectonanoseconds(Time), Uuid, File, Data);
	}

	void DeleteInternal(primary_key const id) const
	{
		ExecuteStatement(stmtDel, id);
	}

	primary_key GetPrevImpl(const unsigned int TypeHistory, const string_view HistoryName, primary_key const id, string& Name, os::chrono::time_point& Time, function_ref<primary_key()> const Fallback) const
	{
		WaitAllAsync();
		Name.clear();

		if (!id)
		{
			const auto GetNewestStmt = AutoStatement(stmtGetNewest);
			if (!GetNewestStmt->Bind(TypeHistory, HistoryName).Step())
				return 0;

			Name = GetNewestStmt->GetColText(1);
			Time = os::chrono::nt_clock::from_hectonanoseconds(GetNewestStmt->GetColInt64(2));
			return GetNewestStmt->GetColInt64(0);
		}

		const auto GetPrevStmt = AutoStatement(stmtGetPrev);
		if (!GetPrevStmt->Bind(id, TypeHistory, HistoryName).Step())
			return Fallback();

		Name = GetPrevStmt->GetColText(1);
		Time = os::chrono::nt_clock::from_hectonanoseconds(GetPrevStmt->GetColInt64(2));

		return GetPrevStmt->GetColInt64(0);
	}

	void BeginTransaction() override { WaitAllAsync(); SQLiteDb::BeginTransaction(); }

	void EndTransaction() override
	{
		WorkQueue.emplace(nullptr);
		WaitAllAsync();
		AsyncCommitDone.reset();
		AsyncWork.set();
	}

	#define EDITORPOSITION_HISTORY_NAME "editorposition_history"
	#define VIEWERPOSITION_HISTORY_NAME "viewerposition_history"
	#define EDITORPOSITION_HISTORY_SCHEMA "(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE COLLATE NOCASE, time INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, codepage INTEGER NOT NULL);"
	#define VIEWERPOSITION_HISTORY_SCHEMA "(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE COLLATE NOCASE, time INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, hex INTEGER NOT NULL, codepage INTEGER NOT NULL);"

	static void Initialise(const db_initialiser& Db)
	{
		Db.SetWALJournalingMode();

		Db.add_nocase_collation();

		static const std::string_view Schema[]
		{
			//command,view,edit,folder,dialog history
			"CREATE TABLE IF NOT EXISTS history(id INTEGER PRIMARY KEY, kind INTEGER NOT NULL, key TEXT NOT NULL, type INTEGER NOT NULL, lock INTEGER NOT NULL, name TEXT NOT NULL, time INTEGER NOT NULL, guid TEXT NOT NULL, file TEXT NOT NULL, data TEXT NOT NULL);"sv,
			"CREATE INDEX IF NOT EXISTS history_idx1 ON history (kind, key);"sv,
			"CREATE INDEX IF NOT EXISTS history_idx2 ON history (kind, key, time);"sv,
			"CREATE INDEX IF NOT EXISTS history_idx3 ON history (kind, key, lock DESC, time DESC);"sv,
			"CREATE INDEX IF NOT EXISTS history_idx4 ON history (kind, key, time DESC);"sv,
			//view,edit file positions and bookmarks history
			"CREATE TABLE IF NOT EXISTS " EDITORPOSITION_HISTORY_NAME EDITORPOSITION_HISTORY_SCHEMA ""sv,
			"CREATE TABLE IF NOT EXISTS editorbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES editorposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"sv,
			"CREATE INDEX IF NOT EXISTS editorposition_history_idx1 ON editorposition_history (time DESC);"sv,
			"CREATE TABLE IF NOT EXISTS " VIEWERPOSITION_HISTORY_NAME VIEWERPOSITION_HISTORY_SCHEMA ""sv,
			"CREATE TABLE IF NOT EXISTS viewerbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES viewerposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"sv,
			"CREATE INDEX IF NOT EXISTS viewerposition_history_idx1 ON viewerposition_history (time DESC);"sv,
		};

		Db.Exec(Schema);

		reindex(Db);

		// Must be after reindex
		Db.EnableForeignKeysConstraints();

		static const stmt_init<statement_id> Statements[]
		{
			{ stmtEnum,                  "SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 AND (?3 OR name COLLATE NOCASE =?4) ORDER BY time;"sv },
			{ stmtEnumDesc,              "SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 AND (?3 OR name COLLATE NOCASE =?4) ORDER BY lock DESC, time DESC;"sv },
			{ stmtDel,                   "DELETE FROM history WHERE id=?1;"sv },
			{ stmtDeleteOldUnlocked,     "DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0 AND time<?3 AND id NOT IN (SELECT id FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT ?4);"sv },
			{ stmtEnumLargeHistories,    "SELECT key FROM (SELECT key, num FROM (SELECT key, count(id) as num FROM history WHERE kind=?1 GROUP BY key)) WHERE num > ?2;"sv },
			{ stmtAdd,                   "INSERT INTO history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9);"sv },
			{ stmtGetName,               "SELECT name FROM history WHERE id=?1;"sv },
			{ stmtGet,                   "SELECT name, type, time, guid, file, data FROM history WHERE id=?1;"sv },
			{ stmtCount,                 "SELECT count(id) FROM history WHERE kind=?1 AND key=?2;"sv },
			{ stmtDelUnlocked,           "DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0;"sv },
			{ stmtGetLock,               "SELECT lock FROM history WHERE id=?1;"sv },
			{ stmtSetLock,               "UPDATE history SET lock=?1 WHERE id=?2"sv },
			{ stmtGetNext,               "SELECT a.id, a.name, a.time FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time>b.time ORDER BY a.time LIMIT 1;"sv },
			{ stmtGetPrev,               "SELECT a.id, a.name, a.time FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time<b.time ORDER BY a.time DESC LIMIT 1;"sv },
			{ stmtGetNewest,             "SELECT id, name, time FROM history WHERE kind=?1 AND key=?2 ORDER BY time DESC LIMIT 1;"sv },
			{ stmtSetEditorPos,          "REPLACE INTO editorposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7);"sv },
			{ stmtSetEditorBookmark,     "REPLACE INTO editorbookmarks_history VALUES (?1,?2,?3,?4,?5,?6);"sv },
			{ stmtGetEditorPos,          "SELECT id, line, linepos, screenline, leftpos, codepage FROM editorposition_history WHERE name=?1;"sv },
			{ stmtGetEditorBookmark,     "SELECT line, linepos, screenline, leftpos FROM editorbookmarks_history WHERE pid=?1 AND num=?2;"sv },
			{ stmtSetViewerPos,          "REPLACE INTO viewerposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6);"sv },
			{ stmtSetViewerBookmark,     "REPLACE INTO viewerbookmarks_history VALUES (?1,?2,?3,?4);"sv },
			{ stmtGetViewerPos,          "SELECT id, filepos, leftpos, hex, codepage FROM viewerposition_history WHERE name=?1;"sv },
			{ stmtGetViewerBookmark,     "SELECT filepos, leftpos FROM viewerbookmarks_history WHERE pid=?1 AND num=?2;"sv },
			{ stmtDeleteOldEditor,       "DELETE FROM editorposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM editorposition_history ORDER BY time DESC LIMIT ?2);"sv },
			{ stmtDeleteOldViewer,       "DELETE FROM viewerposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM viewerposition_history ORDER BY time DESC LIMIT ?2);"sv },
		};

		Db.PrepareStatements(Statements);
	}

	static void reindex(db_initialiser const& Db)
	{
		static constexpr struct
		{
			std::string_view Name;
			string_view WideName;
			std::string_view Schema;
		}
		ReindexTables[]
		{
#define INIT_TABLE(x) CHAR_SV(x ## POSITION_HISTORY_NAME), WIDE_SV(x ## POSITION_HISTORY_NAME), CHAR_SV(x ## POSITION_HISTORY_SCHEMA)
			{ INIT_TABLE(EDITOR) },
			{ INIT_TABLE(VIEWER) },
#undef INIT_TABLE
		};

		for (const auto& Table: ReindexTables)
		{
			const auto reindex = [&]{ Db.Exec(far::format("REINDEX {}"sv, Table.Name)); };

			try
			{
				if (const auto stmtIntegrityCheck = Db.create_stmt(far::format("PRAGMA INTEGRITY_CHECK({})"sv, Table.Name)); stmtIntegrityCheck.Step())
				{
					if (const auto Result = stmtIntegrityCheck.GetColText(0); Result != L"ok"sv)
					{
						LOGWARNING(L"Integrity issue in {}: {}, reindexing the table"sv, Table.WideName, Result);
						reindex();
						LOGINFO(L"Reindexing of {} completed"sv, Table.WideName);
					}
				}
			}
			catch (far_sqlite_exception const& e)
			{
				LOGWARNING(L"{}"sv, e);

				if (!e.is_constraint_unique())
					throw;

				recreate_position_history(Db, Table.Name, Table.Schema);
				reindex();
			}
		}
	}

	static void recreate_position_history(const db_initialiser& Db, std::string_view const Table, std::string_view const Schema)
	{
		LOGNOTICE(L"Recreating {}"sv, encoding::utf8::get_chars(Table));

		SCOPED_ACTION(auto)(Db.ScopedTransaction());

		// The order is important - https://sqlite.org/lang_altertable.html

		// 1. Create new table
		Db.Exec(far::format("CREATE TABLE {}_new{}"sv, Table, Schema));

		// 2. Copy data. "WHERE 1=1" is a dirty hack to prevent xfer optimization.
		Db.Exec(far::format("INSERT OR IGNORE INTO {0}_new SELECT * FROM {0} WHERE 1=1"sv, Table));

		// 3. Drop old table
		Db.Exec(far::format("DROP TABLE {}"sv, Table));

		// 4. Rename new into old
		Db.Exec(far::format("ALTER TABLE {0}_new RENAME TO {0}"sv, Table));
	}

	void Delete(primary_key const id) override
	{
		WaitAllAsync();
		DeleteInternal(id);
	}

	auto EnumStmt(bool const Reverse) const
	{
		return AutoStatement(Reverse? stmtEnumDesc : stmtEnum);
	}

	bool Enum(const bool Reset, const unsigned int TypeHistory, const string_view HistoryName, const std::optional<string_view> ItemName, primary_key& id, string& Name, history_record_type& Type, bool& Lock, os::chrono::time_point& Time, string& strUuid, string& strFile, string& strData, const bool Reverse) override
	{
		WaitAllAsync();
		auto Stmt = EnumStmt(Reverse);

		if (Reset)
			Stmt->Reset().Bind(TypeHistory, HistoryName, !ItemName, ItemName.value_or(L""sv));

		if (!Stmt->Step())
			return false;

		id = Stmt->GetColInt64(0);
		Name = Stmt->GetColText(1);
		Type = static_cast<history_record_type>(Stmt->GetColInt(2));
		Lock = Stmt->GetColInt(3) != 0;
		Time = os::chrono::nt_clock::from_hectonanoseconds(Stmt->GetColInt64(4));
		strUuid = Stmt->GetColText(5);
		strFile = Stmt->GetColText(6);
		strData = Stmt->GetColText(7);
		KeepStatement(Stmt);
		return true;
	}

	void CloseEnum(bool const Reverse) const override
	{
		(void)EnumStmt(Reverse);
	}

	void DeleteAndAddAsync(primary_key const DeleteId, unsigned int const TypeHistory, string_view const HistoryName, string_view const Name, int const Type, bool const Lock, os::chrono::time_point const Time, string_view const Uuid, string_view const File, string_view const Data) override
	{
		auto item = std::make_unique<AsyncWorkItem>();
		item->DeleteId=DeleteId;
		item->TypeHistory=TypeHistory;
		item->HistoryName = HistoryName;
		item->Type=Type;
		item->Lock=Lock;
		item->strName = Name;
		item->Time = Time;
		item->strUuid = Uuid;
		item->strFile = File;
		item->strData = Data;

		WorkQueue.emplace(std::move(item));

		WaitAllAsync();
		AsyncDeleteAddDone.reset();
		AsyncWork.set();
	}

	void DeleteOldUnlocked(const unsigned int TypeHistory, const string_view HistoryName, const int DaysToKeep, const int MinimumEntries) override
	{
		WaitAllAsync();

		const auto older = os::chrono::nt_clock::to_hectonanoseconds(os::chrono::nt_clock::now() - std::chrono::days(DaysToKeep));
		ExecuteStatement(stmtDeleteOldUnlocked, TypeHistory, HistoryName, older, MinimumEntries);
	}

	auto EnumLargeHistoriesStmt() const
	{
		return AutoStatement(stmtEnumLargeHistories);
	}

	bool EnumLargeHistories(const bool Reset, const unsigned int TypeHistory, const int MinimumEntries, string& strHistoryName) override
	{
		WaitAllAsync();
		auto Stmt = EnumLargeHistoriesStmt();

		if (Reset)
			Stmt->Reset().Bind(TypeHistory, MinimumEntries);

		if (!Stmt->Step())
			return false;

		strHistoryName = Stmt->GetColText(0);
		KeepStatement(Stmt);
		return true;
	}

	void CloseEnumLargeHistories() const override
	{
		(void)EnumLargeHistoriesStmt();
	}

	bool Get(primary_key const id, string* const Name = {}, history_record_type* const Type = {}, os::chrono::time_point* const Time = {}, string* const Uuid = {}, string* const File = {}, string* const Data = {}) override
	{
		WaitAllAsync();

		const auto StmtId = (Type || Time || Uuid || File || Data)? stmtGet : stmtGetName;

		const auto Stmt = AutoStatement(StmtId);
		if (!Stmt->Bind(id).Step())
			return false;

		if (Name)
			*Name = Stmt->GetColText(0);

		if (Type)
			*Type = static_cast<history_record_type>(Stmt->GetColInt(1));

		if (Time)
			*Time = os::chrono::nt_clock::from_hectonanoseconds(Stmt->GetColInt64(2));

		if (Uuid)
			*Uuid = Stmt->GetColText(3);

		if (File)
			*File = Stmt->GetColText(4);

		if (Data)
			*Data = Stmt->GetColText(5);

		return true;
	}

	DWORD Count(const unsigned int TypeHistory, const string_view HistoryName) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtCount);
		return Stmt->Bind(TypeHistory, HistoryName).Step()? static_cast<DWORD>(Stmt-> GetColInt(0)) : 0;
	}

	void FlipLock(primary_key const id) override
	{
		WaitAllAsync();
		ExecuteStatement(stmtSetLock, !IsLocked(id), id);
	}

	bool IsLocked(primary_key const id) override
	{
		WaitAllAsync();
		const auto Stmt = AutoStatement(stmtGetLock);
		return Stmt->Bind(id).Step() && Stmt->GetColInt(0) != 0;
	}

	void DeleteAllUnlocked(const unsigned int TypeHistory, const string_view HistoryName) override
	{
		WaitAllAsync();
		ExecuteStatement(stmtDelUnlocked, TypeHistory, HistoryName);
	}

	primary_key GetNext(const unsigned int TypeHistory, const string_view HistoryName, primary_key const id, string& Name, os::chrono::time_point& Time) override
	{
		WaitAllAsync();
		Name.clear();

		if (!id)
			return 0;

		const auto Stmt = AutoStatement(stmtGetNext);
		if (!Stmt->Bind(id, TypeHistory, HistoryName).Step())
			return 0;

		Name = Stmt->GetColText(1);
		Time = os::chrono::nt_clock::from_hectonanoseconds(Stmt->GetColInt64(2));
		return Stmt->GetColInt64(0);
	}

	primary_key GetPrev(const unsigned int TypeHistory, const string_view HistoryName, primary_key const id, string& Name, os::chrono::time_point& Time) override
	{
		return GetPrevImpl(TypeHistory, HistoryName, id, Name, Time, [&]{ return Get(id, &Name)? id : 0; });
	}

	primary_key CyclicGetPrev(const unsigned int TypeHistory, const string_view HistoryName, primary_key const id, string& Name, os::chrono::time_point& Time) override
	{
		return GetPrevImpl(TypeHistory, HistoryName, id, Name, Time, []{ return 0; });
	}

	primary_key SetEditorPos(const string_view Name, os::chrono::time_point const Time, const int Line, const int LinePos, const int ScreenLine, const int LeftPos, const uintptr_t CodePage) override
	{
		WaitCommitAsync();
		ExecuteStatement(stmtSetEditorPos, Name, os::chrono::nt_clock::to_hectonanoseconds(Time), Line, LinePos, ScreenLine, LeftPos, CodePage);
		return LastInsertRowID();
	}

	unsigned long long GetEditorPos(const string_view Name, int& Line, int& LinePos, int& ScreenLine, int& LeftPos, uintptr_t& CodePage) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetEditorPos);
		if (!Stmt->Bind(Name).Step())
			return 0;

		Line = Stmt->GetColInt(1);
		LinePos = Stmt->GetColInt(2);
		ScreenLine = Stmt->GetColInt(3);
		LeftPos = Stmt->GetColInt(4);
		CodePage = Stmt->GetColInt(5);
		return Stmt->GetColInt64(0);
	}

	void SetEditorBookmark(primary_key const id, size_t i, int Line, int LinePos, int ScreenLine, int LeftPos) override
	{
		WaitCommitAsync();
		ExecuteStatement(stmtSetEditorBookmark, id, i, Line, LinePos, ScreenLine, LeftPos);
	}

	bool GetEditorBookmark(primary_key const id, size_t i, int& Line, int& LinePos, int& ScreenLine, int& LeftPos) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetEditorBookmark);
		if (!Stmt->Bind(id, i).Step())
			return false;

		Line = Stmt->GetColInt(0);
		LinePos = Stmt->GetColInt(1);
		ScreenLine = Stmt->GetColInt(2);
		LeftPos = Stmt->GetColInt(3);
		return true;
	}

	primary_key SetViewerPos(const string_view Name, os::chrono::time_point const Time, const long long FilePos, const long long LeftPos, const int Hex_Wrap, uintptr_t const CodePage) override
	{
		WaitCommitAsync();
		ExecuteStatement(stmtSetViewerPos, Name, os::chrono::nt_clock::to_hectonanoseconds(Time), FilePos, LeftPos, Hex_Wrap, CodePage);
		return LastInsertRowID();
	}

	unsigned long long GetViewerPos(const string_view Name, long long& FilePos, long long& LeftPos, int& Hex, uintptr_t& CodePage) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetViewerPos);

		if (!Stmt->Bind(Name).Step())
			return 0;

		FilePos = Stmt->GetColInt64(1);
		LeftPos = Stmt->GetColInt64(2);
		Hex = Stmt->GetColInt(3);
		CodePage = Stmt->GetColInt(4);
		return Stmt->GetColInt64(0);
	}

	void SetViewerBookmark(primary_key const id, size_t i, long long FilePos, long long LeftPos) override
	{
		WaitCommitAsync();
		ExecuteStatement(stmtSetViewerBookmark, id, i, FilePos, LeftPos);
	}

	bool GetViewerBookmark(primary_key const id, size_t i, long long& FilePos, long long& LeftPos) override
	{
		WaitCommitAsync();
		const auto Stmt = AutoStatement(stmtGetViewerBookmark);
		if (!Stmt->Bind(id, i).Step())
			return false;

		FilePos = Stmt->GetColInt64(0);
		LeftPos = Stmt->GetColInt64(1);
		return true;
	}

	void DeleteOldPositions(int DaysToKeep, int MinimumEntries) override
	{
		WaitCommitAsync();
		const auto older = os::chrono::nt_clock::to_hectonanoseconds(os::chrono::nt_clock::now() - std::chrono::days(DaysToKeep));
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
		stmtGet,
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

class HistoryConfigDb final: public HistoryConfigCustom
{
public:
	explicit HistoryConfigDb(string_view const Name):
		HistoryConfigCustom(Name)
	{
	}

private:
	// TODO: implementation
	void Import(const representation_source&) override
	{
		LOGNOTICE(L"History import not implemented"sv);
	}

	void Export(representation_destination&) const override
	{
		LOGNOTICE(L"History export not implemented"sv);
	}
};

class HistoryConfigMemory final: public HistoryConfigCustom
{
public:
	explicit HistoryConfigMemory(string_view const Name):
		HistoryConfigCustom(Name)
	{
	}

private:
	void Import(const representation_source&) override {}
	void Export(representation_destination&) const override {}
};

bool is_uuid(string_view const Str)
{
	return uuid::try_parse(Str).has_value();
}

}

void config_provider::TryImportDatabase(representable& p, const char* NodeName, bool IsPlugin)
{
	if (!m_TemplateSource && !Global->Opt->TemplateProfilePath.empty() && os::fs::exists(Global->Opt->TemplateProfilePath))
	{
		m_TemplateSource = std::make_unique<representation_source>(Global->Opt->TemplateProfilePath);
	}

	if (m_TemplateSource && m_TemplateSource->Root().ToNode())
	{
		auto root = m_TemplateSource->Root();

		if (!NodeName)
		{
			p.Import(*m_TemplateSource);
		}
		else if (!IsPlugin)
		{
			m_TemplateSource->SetRoot(root.FirstChildElement(NodeName));
			p.Import(*m_TemplateSource);
		}
		else
		{
			for (const auto& i: xml_enum(root.FirstChildElement("pluginsconfig"), "plugin"))
			{
				const auto Uuid = i.Attribute("guid");
				if (Uuid && 0 == strcmp(Uuid, NodeName))
				{
					m_TemplateSource->SetRoot(&const_cast<tinyxml::XMLElement&>(i));
					p.Import(*m_TemplateSource);
					break;
				}
			}
		}
		m_TemplateSource->SetRoot(root);
	}
}

void config_provider::ImportDatabase(auto& Database, const char* ImportNodeName, bool IsPlugin)
{
	if (m_Mode != mode::m_import && Database.IsNew())
	{
		TryImportDatabase(Database, ImportNodeName, IsPlugin);
	}
}

static string GetDatabasePath(string_view const FileName, bool const Local)
{
	return FileName == SQLiteDb::memory_db_name?
		string(FileName) :
		path::join(Local? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath, FileName) + L".db"sv;
}

static string rename_bad_database(string_view const Name)
{
	for (size_t i = 0; ; ++i)
	{
		const auto Dest = far::format(L"{}.bad{}"sv, Name, i? far::format(L".{}"sv, Name) : L""sv);
		if (os::fs::move_file(Name, Dest))
			return Dest;

		const auto ErrorState = os::last_error();
		if (ErrorState.Win32Error == ERROR_ALREADY_EXISTS)
			continue;

		LOGWARNING(L"move_file({}): {}"sv, Name, ErrorState);
		return {};
	}
}


template<class T>
std::unique_ptr<T> config_provider::CreateWithFallback(string_view const Name)
{
	const auto Report = [&](string_view const Msg)
	{
		if (m_Mode != mode::m_default)
			std::wcerr << Msg << std::endl;
		else
			m_Problems.emplace_back(Msg);
	};

	try
	{
		return std::make_unique<T>(Name);
	}
	catch (far_sqlite_exception const& e1)
	{
		Report(concat(Name, L':'));
		Report(concat(L"  "sv, e1.message()));

		string NewName;

		auto OpenInMemory = Global->Opt->ReadOnlyConfig;
		if (!OpenInMemory)
		{
			NewName = rename_bad_database(Name);
			OpenInMemory = NewName.empty();
		}

		if (OpenInMemory)
		{
			Report(L"  - database is opened in memory"sv);
			return std::make_unique<T>(SQLiteDb::memory_db_name);
		}

		try
		{
			auto Result = std::make_unique<T>(Name);
			Report(far::format(L"  - database file is renamed to {} and new one is created"sv, PointToName(NewName)));
			return Result;
		}
		catch (far_sqlite_exception const& e2)
		{
			Report(concat(L"  "sv, e2.message()));
			Report(L"  - database is opened in memory"sv);
			return std::make_unique<T>(SQLiteDb::memory_db_name);
		}
	}
}

template<class T>
std::unique_ptr<T> config_provider::CreateDatabase(string_view const Name, bool const Local)
{
	const auto FullName = GetDatabasePath(Name, Local);

	os::mutex m(os::make_name<os::mutex>(Local? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath, Name));
	SCOPED_ACTION(std::scoped_lock)(m);

	auto Database = CreateWithFallback<T>(FullName);

	ImportDatabase(*Database, nullptr, false);
	return Database;
}

template<class T>
HierarchicalConfigUniquePtr config_provider::CreateHierarchicalConfig(dbcheck DbId, string_view const DbName, const char* ImportNodeName, bool Local, bool IsPlugin, bool UseFallback)
{
	const auto FullName = GetDatabasePath(DbName, Local);

	auto Database = UseFallback? CreateWithFallback<T>(FullName) : std::make_unique<T>(FullName);

	if (!m_CheckedDb.Check(DbId))
	{
		ImportDatabase(*Database, ImportNodeName, IsPlugin);
		m_CheckedDb.Set(DbId);
	}
	return HierarchicalConfigUniquePtr(Database.release());
}

enum dbcheck: int
{
	CHECK_NONE       = 0,
	CHECK_FILTERS    = 0_bit,
	CHECK_HIGHLIGHT  = 1_bit,
	CHECK_SHORTCUTS  = 2_bit,
	CHECK_PANELMODES = 3_bit,
};

HierarchicalConfigUniquePtr config_provider::CreatePluginsConfig(const string_view Uuid, const bool Local, bool UseFallback)
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_NONE, path::join(L"PluginsData"sv, Uuid), encoding::utf8::get_bytes(Uuid).c_str(), Local, true, UseFallback);
}

HierarchicalConfigUniquePtr config_provider::CreateFiltersConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_FILTERS, L"filters"sv, "filters");
}

HierarchicalConfigUniquePtr config_provider::CreateHighlightConfig()
{
	return CreateHierarchicalConfig<HighlightHierarchicalConfigDb>(CHECK_HIGHLIGHT, L"highlight"sv, "highlight");
}

HierarchicalConfigUniquePtr config_provider::CreateShortcutsConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_SHORTCUTS, L"shortcuts"sv, "shortcuts", true);
}

HierarchicalConfigUniquePtr config_provider::CreatePanelModesConfig()
{
	return CreateHierarchicalConfig<HierarchicalConfigDb>(CHECK_PANELMODES, L"panelmodes"sv, "panelmodes");
}

config_provider::implementation::implementation()
{
	SQLiteDb::library_load();
}

config_provider::implementation::~implementation()
{
	SQLiteDb::library_free();
}


static auto pluginscache_db_name()
{
	return far::format(L"plugincache.{}"sv, build::platform());
}


config_provider::config_provider(mode Mode):
	m_Mode(Mode),
	m_GeneralCfg([this]{ return CreateDatabase<GeneralConfigDb>(L"generalconfig"sv, false); }),
	m_LocalGeneralCfg([this]{ return CreateDatabase<LocalGeneralConfigDb>(L"localconfig"sv, true); }),
	m_ColorsCfg([this]{ return CreateDatabase<ColorsConfigDb>(L"colors"sv, false); }),
	m_AssocConfig([this]{ return CreateDatabase<AssociationsConfigDb>(L"associations"sv, false); }),
	m_PlCacheCfg([this]{ return CreateDatabase<PluginsCacheConfigDb>(pluginscache_db_name(), true); }),
	m_PlHotkeyCfg([this]{ return CreateDatabase<PluginsHotkeysConfigDb>(L"pluginhotkeys"sv, false); }),
	m_HistoryCfg([this]{ return CreateDatabase<HistoryConfigDb>(L"history"sv, true); }),
	m_HistoryCfgMem([this]{ return CreateDatabase<HistoryConfigMemory>(SQLiteDb::memory_db_name, true); })
{
}

config_provider::config_provider(clear_cache)
{
	PluginsCacheConfigDb(GetDatabasePath(pluginscache_db_name(), true)).DiscardCache();
}

config_provider::~config_provider()
{
	// Make sure all threads are joined before freeing the library
	m_Threads.clear();
}

void config_provider::Export(string_view const File)
{
	representation_destination Representation;
	auto& root = Representation.Root();
	const auto Version = build::version();
	SetAttribute(root, "version", far::format("{}.{}.{}"sv, Version.Major, Version.Minor, Version.Build));

	GeneralCfg()->Export(Representation);
	LocalGeneralCfg()->Export(Representation);
	ColorsCfg()->Export(Representation);
	AssocConfig()->Export(Representation);
	PlHotkeyCfg()->Export(Representation);
	HistoryCfg()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "filters"));
	CreateFiltersConfig()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "highlight"));
	CreateHighlightConfig()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "panelmodes"));
	CreatePanelModesConfig()->Export(Representation);
	Representation.SetRoot(CreateChild(root, "shortcuts"));
	CreateShortcutsConfig()->Export(Representation);

	{
		const auto Ext = L"*.db"sv;
		//TODO: export local plugin settings
		auto& e = CreateChild(root, "pluginsconfig");
		for(const auto& i: os::fs::enum_files(path::join(Global->Opt->ProfilePath, L"PluginsData"sv, Ext)))
		{
			if (!os::fs::is_file(i))
				continue;

			const auto FileName = name_ext(i.FileName).first;
			if (!is_uuid(FileName))
				continue;

			auto& PluginRoot = CreateChild(e, "plugin");
			SetAttribute(PluginRoot, "guid", encoding::utf8::get_bytes(FileName));
			Representation.SetRoot(PluginRoot);
			CreatePluginsConfig(FileName)->Export(Representation);
		}
	}

	return Representation.Save(File);
}

void config_provider::ServiceMode(string_view const File)
{
	switch (m_Mode)
	{
	case mode::m_import: return Import(File);
	case mode::m_export: return Export(File);
	default:
		std::unreachable();
	}
}

void config_provider::Import(string_view const File)
{
	representation_source Representation(File);

	auto root = Representation.Root();

	GeneralCfg()->Import(Representation);
	LocalGeneralCfg()->Import(Representation);
	ColorsCfg()->Import(Representation);
	AssocConfig()->Import(Representation);
	PlHotkeyCfg()->Import(Representation);
	HistoryCfg()->Import(Representation);
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
		const auto UuidStr = plugin.Attribute("guid");
		if (!UuidStr)
			continue;

		if (const auto Uuid = encoding::utf8::get_chars(UuidStr); is_uuid(Uuid))
		{
			Representation.SetRoot(&const_cast<tinyxml::XMLElement&>(plugin));
			CreatePluginsConfig(Uuid)->Import(Representation);
		}
	}
}

bool config_provider::ShowProblems() const
{
	if (m_Problems.empty())
		return false;

	return Message(MSG_WARNING | MSG_LEFTALIGN,
		msg(lng::MProblemDb),
		std::move(m_Problems),
		{ lng::MShowConfigFolders, lng::MIgnore }) == message_result::first_button;
}

void config_provider::AsyncCall(async_key, const std::function<void()>& Routine)
{
	std::erase_if(m_Threads, [](const os::thread& i){ return i.is_signaled(); });
	m_Threads.emplace_back(Routine);
}

config_provider& ConfigProvider()
{
	return *Global->m_ConfigProvider;
}

int HierarchicalConfig::ToSettingsType(int Type)
{
	switch (static_cast<SQLiteDb::column_type>(Type))
	{
	case SQLiteDb::column_type::integer:
		return FST_QWORD;

	case SQLiteDb::column_type::string:
		return FST_STRING;

	case SQLiteDb::column_type::blob:
		return FST_DATA;

	case SQLiteDb::column_type::unknown:
	default:
		return FST_UNKNOWN;
	}
}
