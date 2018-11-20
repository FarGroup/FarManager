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

#include "sqlitedb.hpp"

#include "pathmix.hpp"
#include "config.hpp"
#include "components.hpp"
#include "encoding.hpp"
#include "delete.hpp"
#include "mix.hpp"
#include "global.hpp"

#include "platform.concurrency.hpp"
#include "platform.fs.hpp"

#include "common/bytes_view.hpp"

#include "format.hpp"
#include "sqlite.hpp"
#include "sqlite_unicode.hpp"

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

	string GetErrorString(int ErrorCode)
	{
		return encoding::utf8::get_chars(sqlite::sqlite3_errstr(ErrorCode));
	}

	string GetLastErrorString(sqlite::sqlite3* Db)
	{
		return static_cast<const wchar_t*>(sqlite::sqlite3_errmsg16(Db));
	}

	[[noreturn]]
	void throw_exception(int ErrorCode, const string& ErrorString = {})
	{
		throw MAKE_EXCEPTION(far_sqlite_exception, format(L"SQLite error {0} - {1}"sv, ErrorCode, ErrorString.empty()? GetErrorString(ErrorCode) : ErrorString));
	}

	[[noreturn]]
	void throw_exception(sqlite::sqlite3* Db)
	{
		throw_exception(GetLastErrorCode(Db), GetLastErrorString(Db));
	}

	template<typename callable>
	void invoke(sqlite::sqlite3* Db, const callable& Callable)
	{
		SCOPED_ACTION(lock)(Db);

		if (!Callable())
			throw_exception(Db);
	}

	SCOPED_ACTION(components::component)([]
	{
		return components::component::info{ L"SQLite"s, WIDE_S(SQLITE_VERSION) };
	});

	SCOPED_ACTION(components::component)([]
	{
		return components::component::info{ L"SQLite Unicode extension"s, sqlite_unicode::SQLite_Unicode_Version };
	});
}

bool SQLiteDb::library_load()
{
	return sqlite::sqlite3_initialize() == SQLITE_OK && sqlite_unicode::sqlite3_unicode_load() == SQLITE_OK;
}

void SQLiteDb::library_free()
{
	sqlite_unicode::sqlite3_unicode_free();
	sqlite::sqlite3_shutdown();
}

