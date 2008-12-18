/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций
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

#include "global.hpp"
#include "fn.hpp"
#include "imports.hpp"
#include "flink.hpp"

BOOL apiDeleteFile (const wchar_t *lpwszFileName)
{
	// IS: сначала попробуем удалить стандартной функцией, чтобы
	// IS: не осуществлять лишние телодвижения

	BOOL rc = DeleteFileW (lpwszFileName);

	if ( !rc ) // IS: вот тут лишние телодвижения и начнем...
	{
		SetLastError((_localLastError = GetLastError()));

		if ( CheckErrorForProcessed(_localLastError) )
		{
			string strFullName;

			ConvertNameToFull (lpwszFileName, strFullName);

			strFullName = L"\\\\?\\"+strFullName;

			if( (strFullName.At(4)==L'/' && strFullName.At(5)==L'/') ||
				(strFullName.At(4)==L'\\' && strFullName.At(5)==L'\\') )
				rc = DeleteFileW((const wchar_t*)strFullName+4);
			else
				rc = DeleteFileW(strFullName);
		}
	}

	return rc;
}


BOOL apiRemoveDirectory (const wchar_t *DirName)
{
	BOOL rc = RemoveDirectoryW (DirName);

  //BUGBUG
  /*
  if(!rc) // IS: вот тут лишние телодвижения и начнем...
  {
    SetLastError((_localLastError = GetLastError()));
    if(CheckErrorForProcessed(_localLastError))
    {
      char FullName[NM+16]="\\\\?\\";
      // IS: +4 - чтобы не затереть наши "\\?\"
      if(ConvertNameToFull(DirName, FullName+4, sizeof(FullName)-4) < sizeof(FullName)-4)
      {
        // IS: проверим, а вдруг уже есть есть нужные символы в пути
        if( (FullName[4]=='/' && FullName[5]=='/') ||
            (FullName[4]=='\\' && FullName[5]=='\\') )
          rc=RemoveDirectory(FullName+4);
        // IS: нужных символов в пути нет, поэтому используем наши
        else
          rc=RemoveDirectory(FullName);
      }
    }
  }
  */
	return rc;
}


HANDLE apiCreateFile (
		const wchar_t *lpwszFileName,     // pointer to name of the file
		DWORD dwDesiredAccess,  // access (read-write) mode
		DWORD dwShareMode,      // share mode
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
		DWORD dwCreationDistribution, // how to create
		DWORD dwFlagsAndAttributes,   // file attributes
		HANDLE hTemplateFile          // handle to file with attributes to copy
		)
{
	HANDLE hFile = CreateFileW (
			lpwszFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDistribution,
			dwFlagsAndAttributes,
			hTemplateFile
			);

	return hFile;
}


BOOL WINAPI FAR_OemToCharBuff(LPCSTR lpszSrc,LPSTR lpszDst,DWORD cchDstLength)
{
  return OemToCharBuffA(lpszSrc,lpszDst,cchDstLength);
}

BOOL WINAPI FAR_CharToOemBuff(LPCSTR lpszSrc,LPSTR lpszDst,DWORD cchDstLength)
{
  return CharToOemBuffA(lpszSrc,lpszDst,cchDstLength);
}


BOOL WINAPI FAR_OemToChar(LPCSTR lpszSrc,LPSTR lpszDst)
{
  return OemToCharA(lpszSrc,lpszDst);
}

BOOL WINAPI FAR_CharToOem(LPCSTR lpszSrc,LPSTR lpszDst)
{
  return CharToOemA(lpszSrc,lpszDst);
}

BOOL apiCopyFileEx (
		const wchar_t *lpwszExistingFileName,
		const wchar_t *lpwszNewFileName,
		LPPROGRESS_ROUTINE lpProgressRoutine,
		LPVOID lpData,
		LPBOOL pbCancel,
		DWORD dwCopyFlags
		)
{
		return CopyFileExW(
				lpwszExistingFileName,
				lpwszNewFileName,
				lpProgressRoutine,
				lpData,
				pbCancel,
				dwCopyFlags
				);
}


