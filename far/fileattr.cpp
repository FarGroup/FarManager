/*
fileattr.cpp

Работа с атрибутами файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
#include "flink.hpp"
#include "language.hpp"
#include "message.hpp"

static int SetFileEncryption(const wchar_t *Name,int State);
static int SetFileCompression(const wchar_t *Name,int State);
static bool SetFileSparse(const wchar_t *Name,bool State);


int ESetFileAttributes(const wchar_t *Name,DWORD Attr,int SkipMode)
{
//_SVS(SysLog(L"Attr=0x%08X",Attr));
	if(Attr&FILE_ATTRIBUTE_DIRECTORY && Attr&FILE_ATTRIBUTE_TEMPORARY)
		Attr&=~FILE_ATTRIBUTE_TEMPORARY;
	while (!apiSetFileAttributes(Name,Attr))
  {
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
             MSG(MSetAttrCannotFor),Name,MSG(MHRetry),MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));
    switch(Code)
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

static int SetFileCompression(const wchar_t *Name,int State)
{
  HANDLE hFile=apiCreateFile(Name,FILE_READ_DATA|FILE_WRITE_DATA,
                 FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                 FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FALSE);
  USHORT NewState=State ? COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
  DWORD Result;
  int RetCode=DeviceIoControl(hFile,FSCTL_SET_COMPRESSION,&NewState,
                              sizeof(NewState),NULL,0,&Result,NULL);
  CloseHandle(hFile);
  return(RetCode);
}


int ESetFileCompression(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode)
{
  if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
    return SETATTR_RET_OK;

  int Ret=SETATTR_RET_OK;
  if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

  // Drop Encryption
  if ((FileAttr & FILE_ATTRIBUTE_ENCRYPTED) && State)
    SetFileEncryption(Name,0);

  while (!SetFileCompression(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
    {
      Ret=SETATTR_RET_OK;
      break;
    }
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
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


static int SetFileEncryption(const wchar_t *Name,int State)
{
	return State?EncryptFile(Name):DecryptFile(Name,0);
}


int ESetFileEncryption(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode,int Silent)
{
  if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
    return SETATTR_RET_OK;

  int Ret=SETATTR_RET_OK;

  // Drop ReadOnly
  if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

  while (!SetFileEncryption(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
      break;

    if(Silent)
    {
      Ret=SETATTR_RET_ERROR;
      break;
    }
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
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


int ESetFileTime(const wchar_t *Name,FILETIME *LastWriteTime,FILETIME *CreationTime,
                  FILETIME *LastAccessTime,DWORD FileAttr,int SkipMode)
{
  if (LastWriteTime==NULL && CreationTime==NULL && LastAccessTime==NULL)
    return SETATTR_RET_OK;

  while (1)
  {
    if (FileAttr & FILE_ATTRIBUTE_READONLY)
			apiSetFileAttributes(Name,FileAttr & ~FILE_ATTRIBUTE_READONLY);

    HANDLE hFile=apiCreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                 NULL,OPEN_EXISTING,
                 (FileAttr & FILE_ATTRIBUTE_DIRECTORY) ? FILE_FLAG_BACKUP_SEMANTICS:0);
    int SetTime;
    DWORD LastError=0;
    if (hFile==INVALID_HANDLE_VALUE)
    {
      SetTime=FALSE;
      LastError=GetLastError();
    }
    else
    {
      SetTime=SetFileTime(hFile,CreationTime,LastAccessTime,LastWriteTime);
      LastError=GetLastError();
      CloseHandle(hFile);

      if ( (FileAttr & FILE_ATTRIBUTE_DIRECTORY) && LastError==ERROR_NOT_SUPPORTED ) // FIX: Mantis#223
      {
        string strDriveRoot;
        GetPathRoot (Name, strDriveRoot);
				if ( GetDriveType(strDriveRoot)==DRIVE_REMOTE ) break;
      }
    }

    if (FileAttr & FILE_ATTRIBUTE_READONLY)
			apiSetFileAttributes(Name,FileAttr);
    SetLastError(LastError);

    if (SetTime)
      break;
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                MSG(MSetAttrTimeCannotFor),Name,MSG(MHRetry), //BUGBUG
                MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));
    switch(Code)
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

static bool SetFileSparse(const wchar_t *Name,bool State)
{
	bool Ret=false;
	HANDLE hFile=apiCreateFile(Name,FILE_WRITE_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD BytesReturned;
		FILE_SET_SPARSE_BUFFER sb={State};
		Ret=(DeviceIoControl(hFile,FSCTL_SET_SPARSE,&sb,sizeof(sb),NULL,0,&BytesReturned,NULL)!=0);
		CloseHandle(hFile);
	}
	return Ret;
}

int ESetFileSparse(const wchar_t *Name,bool State,DWORD FileAttr,int SkipMode)
{
	int Ret=SETATTR_RET_OK;
	if((((FileAttr&FILE_ATTRIBUTE_SPARSE_FILE)!=0)!=State) && !(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
	{
		if (FileAttr&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
			apiSetFileAttributes(Name,FileAttr&~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

		while (!SetFileSparse(Name,State))
		{
			int Code;
			if(SkipMode!=-1)
				Code=SkipMode;
			else
				Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
									MSG(MSetAttrSparseCannotFor),Name,MSG(MHRetry),
									MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));
			if(Code==1 || Code<0)
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

	if(FileAttr&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
		apiSetFileAttributes(Name,FileAttr);
	}
	return Ret;
}
