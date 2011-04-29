#pragma once

/*
elevation.hpp

Elevation
*/
/*
Copyright © 2010 Far Group
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

enum ELEVATION_COMMAND
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
	C_FUNCTION_SETOWNER,
	C_FUNCTION_CREATEFILE,
};

class AutoObject;

class DisableElevation : private NonCopyable
{
public:
	DisableElevation();
	~DisableElevation();
private:
	DWORD Value;
};

class elevation : private NonCopyable
{
public:
	elevation();
	~elevation();
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
	bool fSetOwner(LPCWSTR Object, LPCWSTR Owner);
	HANDLE fCreateFile(LPCWSTR Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile);

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
	string strPipeID;

	bool ReadData(AutoObject& Data) const;
	bool WriteData(LPCVOID Data, size_t DataSize) const;
	bool ReadInt(int& Data) const;
	bool ReadInt64(INT64& Data) const;
	bool WriteInt(int Data) const;
	bool WriteInt64(INT64 Data) const;
	bool SendCommand(ELEVATION_COMMAND Command) const;
	bool ReceiveLastError() const;
	bool Initialize();
	bool ElevationApproveDlg(int Why, LPCWSTR Object);
};

extern elevation Elevation;

void ElevationApproveDlgSync(LPVOID Param);

bool ElevationRequired(ELEVATION_MODE Mode);
bool IsUserAdmin();
int ElevationMain(LPCWSTR guid, DWORD PID, bool UsePrivileges);
