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

#include "headers.hpp"
#pragma hdrstop

#include "sqlitedb.hpp"
#include "sqlite.hpp"
#include "pathmix.hpp"
#include "config.hpp"
#include "synchro.hpp"
#include "components.hpp"

static string getInfo() { return L"SQLite, version " + wide(SQLITE_VERSION); }
SCOPED_ACTION(components::component)(getInfo);

static void GetDatabasePath(const string& FileName, string &strOut, bool Local)
{
	if(FileName != L":memory:")
	{
		strOut = Local ? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath;
		AddEndSlash(strOut);
		strOut += FileName;
	}
	else
	{
		strOut = FileName;
	}
}

void SQLiteDb::SQLiteStmt::stmt_deleter::operator()(sqlite::sqlite3_stmt* object) const
{
	sqlite::sqlite3_finalize(object);
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Reset()
{
	m_Param = 1;
	sqlite::sqlite3_clear_bindings(m_Stmt.get());
	sqlite::sqlite3_reset(m_Stmt.get());
	return *this;
}

bool SQLiteDb::SQLiteStmt::Step() const
{
	return sqlite::sqlite3_step(m_Stmt.get()) == SQLITE_ROW;
}

bool SQLiteDb::SQLiteStmt::StepAndReset()
{
	const bool b = sqlite::sqlite3_step(m_Stmt.get()) == SQLITE_DONE;
	Reset();
	return b;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(int Value)
{
	sqlite::sqlite3_bind_int(m_Stmt.get(), m_Param++, Value);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(__int64 Value)
{
	sqlite::sqlite3_bind_int64(m_Stmt.get(), m_Param++, Value);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(const wchar_t* Value, bool bStatic)
{
	using sqlite::sqlite3_destructor_type; // for SQLITE_* macros
	sqlite::sqlite3_bind_text16(m_Stmt.get(), m_Param++, Value, -1, bStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(const string& Value, bool bStatic)
{
	using sqlite::sqlite3_destructor_type; // for SQLITE_* macros
	sqlite::sqlite3_bind_text16(m_Stmt.get(), m_Param++, Value.data(), -1, bStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::BindImpl(const blob& Value, bool bStatic)
{
	using sqlite::sqlite3_destructor_type; // for SQLITE_* macros
	sqlite::sqlite3_bind_blob(m_Stmt.get(), m_Param++, Value.data(), static_cast<int>(Value.size()), bStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	return *this;
}

const wchar_t* SQLiteDb::SQLiteStmt::GetColText(int Col) const
{
	return static_cast<const wchar_t*>(sqlite::sqlite3_column_text16(m_Stmt.get(), Col));
}

const char* SQLiteDb::SQLiteStmt::GetColTextUTF8(int Col) const
{
	return reinterpret_cast<const char*>(sqlite::sqlite3_column_text(m_Stmt.get(), Col));
}

int SQLiteDb::SQLiteStmt::GetColInt(int Col) const
{
	return sqlite::sqlite3_column_int(m_Stmt.get(), Col);
}

unsigned __int64 SQLiteDb::SQLiteStmt::GetColInt64(int Col) const
{
	return sqlite::sqlite3_column_int64(m_Stmt.get(), Col);
}

blob SQLiteDb::SQLiteStmt::GetColBlob(int Col) const
{
	return blob(sqlite::sqlite3_column_blob(m_Stmt.get(), Col), sqlite::sqlite3_column_bytes(m_Stmt.get(), Col));
}

SQLiteDb::ColumnType SQLiteDb::SQLiteStmt::GetColType(int Col) const
{
	switch (sqlite::sqlite3_column_type(m_Stmt.get(), Col))
	{
	case SQLITE_INTEGER:
		return TYPE_INTEGER;
	case SQLITE_TEXT:
		return TYPE_STRING;
	case SQLITE_BLOB:
		return TYPE_BLOB;
	default:
		return TYPE_UNKNOWN;
	}
}


SQLiteDb::SQLiteDb():
	init_status(-1),
	db_exists(-1)
{
}

SQLiteDb::~SQLiteDb()
{
	Close();
}

static bool can_create_file(const string& fname)
{
	return api::fs::file().Open(fname, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE);
}


void SQLiteDb::db_closer::operator()(sqlite::sqlite3* Object) const
{
	sqlite::sqlite3_close(Object);
}

bool SQLiteDb::Open(const string& DbFile, bool Local, bool WAL)
{
	const auto v1_opener = [](const string& Name, sqlite::sqlite3*& pDb)
	{
		return sqlite::sqlite3_open16(Name.data(), &pDb);
	};

	const auto v2_opener = [WAL](const string& Name, sqlite::sqlite3*& pDb)
	{
		return sqlite::sqlite3_open_v2(Utf8String(Name).data(), &pDb, WAL? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY, nullptr);
	};

	const auto OpenDatabase = [](database_ptr& Db, const string& Name, const std::function<int(const string&, sqlite::sqlite3*&)>& opener) -> bool
	{
		sqlite::sqlite3* pDb;
		auto Result = opener(Name, pDb);
		Db.reset(pDb);
		return Result == SQLITE_OK;
	};

	GetDatabasePath(DbFile, strPath, Local);
	bool mem_db = DbFile == L":memory:";

	if (!Global->Opt->ReadOnlyConfig || mem_db)
	{
		if (!mem_db && db_exists < 0) {
			db_exists = api::fs::is_file(strPath)? +1 : 0;
		}
		bool ret = OpenDatabase(m_Db, strPath, v1_opener);
		if (ret)
			sqlite::sqlite3_busy_timeout(m_Db.get(), 1000);
		return ret;
	}

	// copy db to memory
	//
	if (!OpenDatabase(m_Db, L":memory:", v1_opener))
		return false;

	bool ok = true, copied = false;

	if (api::fs::is_file(strPath))
	{
		database_ptr db_source;

		if (db_exists < 0)
			db_exists = +1;
		UUID Id;
		UuidCreate(&Id);
		if (WAL && !can_create_file(strPath + L"." + GuidToStr(Id))) // can't open db -- copy to %TEMP%
		{
			FormatString strTmp;
			api::GetTempPath(strTmp);
			strTmp << GetCurrentProcessId() << L'-' << DbFile;
			ok = copied = FALSE != api::CopyFileEx(strPath, strTmp, nullptr, nullptr, nullptr, 0);
			api::SetFileAttributes(strTmp, FILE_ATTRIBUTE_NORMAL);
			if (ok)
				strPath = strTmp;
			ok = ok && OpenDatabase(db_source, strPath, v1_opener);
		}
		else
		{
			ok = OpenDatabase(db_source, strPath, v2_opener);
		}
		if (ok)
		{
			sqlite::sqlite3_busy_timeout(db_source.get(), 1000);
			auto db_backup = sqlite::sqlite3_backup_init(m_Db.get(), "main", db_source.get(), "main");
			ok = (nullptr != db_backup);
			if (ok)
			{
				sqlite::sqlite3_backup_step(db_backup, -1);
				sqlite::sqlite3_backup_finish(db_backup);
				int rc = sqlite::sqlite3_errcode(m_Db.get());
				ok = (SQLITE_OK == rc);
			}
		}
	}

	if (copied)
		api::DeleteFile(strPath);

	strPath = L":memory:";
	if (!ok)
		Close();
	return ok;
}

void SQLiteDb::Initialize(const string& DbName, bool Local)
{
	Mutex m(make_name<Mutex>(Local? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath, DbName).data());
	SCOPED_ACTION(lock_guard<Mutex>)(m);

	m_Name = DbName;
	init_status = 0;

	if (!InitializeImpl(DbName, Local))
	{
		Close();
		++init_status;

		bool in_memory = (Global->Opt->ReadOnlyConfig != 0) ||
			!api::MoveFileEx(strPath, strPath+L".bad",MOVEFILE_REPLACE_EXISTING) || !InitializeImpl(DbName,Local);

		if (in_memory)
		{
			Close();
			++init_status;
			InitializeImpl(L":memory:", Local);
		}
	}
}

int SQLiteDb::InitStatus(string& name, bool full_name)
{
	name = (full_name && !strPath.empty() && strPath != L":memory:") ? strPath : m_Name;
	return init_status;
}

bool SQLiteDb::Exec(const char *Command) const
{
	return sqlite::sqlite3_exec(m_Db.get(), Command, nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool SQLiteDb::BeginTransaction()
{
	return Exec("BEGIN TRANSACTION;");
}

bool SQLiteDb::EndTransaction()
{
	return Exec("END TRANSACTION;");
}

bool SQLiteDb::RollbackTransaction()
{
	return Exec("ROLLBACK TRANSACTION;");
}

SQLiteDb::SQLiteStmt SQLiteDb::create_stmt(const wchar_t* Stmt) const
{
	sqlite::sqlite3_stmt* pStmt;
	if (sqlite::sqlite3_prepare16_v2(m_Db.get(), Stmt, -1, &pStmt, nullptr) != SQLITE_OK)
	{
		throw std::runtime_error("SQLiteDb::create_stmt failed");
	}
	return SQLiteStmt(pStmt);
}

bool SQLiteDb::PrepareStatements(const stmt_init_t* Init, size_t Size)
{
	assert(m_Statements.empty());

	m_Statements.reserve(Size);
	std::transform(Init, Init + Size, std::back_inserter(m_Statements), [this](decltype(*Init) i) -> SQLiteStmt
	{
		assert(i.first == m_Statements.size());
		return create_stmt(i.second);
	});
	return true;
}

bool SQLiteDb::Changes() const
{
	return sqlite::sqlite3_changes(m_Db.get()) != 0;
}

unsigned __int64 SQLiteDb::LastInsertRowID() const
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

bool SQLiteDb::SetWALJournalingMode() const
{
	return Exec("PRAGMA journal_mode = WAL;");
}

bool SQLiteDb::EnableForeignKeysConstraints() const
{
	return Exec("PRAGMA foreign_keys = ON;");
}

int SQLiteDb::GetLastErrorCode() const
{
	return sqlite::sqlite3_errcode(m_Db.get());
}

string SQLiteDb::GetLastErrorString() const
{
	return wide(sqlite::sqlite3_errmsg(m_Db.get()));
}