BOOL apiMoveFile (
		const wchar_t *lpwszExistingFileName, // address of name of the existing file
		const wchar_t *lpwszNewFileName   // address of new name for the file
		)
{
	return MoveFileW (lpwszExistingFileName,lpwszNewFileName);
}

BOOL apiMoveFileEx (
		const wchar_t *lpwszExistingFileName, // address of name of the existing file
		const wchar_t *lpwszNewFileName,   // address of new name for the file
		DWORD dwFlags   // flag to determine how to move file
		)
{
	return MoveFileExW (lpwszExistingFileName,lpwszNewFileName,dwFlags);
}


BOOL MoveFileThroughTemp(const wchar_t *Src, const wchar_t *Dest)
{
  string strTemp;
  BOOL rc = FALSE;

  if ( FarMkTempEx(strTemp, NULL, FALSE) )
  {
      if ( MoveFileW (Src, strTemp) )
          rc = MoveFileW (strTemp, Dest);
  }

  return rc;
}

DWORD apiGetEnvironmentVariable (const wchar_t *lpwszName, string &strBuffer)
{
	int nSize = GetEnvironmentVariableW (lpwszName, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszBuffer = strBuffer.GetBuffer (nSize);

		nSize = GetEnvironmentVariableW (lpwszName, lpwszBuffer, nSize);

		strBuffer.ReleaseBuffer ();
	}

	return nSize;
}

DWORD apiGetCurrentDirectory (string &strCurDir)
{
	DWORD dwSize = GetCurrentDirectoryW (0, NULL);

	wchar_t *lpwszCurDir = strCurDir.GetBuffer (dwSize);

	dwSize = GetCurrentDirectoryW (dwSize, lpwszCurDir);

	strCurDir.ReleaseBuffer ();

	return dwSize;
}

DWORD apiGetTempPath (string &strBuffer)
{
	DWORD dwSize = GetTempPathW (0, NULL);

	wchar_t *lpwszBuffer = strBuffer.GetBuffer (dwSize);

	dwSize = GetTempPathW (dwSize, lpwszBuffer);

	strBuffer.ReleaseBuffer ();

	return dwSize;
};


DWORD apiGetModuleFileName (HMODULE hModule, string &strFileName)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszFileName = NULL;

	do {
		dwBufferSize <<= 1;

		lpwszFileName = (wchar_t*)xf_realloc (lpwszFileName, dwBufferSize*sizeof (wchar_t));

		dwSize = GetModuleFileNameW (hModule, lpwszFileName, dwBufferSize);
	} while ( dwSize && (dwSize >= dwBufferSize) );

	if ( dwSize )
		strFileName = lpwszFileName;

	xf_free (lpwszFileName);

	return dwSize;
}

DWORD apiExpandEnvironmentStrings (const wchar_t *src, string &strDest)
{
	string strSrc = src;

	DWORD length = ExpandEnvironmentStringsW(strSrc, NULL, 0);

	if ( length )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (length);

		ExpandEnvironmentStringsW(strSrc, lpwszDest, length);

		strDest.ReleaseBuffer ();

		length = (DWORD)strDest.GetLength ();
	}

	return length;
}


DWORD apiGetConsoleTitle (string &strConsoleTitle)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszTitle = NULL;

	do {
		dwBufferSize <<= 1;

		lpwszTitle = (wchar_t*)xf_realloc (lpwszTitle, dwBufferSize*sizeof (wchar_t));

		dwSize = GetConsoleTitleW (lpwszTitle, dwBufferSize);

	} while ( !dwSize && GetLastError() == ERROR_SUCCESS );

	if ( dwSize )
		strConsoleTitle = lpwszTitle;

	xf_free (lpwszTitle);

	return dwSize;
}


