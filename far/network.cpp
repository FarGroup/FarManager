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
#include "language.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "drivemix.hpp"
#include "flink.hpp"
#include "cddrv.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"

void GetStoredUserName(wchar_t cDrive, string &strUserName)
{
	//Тут может быть надо заюзать WNetGetUser
	strUserName.clear();
	const wchar_t KeyName[]={L'N',L'e',L't',L'w',L'o',L'r',L'k',L'\\',cDrive,L'\0'};

	os::reg::GetValue(HKEY_CURRENT_USER, KeyName, L"UserName", strUserName);
}

os::drives_set AddSavedNetworkDisks(os::drives_set& Mask)
{
	FN_RETURN_TYPE(AddSavedNetworkDisks) Result;
	HANDLE hEnum;

	if (!WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_DISK, 0, 0, &hEnum))
	{
		DWORD bufsz = 16*1024;
		block_ptr<NETRESOURCE> netResource(bufsz);

		if (netResource)
		{
			for (;;)
			{
				DWORD size=1;
				bufsz = 16*1024;
				memset(netResource.get(),0,bufsz);
				DWORD res = WNetEnumResource(hEnum, &size, netResource.get(), &bufsz);

				if (res == NO_ERROR && size > 0 && netResource->lpLocalName )
				{
					wchar_t letter = ToLower(netResource->lpLocalName[0]);

					if (letter >= L'a' && letter <= L'z' && !wcscmp(netResource->lpLocalName+1, L":"))
					{
						size_t index = letter - L'a';
						if (!Mask[index])
						{
							Mask[index] = 1;
							Result[index] = 1;
						}
					}
				}
				else
				{
					break;
				}
			}
		}

		WNetCloseEnum(hEnum);
	}
	return Result;
}

void ConnectToNetworkDrive(const string& NewDir)
{
	string strRemoteName;
	DriveLocalToRemoteName(DRIVE_REMOTE_NOT_CONNECTED,NewDir[0],strRemoteName);
	string strUserName, strPassword;
	GetStoredUserName(NewDir[0], strUserName);
	NETRESOURCE netResource;
	netResource.dwType = RESOURCETYPE_DISK;
	netResource.lpLocalName = UNSAFE_CSTR(NewDir);
	netResource.lpRemoteName = UNSAFE_CSTR(strRemoteName);
	netResource.lpProvider = 0;
	DWORD res = WNetAddConnection2(&netResource, nullptr, EmptyToNull(strUserName.data()), 0);

	if (res == ERROR_SESSION_CREDENTIAL_CONFLICT)
		res = WNetAddConnection2(&netResource, nullptr, nullptr, 0);

	if (res)
	{
		for (;;)
		{
			if (!GetNameAndPassword(strRemoteName, strUserName, strPassword, nullptr, GNP_USELAST))
				break;

			res = WNetAddConnection2(&netResource, strPassword.data(), EmptyToNull(strUserName.data()), 0);

			if (!res)
				break;

			Global->CatchError();

			if (res != ERROR_ACCESS_DENIED && res != ERROR_INVALID_PASSWORD && res != ERROR_LOGON_FAILURE)
			{
				Message(MSG_WARNING, 1, MSG(MError), GetErrorString().data(), MSG(MOk));
				break;
			}
		}
	}
}

string &CurPath2ComputerName(const string& CurDir, string &strComputerName, string &strTail)
{
	string strNetDir;
	strComputerName.clear();
	strTail.clear();

	if (!CurDir.compare(0, 2, L"\\\\"))
	{
		strNetDir = CurDir;
	}
	else
	{
		string LocalName(CurDir.data(), 2);
		os::WNetGetConnection(LocalName, strNetDir);
	}

	if (!strNetDir.compare(0, 2, L"\\\\"))
	{
		strComputerName = strNetDir.substr(2);
		size_t pos;

		if (!FindSlash(pos,strComputerName))
			strComputerName.clear();
		else
		{
			strTail = strComputerName.substr(pos + 1);
			strComputerName.resize(pos);
		}
	}

	return strComputerName;
}

bool DriveLocalToRemoteName(int DriveType, wchar_t Letter, string &strDest)
{
	bool NetPathShown=false, IsOK=false;
	wchar_t LocalName[8]=L" :\0\0\0";
	string strRemoteName;

	*LocalName=Letter;
	strDest.clear();

	if (DriveType == DRIVE_UNKNOWN)
	{
		LocalName[2]=L'\\';
		DriveType = FAR_GetDriveType(LocalName);
		LocalName[2]=0;
	}

	if (IsDriveTypeRemote(DriveType))
	{
		DWORD res = os::WNetGetConnection(LocalName,strRemoteName);

		if (res == NO_ERROR || res == ERROR_CONNECTION_UNAVAIL)
		{
			NetPathShown=true;
			IsOK=true;
		}
	}

	if (!NetPathShown && GetSubstName(DriveType, LocalName, strRemoteName))
		IsOK=true;

	if (IsOK)
		strDest = strRemoteName;

	return IsOK;
}
