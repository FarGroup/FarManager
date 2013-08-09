/*
cvtname.cpp

Функций для преобразования имен файлов/путей.
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "cvtname.hpp"
#include "flink.hpp"
#include "cddrv.hpp"
#include "syslog.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "network.hpp"
#include "imports.hpp"
#include "strmix.hpp"
#include "elevation.hpp"

#define IsColon(str)         (str == L':')
#define IsDot(str)           (str == L'.')
#define IsSlashBackward(str) (str == L'\\')
#define IsSlashForward(str)  (str == L'/')
#define IsQuestion(str)      (str == L'?')

bool MixToFullPath(string& strPath)
{
	//Skip all path to root (with slash if exists)
	size_t DirOffset = 0;
	ParsePath(strPath, &DirOffset);
	bool ok = true;

	//Process "." and ".." if exists
	for (size_t Pos = DirOffset; Pos < strPath.size();)
	{
		//fragment "."
		if (IsDot(strPath[Pos]) && (!Pos || IsSlash(strPath[Pos - 1])))
		{
			switch (strPath[Pos + 1])
			{
					//fragment ".\"
				case L'\\':
					//fragment "./"
				case L'/':
				{
					strPath.erase(Pos, 2);
					continue;
				}
				break;

				//fragment "." at the end
				case 0:
				{
					strPath.resize(Pos);
					// don't change x:\ to x:
					if (strPath[Pos - 2] != L':')
					{
						strPath.pop_back();
					}
					continue;
				}
				break;

				//fragment "..\" or "../" or ".." at the end
				case L'.':
				{
					if (Pos + 2 == strPath.size() || IsSlash(strPath[Pos + 2]))
					{
						if (Pos == DirOffset) // ".." on the top level
							ok = false;

						//Calculate subdir name offset
						size_t n = strPath.find_last_of(L"\\/", Pos-2);
						n = (n == string::npos || n < DirOffset) ? DirOffset : n+1;

						//fragment "..\" or "../"
						if (Pos + 2 < strPath.size())
						{
							strPath.erase(n, Pos + 3 - n);
						}
						//fragment ".." at the end
						else
						{
							strPath.resize(n);
						}

						Pos = n;
						continue;
					}
				}
				break;
			}
		}

		++Pos;
	}

	return ok;
}

bool RemoveDots(const string &Src, string &strDest)
{
	string strSrc(Src);
	string strRoot = ExtractPathRoot(Src);
	if (!strRoot.empty())
		strSrc = strSrc.substr(strRoot.size());
	bool ok = MixToFullPath(strSrc);
	strDest = strRoot + strSrc;
	return ok;
}

bool MixToFullPath(const string& stPath, string& strDest, const string& stCurrentDir)
{
	size_t lPath = stPath.size(), lCurrentDir = stCurrentDir.size(), lFullPath = lPath + lCurrentDir;

	if (lFullPath > 0)
	{
		strDest.clear();
		LPCWSTR pstPath = stPath.data(), pstCurrentDir = nullptr;
		bool blIgnore = false;
		size_t DirOffset = 0;
		PATH_TYPE PathType = ParsePath(stPath, &DirOffset);
		pstPath+=DirOffset;
		switch (PathType)
		{
			case PATH_UNKNOWN:
			{
				if(!stPath.empty() && IsSlash(stPath[0]) && (stPath.size() == 1 || !IsSlash(stPath[1]))) //"\" or "\abc"
				{
					if (!stCurrentDir.empty())
					{
						size_t DirOffset = 0;
						if (ParsePath(stCurrentDir, &DirOffset)!=PATH_UNKNOWN)
						{
							strDest=string(stCurrentDir.data(), DirOffset);
						}
					}
				}
				else
				{
					if(HasPathPrefix(stPath)) // \\?\<ANY_UNKNOWN_FORMAT>
					{
						blIgnore = true;
					}
					else //"abc" or whatever
					{
						pstCurrentDir=stCurrentDir.data();
					}
				}
			}
			break;
			case PATH_DRIVELETTER: //"C:" or "C:abc"
			{
				if(stPath.size() > 2 && IsSlash(stPath[2]))
				{
					pstPath=stPath.data();
				}
				else
				{
					WCHAR _DriveVar[]={L'=',stPath[0],L':',L'\0'};
					string DriveVar(_DriveVar);
					string strValue;

					if (apiGetEnvironmentVariable(DriveVar,strValue))
					{
						strDest=strValue;
					}
					else
					{
						if (Upper(stPath[0])==Upper(stCurrentDir[0]))
						{
							strDest=stCurrentDir;
						}
						else
						{
							strDest=DriveVar.data()+1;
						}
					}
					AddEndSlash(strDest);
				}
			}
			break;
			case PATH_REMOTE: //"\\abc"
			{
				pstPath=stPath.data();
			}
			break;
			case PATH_DRIVELETTERUNC: //"\\?\whatever"
			case PATH_REMOTEUNC:
			case PATH_VOLUMEGUID:
			case PATH_PIPE:
			{
				blIgnore=true;
				pstPath=stPath.data();
			}
			break;
		}

		if (pstCurrentDir)
		{
			strDest+=pstCurrentDir;
			AddEndSlash(strDest);
		}

		if (pstPath)
		{
			strDest+=pstPath;
		}

		if (!blIgnore && !HasPathPrefix(strDest))
			MixToFullPath(strDest);

		return true;
	}

	return false;
}

void ConvertNameToFull(const string& Src, string &strDest, LPCWSTR CurrentDirectory)
{
	string strCurDir;
	if(!CurrentDirectory)
	{
		apiGetCurrentDirectory(strCurDir);
	}
	else
	{
		strCurDir = CurrentDirectory;
	}
	string strSrc = Src;
	MixToFullPath(strSrc,strDest,strCurDir);
}

// try to replace volume GUID (if present) with drive letter
// used by ConvertNameToReal() only
static string TryConvertVolumeGuidToDrivePath(const string& Path, const wchar_t *path=nullptr, size_t path_len=0)
{
	string Result = Path;
	size_t DirectoryOffset;
	if (ParsePath(Path, &DirectoryOffset) == PATH_VOLUMEGUID)
	{
		if (Global->ifn->GetVolumePathNamesForVolumeNameWPresent())
		{
			wchar_t_ptr Buffer(NT_MAX_PATH);
			DWORD RetSize;
			BOOL Res = Global->ifn->GetVolumePathNamesForVolumeName(ExtractPathRoot(Path).data(), Buffer.get(), static_cast<DWORD>(Buffer.size()), &RetSize);

			if (!Res && RetSize > Buffer.size())
			{
				Buffer.reset(RetSize);
				Res = Global->ifn->GetVolumePathNamesForVolumeName(ExtractPathRoot(Path).data(), Buffer.get(), static_cast<DWORD>(Buffer.size()), &RetSize);
			}

			if (Res)
			{
				const wchar_t* PathName = Buffer.get();

				while (*PathName)
				{
					string strPath(PathName);

					if (path && strPath.size() <= path_len && 0 == StrCmpNI(path, PathName, strPath.size()))
						return strPath;

					if (IsRootPath(strPath))
					{
						Result.replace(0, DirectoryOffset + 1, strPath);
						break;
					}

					PathName += strPath.size() + 1;
				}
			}

			if (path)
				Result.clear();
		}

		else if (path)
			Result.clear();

		else
		{
			string DriveStrings;

			if (apiGetLogicalDriveStrings(DriveStrings))
			{
				const wchar_t* Drive = DriveStrings.data();
				string strVolumeGuid;

				while (*Drive)
				{
					if (apiGetVolumeNameForVolumeMountPoint(Drive, strVolumeGuid))
					{
						if (Path.compare(0, DirectoryOffset, strVolumeGuid.data(), DirectoryOffset) == 0)
						{
							Result.replace(0, DirectoryOffset + 1, Drive);
							break;
						}
					}

					Drive += StrLength(Drive) + 1;
				}
			}
		}
	}

	else if (path)
		Result.clear();

	return Result;
}

size_t GetMountPointLen(const string& abs_path, const string& drive_root)
{
	size_t n = drive_root.size();
	if (abs_path.size() >= n && 0 == StrCmpNI(abs_path.data(), drive_root.data(), n))
		return n;

	size_t dir_offset = 0;
	if (ParsePath(abs_path, &dir_offset) == PATH_VOLUMEGUID)
		return dir_offset;

	string vol_guid(drive_root);
	switch (ParsePath(drive_root))
	{
	case PATH_VOLUMEGUID:
		break;
	case PATH_DRIVELETTER:
		if (apiGetVolumeNameForVolumeMountPoint(drive_root, vol_guid))
			break;
		// else fall down to default:
	default:          //
		assert(false); // really it is incorrect
	return n;         //
	}

	string mount_point = TryConvertVolumeGuidToDrivePath(vol_guid, abs_path.data(), abs_path.size());
	assert(!mount_point.empty());
	if (!mount_point.empty())
		n = mount_point.size();

	return n;
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point.
  Note that Src can be partially non-existent.
*/
void ConvertNameToReal(const string& Src, string &strDest)
{
	DisableElevation de;
	// Получим сначала полный путь до объекта обычным способом
	string FullPath;
	ConvertNameToFull(Src, FullPath);
	strDest = FullPath;

	string Path = FullPath;
	HANDLE hFile;

	for (;;)
	{
		hFile = apiCreateFile(Path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0);

		if (hFile != INVALID_HANDLE_VALUE)
			break;

		if (IsRootPath(Path))
			break;

		Path = ExtractFilePath(Path);
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		string FinalFilePath;
		apiGetFinalPathNameByHandle(hFile, FinalFilePath);

		CloseHandle(hFile);

		//assert(!FinalFilePath.empty());

		if (!FinalFilePath.empty())
		{
			// append non-existent path part (if present)
			DeleteEndSlash(Path);

			if (FullPath.size() > Path.size() + 1)
			{
				AddEndSlash(FinalFilePath);
				FinalFilePath.append(FullPath.data() + Path.size() + 1, FullPath.size() - Path.size() - 1);
			}

			FinalFilePath = TryConvertVolumeGuidToDrivePath(FinalFilePath);
			strDest = FinalFilePath;
		}
	}
}

