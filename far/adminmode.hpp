#pragma once

/*
adminmode.hpp

Admin mode
*/
/*
Copyright (c) 2010 Far Group
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

#include "CriticalSections.hpp"

enum ELEVATION_MODE
{
	ELEVATION_MODIFY_REQUEST = 0x00000001,
	ELEVATION_READ_REQUEST   = 0x00000002,
	ELEVATION_USE_PRIVILEGES = 0xf0000000,
};

enum ADMIN_COMMAND
{
	C_SERVICE_EXIT,
	C_FUNCTION_CREATEDIRECTORYEX,
	C_FUNCTION_REMOVEDIRECTORY,
	C_FUNCTION_DELETEFILE,
	C_FUNCTION_COPYFILEEX,
	C_FUNCTION_MOVEFILEEX,
	C_FUNCTION_GETFILEATTRIBUTES,
	C_FUNCTION_SETFILEATTRIBUTES,
	C_FUNCTION_CREATEHARDLINK,
	C_FUNCTION_CREATESYMBOLICLINK,
	C_FUNCTION_MOVETORECYCLEBIN,
	C_FUNCTION_FINDFIRSTFILEEX,
	C_FUNCTION_FINDNEXTFILE,
	C_FUNCTION_FINDCLOSE,
	C_FUNCTION_SETOWNER,
	C_FUNCTION_CREATEFILE,
	C_FUNCTION_CLOSEHANDLE,
	C_FUNCTION_READFILE,
	C_FUNCTION_WRITEFILE,
	C_FUNCTION_SETFILEPOINTEREX,
	C_FUNCTION_SETENDOFFILE,
	C_FUNCTION_GETFILETIME,
	C_FUNCTION_SETFILETIME,
	C_FUNCTION_GETFILESIZEEX,
	C_FUNCTION_FLUSHFILEBUFFERS,
	C_FUNCTION_GETFILEINFORMATIONBYHANDLE,
	C_FUNCTION_DEVICEIOCONTROL,
	C_FUNCTION_GETSTORAGEDEPENDENCYINFORMATION,
};

class AutoObject;

class AdminMode
{
public:
	AdminMode();
	~AdminMode();
	void ResetApprove();
	bool Elevated(){return Elevation;}

	bool fCreateDirectoryEx(LPCWSTR TemplateObject, LPCWSTR Object, LPSECURITY_ATTRIBUTES Attributes);
	bool fRemoveDirectory(LPCWSTR Object);
	bool fDeleteFile(LPCWSTR Object);
	void fCallbackRoutine(LPPROGRESS_ROUTINE ProgressRoutine) const;
	bool fCopyFileEx(LPCWSTR From, LPCWSTR To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags);
	bool fMoveFileEx(LPCWSTR From, LPCWSTR To, DWORD Flags);
	DWORD fGetFileAttributes(LPCWSTR Object);
	bool fSetFileAttributes(LPCWSTR Object, DWORD FileAttributes);
	bool fCreateHardLink(LPCWSTR Object,LPCWSTR Target,LPSECURITY_ATTRIBUTES SecurityAttributes);
	bool fCreateSymbolicLink(LPCWSTR Object, LPCWSTR Target, DWORD Flags);
	int fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct);
	HANDLE fFindFirstFileEx(LPCWSTR Object, FINDEX_INFO_LEVELS InfoLevelId, LPVOID FindFileData, FINDEX_SEARCH_OPS SearchOp, LPVOID lpSearchFilter, DWORD AdditionalFlags);
	bool fFindNextFile(HANDLE Handle, PWIN32_FIND_DATA W32FindData);
	bool fFindClose(HANDLE Handle);
	bool fSetOwner(LPCWSTR Object, LPCWSTR Owner);
	HANDLE fCreateFile(LPCWSTR Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile);
	bool fCloseHandle(HANDLE Handle);
	bool fReadFile(HANDLE Handle, LPVOID Buffer, DWORD NumberOfBytesToRead, LPDWORD NumberOfBytesRead, LPOVERLAPPED Overlapped);
	bool fWriteFile(HANDLE Handle, LPCVOID Buffer, DWORD NumberOfBytesToWrite, LPDWORD NumberOfBytesWritten, LPOVERLAPPED Overlapped);
	bool fSetFilePointerEx(HANDLE Handle, INT64 DistanceToMove, PINT64 NewFilePointer, DWORD MoveMethod);
	bool fSetEndOfFile(HANDLE Handle);
	bool fGetFileTime(HANDLE Handle, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime);
	bool fSetFileTime(HANDLE Handle, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime);
	bool fGetFileSizeEx(HANDLE Handle, UINT64& Size);
	bool fFlushFileBuffers(HANDLE Handle);
	bool fGetFileInformationByHandle(HANDLE Handle, BY_HANDLE_FILE_INFORMATION& bhfi);
	bool fDeviceIoControl(HANDLE Handle, DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped);
	DWORD fGetStorageDependencyInformation(HANDLE Handle, GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed);

private:
	HANDLE Pipe;
	HANDLE Process;
	int PID;
	DWORD MainThreadID;
	bool Elevation;
	bool DontAskAgain;
	bool Approve;
	bool AskApprove;
	bool Recurse;
	CriticalSection CS;

	bool ReadData(AutoObject& Data) const;
	bool WriteData(LPCVOID Data, DWORD DataSize) const;
	bool ReadInt(int& Data) const;
	bool ReadInt64(INT64& Data) const;
	bool WriteInt(int Data) const;
	bool WriteInt64(INT64 Data) const;
	bool SendCommand(ADMIN_COMMAND Command) const;
	bool ReceiveLastError() const;
	bool Initialize();
	bool AdminApproveDlg(int Why, LPCWSTR Object);
};

extern AdminMode Admin;

void AdminApproveDlgSync(LPVOID Param);

bool ElevationRequired(ELEVATION_MODE Mode);
bool IsUserAdmin();
int AdminMain(int PID, bool UsePrivileges);