void SQLiteDb::SQLiteStmt::stmt_deleter::operator()(sqlite::sqlite3_stmt* Object) const
{
	// https://www.sqlite.org/c3ref/finalize.html
	// If the most recent evaluation of the statement encountered no errors
	// or if the statement is never been evaluated, then sqlite3_finalize() returns SQLITE_OK.
	// If the most recent evaluation of statement S failed, then sqlite3_finalize(S)
	// returns the appropriate error code or extended error code.
	invoke(sqlite::sqlite3_db_handle(Object), [&]{ sqlite::sqlite3_finalize(Object); return true; });
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Reset()
{
	invoke(db(), [&]{ return sqlite::sqlite3_clear_bindings(m_Stmt.get()) == SQLITE_OK; });
	invoke(db(), [&]{ return sqlite::sqlite3_reset(m_Stmt.get()) == SQLITE_OK; });
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

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(std::nullptr_t)
{
	invoke(db(), [&]{ return sqlite::sqlite3_bind_null(m_Stmt.get(), ++m_Param) == SQLITE_OK; });
	return *this;
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

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(string&& Value)
{
	return BindImpl(Value, false);
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(const string_view Value, const bool bStatic)
{
	invoke(db(), [&]{ return sqlite::sqlite3_bind_text16(m_Stmt.get(), ++m_Param, Value.data(), static_cast<int>(Value.size() * sizeof(wchar_t)), bStatic? sqlite::static_destructor : sqlite::transient_destructor) == SQLITE_OK; });
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(bytes_view&& Value)
{
	return BindImpl(Value, false);
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(const bytes_view& Value, bool bStatic)
{
	invoke(db(), [&]{ return sqlite::sqlite3_bind_blob(m_Stmt.get(), ++m_Param, Value.data(), static_cast<int>(Value.size()), bStatic? sqlite::static_destructor : sqlite::transient_destructor) == SQLITE_OK; });
	return *this;
}

sqlite::sqlite3 * SQLiteDb::SQLiteStmt::db() const
{
	return sqlite::sqlite3_db_handle(m_Stmt.get());
}

string SQLiteDb::SQLiteStmt::GetColText(int Col) const
{
	return
	{
		static_cast<const wchar_t*>(sqlite::sqlite3_column_text16(m_Stmt.get(), Col)),
		static_cast<size_t>(sqlite::sqlite3_column_bytes16(m_Stmt.get(), Col) / sizeof(wchar_t))
	};
}

std::string SQLiteDb::SQLiteStmt::GetColTextUTF8(int Col) const
{
	return
	{
		reinterpret_cast<const char*>(sqlite::sqlite3_column_text(m_Stmt.get(), Col)),
		static_cast<size_t>(sqlite::sqlite3_column_bytes(m_Stmt.get(), Col))
	};
}

int SQLiteDb::SQLiteStmt::GetColInt(int Col) const
{
	return sqlite::sqlite3_column_int(m_Stmt.get(), Col);
}

unsigned long long SQLiteDb::SQLiteStmt::GetColInt64(int Col) const
{
	return sqlite::sqlite3_column_int64(m_Stmt.get(), Col);
}

bytes_view SQLiteDb::SQLiteStmt::GetColBlob(int Col) const
{
	return bytes_view(sqlite::sqlite3_column_blob(m_Stmt.get(), Col), sqlite::sqlite3_column_bytes(m_Stmt.get(), Col));
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


SQLiteDb::SQLiteDb(initialiser Initialiser, string_view const DbName, bool WAL):
	m_Db(Open(DbName, WAL)),
	m_stmt_BeginTransaction(create_stmt("BEGIN;"sv)),
	m_stmt_EndTransaction(create_stmt("END;"sv)),
	m_Init((Initialiser(db_initialiser(this)), init{})) // yay, operator comma!
{
}

void SQLiteDb::db_closer::operator()(sqlite::sqlite3* Object) const
{
	const auto Result = sqlite::sqlite3_close(Object);
	(void)Result;
	assert(Result == SQLITE_OK);
}

class SQLiteDb::implementation
{
public:
	static database_ptr open(string_view const Name, bool Writable = true)
	{
		database_ptr Db;
		const auto Result = sqlite::sqlite3_open_v2(encoding::utf8::get_bytes(Name).c_str(), &ptr_setter(Db), Writable? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READONLY, nullptr);
		if (Result != SQLITE_OK)
			Db? throw_exception(Db.get()) : throw_exception(Result);

		return Db;
	}

	static database_ptr copy_db_to_memory(string_view const Path, bool WAL)
	{
		auto Destination = open(memory_db_name());

		database_ptr SourceDb;
		delayed_deleter Deleter;

		if (WAL && !os::fs::can_create_file(concat(Path, L'.', GuidToStr(CreateUuid())))) // can't open db -- copy to %TEMP%
		{
			const auto TmpDbPath = concat(MakeTemp(), str(GetCurrentProcessId()), L'-', PointToName(Path));
			if (!os::fs::copy_file(Path, TmpDbPath, nullptr, nullptr, nullptr, 0))
				throw_exception(SQLITE_READONLY);

			Deleter.set(TmpDbPath);
			os::fs::set_file_attributes(TmpDbPath, FILE_ATTRIBUTE_NORMAL);
			SourceDb = open(TmpDbPath);
		}
		else
		{
			SourceDb = open(Path, WAL);
		}

		struct backup_closer
		{
			void operator()(sqlite::sqlite3_backup* Backup) const
			{
				const auto Result = sqlite::sqlite3_backup_finish(Backup);
				(void)Result;
				assert(Result == SQLITE_OK);
			}
		};

		using backup_ptr = std::unique_ptr<sqlite::sqlite3_backup, backup_closer>;

		backup_ptr DbBackup;
		invoke(Destination.get(), [&]{ DbBackup.reset(sqlite::sqlite3_backup_init(Destination.get(), "main", SourceDb.get(), "main")); return DbBackup != nullptr; });

		const auto StepResult = sqlite::sqlite3_backup_step(DbBackup.get(), -1);
		if (StepResult != SQLITE_DONE)
			throw_exception(StepResult);

		return Destination;
	}

	static database_ptr try_copy_db_to_memory(string_view const Path, bool WAL)
	{
		try
		{
			return copy_db_to_memory(Path, WAL);
		}
		catch (const far_sqlite_exception&)
		{
			return open(memory_db_name());
		}
	}
};

SQLiteDb::database_ptr SQLiteDb::Open(string_view const Path, bool const WAL)
{
	const auto MemDb = Path == memory_db_name();
	m_DbExists = !MemDb && os::fs::is_file(Path);

	if (!Global->Opt->ReadOnlyConfig || MemDb)
	{
		m_Path = Path;
		return implementation::open(Path);
	}

	m_Path = memory_db_name();
	return m_DbExists?
		implementation::try_copy_db_to_memory(Path, WAL) :
		implementation::open(memory_db_name());
}

void SQLiteDb::Exec(range<const std::string_view*> Command) const
{
	for (const auto& i: Command)
	{
		create_stmt(i, false).Execute();
	}
}

void SQLiteDb::BeginTransaction()
{
	if (!m_ActiveTransactions)
		m_stmt_BeginTransaction.Execute();
	++m_ActiveTransactions;
}

void SQLiteDb::EndTransaction()
{
	--m_ActiveTransactions;
	if (!m_ActiveTransactions)
		m_stmt_EndTransaction.Execute();
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

void SQLiteDb::SetBusyHandler(busy_handler Handler) const
{
	invoke(m_Db.get(), [&]{ return sqlite::sqlite3_busy_handler(m_Db.get(), Handler, const_cast<SQLiteDb*>(this)) == SQLITE_OK; });
}

unsigned long long SQLiteDb::LastInsertRowID() const
{
	return sqlite::sqlite3_last_insert_rowid(m_Db.get());
}

void SQLiteDb::Close()
{
	// sqlite3_close() returns SQLITE_BUSY and leaves the connection option
	// if there are unfinalized prepared statements or unfinished sqlite3_backups
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
