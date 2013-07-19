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
	LPWSTR pstPath = GetStringBuffer(strPath);
	size_t DirOffset = 0;
	ParsePath(strPath, &DirOffset);
	pstPath+=DirOffset;
	bool ok = true;

	//Process "." and ".." if exists
	for (size_t m = 0; pstPath[m];)
	{
		//fragment "."
		if (IsDot(pstPath[m]) && (!m || IsSlash(pstPath[m - 1])))
		{
			LPCWSTR pstSrc;
			LPWSTR pstDst;

			switch (pstPath[m + 1])
			{
					//fragment ".\"
				case L'\\':
					//fragment "./"
				case L'/':
				{
					for (pstSrc = pstPath + m + 2, pstDst = pstPath + m; *pstSrc; pstSrc++, pstDst++)
					{
						*pstDst = *pstSrc;
					}

					*pstDst = 0;
					continue;
				}
				break;
				//fragment "." at the end
				case 0:
				{
					pstPath[m] = 0;
					// don't change x:\ to x:
					if (pstPath[m-2] != L':')
					{
						pstPath[m-1] = 0;
					}
					continue;
				}
				break;
				//fragment "..\" or "../" or ".." at the end
				case L'.':
				{
					if (IsSlash(pstPath[m + 2]) || !pstPath[m + 2])
					{
						if (!m) // ".." on the top level
							ok = false;

						int n;

						//Calculate subdir name offset
						for (n = static_cast<int>(m - 2); (n >= 0) && (!IsSlash(pstPath[n])); n--);

						n = (n < 0) ? 0 : n + 1;

						//fragment "..\" or "../"
						if (pstPath[m + 2])
						{
							for (pstSrc = pstPath + m + 3, pstDst = pstPath + n; *pstSrc; pstSrc++, pstDst++)
							{
								*pstDst = *pstSrc;
							}

							*pstDst = 0;
						}
						//fragment ".." at the end
						else
						{
							pstPath[n] = 0;
						}

						m = n;
						continue;
					}
				}
				break;
			}
		}

		m++;
	}

	ReleaseStringBuffer(strPath);
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
				if(IsSlash(stPath[0]) && !IsSlash(stPath[1])) //"\" or "\abc"
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
string TryConvertVolumeGuidToDrivePath(const string& Path)
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

					if (IsRootPath(strPath))
					{
						Result.replace(0, DirectoryOffset + 1, strPath);
						break;
					}

					PathName += strPath.size() + 1;
				}
			}
		}
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

	return Result;
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
	else if (strFileName[1] == L':')
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
		if (strPath[1]==L':' || (strPath[0]==L'\\' && strPath[1]==L'\\'))
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
				ConvertNameToFull(strPath,strPath);
				size_t FullLen=strPath.size();
				wchar_t *lpwszPath = GetStringBuffer(strPath), *Src=lpwszPath;

				size_t DirOffset;
				PATH_TYPE Type = ParsePath(strPath, &DirOffset);
				if (Type != PATH_UNKNOWN)
				{
					Src = const_cast<wchar_t*>(strPath.data() + DirOffset);
					if (IsSlash(*Src))
						Src++;
				}
				else
				{
					if(HasPathPrefix(lpwszPath)) // \\?\<ANY_UNKNOWN_FORMAT>
					{
						Src = lpwszPath + 4;
					}
				}

				if (*Src)
				{
					wchar_t *Dst=Src;

					for (wchar_t c=*Src; ; Src++,c=*Src)
					{
						if (IsSlash(c) || (!c && !IsSlash(*(Src-1))))
						{
							*Src=0;
							FAR_FIND_DATA fd;
							bool find=apiGetFindDataEx(lpwszPath,fd);
							*Src=c;

							if (find)
							{
								size_t n=fd.strFileName.size();
								int n1=(int)(n-(Src-Dst));

								if (n1>0)
								{
									size_t dSrc=Src-lpwszPath,dDst=Dst-lpwszPath;
									ReleaseStringBuffer(strPath, FullLen);
									lpwszPath = GetStringBuffer(strPath, FullLen + n1 + 1);
									Src=lpwszPath+dSrc;
									Dst=lpwszPath+dDst;
								}

								if (n1)
								{
									wmemmove(Src+n1,Src,FullLen-(Src-lpwszPath)+1);
									Src+=n1;
									FullLen+=n1;
								}

								wmemcpy(Dst,fd.strFileName.data(),n);
							}

							if (c)
							{
								Dst=Src+1;
							}
						}

						if (!*Src)
							break;
					}
				}

				ReleaseStringBuffer(strPath, FullLen);
			}

			if (ParsePath(strPath) == PATH_DRIVELETTER)
			{
				strPath[0] = Upper(strPath[0]);
			}
		}
	}

	return strPath;
}
