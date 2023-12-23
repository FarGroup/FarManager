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

// Internal:

// Platform:
#include "platform.concurrency.hpp"
#include "platform.fwd.hpp"
#include "platform.security.hpp"

// Common:
#include "common/singleton.hpp"

// External:

//----------------------------------------------------------------------------

enum class lng : int;

enum ELEVATION_MODE
{
	ELEVATION_MODIFY_REQUEST = 0_bit,
	ELEVATION_READ_REQUEST   = 1_bit,
	ELEVATION_USE_PRIVILEGES = 31_bit
};

enum ELEVATION_COMMAND: int;

class elevation: public singleton<elevation>
{
	IMPLEMENTS_SINGLETON;

public:
	void ResetApprove();
	bool Elevated() const {return m_Elevation;}

	bool create_directory(const string& TemplateObject, const string& Object, SECURITY_ATTRIBUTES* Attributes);
	bool remove_directory(const string& Object);
	bool delete_file(const string& Object);
	bool copy_file(const string& From, const string& To, LPPROGRESS_ROUTINE ProgressRoutine, void* Data, BOOL* Cancel, DWORD Flags);
	bool move_file(const string& From, const string& To, DWORD Flags);
	bool replace_file(const string& To, const string& From, const string& Backup, DWORD Flags);
	os::fs::attributes get_file_attributes(const string& Object);
	bool set_file_attributes(const string& Object, os::fs::attributes FileAttributes);
	bool create_hard_link(const string& Object, const string& Target, SECURITY_ATTRIBUTES* SecurityAttributes);

	bool fCreateSymbolicLink(string_view Object, string_view Target, DWORD Flags);
	bool fMoveToRecycleBin(string_view Object);
	bool fSetOwner(const string& Object, const string& Owner);

	HANDLE create_file(const string& Object, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile);
	bool set_file_encryption(const string& Object, bool Encrypt);
	bool detach_virtual_disk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);
	bool get_disk_free_space(const string& Object, unsigned long long* FreeBytesAvailableToCaller, unsigned long long* TotalNumberOfBytes, unsigned long long* TotalNumberOfFreeBytes);

	os::security::descriptor get_file_security(string const& Object, SECURITY_INFORMATION RequestedInformation);
	bool set_file_security(string const& Object, SECURITY_INFORMATION RequestedInformation, os::security::descriptor const& Descriptor);
	bool reset_file_security(string const& Object);

	class suppress
	{
	public:
		NONCOPYABLE(suppress);

		explicit(false) suppress(bool Completely = false);
		~suppress();

	private:
		elevation* m_owner;
		bool m_Completely;
	};

private:
	elevation() = default;
	~elevation();

	template<typename T>
	T Read() const;

	template<typename T>
	void Read(T& Data) const { Data = Read<T>(); }

	void Write(const auto&... Args) const;

	void RetrieveLastError() const;

	template<typename T>
	T RetrieveLastErrorAndResult() const;

	bool Initialize();
	bool ElevationApproveDlg(lng Why, string_view Object);
	void TerminateChildProcess() const;

	auto execute(lng Why, string_view Object, auto Fallback, const auto& PrivilegedHander, const auto& ElevatedHandler);

	void progress_routine(LPPROGRESS_ROUTINE ProgressRoutine) const;

	std::atomic_size_t m_Suppressions{};
	std::atomic_size_t m_CompleteSuppressions{};
	os::handle m_Pipe;
	os::handle m_Process;
	os::handle m_Job;

	bool m_IsApproved{};
	bool m_AskApprove{true};
	bool m_Elevation{};
	bool m_DontAskAgain{};
	bool m_DuplicateHandleGranted{};
	int m_Recurse{};
	os::critical_section m_CS;
	string m_PipeName;
};

bool ElevationRequired(ELEVATION_MODE Mode, bool UseNtStatus = true);
int ElevationMain(string_view Uuid, DWORD PID, bool UsePrivileges);
bool IsElevationArgument(const wchar_t* Argument);

#endif // ELEVATION_HPP_19857862_0EE5_4709_B3E9_C7E50239C2E0
