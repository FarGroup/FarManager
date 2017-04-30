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

#include "headers.hpp"
#pragma hdrstop

#include "network.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "drivemix.hpp"
#include "flink.hpp"
#include "cddrv.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"

static bool GetStoredUserName(wchar_t Drive, string &strUserName)
{
	//Тут может быть надо заюзать WNetGetUser
	strUserName.clear();
	wchar_t KeyName[] = L"Network\?";
	*std::prev(std::end(KeyName)) = Drive;

	return os::reg::GetValue(HKEY_CURRENT_USER, KeyName, L"UserName", strUserName);
}

os::fs::drives_set GetSavedNetworkDrives()
{
	HANDLE hEnum;
	if (WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_DISK, 0, nullptr, &hEnum) != NO_ERROR)
		return 0;

	SCOPE_EXIT{ WNetCloseEnum(hEnum); };

	os::fs::drives_set Drives;

	DWORD BufferSize = 16 * 1024;
	block_ptr<NETRESOURCE> netResource(BufferSize);

	for (;;)
	{
		DWORD Size = 1;
		BufferSize = 16 * 1024;
		memset(netResource.get(), 0, BufferSize);
		const auto Result = WNetEnumResource(hEnum, &Size, netResource.get(), &BufferSize);

		if (Result != NO_ERROR || !Size || !netResource->lpLocalName)
			break;

		if (os::fs::is_standard_drive_letter(netResource->lpLocalName[0]) && netResource->lpLocalName[1] == L':')
		{
			Drives.set(os::fs::get_drive_number(netResource->lpLocalName[0]));
		}
	}

	return Drives;
}

bool ConnectToNetworkResource(const string& NewDir)
{
	string LocalName, RemoteName;

	const auto IsDrive = ParsePath(NewDir) == PATH_DRIVELETTER;
	if (IsDrive)
	{
		LocalName = NewDir.substr(0, 2);
		// TODO: check result
		DriveLocalToRemoteName(DRIVE_REMOTE_NOT_CONNECTED, NewDir[0], RemoteName);
	}
	else
	{
		LocalName = NewDir;
		RemoteName = NewDir;
	}

	string strUserName, strPassword;
	if (IsDrive)
	{
		GetStoredUserName(NewDir[0], strUserName);
	}

	NETRESOURCE netResource {};
	netResource.dwType = RESOURCETYPE_DISK;
	netResource.lpLocalName = IsDrive? UNSAFE_CSTR(LocalName) : nullptr;
	netResource.lpRemoteName = UNSAFE_CSTR(RemoteName);
	netResource.lpProvider = nullptr;
	DWORD res = WNetAddConnection2(&netResource, nullptr, EmptyToNull(strUserName.data()), 0);

	if (res == ERROR_SESSION_CREDENTIAL_CONFLICT)
		res = WNetAddConnection2(&netResource, nullptr, nullptr, 0);

	if (res != NO_ERROR)
	{
		for (;;)
		{
			if (!GetNameAndPassword(RemoteName, strUserName, strPassword, nullptr, GNP_USELAST))
				break;

			res = WNetAddConnection2(&netResource, strPassword.data(), EmptyToNull(strUserName.data()), 0);

			if (res == NO_ERROR)
				break;

			Global->CatchError();

			if (res != ERROR_ACCESS_DENIED && res != ERROR_INVALID_PASSWORD && res != ERROR_LOGON_FAILURE)
			{
				Message(MSG_WARNING,
					msg(lng::MError),
					{
						GetErrorString()
					},
					{ lng::MOk });
				break;
			}
		}
	}
	return res == NO_ERROR;
}

string ExtractComputerName(const string& CurDir, string* strTail)
{
	string Result;

	string strNetDir;

	if (strTail)
		strTail->clear();

	const auto CurDirPathType = ParsePath(CurDir);
	if (CurDirPathType == PATH_REMOTE || CurDirPathType == PATH_REMOTEUNC)
	{
		strNetDir = CurDir;
	}
	else
	{
		string LocalName(CurDir, 0, 2);
		os::WNetGetConnection(LocalName, strNetDir);
	}

	if (!strNetDir.empty())
	{
		const auto NetDirPathType = ParsePath(strNetDir);
		if (NetDirPathType == PATH_REMOTE || NetDirPathType == PATH_REMOTEUNC)
		{
			Result = strNetDir.substr(NetDirPathType == PATH_REMOTE? 2 : 8);

			const auto pos = FindSlash(Result);
			if (pos != string::npos)
			{
				if (strTail)
				{
					*strTail = Result.substr(pos + 1);
				}
				Result.resize(pos);
			}
			else
			{
				Result.clear();
			}
		}
	}
	return Result;
}

bool DriveLocalToRemoteName(int DriveType, wchar_t Letter, string &strDest)
{
	const auto LocalName = os::fs::get_drive(Letter);

	if (DriveType == DRIVE_UNKNOWN)
	{
		DriveType = FAR_GetDriveType(LocalName);
	}

	string strRemoteName;

	if (IsDriveTypeRemote(DriveType) && os::WNetGetConnection(LocalName, strRemoteName))
	{
		strDest = strRemoteName;
		return true;
	}

	if (GetSubstName(DriveType, LocalName, strRemoteName))
	{
		strDest = strRemoteName;
		return true;
	}

	return false;
}
