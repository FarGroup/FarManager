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

#include "sqlite.h"
#include "sqlitedb.hpp"
#include "pathmix.hpp"
#include "config.hpp"

static void GetDatabasePath(const wchar_t *FileName, string &strOut, bool Local)
{
	if(StrCmp(FileName, L":memory:"))
	{
		strOut = Local ? Opt.LocalProfilePath : Opt.ProfilePath;
		AddEndSlash(strOut);
		strOut += FileName;
	}
	else
	{
		strOut = FileName;
	}
}

SQLiteStmt::SQLiteStmt():
	param(1),
	pStmt(nullptr)
{
}

bool SQLiteStmt::Finalize()
{
	if (pStmt)
		return sqlite3_finalize(pStmt) == SQLITE_OK;
	else
		return true;
}

SQLiteStmt::~SQLiteStmt()
{
	Finalize();
}

SQLiteStmt& SQLiteStmt::Reset()
{
	param=1;
	sqlite3_clear_bindings(pStmt);
	sqlite3_reset(pStmt);
	return *this;
}

bool SQLiteStmt::Step()
{
	return sqlite3_step(pStmt) == SQLITE_ROW;
}

bool SQLiteStmt::StepAndReset()
{
	bool b = sqlite3_step(pStmt) == SQLITE_DONE;
	Reset();
	return b;
}

SQLiteStmt& SQLiteStmt::Bind(int Value)
{
	sqlite3_bind_int(pStmt,param++,Value);
	return *this;
}

SQLiteStmt& SQLiteStmt::Bind(__int64 Value)
{
	sqlite3_bind_int64(pStmt,param++,Value);
	return *this;
}

SQLiteStmt& SQLiteStmt::Bind(const wchar_t *Value, bool bStatic)
{
	if (Value)
		sqlite3_bind_text16(pStmt,param++,Value,-1,bStatic?SQLITE_STATIC:SQLITE_TRANSIENT);
	else
		sqlite3_bind_null(pStmt,param++);
	return *this;
}

SQLiteStmt& SQLiteStmt::Bind(const void *Value, size_t Size, bool bStatic)
{
	sqlite3_bind_blob(pStmt, param++, Value, static_cast<int>(Size), bStatic? SQLITE_STATIC : SQLITE_TRANSIENT);
	return *this;
}

const wchar_t* SQLiteStmt::GetColText(int Col)
{
	return (const wchar_t *)sqlite3_column_text16(pStmt,Col);
}

const char* SQLiteStmt::GetColTextUTF8(int Col)
{
	return (const char *)sqlite3_column_text(pStmt,Col);
}

int SQLiteStmt::GetColBytes(int Col)
{
	return sqlite3_column_bytes(pStmt,Col);
}

int SQLiteStmt::GetColInt(int Col)
{
	return sqlite3_column_int(pStmt,Col);
}

unsigned __int64 SQLiteStmt::GetColInt64(int Col)
{
	return sqlite3_column_int64(pStmt,Col);
}

const char* SQLiteStmt::GetColBlob(int Col)
{
	return (const char *)sqlite3_column_blob(pStmt,Col);
}

int SQLiteStmt::GetColType(int Col)
{
	return sqlite3_column_type(pStmt,Col);
}


SQLiteDb::SQLiteDb():
	pDb(nullptr), init_status(-1)
{
}

SQLiteDb::~SQLiteDb()
{
	Close();
}

static bool can_create_file(const string& fname)
{
	HANDLE fh = CreateFile(fname.CPtr(),
		GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		nullptr, CREATE_ALWAYS, 0, nullptr
	);
	if (INVALID_HANDLE_VALUE == fh)
		return false;
	CloseHandle(fh);
	apiDeleteFile(fname);
	return true;
}

