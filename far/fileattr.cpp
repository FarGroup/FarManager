/*
fileattr.cpp

Работа с атрибутами файлов
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "fileattr.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "fileowner.hpp"
#include "exception.hpp"

struct response
{
	enum
	{
		retry = 0,
		skip = 1,
		skip_all = 2,
		cancel = 3
	};
};

static int ShowErrorMessage(const error_state_ex& ErrorState, lng Id, const string& Name)
{
	return Message(MSG_WARNING, ErrorState,
		msg(lng::MError),
		{
			msg(Id),
			Name
		},
		{ lng::MHRetry, lng::MHSkip, lng::MHSkipAll, lng::MHCancel }
	);
}

int ESetFileAttributes(const string& Name,DWORD Attr,int SkipMode)
{
	if (Attr&FILE_ATTRIBUTE_DIRECTORY && Attr&FILE_ATTRIBUTE_TEMPORARY)
		Attr&=~FILE_ATTRIBUTE_TEMPORARY;

	while (!os::fs::set_file_attributes(Name,Attr))
	{
		const auto ErrorState = error_state::fetch();

		switch (SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}

static bool SetFileCompression(const string& Name,int State)
{
	const os::fs::file File(Name, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if (!File)
		return false;

	USHORT NewState=State? COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
	DWORD BytesReturned;
	return File.IoControl(FSCTL_SET_COMPRESSION, &NewState, sizeof(NewState), nullptr, 0, &BytesReturned);
}


int ESetFileCompression(const string& Name,int State,DWORD FileAttr,int SkipMode)
{
	if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
		return SETATTR_RET_OK;

	if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		os::fs::set_file_attributes(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

	SCOPE_EXIT
	{
		if (FileAttr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
			os::fs::set_file_attributes(Name, FileAttr);
	};

	// Drop Encryption
	if ((FileAttr & FILE_ATTRIBUTE_ENCRYPTED) && State)
		os::fs::set_file_encryption(Name, false);

	while (!SetFileCompression(Name,State))
	{
		const auto ErrorState = error_state::fetch();

		switch(SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrCompressedCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}


int ESetFileEncryption(const string& Name, bool State, DWORD FileAttr, int SkipMode, int Silent)
{
	if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
		return SETATTR_RET_OK;

	if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		os::fs::set_file_attributes(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

	SCOPE_EXIT
	{
		if (FileAttr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
		os::fs::set_file_attributes(Name, FileAttr);
	};

	while (!os::fs::set_file_encryption(Name, State))
	{
		if (Silent)
			return SETATTR_RET_ERROR;

		const auto ErrorState = error_state::fetch();

		switch (SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrEncryptedCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}


int ESetFileTime(const string& Name, const os::chrono::time_point* LastWriteTime, const os::chrono::time_point* CreationTime, const os::chrono::time_point* LastAccessTime, const os::chrono::time_point* ChangeTime, DWORD FileAttr, int SkipMode)
{
	if (!LastWriteTime && !CreationTime && !LastAccessTime && !ChangeTime)
		return SETATTR_RET_OK;

	for(;;)
	{
		if (FileAttr & FILE_ATTRIBUTE_READONLY)
			os::fs::set_file_attributes(Name,FileAttr & ~FILE_ATTRIBUTE_READONLY);

		bool SetTime=false;
		DWORD LastError;
		if (auto File = os::fs::file(Name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
		{
			SetTime = File.SetTime(CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
			LastError=GetLastError();
			File.Close();

			if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && LastError==ERROR_NOT_SUPPORTED)   // FIX: Mantis#223
			{
				if (GetDriveType(GetPathRoot(Name).data()) == DRIVE_REMOTE)
					break;
			}
		}
		else
		{
			LastError = GetLastError();
		}

		if (FileAttr & FILE_ATTRIBUTE_READONLY)
			os::fs::set_file_attributes(Name,FileAttr);

		SetLastError(LastError);
		const auto ErrorState = error_state::fetch();

		if (SetTime)
			break;

		switch (SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrTimeCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}

static bool SetFileSparse(const string& Name,bool State)
{
	const os::fs::file File(Name, FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
	if (!File)
		return false;

	DWORD BytesReturned;
	FILE_SET_SPARSE_BUFFER sb={static_cast<BOOLEAN>(State)};
	return File.IoControl(FSCTL_SET_SPARSE,&sb,sizeof(sb),nullptr,0,&BytesReturned,nullptr);
}

int ESetFileSparse(const string& Name,bool State,DWORD FileAttr,int SkipMode)
{
	if (((FileAttr & FILE_ATTRIBUTE_SPARSE_FILE) != 0) == State || FileAttr & FILE_ATTRIBUTE_DIRECTORY)
		return SETATTR_RET_OK;

	if (FileAttr&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		os::fs::set_file_attributes(Name,FileAttr&~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

	SCOPE_EXIT
	{
		if (FileAttr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
		os::fs::set_file_attributes(Name, FileAttr);
	};

	while (!SetFileSparse(Name,State))
	{
		const auto ErrorState = error_state::fetch();

		switch (SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrSparseCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}

int ESetFileOwner(const string& Name, const string& Owner,int SkipMode)
{
	while (!SetFileOwner(Name, Owner))
	{
		const auto ErrorState = error_state::fetch();

		switch (SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrOwnerCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}
	return SETATTR_RET_OK;
}

int EDeleteReparsePoint(const string& Name, DWORD FileAttr, int SkipMode)
{
	if (!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
		return SETATTR_RET_OK;

	while (!DeleteReparsePoint(Name))
	{
		const auto ErrorState = error_state::fetch();

		switch (SkipMode != -1? SkipMode : ShowErrorMessage(ErrorState, lng::MSetAttrCannotFor, Name))
		{
		case response::retry:
			break;
		case response::skip:
			return SETATTR_RET_SKIP;
		case response::skip_all:
			return SETATTR_RET_SKIPALL;
		case -2:
		case -1:
		case response::cancel:
			return SETATTR_RET_ERROR;
		}
	}
	return SETATTR_RET_OK;
}

void enum_attributes(const std::function<bool(DWORD, wchar_t)>& Pred)
{
	static const std::pair<DWORD, wchar_t> AttrMap[] =
	{
		{FILE_ATTRIBUTE_READONLY, L'R'},
		{FILE_ATTRIBUTE_ARCHIVE, L'A'},
		{FILE_ATTRIBUTE_HIDDEN, L'H'},
		{FILE_ATTRIBUTE_SYSTEM, L'S'},
		{FILE_ATTRIBUTE_COMPRESSED, L'C'},
		{FILE_ATTRIBUTE_ENCRYPTED, L'E'},
		{FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, L'I'},
		{FILE_ATTRIBUTE_DIRECTORY, L'D'},
		{FILE_ATTRIBUTE_SPARSE_FILE, L'$'},
		{FILE_ATTRIBUTE_TEMPORARY, L'T'},
		{FILE_ATTRIBUTE_OFFLINE, L'O'},
		{FILE_ATTRIBUTE_REPARSE_POINT, L'L'},
		{FILE_ATTRIBUTE_VIRTUAL, L'V'},
		{FILE_ATTRIBUTE_INTEGRITY_STREAM, L'G'},
		{FILE_ATTRIBUTE_NO_SCRUB_DATA, L'N'},
	};

	for (const auto& i: AttrMap)
	{
		if (!Pred(i.first, i.second))
			break;
	}
}
