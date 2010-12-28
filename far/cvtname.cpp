/*
cvtname.cpp

Функций для преобразования имен файлов/путей.
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

enum PATH_PFX_TYPE
{
	PPT_NONE,
	PPT_DRIVE,
	PPT_ROOT,
	PPT_PREFIX,
	PPT_NT
};

PATH_PFX_TYPE Point2Root(LPCWSTR stPath, size_t& PathOffset)
{
	if (stPath)
	{
		PATH_PFX_TYPE nPrefix = PPT_NONE;
		LPCWSTR pstPath=stPath;

		//Skip root entry, network share, device, nt notation or symlink prefix: "\", "\\", "\\.\", "\\?\", "\??\"
		//prefix "\" or "/"
		if (IsSlash(*pstPath))
		{
			pstPath++;
			nPrefix = PPT_ROOT;

			//prefix "\"
			if (IsSlashBackward(pstPath[-1]))
			{
				//prefix "\\" - network
				if (IsSlashBackward(pstPath[0]))
				{
					pstPath++;
					nPrefix = PPT_PREFIX;

					//prefix "\\.\" - device
					if (IsDot(pstPath[0]) && IsSlashBackward(pstPath[1]))
					{
						pstPath += 2;
					}
					else
					{
						//prefix "\\?\" - nt notation
						if (IsQuestion(pstPath[0]) && IsSlashBackward(pstPath[1]))
						{
							pstPath += 2;
							nPrefix = PPT_NT;

							//prefix "\\?\UNC\" - nt notation (UNC)
							if (!StrCmpN(pstPath, L"UNC\\", 4))
							{
								pstPath += 4;
							}
						}
					}
				}
				else
				{
					if (IsQuestion(pstPath[0]) && IsQuestion(pstPath[1]) && IsSlashBackward(pstPath[2]))    //prefix "\??\" symlink
					{
						pstPath += 3;
						nPrefix = PPT_NT;
					};
				}
			}
		}

		//Skip path to next slash (or path end) if was "special" prefix
		if (nPrefix == PPT_PREFIX || nPrefix == PPT_NT)
		{
			while (*pstPath)
			{
				if (IsSlash(*pstPath))
				{
					pstPath++;
					break;
				}

				pstPath++;
			}
		}
		else
		{
			//Skip logical drive letter name
			if (pstPath[0] && IsColon(pstPath[1]))
			{
				pstPath += 2;
				nPrefix = PPT_DRIVE;

				//Skip root slash
				if (IsSlash(*pstPath))
				{
					pstPath++;
					nPrefix = PPT_PREFIX;
				}
			}
		}

		PathOffset=pstPath-stPath;
		return (nPrefix);
	}

	return (PPT_NONE);
}

void MixToFullPath(string& strPath)
{
	//Skip all path to root (with slash if exists)
	LPWSTR pstPath=strPath.GetBuffer();
	size_t PathOffset=0;
	Point2Root(pstPath,PathOffset);
	pstPath+=PathOffset;

	//Process "." and ".." if exists
	for (int m = 0; pstPath[m];)
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
					continue;
				}
				break;
				//fragment "..\" or "../" or ".." at the end
				case L'.':
				{
					if (IsSlash(pstPath[m + 2]) || !pstPath[m + 2])
					{
						int n;

						//Calculate subdir name offset
						for (n = m - 2; (n >= 0) && (!IsSlash(pstPath[n])); n--);

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

	strPath.ReleaseBuffer();
}

bool MixToFullPath(LPCWSTR stPath, string& strDest, LPCWSTR stCurrentDir)
{
	size_t lPath=wcslen(NullToEmpty(stPath)),
	       lCurrentDir=wcslen(NullToEmpty(stCurrentDir)),
	       lFullPath=lPath+lCurrentDir;

	if (lFullPath > 0)
	{
		strDest.Clear();
		LPCWSTR pstPath = nullptr, pstCurrentDir = nullptr;
		bool blIgnore = false;
		size_t PathOffset=0;
		PATH_PFX_TYPE PathType=Point2Root(stPath,PathOffset);
		pstPath=stPath+PathOffset;

		switch (PathType)
		{
			case PPT_NONE: //"abc"
			{
				pstCurrentDir=stCurrentDir;
			}
			break;
			case PPT_DRIVE: //"C:" or "C:abc"
			{
				WCHAR DriveVar[]={L'=',*stPath,L':',L'\0'};
				string strValue;

				if (apiGetEnvironmentVariable(DriveVar,strValue))
				{
					strDest=strValue;
				}
				else
				{
					if (Upper(*stPath)==Upper(*stCurrentDir))
					{
						strDest=stCurrentDir;
					}
					else
					{
						strDest=DriveVar+1;
					}
				}

				AddEndSlash(strDest);
			}
			break;
			case PPT_ROOT: //"\" or "\abc"
			{
				if (stCurrentDir)
				{
					size_t PathOffset=0;

					if (Point2Root(stCurrentDir,PathOffset)!=PPT_NONE)
					{
						strDest=string(stCurrentDir,PathOffset);
					}
				}
			}
			break;
			case PPT_PREFIX: //"C:\abc"
			{
				pstPath=stPath;
			}
			break;
			case PPT_NT: //"\\?\abc"
			{
				blIgnore=true;
				pstPath=stPath;
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

		if (!blIgnore)
			MixToFullPath(strDest);

		return true;
	}

	return false;
}

void ConvertNameToFull(const wchar_t *lpwszSrc, string &strDest)
{
	string strCurDir;
	apiGetCurrentDirectory(strCurDir);
	string strSrc = lpwszSrc;
	MixToFullPath(strSrc,strDest,strCurDir);
}

// try to replace volume GUID (if present) with drive letter
// used by ConvertNameToReal() only
string TryConvertVolumeGuidToDrivePath(const string& Path)
{
	string Result = Path;

	if (Path.GetLength() >= cVolumeGuidLen && Path.Equal(0, L"\\\\?\\Volume"))
	{
		if (ifn.pfnGetVolumePathNamesForVolumeName)
		{
			DWORD BufSize = NT_MAX_PATH;
			string PathNames;
			DWORD RetSize;
			BOOL Res = ifn.pfnGetVolumePathNamesForVolumeName(ExtractPathRoot(Path), PathNames.GetBuffer(BufSize), BufSize, &RetSize);

			if (!Res && RetSize > BufSize)
			{
				BufSize = RetSize;
				Res = ifn.pfnGetVolumePathNamesForVolumeName(ExtractPathRoot(Path), PathNames.GetBuffer(BufSize), BufSize, &RetSize);
			}

			if (Res)
			{
				const wchar_t* PathName = PathNames.GetBuffer();

				while (*PathName)
				{
					string strPath(PathName);

					if (IsRootPath(strPath))
					{
						DeleteEndSlash(strPath);
						Result.Replace(0, cVolumeGuidLen, strPath);
						break;
					}

					PathName += strPath.GetLength() + 1;
				}
			}
		}
		else
		{
			string DriveStrings;

			if (apiGetLogicalDriveStrings(DriveStrings))
			{
				wchar_t* Drive = DriveStrings.GetBuffer();
				string strVolumeGuid;

				while (*Drive)
				{
					if (apiGetVolumeNameForVolumeMountPoint(Drive,strVolumeGuid))
					{
						if (Path.Equal(0, strVolumeGuid, cVolumeGuidLen))
						{
							DeleteEndSlash(Drive);
							Result.Replace(0, cVolumeGuidLen, Drive);
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
void ConvertNameToReal(const wchar_t *Src, string &strDest)
{
	// Получим сначала полный путь до объекта обычным способом
	string FullPath;
	ConvertNameToFull(Src, FullPath);
	strDest = FullPath;

	string Path = FullPath;
	HANDLE hFile;

	for (;;)
	{
		hFile = apiCreateFile(Path.CPtr(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0);

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

		//assert(!FinalFilePath.IsEmpty());

		if (!FinalFilePath.IsEmpty())
		{
			// append non-existent path part (if present)
			DeleteEndSlash(Path);

			if (FullPath.GetLength() > Path.GetLength() + 1)
			{
				AddEndSlash(FinalFilePath);
				FinalFilePath.Append(FullPath.CPtr() + Path.GetLength() + 1, FullPath.GetLength() - Path.GetLength() - 1);
			}

			FinalFilePath = TryConvertVolumeGuidToDrivePath(FinalFilePath);
			strDest = FinalFilePath;
		}
	}
}

void ConvertNameToShort(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;
	int nSize = GetShortPathName(strCopy, nullptr, 0);

	if (nSize)
	{
		wchar_t *lpwszDest = strDest.GetBuffer(nSize);
		GetShortPathName(strCopy, lpwszDest, nSize);
		strDest.ReleaseBuffer();
	}
	else
	{
		strDest = strCopy;
	}

	strDest.Upper();
}

void ConvertNameToLong(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;
	WCHAR Buffer[MAX_PATH];
	DWORD nSize = GetLongPathName(strCopy, Buffer, ARRAYSIZE(Buffer));

	if (nSize)
	{
		if (nSize>ARRAYSIZE(Buffer))
		{
			wchar_t *lpwszDest = strDest.GetBuffer(nSize);
			GetLongPathName(strCopy, lpwszDest, nSize);
			strDest.ReleaseBuffer();
		}
		else
		{
			strDest = Buffer;
		}
	}
	else
	{
		strDest = strCopy;
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
		strFileSystemName.Clear();

	DWORD uniSize = 1024;
	UNIVERSAL_NAME_INFO *uni=(UNIVERSAL_NAME_INFO*)xf_malloc(uniSize);

	// применяем WNetGetUniversalName для чего угодно, только не для Novell`а
	if (StrCmpI(strFileSystemName,L"NWFS"))
	{
		DWORD dwRet=WNetGetUniversalName(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize);

		switch (dwRet)
		{
			case NO_ERROR:
				strFileName = uni->lpUniversalName;
				break;
			case ERROR_MORE_DATA:
				uni=(UNIVERSAL_NAME_INFOW*)xf_realloc(uni,uniSize);

				if (WNetGetUniversalName(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize)==NO_ERROR)
					strFileName = uni->lpUniversalName;

				break;
		}
	}
	else if (strFileName.At(1) == L':')
	{
		// BugZ#449 - Неверная работа CtrlAltF с ресурсами Novell DS
		// Здесь, если не получилось получить UniversalName и если это
		// мапленный диск - получаем как для меню выбора дисков
		if (!DriveLocalToRemoteName(DRIVE_UNKNOWN,strFileName.At(0),strTemp).IsEmpty())
		{
			const wchar_t *NamePtr=FirstSlash(strFileName);

			if (NamePtr )
			{
				AddEndSlash(strTemp);
				strTemp += &NamePtr[1];
			}

			strFileName = strTemp;
		}
	}

	xf_free(uni);
	ConvertNameToReal(strFileName,strFileName);
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR
string& PrepareDiskPath(string &strPath, bool CheckFullPath)
{
	// elevation not required during cosmetic operation 
	DisableElevation de; 

	if (!strPath.IsEmpty())
	{
		if (strPath.At(1)==L':' || (strPath.At(0)==L'\\' && strPath.At(1)==L'\\'))
		{
			ReplaceSlashToBSlash(strPath);
			bool DoubleSlash = strPath.At(1)==L'\\';
			while(ReplaceStrings(strPath,L"\\\\",L"\\"));
			if(DoubleSlash)
			{
				strPath = "\\"+strPath;
			}

			if (CheckFullPath)
			{
				ConvertNameToFull(strPath,strPath);
				size_t FullLen=strPath.GetLength();
				wchar_t *lpwszPath=strPath.GetBuffer(),*Src=lpwszPath;

				if (IsLocalPath(lpwszPath))
				{
					Src+=2;

					if (IsSlash(*Src))
						Src++;
				}

				if (*Src)
				{
					wchar_t *Dst=Src;

					for (wchar_t c=*Src; ; Src++,c=*Src)
					{
						if (IsSlash(c) || (!c && !IsSlash(*(Src-1))))
						{
							*Src=0;
							FAR_FIND_DATA_EX fd;
							BOOL find=apiGetFindDataEx(lpwszPath,fd);
							*Src=c;

							if (find)
							{
								size_t n=fd.strFileName.GetLength();
								int n1=(int)(n-(Src-Dst));

								if (n1>0)
								{
									size_t dSrc=Src-lpwszPath,dDst=Dst-lpwszPath;
									strPath.ReleaseBuffer(FullLen);
									lpwszPath=strPath.GetBuffer(FullLen+n1+1);
									Src=lpwszPath+dSrc;
									Dst=lpwszPath+dDst;
								}

								if (n1)
								{
									wmemmove(Src+n1,Src,FullLen-(Src-lpwszPath)+1);
									Src+=n1;
									FullLen+=n1;
								}

								wmemcpy(Dst,fd.strFileName,n);
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

				strPath.ReleaseBuffer(FullLen);
			}

			wchar_t *lpwszPath = strPath.GetBuffer();

			if (lpwszPath[0]==L'\\' && lpwszPath[1]==L'\\')
			{
				if (IsLocalPrefixPath(lpwszPath))
				{
					lpwszPath[4] = Upper(lpwszPath[4]);
				}
				else
				{
					wchar_t *ptr=&lpwszPath[2];

					while (*ptr && !IsSlash(*ptr))
					{
						*ptr=Upper(*ptr);
						ptr++;
					}
				}
			}
			else
			{
				lpwszPath[0]=Upper(lpwszPath[0]);
			}

			strPath.ReleaseBuffer(strPath.GetLength());
		}
	}

	return strPath;
}
