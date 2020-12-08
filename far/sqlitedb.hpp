#ifndef SQLITEDB_HPP_1C228281_1C8E_467F_9070_520E01F7DB70
#define SQLITEDB_HPP_1C228281_1C8E_467F_9070_520E01F7DB70
#pragma once

/*
sqlitedb.hpp

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

// Internal:
#include "transactional.hpp"
#include "exception.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

namespace sqlite
{
	struct sqlite3;
	struct sqlite3_stmt;
}

class far_sqlite_exception : public far_exception
{
	using far_exception::far_exception;
};

class SQLiteDb: noncopyable, virtual protected transactional
{
public:
	using busy_handler = int(*)(void*, int) noexcept;
	static void library_load();
	static void library_free();

	const string& GetPath() const { return m_Path; }
	bool IsNew() const { return !m_DbExists; }

	enum class column_type
	{
		unknown,
		integer,
		string,
		blob,
	};

	static constexpr auto memory_db_name = L":memory:"sv;

protected:
	class db_initialiser;

	using initialiser = void(const db_initialiser& Db);

	SQLiteDb(busy_handler BusyHandler, initialiser Initialiser, string_view DbName, bool WAL = false);

	void BeginTransaction() override;
	void EndTransaction() override;

	class SQLiteStmt
	{
	public:
		NONCOPYABLE(SQLiteStmt);
		MOVE_CONSTRUCTIBLE(SQLiteStmt);

		explicit SQLiteStmt(sqlite::sqlite3_stmt* Stmt): m_Stmt(Stmt) {}

		SQLiteStmt& Reset();
		bool Step() const;
		void Execute() const;

		template<typename... args>
		auto& Bind(args&&... Args)
		{
			(..., BindImpl(FWD(Args)));
			return *this;
		}

		string GetColText(int Col) const;
		std::string GetColTextUTF8(int Col) const;
		int GetColInt(int Col) const;
		unsigned long long GetColInt64(int Col) const;
		bytes GetColBlob(int Col) const;
		column_type GetColType(int Col) const;

	private:
		SQLiteStmt& BindImpl(int Value);
		SQLiteStmt& BindImpl(long long Value);
		SQLiteStmt& BindImpl(string_view Value);
		SQLiteStmt& BindImpl(bytes_view Value);
		SQLiteStmt& BindImpl(unsigned int Value) { return BindImpl(static_cast<int>(Value)); }
		SQLiteStmt& BindImpl(unsigned long long Value) { return BindImpl(static_cast<long long>(Value)); }

		sqlite::sqlite3* db() const;

		struct stmt_deleter { void operator()(sqlite::sqlite3_stmt*) const noexcept; };
		std::unique_ptr<sqlite::sqlite3_stmt, stmt_deleter> m_Stmt;
		int m_Param{};
	};

	struct statement_reset
	{
		void operator()(SQLiteStmt* Statement) const { Statement->Reset(); }
	};

	using auto_statement = std::unique_ptr<SQLiteStmt, statement_reset>;

	template<typename T>
	using stmt_init = std::pair<T, std::string_view>;

	SQLiteStmt create_stmt(std::string_view Stmt, bool Persistent = true) const;

	template<typename T, size_t N>
	void PrepareStatements(const stmt_init<T> (&Init)[N])
	{
		static_assert(N == T::stmt_count);

		assert(m_Statements.empty());

		m_Statements.reserve(N);
		std::transform(ALL_CONST_RANGE(Init), std::back_inserter(m_Statements), [this](const auto& i)
		{
			assert(static_cast<size_t>(i.first) == m_Statements.size());
			return create_stmt(i.second);
		});
	}

	void Exec(span<std::string_view const> Commands) const;
	void SetWALJournalingMode() const;
	void EnableForeignKeysConstraints() const;
	void CreateNumericCollation() const;

	unsigned long long LastInsertRowID() const;

	auto_statement AutoStatement(size_t Index) const { return auto_statement(&m_Statements[Index]); }
	static void KeepStatement(auto_statement& Stmt) { (void)Stmt.release(); }

	// No forwarding here - ExecuteStatement is atomic so we don't have to deal with lifetimes
	template<typename... args>
	auto ExecuteStatement(size_t Index, const args&... Args) const
	{
		return AutoStatement(Index)->Bind(Args...).Execute();
	}

	class db_initialiser
	{
	public:
		explicit db_initialiser(SQLiteDb* Db):
			m_Db(Db)
		{
		}

#define FORWARD_FUNCTION(FunctionName) \
		template<typename... args> \
		decltype(auto) FunctionName(args&&... Args) const \
		{ \
			return m_Db->FunctionName(FWD(Args)...); \
		}

		FORWARD_FUNCTION(Exec)
		FORWARD_FUNCTION(SetWALJournalingMode)
		FORWARD_FUNCTION(EnableForeignKeysConstraints)
		FORWARD_FUNCTION(CreateNumericCollation)
		FORWARD_FUNCTION(PrepareStatements)

#undef FORWARD_FUNCTION

	private:
		SQLiteDb* m_Db;
	};

private:
	class implementation;
	friend class implementation;

	struct db_closer { void operator()(sqlite::sqlite3*) const noexcept; };
	using database_ptr = std::unique_ptr<sqlite::sqlite3, db_closer>;

	database_ptr Open(string_view Path, busy_handler BusyHandler, bool WAL);
	void Close();

	// The order is important
	bool m_DbExists{};
	string m_Path;
	database_ptr m_Db;
	SQLiteStmt m_stmt_BeginTransaction;
	SQLiteStmt m_stmt_EndTransaction;
	mutable std::vector<SQLiteStmt> m_Statements;
	struct init{} m_Init;
	std::atomic_size_t m_ActiveTransactions{};
};

#endif // SQLITEDB_HPP_1C228281_1C8E_467F_9070_520E01F7DB70
