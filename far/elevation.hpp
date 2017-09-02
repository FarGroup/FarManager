#ifndef ELEVATION_HPP_19857862_0EE5_4709_B3E9_C7E50239C2E0
#define ELEVATION_HPP_19857862_0EE5_4709_B3E9_C7E50239C2E0
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

#include "synchro.hpp"

enum class lng : int;

enum ELEVATION_MODE
{
	ELEVATION_MODIFY_REQUEST = 0x00000001,
	ELEVATION_READ_REQUEST   = 0x00000002,
	ELEVATION_USE_PRIVILEGES = 0xf0000000,
};

enum ELEVATION_COMMAND: int;

class elevation: noncopyable, public singleton<elevation>
{
	IMPLEMENTS_SINGLETON(elevation);

public:
	~elevation();
	void ResetApprove();
	bool Elevated() const {return m_Elevation;}

	bool fCreateDirectoryEx(const string& TemplateObject, const string& Object, LPSECURITY_ATTRIBUTES Attributes);
	bool fRemoveDirectory(const string& Object);
	bool fDeleteFile(const string& Object);
	void fCallbackRoutine(LPPROGRESS_ROUTINE ProgressRoutine) const;
	bool fCopyFileEx(const string& From, const string& To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags);
	bool fMoveFileEx(const string& From, const string& To, DWORD Flags);
	DWORD fGetFileAttributes(const string& Object);
	bool fSetFileAttributes(const string& Object, DWORD FileAttributes);
	bool fCreateHardLink(const string& Object,const string& Target,LPSECURITY_ATTRIBUTES SecurityAttributes);
	bool fCreateSymbolicLink(const string& Object, const string& Target, DWORD Flags);
	int fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct);
	bool fSetOwner(const string& Object, const string& Owner);
	HANDLE fCreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile);
	bool fSetFileEncryption(const string& Object, bool Encrypt);
	bool fDetachVirtualDisk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);
	bool fGetDiskFreeSpaceEx(const string& Object, ULARGE_INTEGER* FreeBytesAvailableToCaller, ULARGE_INTEGER* TotalNumberOfBytes, ULARGE_INTEGER* TotalNumberOfFreeBytes);

	class suppress: noncopyable
	{
	public:
		suppress(): m_owner(Global? &instance() : nullptr) { if (m_owner) ++m_owner->m_Suppressions; }
		~suppress() { if (m_owner) --m_owner->m_Suppressions; }

	private:
		elevation* m_owner;
	};

private:
	elevation();

	template<typename T>
	T Read() const;

	template<typename T>
	void Read(T& Data) const { Data = Read<T>(); }

	static void Write() {}

	template<typename T, typename... args>
	void Write(const T& Data, args&&... Args) const;

	template<typename T>
	void WriteArg(const T& Data) const;

	void WriteArg(const blob_view& Data) const;

	void RetrieveLastError() const;

	template<typename T>
	T RetrieveLastErrorAndResult() const;

	bool Initialize();
	bool ElevationApproveDlg(lng Why, const string& Object);
	void TerminateChildProcess() const;

	template<typename T, typename F1, typename F2>
	auto execute(lng Why, const string& Object, T Fallback, const F1& PrivilegedHander, const F2& ElevatedHandler);

	std::atomic_ulong m_Suppressions;
	os::handle m_Pipe;
	os::handle m_Process;
	os::handle m_Job;

	bool m_IsApproved;
	bool m_AskApprove;
	bool m_Elevation;
	bool m_DontAskAgain;
	int m_Recurse;
	os::critical_section m_CS;
	string m_PipeName;
};

bool ElevationRequired(ELEVATION_MODE Mode, bool UseNtStatus = true);
int ElevationMain(const wchar_t* guid, DWORD PID, bool UsePrivileges);
bool IsElevationArgument(const wchar_t* Argument);

#endif // ELEVATION_HPP_19857862_0EE5_4709_B3E9_C7E50239C2E0