bool SQLiteDb::Open(const wchar_t *DbFile, bool Local, bool WAL)
{
	GetDatabasePath(DbFile, strPath, Local);

	if (!Opt.ReadOnlyConfig || 0 == wcscmp(DbFile, L":memory:"))
		return SQLITE_OK == sqlite3_open16(strPath.CPtr(), &pDb);

	// copy db to memory
	//
	if (SQLITE_OK != sqlite3_open16(L":memory:", &pDb))
		return false;

   bool ok = true, copied = false;
	struct sqlite3 *db_source = nullptr;

	DWORD attrs = apiGetFileAttributes(strPath);
	if (0 == (attrs & FILE_ATTRIBUTE_DIRECTORY)) // source exists and not directory
	{
		if (WAL && !can_create_file(strPath+L"-$test$")) // can't open db -- copy to %TEMP%
		{
			string strTmp;
			apiGetTempPath(strTmp);
			wchar_t sPid[20];
			wsprintf(sPid, L"%u-", GetCurrentProcessId());
			strTmp += sPid; strTmp += DbFile;
			ok = copied = FALSE != apiCopyFileEx(strPath, strTmp, nullptr, nullptr, nullptr, 0);
			apiSetFileAttributes(strTmp, attrs & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));
			if (ok)
				strPath = strTmp;
			ok = ok && (SQLITE_OK == sqlite3_open16(strPath.CPtr(), &db_source));
		}
		else
		{
			int n8 = WideCharToMultiByte(CP_UTF8,0, strPath.CPtr(),-1, nullptr,0, nullptr,nullptr);
			char *name8 = new char[n8];
			WideCharToMultiByte(CP_UTF8,0, strPath.CPtr(),-1, name8,n8, nullptr,nullptr);
			int flags = (WAL ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY);
			ok = (SQLITE_OK == sqlite3_open_v2(name8, &db_source, flags, nullptr));
			delete[] name8;
		}
		if (ok)
		{
			sqlite3_busy_timeout(db_source, 1000);
			struct sqlite3_backup *db_backup = sqlite3_backup_init(pDb, "main", db_source, "main");
			ok = (nullptr != db_backup);
			if (ok)
			{
				sqlite3_backup_step(db_backup, -1);
				sqlite3_backup_finish(db_backup);
				int rc = sqlite3_errcode(pDb);
				ok = (SQLITE_OK == rc);
			}
		}
	}

	if (db_source)
		sqlite3_close(db_source);
	if (copied)
		apiDeleteFile(strPath);

	strPath = L":memory:";
	if (!ok)
		Close();
	return ok;
}

void SQLiteDb::Initialize(const wchar_t* DbName, bool Local)
{
	strName = DbName;
	init_status = 0;
	if (!InitializeImpl(DbName, Local))
	{
		Close();
		++init_status;

		bool in_memory = (Opt.ReadOnlyConfig != 0)
		 || !apiMoveFileEx(strPath, strPath+L".bad",MOVEFILE_REPLACE_EXISTING) || !InitializeImpl(DbName,Local);

		if (in_memory)
		{
			Close();
			++init_status;
			InitializeImpl(L":memory:", Local);
		}
	}
}

int SQLiteDb::InitStatus(const wchar_t* &name, bool full_name)
{
	name = (full_name && !strPath.IsEmpty() && strPath != L":memory:") ? strPath.CPtr() : strName.CPtr();
	return init_status;
}

bool SQLiteDb::Exec(const char *Command)
{
	return sqlite3_exec(pDb, Command, nullptr, nullptr, nullptr) == SQLITE_OK;
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

bool SQLiteDb::IsOpen()
{
	return pDb != nullptr;
}

bool SQLiteDb::InitStmt(SQLiteStmt &stmtStmt, const wchar_t *Stmt)
{
	return sqlite3_prepare16_v2(pDb, Stmt, -1, &stmtStmt.pStmt, nullptr) == SQLITE_OK;
}

int SQLiteDb::Changes()
{
	return sqlite3_changes(pDb);
}

unsigned __int64 SQLiteDb::LastInsertRowID()
{
	return sqlite3_last_insert_rowid(pDb);
}

bool SQLiteDb::Close()
{
	bool Result = sqlite3_close(pDb) == SQLITE_OK;
	pDb = nullptr;
	return Result;
}

bool SQLiteDb::SetWALJournalingMode()
{
	return Exec("PRAGMA journal_mode = WAL;");
}

bool SQLiteDb::EnableForeignKeysConstraints()
{
	return Exec("PRAGMA foreign_keys = ON;");
}