void ConvertNameToShort(const string& Src, string &strDest)
{
	WCHAR Buffer[MAX_PATH];
	DWORD Size = GetShortPathName(Src.data(), Buffer, ARRAYSIZE(Buffer));

	if(Size)
	{
		if (Size < ARRAYSIZE(Buffer))
		{
			strDest.assign(Buffer, Size);
		}
		else
		{
			wchar_t_ptr vBuffer(Size);
			Size = GetShortPathName(Src.data(), vBuffer.get(), Size);
			strDest.assign(vBuffer.get(), Size);
		}
	}
	else
	{
		strDest = Src;
	}
}

void ConvertNameToLong(const string& Src, string &strDest)
{
	WCHAR Buffer[MAX_PATH];
	DWORD nSize = GetLongPathName(Src.data(), Buffer, ARRAYSIZE(Buffer));

	if (nSize)
	{
		if (nSize < ARRAYSIZE(Buffer))
		{
			strDest.assign(Buffer, nSize);
		}
		else
		{
			wchar_t_ptr vBuffer(nSize);
			nSize = GetLongPathName(Src.data(), vBuffer.get(), nSize);
			strDest.assign(vBuffer.get(), nSize);
		}
	}
	else
	{
		strDest = Src;
	}
}

void ConvertNameToUNC(string &strFileName)
{
	ConvertNameToFull(strFileName,strFileName);
	// Посмотрим на тип файловой системы
	string strFileSystemName;
	string strTemp;
	GetPathRoot(strFileName,strTemp);

	if (!apiGetVolumeInformation(strTemp,nullptr,nullptr,nullptr,nullptr,&strFileSystemName))
		strFileSystemName.clear();

	DWORD uniSize = 1024;
	block_ptr<UNIVERSAL_NAME_INFO> uni(uniSize);

	// применяем WNetGetUniversalName для чего угодно, только не для Novell`а
	if (StrCmpI(strFileSystemName.data(),L"NWFS"))
	{
		DWORD dwRet=WNetGetUniversalName(strFileName.data(),UNIVERSAL_NAME_INFO_LEVEL,uni.get(),&uniSize);

		switch (dwRet)
		{
			case NO_ERROR:
				strFileName = uni->lpUniversalName;
				break;
			case ERROR_MORE_DATA:
				uni.reset(uniSize);

				if (WNetGetUniversalName(strFileName.data(),UNIVERSAL_NAME_INFO_LEVEL,uni.get(),&uniSize)==NO_ERROR)
					strFileName = uni->lpUniversalName;

				break;
		}
	}
	else if (strFileName.size() > 1 && strFileName[1] == L':')
	{
		// BugZ#449 - Неверная работа CtrlAltF с ресурсами Novell DS
		// Здесь, если не получилось получить UniversalName и если это
		// мапленный диск - получаем как для меню выбора дисков
		if (DriveLocalToRemoteName(DRIVE_UNKNOWN,strFileName[0],strTemp))
		{
			const wchar_t *NamePtr=FirstSlash(strFileName.data());

			if (NamePtr )
			{
				AddEndSlash(strTemp);
				strTemp += &NamePtr[1];
			}

			strFileName = strTemp;
		}
	}

	ConvertNameToReal(strFileName,strFileName);
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR
string& PrepareDiskPath(string &strPath, bool CheckFullPath)
{
	// elevation not required during cosmetic operation
	DisableElevation de;

	if (!strPath.empty())
	{
		if (strPath.size() > 1 && (strPath[1]==L':' || (strPath[0]==L'\\' && strPath[1]==L'\\')))
		{
			ReplaceSlashToBSlash(strPath);
			bool DoubleSlash = strPath[1]==L'\\';
			while(ReplaceStrings(strPath,L"\\\\",L"\\"));
			if(DoubleSlash)
			{
				strPath = L"\\" + strPath;
			}

			if (CheckFullPath)
			{
				ConvertNameToFull(strPath, strPath);

				size_t DirOffset = 0;
				PATH_TYPE Type = ParsePath(strPath, &DirOffset);
				if (Type == PATH_UNKNOWN && HasPathPrefix(strPath))
				{
					DirOffset = 4;
				}

				size_t StartPos = DirOffset + (IsSlash(strPath[DirOffset])? 1 : 0);

				if (StartPos < strPath.size())
				{
					string TmpStr;
					TmpStr.reserve(strPath.size());
					size_t LastPos = StartPos;
					bool EndsWithSlash = IsSlash(strPath.back());

					for (size_t i = StartPos; i <= strPath.size(); ++i)
					{
						if ((i < strPath.size() && IsSlash(strPath[i])) || (i == strPath.size() && !EndsWithSlash))
						{
							TmpStr = strPath.substr(0, i);
							FAR_FIND_DATA fd;

							if (apiGetFindDataEx(TmpStr, fd))
							{
								strPath.replace(LastPos, i - LastPos, fd.strFileName);
								i += fd.strFileName.size() - (i - LastPos);
							}

							if (i != strPath.size())
							{
								LastPos = i + 1;
							}
						}
					}
				}
			}

			if (ParsePath(strPath) == PATH_DRIVELETTER)
			{
				strPath[0] = Upper(strPath[0]);
			}
		}
	}

	return strPath;
}
