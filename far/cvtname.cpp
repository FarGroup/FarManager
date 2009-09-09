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

int ConvertNameToFull (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strSrc = lpwszSrc; //копирование в другую переменную на случай dest == src
	lpwszSrc = strSrc;

	// путь с префиксом - по определению полный.
	if(PathPrefix(lpwszSrc))
	{
		strDest=lpwszSrc;
		return (int)strDest.GetLength();
	}

	const wchar_t *lpwszName = PointToName(lpwszSrc);

	if ( (lpwszName == lpwszSrc) &&
				!TestParentFolderName(lpwszName) && !TestCurrentFolderName(lpwszName))
	{
		apiGetCurrentDirectory(strDest);
		AddEndSlash(strDest);

		strDest += lpwszSrc;

		return (int)strDest.GetLength ();
	}

	if ( PathMayBeAbsolute(lpwszSrc) )
	{
		if ( *lpwszName &&
				(*lpwszName != L'.' || (lpwszName[1] != 0 && (lpwszName[1] != L'.' || lpwszName[2] != 0)) ) &&
				(wcsstr (lpwszSrc, L"\\..\\") == NULL && wcsstr (lpwszSrc, L"\\.\\") == NULL) )
		{
			strDest = lpwszSrc;

			return (int)strDest.GetLength ();
		}
	}
	return (int)apiGetFullPathName(lpwszSrc,strDest);
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point.
  Note that Src can be partially non-existent.
*/
bool ConvertNameToReal(const wchar_t *Src, string &strDest)
{
  bool Result = false;
  // Получим сначала полный путь до объекта обычным способом
  string FullPath;
  ConvertNameToFull(Src, FullPath);
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog(L"ConvertNameToFull('%s') -> '%s'",Src,(const wchar_t*)strTempDest));

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
        size_t BufSize = 0x10000;
        OBJECT_NAME_INFORMATION* oni = reinterpret_cast<OBJECT_NAME_INFORMATION*>(xf_malloc(BufSize));
        NTSTATUS res = ifn.pfnNtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);
        if (res == STATUS_BUFFER_OVERFLOW || res == STATUS_BUFFER_TOO_SMALL)
        {
          BufSize = RetLen;
          oni = reinterpret_cast<OBJECT_NAME_INFORMATION*>(xf_realloc_nomove(oni, BufSize));
          res = ifn.pfnNtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);
        }
        if (res == STATUS_SUCCESS)
        {
          FinalFilePath.Copy(oni->Name.Buffer, oni->Name.Length / sizeof(WCHAR));
        }
        xf_free(oni);
        wchar_t VolumeName[MAX_PATH];
        HANDLE hEnum = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));
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
              if (FinalFilePath.Equal(0, TargetPath))
              {
                FinalFilePath.Replace(0, TargetPath.GetLength(), VolumeName);
                break;
              }
            }
          }
          Res = FindNextVolumeW(hEnum, VolumeName, ARRAYSIZE(VolumeName));
        }
        if (hEnum != INVALID_HANDLE_VALUE)
          FindVolumeClose(hEnum);
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
        size_t BufSize = MAX_PATH;
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
            if (GetVolumeNameForVolumeMountPointW(Drive, VolumeGuid, ARRAYSIZE(VolumeGuid)))
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
        Result = true;
      }
    }
  }

  OutputDebugStringW(strDest.CPtr());
  return Result;
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
