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
#include "language.hpp"
#include "message.hpp"
#include "pathmix.hpp"
#include "fileowner.hpp"

int ESetFileAttributes(const string& Name,DWORD Attr,int SkipMode)
{
//_SVS(SysLog(L"Attr=0x%08X",Attr));
	if (Attr&FILE_ATTRIBUTE_DIRECTORY && Attr&FILE_ATTRIBUTE_TEMPORARY)
		Attr&=~FILE_ATTRIBUTE_TEMPORARY;

	while (!apiSetFileAttributes(Name,Attr))
	{
		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrCannotFor),Name,MSG(MHRetry),MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		switch (Code)
		{
			case -2:
			case -1:
			case 1:
				return SETATTR_RET_SKIP;
			case 2:
				return SETATTR_RET_SKIPALL;
			case 3:
				return SETATTR_RET_ERROR;
		}
	}

	return SETATTR_RET_OK;
}

static int SetFileCompression(const string& Name,int State)
{
	File file;

	if (!file.Open(Name, FILE_READ_DATA|FILE_WRITE_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
		return FALSE;

	USHORT NewState=State ? COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
	DWORD Result;
	int RetCode=file.IoControl(FSCTL_SET_COMPRESSION,&NewState,
	                            sizeof(NewState),nullptr,0,&Result);
	file.Close();
	return(RetCode);
}


int ESetFileCompression(const string& Name,int State,DWORD FileAttr,int SkipMode)
{
	if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
		return SETATTR_RET_OK;

	int Ret=SETATTR_RET_OK;

	if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

	// Drop Encryption
	if ((FileAttr & FILE_ATTRIBUTE_ENCRYPTED) && State)
		apiSetFileEncryption(Name, false);

	while (!SetFileCompression(Name,State))
	{
		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrCompressedCannotFor),Name,MSG(MHRetry),
			             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		if (Code==1 || Code<0)
		{
			Ret=SETATTR_RET_SKIP;
			break;
		}
		else if (Code==2)
		{
			Ret=SETATTR_RET_SKIPALL;
			break;
		}
		else if (Code==3)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}
	}

	// Set ReadOnly
	if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr);

	return(Ret);
}


int ESetFileEncryption(const string& Name, bool State, DWORD FileAttr, int SkipMode, int Silent)
{
	if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
		return SETATTR_RET_OK;

	int Ret=SETATTR_RET_OK;

	// Drop ReadOnly
	if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

	while (!apiSetFileEncryption(Name, State))
	{
		if (Silent)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}

		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrEncryptedCannotFor),Name,MSG(MHRetry), //BUGBUG
			             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		if (Code==1 || Code<0)
		{
			Ret=SETATTR_RET_SKIP;
			break;
		}

		if (Code==2)
		{
			Ret=SETATTR_RET_SKIPALL;
			break;
		}

		if (Code==3)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}
	}

	// Set ReadOnly
	if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr);

	return(Ret);
}


int ESetFileTime(const string& Name,FILETIME *LastWriteTime,FILETIME *CreationTime,FILETIME *LastAccessTime,FILETIME *ChangeTime,DWORD FileAttr,int SkipMode)
{
	if (!LastWriteTime && !CreationTime && !LastAccessTime && !ChangeTime)
		return SETATTR_RET_OK;

	for(;;)
	{
		if (FileAttr & FILE_ATTRIBUTE_READONLY)
			apiSetFileAttributes(Name,FileAttr & ~FILE_ATTRIBUTE_READONLY);

		bool SetTime=false;
		DWORD LastError=ERROR_SUCCESS;
		File file;
		if (!file.Open(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
		                           nullptr,OPEN_EXISTING,
		                           FILE_FLAG_OPEN_REPARSE_POINT))
		{
			LastError=GetLastError();
		}
		else
		{
			SetTime=file.SetTime(CreationTime,LastAccessTime,LastWriteTime, ChangeTime);
			LastError=GetLastError();
			file.Close();

			if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && LastError==ERROR_NOT_SUPPORTED)   // FIX: Mantis#223
			{
				string strDriveRoot;
				GetPathRoot(Name, strDriveRoot);

				if (GetDriveType(strDriveRoot)==DRIVE_REMOTE) break;
			}
		}

		if (FileAttr & FILE_ATTRIBUTE_READONLY)
			apiSetFileAttributes(Name,FileAttr);

		SetLastError(LastError);

		if (SetTime)
			break;

		int Code;

		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
			             MSG(MSetAttrTimeCannotFor),Name,MSG(MHRetry), //BUGBUG
			             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		switch (Code)
		{
			case -2:
			case -1:
			case 3:
				return SETATTR_RET_ERROR;
			case 2:
				return SETATTR_RET_SKIPALL;
			case 1:
				return SETATTR_RET_SKIP;
		}
	}

	return SETATTR_RET_OK;
}

static bool SetFileSparse(const string& Name,bool State)
{
	bool Ret=false;
	File file;
	if (file.Open(Name,FILE_WRITE_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING))
	{
		DWORD BytesReturned;
		FILE_SET_SPARSE_BUFFER sb={State};
		Ret=file.IoControl(FSCTL_SET_SPARSE,&sb,sizeof(sb),nullptr,0,&BytesReturned,nullptr);
		file.Close();
	}
	return Ret;
}

int ESetFileSparse(const string& Name,bool State,DWORD FileAttr,int SkipMode)
{
	int Ret=SETATTR_RET_OK;

	if ((((FileAttr&FILE_ATTRIBUTE_SPARSE_FILE)!=0)!=State) && !(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
	{
		if (FileAttr&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
			apiSetFileAttributes(Name,FileAttr&~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

		while (!SetFileSparse(Name,State))
		{
			int Code;

			if (SkipMode!=-1)
				Code=SkipMode;
			else
				Code=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
				             MSG(MSetAttrSparseCannotFor),Name,MSG(MHRetry),
				             MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

			if (Code==1 || Code<0)
			{
				Ret=SETATTR_RET_SKIP;
				break;
			}
			else if (Code==2)
			{
				Ret=SETATTR_RET_SKIPALL;
				break;
			}
			else if (Code==3)
			{
				Ret=SETATTR_RET_ERROR;
				break;
			}
		}

		if (FileAttr&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
			apiSetFileAttributes(Name,FileAttr);
	}

	return Ret;
}

int ESetFileOwner(const string& Name, const string& Owner,int SkipMode)
{
	int Ret=SETATTR_RET_OK;
	while (!SetOwner(Name,Owner))
	{
		int Code;
		if (SkipMode!=-1)
			Code=SkipMode;
		else
			Code=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),MSG(MSetAttrOwnerCannotFor),Name,MSG(MHRetry),MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));

		if (Code==1 || Code<0)
		{
			Ret=SETATTR_RET_SKIP;
			break;
		}
		else if (Code==2)
		{
			Ret=SETATTR_RET_SKIPALL;
			break;
		}
		else if (Code==3)
		{
			Ret=SETATTR_RET_ERROR;
			break;
		}
	}
	return Ret;
}
