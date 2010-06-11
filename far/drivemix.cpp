/*
drivemix.cpp

Misc functions for drive/disk info
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

#include "drivemix.hpp"
#include "config.hpp"
#include "flink.hpp"
#include "cddrv.hpp"
#include "pathmix.hpp"

/*
  FarGetLogicalDrives
  оболочка вокруг GetLogicalDrives, с учетом скрытых логических дисков
  HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer
  NoDrives:DWORD
    ѕоследние 26 бит определ€ют буквы дисков от A до Z (отсчет справа налево).
    ƒиск виден при установленном 0 и скрыт при значении 1.
    ƒиск A представлен правой последней цифрой при двоичном представлении.
    Ќапример, значение 00000000000000000000010101(0x7h)
    скрывает диски A, C, и E
*/
DWORD WINAPI FarGetLogicalDrives()
{
	static DWORD LogicalDrivesMask = 0;
	DWORD NoDrives=0;

	if ((!Opt.RememberLogicalDrives) || !LogicalDrivesMask)
		LogicalDrivesMask=GetLogicalDrives();

	if (!Opt.Policies.ShowHiddenDrives)
	{
		HKEY hKey;

		if (RegOpenKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS && hKey)
		{
			int ExitCode;
			DWORD Type,Size=sizeof(NoDrives);
			ExitCode=RegQueryValueEx(hKey,L"NoDrives",0,&Type,(BYTE *)&NoDrives,&Size);
			RegCloseKey(hKey);

			if (ExitCode != ERROR_SUCCESS)
				NoDrives=0;
		}
	}

	return LogicalDrivesMask&(~NoDrives);
}

int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType)
{
	string strSrcRoot, strDestRoot;
	int SrcDriveType, DestDriveType;
	DWORD SrcVolumeNumber=0, DestVolumeNumber=0;
	string strSrcVolumeName, strDestVolumeName;
	string strSrcFileSystemName, strDestFileSystemName;
	DWORD SrcFileSystemFlags, DestFileSystemFlags;
	DWORD SrcMaximumComponentLength, DestMaximumComponentLength;
	strSrcRoot=SrcPath;
	strDestRoot=DestPath;
	ConvertNameToUNC(strSrcRoot);
	ConvertNameToUNC(strDestRoot);
	GetPathRoot(strSrcRoot,strSrcRoot);
	GetPathRoot(strDestRoot,strDestRoot);
	SrcDriveType=FAR_GetDriveType(strSrcRoot,nullptr,TRUE);
	DestDriveType=FAR_GetDriveType(strDestRoot,nullptr,TRUE);

	if (!apiGetVolumeInformation(strSrcRoot,&strSrcVolumeName,&SrcVolumeNumber,&SrcMaximumComponentLength,&SrcFileSystemFlags,&strSrcFileSystemName))
		return FALSE;

	if (!apiGetVolumeInformation(strDestRoot,&strDestVolumeName,&DestVolumeNumber,&DestMaximumComponentLength,&DestFileSystemFlags,&strDestFileSystemName))
		return FALSE;

	if (CheckedType == CHECKEDPROPS_ISSAMEDISK)
	{
		if (!wcspbrk(DestPath,L"\\:"))
			return TRUE;

		if (((strSrcRoot.At(0)==L'\\' && strSrcRoot.At(1)==L'\\') || (strDestRoot.At(0)==L'\\' && strDestRoot.At(1)==L'\\')) &&
		        StrCmpI(strSrcRoot,strDestRoot))
			return FALSE;

		if (!*SrcPath || !*DestPath || (SrcPath[1]!=L':' && DestPath[1]!=L':'))  //????
			return TRUE;

		if (Upper(strDestRoot.At(0))==Upper(strSrcRoot.At(0)))
			return TRUE;

		unsigned __int64 SrcTotalSize,SrcTotalFree,SrcUserFree;
		unsigned __int64 DestTotalSize,DestTotalFree,DestUserFree;

		if (!apiGetDiskSize(SrcPath,&SrcTotalSize,&SrcTotalFree,&SrcUserFree))
			return FALSE;

		if (!apiGetDiskSize(DestPath,&DestTotalSize,&DestTotalFree,&DestUserFree))
			return FALSE;

		if (!(SrcVolumeNumber &&
		        SrcVolumeNumber==DestVolumeNumber &&
		        !StrCmpI(strSrcVolumeName, strDestVolumeName) &&
		        SrcTotalSize==DestTotalSize))
			return FALSE;
	}
	else if (CheckedType == CHECKEDPROPS_ISDST_ENCRYPTION)
	{
		if (!(DestFileSystemFlags&FILE_SUPPORTS_ENCRYPTION))
			return FALSE;

		if (!(DestDriveType==DRIVE_REMOVABLE || DestDriveType==DRIVE_FIXED || DestDriveType==DRIVE_REMOTE))
			return FALSE;
	}

	return TRUE;
}

bool IsDriveTypeRemote(UINT DriveType)
{
	return DriveType == DRIVE_REMOTE || DriveType == DRIVE_REMOTE_NOT_CONNECTED;
}

int IsLocalDrive(const wchar_t *Path)
{
	DWORD DriveType = 0;
	wchar_t *lpwszRootDir,wszRootDir[8]=L"A:\\";

	if (IsLocalPath(Path))
	{
		lpwszRootDir = wszRootDir;
		lpwszRootDir[0] = Path[0];
		DriveType = FAR_GetDriveType(lpwszRootDir);
	}
	else
	{
		string strRootDir;
		ConvertNameToFull(Path, strRootDir);

		if (IsLocalPath(strRootDir))
		{
			strRootDir.SetLength(3);
			DriveType = FAR_GetDriveType(strRootDir);
		}
		else if (IsLocalPrefixPath(strRootDir))
		{
			strRootDir.SetLength(7);
			DriveType = FAR_GetDriveType(strRootDir);
		}
		else if (IsLocalVolumePath(strRootDir))
		{
			strRootDir.SetLength(49);
			DriveType = FAR_GetDriveType(strRootDir);
		}
	}

	return (DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_FIXED ||
	        IsDriveTypeCDROM(DriveType) || DriveType == DRIVE_RAMDISK);
}
