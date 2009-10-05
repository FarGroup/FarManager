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

PATH_PFX_TYPE Point2Root (LPCWSTR stPath, size_t& PathOffset)
{
	if (stPath)
	{
		PATH_PFX_TYPE nPrefix = PPT_NONE;
		LPCWSTR pstPath=stPath;

		//Skip root entry, network share, device, nt notation or symlink prefix: "\", "\\", "\\.\", "\\?\", "\??\"
		//prefix "\" or "/"
		if (IsSlash (*pstPath))
		{
			pstPath++;
			nPrefix = PPT_ROOT;

			//prefix "\"
			if (IsSlashBackward (pstPath[-1]))
			{
				//prefix "\\" - network
				if (IsSlashBackward (pstPath[0]))
				{
					pstPath++;
					nPrefix = PPT_PREFIX;

					//prefix "\\.\" - device
					if (IsDot (pstPath[0]) && IsSlashBackward (pstPath[1]))
					{
						pstPath += 2;
					}
					else
					{
						//prefix "\\?\" - nt notation
						if (IsQuestion (pstPath[0]) && IsSlashBackward (pstPath[1]))
						{
							pstPath += 2;
							nPrefix = PPT_NT;

							//prefix "\\?\UNC\" - nt notation (UNC)
							if (!StrCmpN (pstPath, L"UNC\\", 4))
							{
								pstPath += 4;
							}
						}
					}
				}
				else
				{
					if (IsQuestion (pstPath[0]) && IsQuestion (pstPath[1]) && IsSlashBackward (pstPath[2])) //prefix "\??\" symlink
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
			do
			{
				if (IsSlash (*pstPath))
				{
					pstPath++;
					break;
				}
			} while (*pstPath++);
		}
		else
		{
			//Skip logical drive letter name
			if (pstPath[0] && IsColon (pstPath[1]))
			{
				pstPath += 2;
				nPrefix = PPT_DRIVE;

				//Skip root slash
				if (IsSlash (*pstPath))
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

void MixToFullPath (string& strPath)
{
	//Skip all path to root (with slash if exists)
	LPWSTR pstPath=strPath.GetBuffer();
	size_t PathOffset=0;
	Point2Root (strPath,PathOffset);
	pstPath+=PathOffset;

	//Process "." and ".." if exists
	for (int m = 0; pstPath[m];)
	{
		//fragment "."
		if (IsDot (pstPath[m]) && (m == 0 || IsSlash (pstPath[m - 1])))
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
					if (IsSlash (pstPath[m + 2]) || !pstPath[m + 2])
					{
						int n;
						//Calculate subdir name offset
						for (n = m - 2; (n >= 0) && (!IsSlash (pstPath[n])); n--);
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

bool MixToFullPath (LPCWSTR stPath, string& strDest, LPCWSTR stCurrentDir)
{
	size_t lPath=wcslen(NullToEmpty(stPath)),
		lCurrentDir=wcslen(NullToEmpty(stCurrentDir)),
		lFullPath=lPath+lCurrentDir;

	if(lFullPath > 0)
	{
		strDest.SetLength(0);
		LPCWSTR pstPath = NULL, pstCurrentDir = NULL;
		bool blIgnore = false;
		size_t PathOffset=0;
		PATH_PFX_TYPE PathType=Point2Root(stPath,PathOffset);
		pstPath=stPath+PathOffset;
		switch(PathType)
		{
		case PPT_NONE:
			{
				pstCurrentDir=stCurrentDir;
			}
			break;
		case PPT_DRIVE:
			{
				WCHAR DriveVar[]={L'=',*stPath,L':',L'\0'};
				string strValue;
				if(apiGetEnvironmentVariable(DriveVar,strValue))
				{
					strDest=strValue;
					AddEndSlash(strDest);
				}
				else
				{
					pstCurrentDir=stCurrentDir;
				}
			}
			break;
		case PPT_ROOT:
			{
				if (stCurrentDir)
				{
					size_t PathOffset=0;
					if(Point2Root(stCurrentDir,PathOffset)!=PPT_NONE)
					{
						strDest=string(stCurrentDir,PathOffset);
					}
				}
			}
			break;
		case PPT_PREFIX:
			{
				pstPath=stPath;
			}
			break;
		case PPT_NT:
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
			MixToFullPath (strDest);
		return true;
	}
	return false;
}

void ConvertNameToFull (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strCurDir;
	apiGetCurrentDirectory(strCurDir);
	string strSrc = lpwszSrc;
	MixToFullPath(strSrc,strDest,strCurDir);
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
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));

  /* $ 14.06.2003 IS
     Для нелокальных дисков даже и не пытаемся анализировать симлинки
  */
  // также ничего не делаем для нелокальных дисков, т.к. для них невозможно узнать
  // корректную информацию про объект, на который указывает симлинк (т.е. невозможно
  // "разыменовать симлинк")
  if (IsLocalDrive(FullPath))
  {
    const int cVolumeGuidLen = 49;

    string Path = FullPath;
    HANDLE hFile;
    while (true)
    {
      hFile = apiCreateFile(Path.CPtr(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0);
      if (hFile != INVALID_HANDLE_VALUE)
        break;
      if (IsRootPath(Path))
        break;
      Path = ExtractFilePath(Path);
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
      string FinalFilePath;
      if (ifn.pfnGetFinalPathNameByHandle)
      {
        DWORD BufSize = 0x10000;
        DWORD Len = ifn.pfnGetFinalPathNameByHandle(hFile, FinalFilePath.GetBuffer(BufSize), BufSize, VOLUME_NAME_GUID);
        if (Len > BufSize + 1)
        {
          BufSize = Len - 1;
          Len = ifn.pfnGetFinalPathNameByHandle(hFile, FinalFilePath.GetBuffer(BufSize), BufSize, VOLUME_NAME_GUID);
        }
        FinalFilePath.ReleaseBuffer(Len);
      }
      else if (ifn.pfnNtQueryObject)
      {
        ULONG RetLen;
        ULONG BufSize = 0x10000;
        OBJECT_NAME_INFORMATION* oni = reinterpret_cast<OBJECT_NAME_INFORMATION*>(xf_malloc(BufSize));
        NTSTATUS Res = ifn.pfnNtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);
        if (Res == STATUS_BUFFER_OVERFLOW || Res == STATUS_BUFFER_TOO_SMALL)
        {
          BufSize = RetLen;
          oni = reinterpret_cast<OBJECT_NAME_INFORMATION*>(xf_realloc_nomove(oni, BufSize));
          Res = ifn.pfnNtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);
        }
        if (Res == STATUS_SUCCESS)
        {
          FinalFilePath.Copy(oni->Name.Buffer, oni->Name.Length / sizeof(WCHAR));
        }
        xf_free(oni);
        if (Res == STATUS_SUCCESS)
        {
          // need to convert NT path (\Device\HarddiskVolume1) to \\?\Volume{...} path
          wchar_t VolumeName[MAX_PATH];
          HANDLE hEnum = FindFirstVolumeW(VolumeName, countof(VolumeName));
          BOOL Res = hEnum != INVALID_HANDLE_VALUE;
          while (Res)
          {
            if (StrLength(VolumeName) >= cVolumeGuidLen)
            {
              VolumeName[cVolumeGuidLen - 1] = 0; // drop trailing slash
              string TargetPath;
              DWORD Res = QueryDosDeviceW(VolumeName + 4 /* w/o prefix */, TargetPath.GetBuffer(MAX_PATH), MAX_PATH);
              if (Res)
              {
                TargetPath.ReleaseBuffer();
                // path could be an Object Manager symlink, try to resolve
                UNICODE_STRING ObjName;
                ObjName.Length = ObjName.MaximumLength = TargetPath.GetLength() * sizeof(wchar_t);
                ObjName.Buffer = const_cast<PWSTR>(TargetPath.CPtr());
                OBJECT_ATTRIBUTES ObjAttrs;
                InitializeObjectAttributes(&ObjAttrs, &ObjName, 0, NULL, NULL);
                HANDLE hSymLink;
                NTSTATUS Res = ifn.pfnNtOpenSymbolicLinkObject(&hSymLink, GENERIC_READ, &ObjAttrs);
                if (Res == STATUS_SUCCESS)
                {
                  ULONG BufSize = 0x7FFF;
                  string Buffer;
                  UNICODE_STRING LinkTarget;
                  LinkTarget.MaximumLength = static_cast<USHORT>(BufSize * sizeof(wchar_t));
                  LinkTarget.Buffer = Buffer.GetBuffer(BufSize);
                  Res = ifn.pfnNtQuerySymbolicLinkObject(hSymLink, &LinkTarget, NULL);
                  if (Res == STATUS_SUCCESS)
                  {
                    TargetPath.Copy(LinkTarget.Buffer, LinkTarget.Length / sizeof(wchar_t));
                  }
                  ifn.pfnNtClose(hSymLink);
                }
                if (PathStartsWith(FinalFilePath, TargetPath))
                {
                  FinalFilePath.Replace(0, TargetPath.GetLength(), VolumeName);
                  break;
                }
              }
            }
            Res = FindNextVolumeW(hEnum, VolumeName, countof(VolumeName));
          }
          if (hEnum != INVALID_HANDLE_VALUE)
            FindVolumeClose(hEnum);
        }
      }
      CloseHandle(hFile);
      if (!FinalFilePath.IsEmpty())
      {
        // append non-existent path part (if present)
        DeleteEndSlash(Path);
        if (FullPath.GetLength() > Path.GetLength() + 1)
        {
          AddEndSlash(FinalFilePath);
          FinalFilePath.Append(FullPath.CPtr() + Path.GetLength() + 1, FullPath.GetLength() - Path.GetLength() - 1);
        }
        // try to replace volume GUID with drive letter
        string DriveStringsBuf;
        DWORD BufSize = MAX_PATH;
        wchar_t* DriveStrings = DriveStringsBuf.GetBuffer(BufSize);
        DWORD Size = GetLogicalDriveStringsW(BufSize, DriveStrings);
        if (Size > BufSize)
        {
          BufSize = Size;
          DriveStrings = DriveStringsBuf.GetBuffer(BufSize);
          Size = GetLogicalDriveStringsW(BufSize, DriveStrings);
        }
        if (Size)
        {
          wchar_t* Drive = DriveStrings;
          wchar_t VolumeGuid[cVolumeGuidLen + 1];
          while (*Drive)
          {
            if (GetVolumeNameForVolumeMountPointW(Drive, VolumeGuid, countof(VolumeGuid)))
            {
              if (FinalFilePath.Equal(0, VolumeGuid, cVolumeGuidLen))
              {
                FinalFilePath.Replace(0, cVolumeGuidLen, Drive);
                break;
              }
            }
            Drive += StrLength(Drive) + 1;
          }
        }

        strDest = FinalFilePath;
      }
    }
  }
}

void ConvertNameToShort(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;

	int nSize = GetShortPathName(strCopy, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (nSize);

		GetShortPathName(strCopy, lpwszDest, nSize);

		strDest.ReleaseBuffer ();
	}
	else
		strDest = strCopy;

	strDest.Upper ();
}

void ConvertNameToLong(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;

	int nSize = GetLongPathName(strCopy, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (nSize);

		GetLongPathName(strCopy, lpwszDest, nSize);

		strDest.ReleaseBuffer ();
	}
	else
		strDest = strCopy;
}

void ConvertNameToUNC(string &strFileName)
{
	ConvertNameToFull(strFileName,strFileName);
	// Посмотрим на тип файловой системы
	string strFileSystemName;
	string strTemp;
	GetPathRoot(strFileName,strTemp);

	if(!apiGetVolumeInformation (strTemp,NULL,NULL,NULL,NULL,&strFileSystemName))
		strFileSystemName=L"";

	DWORD uniSize = 1024;
	UNIVERSAL_NAME_INFO *uni=(UNIVERSAL_NAME_INFO*)xf_malloc(uniSize);

	// применяем WNetGetUniversalName для чего угодно, только не для Novell`а
	if (StrCmpI(strFileSystemName,L"NWFS"))
	{
		DWORD dwRet=WNetGetUniversalName(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize);
		switch(dwRet)
		{
		case NO_ERROR:
			strFileName = uni->lpUniversalName;
			break;
		case ERROR_MORE_DATA:
			uni=(UNIVERSAL_NAME_INFOW*)xf_realloc(uni,uniSize);
			if(WNetGetUniversalName(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize)==NO_ERROR)
				strFileName = uni->lpUniversalName;
			break;
		}
	}
	else if(strFileName.At(1) == L':')
	{
		// BugZ#449 - Неверная работа CtrlAltF с ресурсами Novell DS
		// Здесь, если не получилось получить UniversalName и если это
		// мапленный диск - получаем как для меню выбора дисков

		if(!DriveLocalToRemoteName(DRIVE_UNKNOWN,strFileName.At(0),strTemp).IsEmpty())
		{
			const wchar_t *NamePtr=FirstSlash(strFileName);
			if(NamePtr != NULL)
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
string& PrepareDiskPath(string &strPath,BOOL CheckFullPath)
{
	if( !strPath.IsEmpty() )
	{
		if(((IsAlpha(strPath.At(0)) && strPath.At(1)==L':') || (strPath.At(0)==L'\\' && strPath.At(1)==L'\\')))
		{
			if(CheckFullPath)
			{
				ConvertNameToFull(strPath,strPath);
				wchar_t *lpwszPath=strPath.GetBuffer(),*Src=lpwszPath,*Dst=lpwszPath;
				if(IsLocalPath(lpwszPath))
				{
					Src+=2;
					if(IsSlash(*Src))
						Src++;
					Dst+=2;
					if(IsSlash(*Dst))
						Dst++;
				}
				if(*Src)
				{
					for(wchar_t c=*Src;;Src++,c=*Src)
					{
						if (!c||IsSlash(c))
						{
							*Src=0;
							FAR_FIND_DATA_EX fd;
							BOOL find=apiGetFindDataEx(lpwszPath,&fd,false);
							*Src=c;
							if(find)
							{
								size_t n=fd.strFileName.GetLength();
								size_t n1 = n-(Src-Dst);
								if((int)n1>0)
								{
									size_t dSrc=Src-lpwszPath,dDst=Dst-lpwszPath;
									strPath.ReleaseBuffer();
									lpwszPath=strPath.GetBuffer(int(strPath.GetLength()+n1));
									Src=lpwszPath+dSrc;
									Dst=lpwszPath+dDst;
									wmemmove(Src+n1,Src,StrLength(Src)+1);
									Src+=n1;
								}
								wcsncpy(Dst,fd.strFileName,n);
								Dst+=n;
								wcscpy(Dst,Src);
								if(c)
								{
									Dst++;
									Src=Dst;
								}
							}
							else
							{
								if(c)
								{
									Src++;
									Dst=Src;
								}
							}
						}
						if(!*Src)
							break;
					}
				}
				strPath.ReleaseBuffer();
			}

			wchar_t *lpwszPath = strPath.GetBuffer ();

			if (lpwszPath[0]==L'\\' && lpwszPath[1]==L'\\')
			{
				if(IsLocalPrefixPath(lpwszPath))
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

			strPath.ReleaseBuffer ();
		}
	}
	return strPath;
}
