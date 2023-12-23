/*
network.cpp

misc network functions
*/
/*
Copyright © 2009 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "network.hpp"

// Internal:
#include "lang.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"
#include "platform.reg.hpp"

// Common:
#include "common.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

static string GetStoredUserName(wchar_t Drive)
{
	//Тут может быть надо заюзать WNetGetUser
	return os::reg::key::current_user.get<string>(concat(L"Network\\"sv, Drive), L"UserName"sv).value_or(L""s);
}

os::fs::drives_set GetSavedNetworkDrives()
{
	HANDLE hEnum;
	if (WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_DISK, 0, nullptr, &hEnum) != NO_ERROR)
		return 0;

	SCOPE_EXIT{ WNetCloseEnum(hEnum); };

	os::fs::drives_set Drives;

	block_ptr<NETRESOURCE> Buffer(16 * 1024);

	for (;;)
	{
		DWORD Count = -1;
		auto BufferSize = static_cast<DWORD>(Buffer.size());
		const auto Result = WNetEnumResource(hEnum, &Count, Buffer.data(), &BufferSize);

		if (Result == ERROR_MORE_DATA)
		{
			Buffer.reset(BufferSize);
			continue;
		}

		if (Result != NO_ERROR || !Count)
			break;

		for (const auto& i: std::span(Buffer.data(), Count))
		{
			const auto Name = i.lpLocalName;
			if (os::fs::drive::is_standard_letter(Name[0]) && Name[1] == L':')
			{
				Drives.set(os::fs::drive::get_number(Name[0]));
			}
		}
	}

	return Drives;
}

bool ConnectToNetworkResource(string_view const NewDir)
{
	string LocalName, RemoteName;

	const auto IsDrive = ParsePath(NewDir) == root_type::drive_letter;
	if (IsDrive)
	{
		LocalName = NewDir.substr(0, 2);
		// TODO: check result
		DriveLocalToRemoteName(false, NewDir, RemoteName);
	}
	else
	{
		LocalName = NewDir;
		RemoteName = NewDir;
		DeleteEndSlash(RemoteName);
	}

	auto UserName = IsDrive? GetStoredUserName(NewDir[0]) : L""s;

	NETRESOURCE netResource{};
	netResource.dwType = RESOURCETYPE_DISK;
	netResource.lpLocalName = IsDrive? UNSAFE_CSTR(LocalName) : nullptr;
	netResource.lpRemoteName = UNSAFE_CSTR(RemoteName);
	netResource.lpProvider = nullptr;

	if (const auto Result = WNetAddConnection2(&netResource, nullptr, EmptyToNull(UserName), 0); Result == NO_ERROR ||
		(Result == ERROR_SESSION_CREDENTIAL_CONFLICT && WNetAddConnection2(&netResource, nullptr, nullptr, 0) == NO_ERROR))
	{
		return true;
	}

	string Password;

	for (;;)
	{
		if (!GetNameAndPassword(RemoteName, UserName, Password, {}, GNP_USELAST))
			return false;

		if (const auto Result = WNetAddConnection2(&netResource, Password.c_str(), EmptyToNull(UserName), 0); Result == NO_ERROR)
			return true;
		else if (Result != ERROR_ACCESS_DENIED && Result != ERROR_INVALID_PASSWORD && Result != ERROR_LOGON_FAILURE)
		{
			Message(MSG_WARNING, os::last_error(),
				msg(lng::MError),
				{
					string(NewDir)
				},
				{ lng::MOk });
			return false;
		}
	}
}

string ExtractComputerName(const string_view CurDir, string* const strTail)
{
	if (strTail)
		strTail->clear();

	string strNetDir;

	const auto PathType = ParsePath(CurDir);

	if (PathType == root_type::remote || PathType == root_type::unc_remote)
	{
		strNetDir = CurDir;
	}
	else if (PathType == root_type::drive_letter || PathType == root_type::win32nt_drive_letter)
	{
		if (!os::WNetGetConnection(CurDir.substr(PathType == root_type::drive_letter? 0 : L"\\\\?\\"sv.size(), L"C:"sv.size()), strNetDir))
			return {};
	}

	if (strNetDir.empty())
		return {};

	const auto NetDirPathType = ParsePath(strNetDir);
	if (NetDirPathType != root_type::remote && NetDirPathType != root_type::unc_remote)
		return {};

	auto Result = strNetDir.substr(NetDirPathType == root_type::remote? L"\\\\"sv.size() : L"\\\\?\\UNC\\"sv.size());
	const auto pos = FindSlash(Result);
	if (pos == string::npos)
		return {};

	if (strTail)
		strTail->assign(Result, pos + 1);

	Result.resize(pos);

	return Result;
}

bool DriveLocalToRemoteName(bool const DetectNetworkDrive, string_view const LocalPath, string &strDest)
{
	const auto PathType = ParsePath(LocalPath);
	if (PathType != root_type::drive_letter && PathType != root_type::win32nt_drive_letter)
		return false;

	if (DetectNetworkDrive && os::fs::drive::get_type(LocalPath) != DRIVE_REMOTE)
		return false;

	string strRemoteName;
	if (!os::WNetGetConnection(LocalPath.substr(PathType == root_type::drive_letter ? 0 : L"\\\\?\\"sv.size(), L"C:"sv.size()), strRemoteName))
		return false;

	strDest = strRemoteName;
	return true;
}