DWORD apiWNetGetConnection (const wchar_t *lpwszLocalName, string &strRemoteName)
{
	DWORD dwRemoteNameSize = 0;
	DWORD dwResult = WNetGetConnectionW(lpwszLocalName, NULL, &dwRemoteNameSize);

	if ( dwResult == ERROR_SUCCESS )
	{
		wchar_t *lpwszRemoteName = strRemoteName.GetBuffer (dwRemoteNameSize);

		dwResult = WNetGetConnectionW (lpwszLocalName, lpwszRemoteName, &dwRemoteNameSize);

		strRemoteName.ReleaseBuffer ();
	}

	return dwResult;
}

BOOL apiGetVolumeInformation (
        const wchar_t *lpwszRootPathName,
        string *pVolumeName,
        LPDWORD lpVolumeSerialNumber,
        LPDWORD lpMaximumComponentLength,
        LPDWORD lpFileSystemFlags,
        string *pFileSystemName
        )
{
    wchar_t *lpwszVolumeName = pVolumeName?pVolumeName->GetBuffer (MAX_PATH+1):NULL; //MSDN!
    wchar_t *lpwszFileSystemName = pFileSystemName?pFileSystemName->GetBuffer (MAX_PATH+1):NULL;

    BOOL bResult = GetVolumeInformationW (
            lpwszRootPathName,
            lpwszVolumeName,
            lpwszVolumeName?MAX_PATH:0,
            lpVolumeSerialNumber,
            lpMaximumComponentLength,
            lpFileSystemFlags,
            lpwszFileSystemName,
            lpwszFileSystemName?MAX_PATH:0
            );

    if ( lpwszVolumeName )
        pVolumeName->ReleaseBuffer ();

    if ( lpwszFileSystemName )
        pFileSystemName->ReleaseBuffer ();

    return bResult;
}

HANDLE apiFindFirstFile (
        const wchar_t *lpwszFileName,
        FAR_FIND_DATA_EX *pFindFileData,
        bool ScanSymLink
        )
{
    WIN32_FIND_DATAW fdata;

    HANDLE hResult = FindFirstFileW (lpwszFileName, &fdata);

		if(hResult==INVALID_HANDLE_VALUE && ScanSymLink)
		{
			string strRealName;
			ConvertNameToReal(lpwszFileName,strRealName);
			hResult=FindFirstFileW(strRealName,&fdata);
		}

    if ( hResult != INVALID_HANDLE_VALUE )
    {
        pFindFileData->dwFileAttributes = fdata.dwFileAttributes;
        pFindFileData->ftCreationTime = fdata.ftCreationTime;
        pFindFileData->ftLastAccessTime = fdata.ftLastAccessTime;
        pFindFileData->ftLastWriteTime = fdata.ftLastWriteTime;
        pFindFileData->nFileSize = fdata.nFileSizeHigh*_ui64(0x100000000)+fdata.nFileSizeLow;
        pFindFileData->dwReserved0 = fdata.dwReserved0;
        pFindFileData->dwReserved1 = fdata.dwReserved1;
        pFindFileData->strFileName = fdata.cFileName;
        pFindFileData->strAlternateFileName = fdata.cAlternateFileName;
    }

    return hResult;
}

BOOL apiFindNextFile (HANDLE hFindFile, FAR_FIND_DATA_EX *pFindFileData)
{
    WIN32_FIND_DATAW fdata;

    BOOL bResult = FindNextFileW (hFindFile, &fdata);

    if ( bResult )
    {
        pFindFileData->dwFileAttributes = fdata.dwFileAttributes;
        pFindFileData->ftCreationTime = fdata.ftCreationTime;
        pFindFileData->ftLastAccessTime = fdata.ftLastAccessTime;
        pFindFileData->ftLastWriteTime = fdata.ftLastWriteTime;
        pFindFileData->nFileSize = fdata.nFileSizeHigh*_ui64(0x100000000)+fdata.nFileSizeLow;
        pFindFileData->dwReserved0 = fdata.dwReserved0;
        pFindFileData->dwReserved1 = fdata.dwReserved1;
        pFindFileData->strFileName = fdata.cFileName;
        pFindFileData->strAlternateFileName = fdata.cAlternateFileName;
    }

    return bResult;
}

