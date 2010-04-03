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

enum ADMIN_COMMAND;

class AutoObject;

class AdminMode
{
public:
	AdminMode();
	~AdminMode();
	void ResetApprove(){Approve=false; AskApprove=true;}

	bool CreateDirectory(LPCWSTR Object, LPSECURITY_ATTRIBUTES Attributes);
	bool RemoveDirectory(LPCWSTR Object);
	bool DeleteFile(LPCWSTR Object);
	void CallbackRoutine();
	bool CopyFileEx(LPCWSTR From, LPCWSTR To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags);
	bool MoveFileEx(LPCWSTR From, LPCWSTR To, DWORD Flags);
	bool SetFileAttributes(LPCWSTR Object, DWORD FileAttributes);
	bool CreateHardLink(LPCWSTR Object,LPCWSTR Target,LPSECURITY_ATTRIBUTES SecurityAttributes);
	bool CreateSymbolicLink(LPCWSTR Object, LPCWSTR Target, DWORD Flags);
	bool SetReparseDataBuffer(LPCWSTR Object,PREPARSE_DATA_BUFFER ReparseDataBuffer);
	int MoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct);

private:
	HANDLE Pipe;
	int PID;
	bool Approve;
	bool AskApprove;
	LPPROGRESS_ROUTINE ProgressRoutine;

	bool ReadData(AutoObject& Data) const;
	bool WriteData(LPCVOID Data, DWORD DataSize) const;
	bool ReadInt(int& Data);
	bool WriteInt(int Data);
	bool SendCommand(ADMIN_COMMAND Command);
	bool ReceiveLastError();
	bool Initialize();
	bool AdminApproveDlg(LPCWSTR Object);
};

extern AdminMode Admin;

bool IsUserAdmin();
int AdminMain(int PID);
