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

#include "transactional.hpp"

namespace sqlite
{
	struct sqlite3;
	struct sqlite3_stmt;
}

class SQLiteDb: noncopyable, virtual transactional
{
public:
	SQLiteDb();
	virtual ~SQLiteDb();

	enum ColumnType
	{
		TYPE_INTEGER,
		TYPE_STRING,
		TYPE_BLOB,
		TYPE_UNKNOWN
	};

	bool IsNew() const { return db_exists <= 0; }
	int InitStatus(string& name, bool full_name);

	static int library_load();
	static void library_free();

protected:
	class SQLiteStmt: noncopyable, swapable<SQLiteStmt>
	{
	public:
		SQLiteStmt(sqlite::sqlite3_stmt* Stmt): m_Stmt(Stmt), m_Param(1) {}
		SQLiteStmt(SQLiteStmt&& rhs) noexcept { *this = std::move(rhs); };

		MOVE_OPERATOR_BY_SWAP(SQLiteStmt);

		void swap(SQLiteStmt& rhs) noexcept
		{
			using std::swap;
			swap(m_Param, rhs.m_Param);
			swap(m_Stmt, rhs.m_Stmt);
		}

		template<class T>
		struct transient_t
		{
			transient_t(const T& Value): m_Value(Value) {}
			const T& m_Value;
		};

		SQLiteStmt& Reset();
		bool Step() const;
		bool StepAndReset();

		template<typename T>
		SQLiteStmt& Bind(T&& Arg) { return BindImpl(std::forward<T>(Arg)); }

#ifdef NO_VARIADIC_TEMPLATES
		#define BIND_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
		template<VTE_TYPENAME(first), TYPENAME_LIST> \
		SQLiteStmt& Bind(VTE_REF_ARG(first), REF_ARG_LIST) \
		{ \
			return Bind(VTE_FWD_ARG(first)), Bind(FWD_ARG_LIST); \
		}

		#include "common/variadic_emulation_helpers_begin.hpp"
		VTE_GENERATE(BIND_VTE)
		#include "common/variadic_emulation_helpers_end.hpp"

		#undef BIND_VTE
#else
		template<typename T, class... Args>
		SQLiteStmt& Bind(T&& arg, Args&&... args)
		{
			return Bind(std::forward<T>(arg)), Bind(std::forward<Args>(args)...);
		}
#endif

		const wchar_t *GetColText(int Col) const;
		const char *GetColTextUTF8(int Col) const;
		int GetColInt(int Col) const;
		unsigned __int64 GetColInt64(int Col) const;
		blob GetColBlob(int Col) const;
		ColumnType GetColType(int Col) const;

	private:
		SQLiteStmt& BindImpl(int Value);
		SQLiteStmt& BindImpl(__int64 Value);
		SQLiteStmt& BindImpl(const wchar_t* Value, bool bStatic = true);
		SQLiteStmt& BindImpl(const string& Value, bool bStatic = true);
		SQLiteStmt& BindImpl(string&& Value);
		SQLiteStmt& BindImpl(const blob& Value, bool bStatic = true);
		SQLiteStmt& BindImpl(unsigned int Value) { return BindImpl(static_cast<int>(Value)); }
		SQLiteStmt& BindImpl(unsigned __int64 Value) { return BindImpl(static_cast<__int64>(Value)); }
		template<class T>
		SQLiteStmt& BindImpl(const transient_t<T>& Value) { return BindImpl(Value.m_Value, false); }

		struct stmt_deleter { void operator()(sqlite::sqlite3_stmt*) const; };
		std::unique_ptr<sqlite::sqlite3_stmt, stmt_deleter> m_Stmt;
		int m_Param;
	};

	typedef simple_pair<size_t, const wchar_t*> stmt_init_t;

	template<class T>
	static SQLiteStmt::transient_t<T> transient(const T& Value) { return SQLiteStmt::transient_t<T>(Value); }

	template<size_t ExpectedSize, class T, size_t N>
	void CheckStmt(const T (&Stmts)[N])
	{
		static_assert(N == ExpectedSize, "Not all statements initialized");
	}

	bool Open(const string& DbName, bool Local, bool WAL=false);
	void Initialize(const string& DbName, bool Local = false);
	SQLiteStmt create_stmt(const wchar_t *Stmt) const;
	template<size_t N>
	bool PrepareStatements(const stmt_init_t (&Init)[N]) { return PrepareStatements(Init, N); }
	bool Exec(const char *Command) const;
	bool SetWALJournalingMode() const;
	bool EnableForeignKeysConstraints() const;
	bool Changes() const;
	unsigned __int64 LastInsertRowID() const;

	virtual bool BeginTransaction() override;
	virtual bool EndTransaction() override;
	virtual bool RollbackTransaction() override;

	// TODO: use in log
	int GetLastErrorCode() const;
	string GetLastErrorString() const;

	const string& GetPath() const { return m_Path; }
	const string& GetName() const { return m_Name; }

	SQLiteStmt& Statement(size_t Index) const { return m_Statements[Index]; }

private:
	void Close();
	virtual bool InitializeImpl(const string& DbName, bool Local) = 0;
	bool PrepareStatements(const stmt_init_t* Init, size_t Size);

	struct db_closer { void operator()(sqlite::sqlite3*) const; };
	typedef std::unique_ptr<sqlite::sqlite3, db_closer> database_ptr;

	// must be destroyed last
	database_ptr m_Db;
	mutable std::vector<SQLiteStmt> m_Statements;
	string m_Path;
	string m_Name;
	int init_status;
	int db_exists;
};