BOOL apiFindClose(HANDLE hFindFile)
{
	return FindClose(hFindFile);
}

void apiFindDataToDataEx (const FAR_FIND_DATA *pSrc, FAR_FIND_DATA_EX *pDest)
{
    pDest->dwFileAttributes = pSrc->dwFileAttributes;
    pDest->ftCreationTime = pSrc->ftCreationTime;
    pDest->ftLastAccessTime = pSrc->ftLastAccessTime;
    pDest->ftLastWriteTime = pSrc->ftLastWriteTime;
    pDest->nFileSize = pSrc->nFileSize;
    pDest->nPackSize = pSrc->nPackSize;
    pDest->strFileName = pSrc->lpwszFileName;
    pDest->strAlternateFileName = pSrc->lpwszAlternateFileName;
}

void apiFindDataExToData (const FAR_FIND_DATA_EX *pSrc, FAR_FIND_DATA *pDest)
{
    pDest->dwFileAttributes = pSrc->dwFileAttributes;
    pDest->ftCreationTime = pSrc->ftCreationTime;
    pDest->ftLastAccessTime = pSrc->ftLastAccessTime;
    pDest->ftLastWriteTime = pSrc->ftLastWriteTime;
    pDest->nFileSize = pSrc->nFileSize;
    pDest->nPackSize = pSrc->nPackSize;
    pDest->lpwszFileName = xf_wcsdup (pSrc->strFileName);
    pDest->lpwszAlternateFileName = xf_wcsdup (pSrc->strAlternateFileName);
}

void apiFreeFindData (FAR_FIND_DATA *pData)
{
    xf_free (pData->lpwszFileName);
    xf_free (pData->lpwszAlternateFileName);
}

BOOL apiGetFindDataEx (const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindData,bool ScanSymLink)
{
    HANDLE hSearch = apiFindFirstFile (lpwszFileName, pFindData,ScanSymLink);

    if ( hSearch != INVALID_HANDLE_VALUE )
    {
        apiFindClose (hSearch);
        return TRUE;
    }
    else
	{
		DWORD dwAttr=GetFileAttributes(lpwszFileName);
		if(dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			// Ага, значит файл таки есть. Заполним структуру ручками.
			if(pFindData)
			{
				pFindData->Clear();
				pFindData->dwFileAttributes=dwAttr;
				HANDLE hFile=apiCreateFile(lpwszFileName,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,(pFindData->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
				if(hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile,&pFindData->ftCreationTime,&pFindData->ftLastAccessTime,&pFindData->ftLastWriteTime);
					apiGetFileSize (hFile,&pFindData->nFileSize);
					CloseHandle(hFile);
				}
				if(pFindData->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				{
					string strTmp;
					GetReparsePointInfo(lpwszFileName,strTmp,&pFindData->dwReserved0); //MSDN
				}
				else
				{
					pFindData->dwReserved0=0;
				}
				pFindData->dwReserved1=0;
				pFindData->strFileName=PointToName(lpwszFileName);
				ConvertNameToShort(lpwszFileName,pFindData->strAlternateFileName);
				return TRUE;
			}
		}
	}
	if(pFindData)
	{
		pFindData->Clear();
		pFindData->dwFileAttributes=INVALID_FILE_ATTRIBUTES; //BUGBUG
	}
    return FALSE;
}

BOOL apiGetFileSize (HANDLE hFile, unsigned __int64 *pSize)
{
	DWORD dwHi, dwLo;

	dwLo = GetFileSize (hFile, &dwHi);

	int nError = GetLastError();
	SetLastError (nError);

	if ( (dwLo == INVALID_FILE_SIZE) && (nError != NO_ERROR) )
		return FALSE;
	else
	{
		if ( pSize )
			*pSize = dwHi*_ui64(0x100000000)+dwLo;

		return TRUE;
	}
}
