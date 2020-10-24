/*
sqlitedb.cpp

обёртка sqlite api для c++.
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
#include "sqlitedb.hpp"

// Internal:
#include "pathmix.hpp"
#include "config.hpp"
#include "components.hpp"
#include "encoding.hpp"
#include "delete.hpp"
#include "mix.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/bytes_view.hpp"
#include "common/function_ref.hpp"
#include "common/string_utils.hpp"
#include "common/uuid.hpp"

// External:
#include "format.hpp"
#include "sqlite.hpp"
#include "sqlite_unicode.hpp"

//----------------------------------------------------------------------------

namespace
{
	class lock
	{
	public:
		NONCOPYABLE(lock);

		explicit lock(sqlite::sqlite3* Db):
			m_Db(Db)
		{
			sqlite::sqlite3_mutex_enter(sqlite::sqlite3_db_mutex(m_Db));
		}

		~lock()
		{
			sqlite::sqlite3_mutex_leave(sqlite::sqlite3_db_mutex(m_Db));
		}

	private:
		sqlite::sqlite3* m_Db;
	};

	int GetLastErrorCode(sqlite::sqlite3* Db)
	{
		return sqlite::sqlite3_extended_errcode(Db);
	}

	int GetLastSystemErrorCode(sqlite::sqlite3* Db)
	{
		return sqlite::sqlite3_system_errno(Db);
	}

	string GetErrorString(int ErrorCode)
	{
		return encoding::utf8::get_chars(sqlite::sqlite3_errstr(ErrorCode));
	}

	string GetDatabaseName(sqlite::sqlite3* Db, string_view const DbName = {})
	{
		if (!DbName.empty())
			return string(PointToName(DbName));

		const auto NamePtr = sqlite::sqlite3_db_filename(Db, "main");
		const string Name(NamePtr? *NamePtr? encoding::utf8::get_chars(NamePtr) : SQLiteDb::memory_db_name() : L"unknown"sv);
		return string(PointToName(Name));
	}

	string GetLastErrorString(sqlite::sqlite3* Db)
	{
		return encoding::utf8::get_chars(sqlite::sqlite3_errmsg(Db));
	}

	[[noreturn]]
	void throw_exception(string_view const DatabaseName, int const ErrorCode, string_view const ErrorString = {}, int const SystemErrorCode = 0)
	{
		throw MAKE_EXCEPTION(far_sqlite_exception, format(FSTR(L"[{0}] - SQLite error {1}: {2}{3}"),
			DatabaseName,
			ErrorCode,
			ErrorString.empty()? GetErrorString(ErrorCode) : ErrorString,
			SystemErrorCode? format(FSTR(L" ({0})"), os::format_system_error(SystemErrorCode, os::GetErrorString(false, SystemErrorCode))) : L""s
		));
	}

	[[noreturn]]
	void throw_exception(sqlite::sqlite3* Db, string_view const DbName = {})
	{
		throw_exception(GetDatabaseName(Db, DbName), GetLastErrorCode(Db), GetLastErrorString(Db), GetLastSystemErrorCode(Db));
	}

	void invoke(sqlite::sqlite3* Db, function_ref<bool()> const Callable)
	{
		SCOPED_ACTION(lock)(Db);

		if (!Callable())
			throw_exception(Db);
	}

	SCOPED_ACTION(components::component)([]
	{
		return components::info{ L"SQLite"sv, WIDE_S(SQLITE_VERSION) };
	});

	SCOPED_ACTION(components::component)([]
	{
		return components::info{ L"SQLite Unicode extension"sv, sqlite_unicode::SQLite_Unicode_Version };
	});
}

void SQLiteDb::library_load()
{
	[[maybe_unused]]
	const auto Result = sqlite::sqlite3_initialize() == SQLITE_OK && sqlite_unicode::sqlite3_unicode_load() == SQLITE_OK;
	assert(Result);
}

void SQLiteDb::library_free()
{
	sqlite_unicode::sqlite3_unicode_free();

	[[maybe_unused]]
	const auto Result = sqlite::sqlite3_shutdown();
	assert(Result == SQLITE_OK);
}

void SQLiteDb::SQLiteStmt::stmt_deleter::operator()(sqlite::sqlite3_stmt* Object) const noexcept
{
	// https://www.sqlite.org/c3ref/finalize.html
	// If the most recent evaluation of the statement encountered no errors
	// or if the statement is never been evaluated, then sqlite3_finalize() returns SQLITE_OK.
	// If the most recent evaluation of statement S failed, then sqlite3_finalize(S)
	// returns the appropriate error code or extended error code.

	// All statement evaluations are checked so returning the error again makes no sense.
	// This is called from a destructor so we can't throw here.
	invoke(sqlite::sqlite3_db_handle(Object), [&]
	{
		[[maybe_unused]]
		const auto Result = sqlite::sqlite3_finalize(Object);
		assert(Result == SQLITE_OK);
		return true;
	});
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Reset()
{
	invoke(db(), [&]{ return sqlite::sqlite3_clear_bindings(m_Stmt.get()) == SQLITE_OK; });

	// https://www.sqlite.org/c3ref/reset.html
	// If the most recent call to sqlite3_step(S) for the prepared statement S returned SQLITE_ROW or SQLITE_DONE,
	// or if sqlite3_step(S) has never before been called on S, then sqlite3_reset(S) returns SQLITE_OK.
	// If the most recent call to sqlite3_step(S) for the prepared statement S indicated an error,
	// then sqlite3_reset(S) returns an appropriate error code.

	// All sqlite3_step calls are checked so returning the error again makes no sense.
	// This is called from a destructor so we can't throw here.
	invoke(db(), [&]
	{
		[[maybe_unused]]
		const auto Result = sqlite::sqlite3_reset(m_Stmt.get());
		assert(Result == SQLITE_OK);
		return true;
	});

	m_Param = 0;
	return *this;
}

bool SQLiteDb::SQLiteStmt::Step() const
{
	bool Result = false;
	invoke(db(), [&]
	{
		const auto StepResult = sqlite::sqlite3_step(m_Stmt.get());
		Result = StepResult == SQLITE_ROW;
		return Result || StepResult == SQLITE_DONE;
	});
	return Result;
}

void SQLiteDb::SQLiteStmt::Execute() const
{
	invoke(db(), [&]
	{
		const auto StepResult = sqlite::sqlite3_step(m_Stmt.get());
		return StepResult == SQLITE_DONE || StepResult == SQLITE_ROW;
	});
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(int Value)
{
	invoke(db(), [&]{ return sqlite::sqlite3_bind_int(m_Stmt.get(), ++m_Param, Value) == SQLITE_OK; });
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(long long Value)
{
	invoke(db(), [&]{ return sqlite::sqlite3_bind_int64(m_Stmt.get(), ++m_Param, Value) == SQLITE_OK; });
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(const string_view Value)
{
	// https://www.sqlite.org/c3ref/bind_blob.html
	// If the third parameter to sqlite3_bind_text() or sqlite3_bind_text16() or sqlite3_bind_blob() is a NULL pointer
	// then the fourth parameter is ignored and the end result is the same as sqlite3_bind_null().

	// And this is how you get a NULL constraint violation with empty strings. Truly amazing design. *facepalm*

	invoke(db(), [&]
	{
		const auto ValueUtf8 = encoding::utf8::get_bytes(Value);
		return sqlite::sqlite3_bind_text(m_Stmt.get(), ++m_Param, NullToEmpty(ValueUtf8.data()), static_cast<int>(ValueUtf8.size()), sqlite::transient_destructor) == SQLITE_OK;
	});
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(bytes_view const Value)
{
	invoke(db(), [&]{ return sqlite::sqlite3_bind_blob(m_Stmt.get(), ++m_Param, Value.data(), static_cast<int>(Value.size()), sqlite::transient_destructor) == SQLITE_OK; });
	return *this;
}

sqlite::sqlite3* SQLiteDb::SQLiteStmt::db() const
{
	return sqlite::sqlite3_db_handle(m_Stmt.get());
}

static std::string_view get_column_text(sqlite::sqlite3_stmt* Stmt, int Col)
{
	// https://www.sqlite.org/c3ref/column_blob.html
	// call sqlite3_column_text() first to force the result into the desired format,
	// then invoke sqlite3_column_bytes() to find the size of the result
	const auto Data = view_as<char const*>(sqlite::sqlite3_column_text(Stmt, Col));
	const auto Size = static_cast<size_t>(sqlite::sqlite3_column_bytes(Stmt, Col));

	return { Data, Size };
}

string SQLiteDb::SQLiteStmt::GetColText(int Col) const
{
	return encoding::utf8::get_chars(get_column_text(m_Stmt.get(), Col));
}

std::string SQLiteDb::SQLiteStmt::GetColTextUTF8(int Col) const
{
	return std::string(get_column_text(m_Stmt.get(), Col));
}

int SQLiteDb::SQLiteStmt::GetColInt(int Col) const
{
	return sqlite::sqlite3_column_int(m_Stmt.get(), Col);
}

unsigned long long SQLiteDb::SQLiteStmt::GetColInt64(int Col) const
{
	return sqlite::sqlite3_column_int64(m_Stmt.get(), Col);
}

bytes SQLiteDb::SQLiteStmt::GetColBlob(int Col) const
{
	// https://www.sqlite.org/c3ref/column_blob.html
	// call sqlite3_column_blob() first to force the result into the desired format,
	// then invoke sqlite3_column_bytes() to find the size of the result
	const auto Data = static_cast<std::byte const*>(sqlite::sqlite3_column_blob(m_Stmt.get(), Col));
	const auto Size = static_cast<size_t>(sqlite::sqlite3_column_bytes(m_Stmt.get(), Col));
	return { Data, Size };
}

SQLiteDb::column_type SQLiteDb::SQLiteStmt::GetColType(int Col) const
{
	switch (sqlite::sqlite3_column_type(m_Stmt.get(), Col))
	{
	case SQLITE_INTEGER:
		return column_type::integer;
	case SQLITE_TEXT:
		return column_type::string;
	case SQLITE_BLOB:
		return column_type::blob;
	default:
		return column_type::unknown;
	}
}


SQLiteDb::SQLiteDb(busy_handler BusyHandler, initialiser Initialiser, string_view const DbName, bool WAL):
	m_Db(Open(DbName, BusyHandler, WAL)),
	// https://www.sqlite.org/isolation.html
	// If X starts a transaction that will initially only read but X knows it will eventually want to write
	// and does not want to be troubled with possible SQLITE_BUSY_SNAPSHOT errors that arise because
	// another connection jumped ahead of it in line, then X can issue BEGIN IMMEDIATE
	// to start its transaction instead of just an ordinary BEGIN.
	// The BEGIN IMMEDIATE command goes ahead and starts a write transaction,
	// and thus blocks all other writers. If the BEGIN IMMEDIATE operation succeeds,
	// then no subsequent operations in that transaction will ever fail with an SQLITE_BUSY error.
	m_stmt_BeginTransaction(create_stmt("BEGIN EXCLUSIVE"sv)),
	m_stmt_EndTransaction(create_stmt("END"sv)),
	m_Init(((void)Initialiser(db_initialiser(this)), init{})) // yay, operator comma!
{
}

void SQLiteDb::db_closer::operator()(sqlite::sqlite3* Object) const noexcept
{
	[[maybe_unused]]
	const auto Result = sqlite::sqlite3_close(Object);
	assert(Result == SQLITE_OK);
}

class SQLiteDb::implementation
{
public:
	static database_ptr open(string_view const Name, const std::pair<busy_handler, void*>& BusyHandler, bool Writable = true)
	{
		database_ptr Db;

		int Retires = 0;

		for (;;)
		{
			const auto Result = sqlite::sqlite3_open_v2(encoding::utf8::get_bytes(Name).c_str(), &ptr_setter(Db), Writable? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READONLY, nullptr);
			if (Result == SQLITE_OK)
				break;

			if (Result == SQLITE_BUSY && BusyHandler.first && BusyHandler.first(BusyHandler.second, Retires++))
				continue;

			if (Db)
				// Even though we have a db connection here, the database is not attached so we need to pass the name explicitly
				throw_exception(Db.get(), Name);
			else
				throw_exception(Name, Result);
		}

		invoke(Db.get(), [&]{ return sqlite::sqlite3_busy_handler(Db.get(), BusyHandler.first, BusyHandler.second) == SQLITE_OK; });

		return Db;
	}

	static database_ptr copy_db_to_memory(string_view const Path, const std::pair<busy_handler, void*>& BusyHandler, bool WAL)
	{
		auto Destination = open(memory_db_name(), {});

		database_ptr SourceDb;
		delayed_deleter Deleter(true);

		if (WAL && !os::fs::can_create_file(concat(Path, L'.', uuid::str(os::uuid::generate())))) // can't open db -- copy to %TEMP%
		{
			const auto TmpDbPath = concat(MakeTemp(), str(GetCurrentProcessId()), L'-', PointToName(Path));
			if (!os::fs::copy_file(Path, TmpDbPath, nullptr, nullptr, nullptr, 0))
				throw_exception(Path, SQLITE_READONLY);

			Deleter.add(TmpDbPath);
			(void)os::fs::set_file_attributes(TmpDbPath, FILE_ATTRIBUTE_NORMAL); //BUGBUG
			SourceDb = open(TmpDbPath, BusyHandler);
		}
		else
		{
			SourceDb = open(Path, BusyHandler, WAL);
		}

		struct backup_closer
		{
			void operator()(sqlite::sqlite3_backup* Backup) const noexcept
			{
				[[maybe_unused]]
				const auto Result = sqlite::sqlite3_backup_finish(Backup);
				assert(Result == SQLITE_OK);
			}
		};

		using backup_ptr = std::unique_ptr<sqlite::sqlite3_backup, backup_closer>;

		backup_ptr DbBackup;
		invoke(Destination.get(), [&]{ DbBackup.reset(sqlite::sqlite3_backup_init(Destination.get(), "main", SourceDb.get(), "main")); return DbBackup != nullptr; });

		const auto StepResult = sqlite::sqlite3_backup_step(DbBackup.get(), -1);
		if (StepResult != SQLITE_DONE)
			throw_exception(GetDatabaseName(SourceDb.get()), StepResult);

		return Destination;
	}

	static database_ptr try_copy_db_to_memory(string_view const Path, const std::pair<busy_handler, void*>& BusyHandler, bool WAL)
	{
		try
		{
			return copy_db_to_memory(Path, BusyHandler, WAL);
		}
		catch (const far_sqlite_exception&)
		{
			return open(memory_db_name(), {});
		}
	}
};

SQLiteDb::database_ptr SQLiteDb::Open(string_view const Path, busy_handler BusyHandler, bool const WAL)
{
	const auto MemDb = Path == memory_db_name();
	m_DbExists = !MemDb && os::fs::is_file(Path);

	if (!Global->Opt->ReadOnlyConfig || MemDb)
	{
		m_Path = Path;
		return implementation::open(Path, { BusyHandler, this });
	}

	m_Path = memory_db_name();
	return m_DbExists?
		implementation::try_copy_db_to_memory(Path, { BusyHandler, this }, WAL) :
		implementation::open(memory_db_name(), {});
}

void SQLiteDb::Exec(span<std::string_view const> const Commands) const
{
	for (const auto& i: Commands)
	{
		create_stmt(i, false).Execute();
	}
}

void SQLiteDb::BeginTransaction()
{
	if (!m_ActiveTransactions)
		auto_statement(&m_stmt_BeginTransaction)->Execute();
	++m_ActiveTransactions;
}

void SQLiteDb::EndTransaction()
{
	--m_ActiveTransactions;
	if (!m_ActiveTransactions)
		auto_statement(&m_stmt_EndTransaction)->Execute();
}

SQLiteDb::SQLiteStmt SQLiteDb::create_stmt(std::string_view const Stmt, bool Persistent) const
{
	sqlite::sqlite3_stmt* pStmt;

	// https://www.sqlite.org/c3ref/prepare.html
	// If the caller knows that the supplied string is nul-terminated,
	// then there is a small performance advantage to passing an nByte parameter
	// that is the number of bytes in the input string *including* the nul-terminator.

	// We use data() instead of operator[] here to bypass any bounds checks in debug mode
	const auto IsNullTerminated = Stmt.data()[Stmt.size()] == L'\0';

	invoke(m_Db.get(), [&]{ return sqlite::sqlite3_prepare_v3(m_Db.get(), Stmt.data(), static_cast<int>(Stmt.size() + (IsNullTerminated? 1 : 0)), Persistent? SQLITE_PREPARE_PERSISTENT : 0, &pStmt, nullptr) == SQLITE_OK; });

	return SQLiteStmt(pStmt);
}

unsigned long long SQLiteDb::LastInsertRowID() const
{
	return sqlite::sqlite3_last_insert_rowid(m_Db.get());
}

void SQLiteDb::Close()
{
	// https://www.sqlite.org/c3ref/close.html
	// If the database connection is associated with unfinalized prepared statements or unfinished sqlite3_backup objects
	// then sqlite3_close() will leave the database connection open and return SQLITE_BUSY.
	m_Statements.clear();
	m_Db.reset();
}

void SQLiteDb::SetWALJournalingMode() const
{
	Exec({ "PRAGMA journal_mode = WAL;"sv });
}

void SQLiteDb::EnableForeignKeysConstraints() const
{
	Exec({ "PRAGMA foreign_keys = ON;"sv });
}

template<typename char_type>
static auto view(const void* const Data, int const Size)
{
	return std::basic_string_view<char_type>{ static_cast<char_type const*>(Data), static_cast<size_t>(Size) / sizeof(char_type) };
}

void SQLiteDb::CreateNumericCollation() const
{
	const auto Comparer = [](void* const Param, int const Size1, const void* const Data1, int const Size2, const void* const Data2)
	{
		const auto Utf8 = reinterpret_cast<intptr_t>(Param) == SQLITE_UTF8;

		return string_sort::keyhole::compare_ordinal_numeric(
			Utf8? encoding::utf8::get_chars(view<char>(Data1, Size1)) : view<wchar_t>(Data1, Size1),
			Utf8? encoding::utf8::get_chars(view<char>(Data2, Size2)) : view<wchar_t>(Data2, Size2)
		);
	};

	invoke(m_Db.get(), [&]
	{
		const auto create_numeric_collation = [&](int const Encoding)
		{
			return sqlite::sqlite3_create_collation(m_Db.get(), "numeric", Encoding, ToPtr(Encoding), Comparer) == SQLITE_OK;
		};

		return
			create_numeric_collation(SQLITE_UTF8) &&
			create_numeric_collation(SQLITE_UTF16);
	});
}


// for sqlite_unicode.c

static void far_value16(void* const Val, void const*& Data, int& Size)
{
	static string Value;
	static const char* LastStr;

	// It is a common pattern to invoke sqlite3_value_bytes16 and sqlite3_value_text16 sequentially.
	// We don't want to invalidate the Value's pointer.
	if (const auto Str = static_cast<const char*>(static_cast<const void*>(sqlite::sqlite3_value_text(static_cast<sqlite::sqlite3_value*>(Val)))); Str != LastStr)
	{
		Value = encoding::utf8::get_chars(Str);
		LastStr = Str;
	}

	Data = Value.c_str();
	Size = static_cast<int>(Value.size()) * sizeof(wchar_t);
}

extern "C" const void* far_value_text16(void* Val);

const void* far_value_text16(void* const Val)
{
	void const* Result;
	int Size;

	far_value16(Val, Result, Size);

	return Result;
}

extern "C" int far_value_bytes16(void* Val);

int far_value_bytes16(void* const Val)
{
	void const* Result;
	int Size;

	far_value16(Val, Result, Size);

	return Size;
}

extern "C" void far_result_text16(void* Ctx, const void* Val, int Length);

void far_result_text16(void* const Ctx, const void* const Val, int const Length)
{
	const auto Data = static_cast<const wchar_t*>(Val);
	const auto Size = Length < 0? std::wcslen(Data) : static_cast<size_t>(Length);
	const auto Value = encoding::utf8::get_bytes({ Data, Size });

	sqlite::sqlite3_result_text(static_cast<sqlite::sqlite3_context*>(Ctx), Value.c_str(), static_cast<int>(Value.size()), sqlite::transient_destructor);
}
