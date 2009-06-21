/*
network.cpp

misc network functions
*/
/*
Copyright (c) 2009 Far Group
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
#include "lang.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "drivemix.hpp"
#include "flink.hpp"
#include "cddrv.hpp"
#include "pathmix.hpp"

void GetStoredUserName(wchar_t cDrive, string &strUserName)
{
	//Тут может быть надо заюзать WNetGetUser
	strUserName = L"";
	const wchar_t KeyName[]={L'N',L'e',L't',L'w',L'o',L'r',L'k',L'\\',cDrive,L'\0'};
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,KeyName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS && hKey)
	{
		RegQueryStringValueEx(hKey, L"UserName", strUserName);
		RegCloseKey(hKey);
	}
}

void AddSavedNetworkDisks(DWORD& Mask, DWORD& NetworkMask)
{
	HANDLE hEnum;

	if (!WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_DISK, 0, 0, &hEnum))
	{
		DWORD bufsz = 16*1024;
		NETRESOURCE *netResource = (NETRESOURCE *)xf_malloc(bufsz);
		if (netResource)
		{
			while (1)
			{
				DWORD size=1;
				bufsz = 16*1024;
				memset(netResource,0,bufsz);
				DWORD res = WNetEnumResource(hEnum, &size, netResource, &bufsz);
				if (res == NO_ERROR && size > 0 && netResource->lpLocalName != NULL)
				{
					wchar_t letter = Lower(netResource->lpLocalName[0]);
					if (letter >= L'a' && letter <= L'z' && !wcscmp(netResource->lpLocalName+1, L":"))
					{
						int CurrBit = 1 << (letter - L'a');
						if (!(Mask&CurrBit))
						{
							Mask |= CurrBit;
							NetworkMask |=  CurrBit;
						}
					}
				}
				else
				{
					break;
				}
			}
			xf_free(netResource);
		}
		WNetCloseEnum(hEnum);
	}
}

void ConnectToNetworkDrive(const wchar_t *NewDir)
{
	string strRemoteName;
	DriveLocalToRemoteName(DRIVE_REMOTE_NOT_CONNECTED,*NewDir,strRemoteName);
	string strUserName, strPassword;
	GetStoredUserName(*NewDir, strUserName);
	NETRESOURCE netResource;
	netResource.dwType = RESOURCETYPE_DISK;
	netResource.lpLocalName = (wchar_t *)NewDir;
	netResource.lpRemoteName = (wchar_t *)strRemoteName.CPtr();
	netResource.lpProvider = 0;
	DWORD res = WNetAddConnection2(&netResource, 0, strUserName, 0);
	if (res == ERROR_SESSION_CREDENTIAL_CONFLICT)
		res = WNetAddConnection2(&netResource, 0, 0, 0);

	if (res)
	{
		while (1)
		{
			if (!GetNameAndPassword(strRemoteName, strUserName, strPassword, NULL, GNP_USELAST))
				break;

			res = WNetAddConnection2(&netResource, strPassword, strUserName, 0);

			if (!res)
				break;

			if ( res != ERROR_ACCESS_DENIED && res != ERROR_INVALID_PASSWORD && res != ERROR_LOGON_FAILURE)
			{
				string strMsgStr;
				GetErrorString(strMsgStr);
				Message(MSG_WARNING, 1,	MSG(MError), strMsgStr, MSG(MOk));
				break;
			}
		}
	}
}

string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName)
{
  string strNetDir;

  strComputerName=L"";

  if ( CurDir[0]==L'\\' && CurDir[1]==L'\\')
  {
    strNetDir = CurDir;
  }
  else
  {
    /* $ 28.03.2002 KM
       - Падение VC на
         char *LocalName="A:";
         *LocalName=*CurDir;
         Так как память в LocalName ReadOnly.
    */
    wchar_t LocalName[3];

    xwcsncpy (LocalName, CurDir, 2);

    apiWNetGetConnection (LocalName, strNetDir);
  }

  if ( strNetDir.At(0)==L'\\' && strNetDir.At(1) == L'\\')
  {
    strComputerName = (const wchar_t*)strNetDir+2;

    size_t pos;
		if (!FirstSlash(strComputerName,pos))
    {
      strComputerName.SetLength(0);
    }
    else
    {
      strComputerName.SetLength(pos);
    }
  }

  return strComputerName;
}

string &DriveLocalToRemoteName(int DriveType,wchar_t Letter,string &strDest)
{
  int NetPathShown=FALSE, IsOK=FALSE;
  wchar_t LocalName[8]=L" :\0\0\0";
  string strRemoteName;

  *LocalName=Letter;
  strDest=L"";

  if(DriveType == DRIVE_UNKNOWN)
  {
    LocalName[2]=L'\\';
    DriveType = FAR_GetDriveType(LocalName);
    LocalName[2]=0;
  }

  if (IsDriveTypeRemote(DriveType))
  {
    DWORD res = apiWNetGetConnection(LocalName,strRemoteName);
    if (res == NO_ERROR || res == ERROR_CONNECTION_UNAVAIL)
    {
      NetPathShown=TRUE;
      IsOK=TRUE;
    }
  }

  if (!NetPathShown)
    if (GetSubstName(DriveType,LocalName,strRemoteName))
      IsOK=TRUE;

  if(IsOK)
    strDest = strRemoteName;

  return strDest;
}
