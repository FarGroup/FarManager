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

#include "flink.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "mix.hpp"

BOOL apiDeleteFile(const wchar_t *lpwszFileName)
{
	return DeleteFile(NTPath(lpwszFileName));
}

BOOL apiRemoveDirectory(const wchar_t *DirName)
{
	string strDirName=DirName;
	AddEndSlash(strDirName);
	return RemoveDirectory(NTPath(strDirName));
}

HANDLE apiCreateFile(
    const wchar_t *lpwszFileName,     // pointer to name of the file
    DWORD dwDesiredAccess,  // access (read-write) mode
    DWORD dwShareMode,      // share mode
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
    DWORD dwCreationDistribution, // how to create
    DWORD dwFlagsAndAttributes,   // file attributes
    HANDLE hTemplateFile          // handle to file with attributes to copy
)
{
	if (dwCreationDistribution==OPEN_EXISTING)
	{
		dwFlagsAndAttributes|=FILE_FLAG_POSIX_SEMANTICS;
	}

	dwFlagsAndAttributes|=FILE_FLAG_BACKUP_SEMANTICS;
	string strName(NTPath(lpwszFileName).Str);
	HANDLE hFile=CreateFile(
	                 strName,
	                 dwDesiredAccess,
	                 dwShareMode,
	                 lpSecurityAttributes,
	                 dwCreationDistribution,
	                 dwFlagsAndAttributes,
	                 hTemplateFile
	             );
	DWORD Error=GetLastError();

	if (hFile==INVALID_HANDLE_VALUE && (Error==ERROR_FILE_NOT_FOUND||Error==ERROR_PATH_NOT_FOUND))
	{
		dwFlagsAndAttributes&=~FILE_FLAG_POSIX_SEMANTICS;
		hFile=CreateFile(
		          strName,
		          dwDesiredAccess,
		          dwShareMode,
		          lpSecurityAttributes,
		          dwCreationDistribution,
		          dwFlagsAndAttributes,
		          hTemplateFile
		      );
	}

	return hFile;
}

BOOL apiCopyFileEx(
    const wchar_t *lpwszExistingFileName,
    const wchar_t *lpwszNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine,
    LPVOID lpData,
    LPBOOL pbCancel,
    DWORD dwCopyFlags
)
{
	return CopyFileEx(
	           NTPath(lpwszExistingFileName),
	           NTPath(lpwszNewFileName),
	           lpProgressRoutine,
	           lpData,
	           pbCancel,
	           dwCopyFlags
	       );
}


BOOL apiMoveFile(
    const wchar_t *lpwszExistingFileName, // address of name of the existing file
    const wchar_t *lpwszNewFileName   // address of new name for the file
)
{
	return MoveFile(NTPath(lpwszExistingFileName),NTPath(lpwszNewFileName));
}

BOOL apiMoveFileEx(
    const wchar_t *lpwszExistingFileName, // address of name of the existing file
    const wchar_t *lpwszNewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
)
{
	return MoveFileEx(NTPath(lpwszExistingFileName),NTPath(lpwszNewFileName),dwFlags);
}


BOOL apiMoveFileThroughTemp(const wchar_t *Src, const wchar_t *Dest)
{
	string strTemp;
	BOOL rc = FALSE;

	if (FarMkTempEx(strTemp, NULL, FALSE))
	{
		if (apiMoveFile(Src, strTemp))
			rc = apiMoveFile(strTemp, Dest);
	}

	return rc;
}

DWORD apiGetEnvironmentVariable(const wchar_t *lpwszName, string &strBuffer)
{
	int nSize = GetEnvironmentVariable(lpwszName, NULL, 0);

	if (nSize)
	{
		wchar_t *lpwszBuffer = strBuffer.GetBuffer(nSize);
		nSize = GetEnvironmentVariable(lpwszName, lpwszBuffer, nSize);
		strBuffer.ReleaseBuffer();
	}

	return nSize;
}

string& strCurrentDirectory()
{
	static string strCurrentDirectory;
	return strCurrentDirectory;
}

DWORD apiGetCurrentDirectory(string &strCurDir)
{
	DeleteEndSlash(strCurrentDirectory());
	LPCWSTR CD=strCurrentDirectory();
	int Offset=HasPathPrefix(CD)?4:0;

	if ((CD[Offset] && CD[Offset+1]==L':' && !CD[Offset+2]) || IsLocalVolumeRootPath(CD))
		AddEndSlash(strCurrentDirectory());

	strCurDir=strCurrentDirectory();
	return static_cast<DWORD>(strCurDir.GetLength());
}

BOOL apiSetCurrentDirectory(LPCWSTR lpPathName)
{
	string strDir=lpPathName;
	AddEndSlash(strDir);
	strDir+=L"*";
	FAR_FIND_DATA_EX fd;

	if (apiGetFindDataEx(strDir,&fd) || GetLastError()==ERROR_FILE_NOT_FOUND) // root dir on empty disk
	{
		strCurrentDirectory()=lpPathName;
		ReplaceSlashToBSlash(strCurrentDirectory());
		return TRUE;
	}

	return FALSE;
}

DWORD apiGetTempPath(string &strBuffer)
{
	DWORD dwSize = GetTempPath(0, NULL);
	wchar_t *lpwszBuffer = strBuffer.GetBuffer(dwSize);
	dwSize = GetTempPath(dwSize, lpwszBuffer);
	strBuffer.ReleaseBuffer();
	return dwSize;
};


DWORD apiGetModuleFileName(HMODULE hModule, string &strFileName)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszFileName = NULL;

	do
	{
		dwBufferSize <<= 1;
		lpwszFileName = (wchar_t*)xf_realloc_nomove(lpwszFileName, dwBufferSize*sizeof(wchar_t));
		dwSize = GetModuleFileName(hModule, lpwszFileName, dwBufferSize);
	}
	while (dwSize && (dwSize >= dwBufferSize));

	if (dwSize)
		strFileName = lpwszFileName;

	xf_free(lpwszFileName);
	return dwSize;
}

DWORD apiExpandEnvironmentStrings(const wchar_t *src, string &strDest)
{
	string strSrc = src;
	DWORD length = ExpandEnvironmentStrings(strSrc, NULL, 0);

	if (length)
	{
		wchar_t *lpwszDest = strDest.GetBuffer(length);
		ExpandEnvironmentStrings(strSrc, lpwszDest, length);
		strDest.ReleaseBuffer();
		length = (DWORD)strDest.GetLength();
	}

	return length;
}


DWORD apiGetConsoleTitle(string &strConsoleTitle)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszTitle = NULL;

	do
	{
		dwBufferSize <<= 1;
		lpwszTitle = (wchar_t*)xf_realloc_nomove(lpwszTitle, dwBufferSize*sizeof(wchar_t));
		dwSize = GetConsoleTitle(lpwszTitle, dwBufferSize);
	}
	while (!dwSize && GetLastError() == ERROR_SUCCESS);

	if (dwSize)
		strConsoleTitle = lpwszTitle;

	xf_free(lpwszTitle);
	return dwSize;
}


DWORD apiWNetGetConnection(const wchar_t *lpwszLocalName, string &strRemoteName)
{
	DWORD dwRemoteNameSize = 0;
	DWORD dwResult = WNetGetConnection(lpwszLocalName, NULL, &dwRemoteNameSize);

	if (dwResult == ERROR_SUCCESS || dwResult == ERROR_MORE_DATA)
	{
		wchar_t *lpwszRemoteName = strRemoteName.GetBuffer(dwRemoteNameSize);
		dwResult = WNetGetConnection(lpwszLocalName, lpwszRemoteName, &dwRemoteNameSize);
		strRemoteName.ReleaseBuffer();
	}

	return dwResult;
}

BOOL apiGetVolumeInformation(
    const wchar_t *lpwszRootPathName,
    string *pVolumeName,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    string *pFileSystemName
)
{
	wchar_t *lpwszVolumeName = pVolumeName?pVolumeName->GetBuffer(MAX_PATH+1):NULL;  //MSDN!
	wchar_t *lpwszFileSystemName = pFileSystemName?pFileSystemName->GetBuffer(MAX_PATH+1):NULL;
	BOOL bResult = GetVolumeInformation(
	                   lpwszRootPathName,
	                   lpwszVolumeName,
	                   lpwszVolumeName?MAX_PATH:0,
	                   lpVolumeSerialNumber,
	                   lpMaximumComponentLength,
	                   lpFileSystemFlags,
	                   lpwszFileSystemName,
	                   lpwszFileSystemName?MAX_PATH:0
	               );

	if (lpwszVolumeName)
		pVolumeName->ReleaseBuffer();

	if (lpwszFileSystemName)
		pFileSystemName->ReleaseBuffer();

	return bResult;
}

HANDLE apiFindFirstFile(
    const wchar_t *lpwszFileName,
    FAR_FIND_DATA_EX *pFindFileData,
    bool ScanSymLink
)
{
	WIN32_FIND_DATA fdata;
	string strName(NTPath(lpwszFileName).Str);
	HANDLE hResult = FindFirstFile(strName, &fdata);

	if (hResult==INVALID_HANDLE_VALUE && ScanSymLink)
	{
		ConvertNameToReal(strName,strName);
		strName=NTPath(strName);
		hResult=FindFirstFile(strName,&fdata);
	}

	if (hResult != INVALID_HANDLE_VALUE)
	{
		pFindFileData->dwFileAttributes = fdata.dwFileAttributes;
		pFindFileData->ftCreationTime = fdata.ftCreationTime;
		pFindFileData->ftLastAccessTime = fdata.ftLastAccessTime;
		pFindFileData->ftLastWriteTime = fdata.ftLastWriteTime;
		pFindFileData->nFileSize = fdata.nFileSizeHigh*0x100000000ull+fdata.nFileSizeLow;
		pFindFileData->nPackSize = 0;
		pFindFileData->dwReserved0 = fdata.dwReserved0;
		pFindFileData->dwReserved1 = fdata.dwReserved1;
		pFindFileData->strFileName = fdata.cFileName;
		pFindFileData->strAlternateFileName = fdata.cAlternateFileName;
	}

	return hResult;
}

BOOL apiFindNextFile(HANDLE hFindFile, FAR_FIND_DATA_EX *pFindFileData)
{
	WIN32_FIND_DATA fdata;
	BOOL bResult = FindNextFile(hFindFile, &fdata);

	if (bResult)
	{
		pFindFileData->dwFileAttributes = fdata.dwFileAttributes;
		pFindFileData->ftCreationTime = fdata.ftCreationTime;
		pFindFileData->ftLastAccessTime = fdata.ftLastAccessTime;
		pFindFileData->ftLastWriteTime = fdata.ftLastWriteTime;
		pFindFileData->nFileSize = fdata.nFileSizeHigh*0x100000000ull+fdata.nFileSizeLow;
		pFindFileData->nPackSize = 0;
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

void apiFindDataToDataEx(const FAR_FIND_DATA *pSrc, FAR_FIND_DATA_EX *pDest)
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

void apiFindDataExToData(const FAR_FIND_DATA_EX *pSrc, FAR_FIND_DATA *pDest)
{
	pDest->dwFileAttributes = pSrc->dwFileAttributes;
	pDest->ftCreationTime = pSrc->ftCreationTime;
	pDest->ftLastAccessTime = pSrc->ftLastAccessTime;
	pDest->ftLastWriteTime = pSrc->ftLastWriteTime;
	pDest->nFileSize = pSrc->nFileSize;
	pDest->nPackSize = pSrc->nPackSize;
	pDest->lpwszFileName = xf_wcsdup(pSrc->strFileName);
	pDest->lpwszAlternateFileName = xf_wcsdup(pSrc->strAlternateFileName);
}

void apiFreeFindData(FAR_FIND_DATA *pData)
{
	xf_free(pData->lpwszFileName);
	xf_free(pData->lpwszAlternateFileName);
}

BOOL apiGetFindDataEx(const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindData,bool ScanSymLink)
{
	HANDLE hSearch = apiFindFirstFile(lpwszFileName, pFindData,ScanSymLink);

	if (hSearch != INVALID_HANDLE_VALUE)
	{
		apiFindClose(hSearch);
		return TRUE;
	}
	else if (GetLastError()==ERROR_ACCESS_DENIED && !wcspbrk(lpwszFileName,L"*?"))
	{
		DWORD dwAttr=apiGetFileAttributes(lpwszFileName);

		if (dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			// Ага, значит файл таки есть. Заполним структуру ручками.
			if (pFindData)
			{
				pFindData->Clear();
				pFindData->dwFileAttributes=dwAttr;
				HANDLE hFile=apiCreateFile(lpwszFileName,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0);

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile,&pFindData->ftCreationTime,&pFindData->ftLastAccessTime,&pFindData->ftLastWriteTime);
					apiGetFileSizeEx(hFile,pFindData->nFileSize);
					CloseHandle(hFile);
				}

				if (pFindData->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
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

	if (pFindData)
	{
		pFindData->Clear();
		pFindData->dwFileAttributes=INVALID_FILE_ATTRIBUTES; //BUGBUG
	}

	return FALSE;
}

bool apiGetFileSizeEx(HANDLE hFile, UINT64 &Size)
{
	bool Result=false;

	if (GetFileSizeEx(hFile,reinterpret_cast<PLARGE_INTEGER>(&Size)))
	{
		Result=true;
	}
	else
	{
		GET_LENGTH_INFORMATION gli;
		DWORD BytesReturned;

		if (DeviceIoControl(hFile,IOCTL_DISK_GET_LENGTH_INFO,NULL,0,&gli,sizeof(gli),&BytesReturned,NULL))
		{
			Size=gli.Length.QuadPart;
			Result=true;
		}
	}

	return Result;
}

int apiRegEnumKeyEx(HKEY hKey,DWORD dwIndex,string &strName,PFILETIME lpftLastWriteTime)
{
	int ExitCode=ERROR_MORE_DATA;

	for (DWORD Size=512; ExitCode==ERROR_MORE_DATA; Size<<=1)
	{
		wchar_t *Name=strName.GetBuffer(Size);
		DWORD Size0=Size;
		ExitCode=RegEnumKeyEx(hKey,dwIndex,Name,&Size0,NULL,NULL,NULL,lpftLastWriteTime);
		strName.ReleaseBuffer();
	}

	return ExitCode;
}

BOOL apiIsDiskInDrive(const wchar_t *Root)
{
	string strVolName;
	string strDrive;
	DWORD  MaxComSize;
	DWORD  Flags;
	string strFS;
	strDrive = Root;
	AddEndSlash(strDrive);
	//UINT ErrMode = SetErrorMode ( SEM_FAILCRITICALERRORS );
	//если не сделать SetErrorMode - выскочит стандартное окошко "Drive Not Ready"
	BOOL Res = apiGetVolumeInformation(strDrive, &strVolName, NULL, &MaxComSize, &Flags, &strFS);
	//SetErrorMode(ErrMode);
	return Res;
}

int apiGetFileTypeByName(const wchar_t *Name)
{
	HANDLE hFile=apiCreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0);

	if (hFile==INVALID_HANDLE_VALUE)
		return FILE_TYPE_UNKNOWN;

	int Type=GetFileType(hFile);
	CloseHandle(hFile);
	return Type;
}

BOOL apiGetDiskSize(const wchar_t *Path,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree)
{
	int ExitCode=0;
	unsigned __int64 uiTotalSize,uiTotalFree,uiUserFree;
	uiUserFree=0;
	uiTotalSize=0;
	uiTotalFree=0;
	string strPath(NTPath(Path).Str);
	AddEndSlash(strPath);
	ExitCode=GetDiskFreeSpaceEx(strPath,(PULARGE_INTEGER)&uiUserFree,(PULARGE_INTEGER)&uiTotalSize,(PULARGE_INTEGER)&uiTotalFree);

	if (TotalSize)
		*TotalSize = uiTotalSize;

	if (TotalFree)
		*TotalFree = uiTotalFree;

	if (UserFree)
		*UserFree = uiUserFree;

	return ExitCode;
}

BOOL apiGetConsoleKeyboardLayoutName(string &strDest)
{
	BOOL ret=FALSE;
	strDest.Clear();

	if (ifn.pfnGetConsoleKeyboardLayoutName)
	{
		wchar_t *p = strDest.GetBuffer(KL_NAMELENGTH+1);

		if (p && ifn.pfnGetConsoleKeyboardLayoutName(p))
			ret=TRUE;

		strDest.ReleaseBuffer();
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}

	return ret;
}

HANDLE apiFindFirstFileName(LPCWSTR lpFileName,DWORD dwFlags,string& strLinkName)
{
	HANDLE hRet=INVALID_HANDLE_VALUE;

	if (ifn.pfnFindFirstFileNameW)
	{
		DWORD StringLength=0;

		if (ifn.pfnFindFirstFileNameW(NTPath(lpFileName),0,&StringLength,NULL)==INVALID_HANDLE_VALUE && GetLastError()==ERROR_MORE_DATA)
		{
			hRet=ifn.pfnFindFirstFileNameW(NTPath(lpFileName),0,&StringLength,strLinkName.GetBuffer(StringLength));
			strLinkName.ReleaseBuffer();
		}
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}

	return hRet;
}

BOOL apiFindNextFileName(HANDLE hFindStream,string& strLinkName)
{
	BOOL Ret=FALSE;

	if (ifn.pfnFindNextFileNameW)
	{
		DWORD StringLength=0;

		if (!ifn.pfnFindNextFileNameW(hFindStream,&StringLength,NULL) && GetLastError()==ERROR_MORE_DATA)
		{
			Ret=ifn.pfnFindNextFileNameW(hFindStream,&StringLength,strLinkName.GetBuffer(StringLength));
			strLinkName.ReleaseBuffer();
		}
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}

	return Ret;
}

BOOL apiCreateDirectory(LPCWSTR lpPathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	string strPathName=NTPath(lpPathName).Str;
	AddEndSlash(strPathName);
	return CreateDirectory(strPathName,lpSecurityAttributes);
}

DWORD apiGetFileAttributes(LPCWSTR lpFileName)
{
	return GetFileAttributes(NTPath(lpFileName));
}

BOOL apiSetFileAttributes(LPCWSTR lpFileName,DWORD dwFileAttributes)
{
	return SetFileAttributes(NTPath(lpFileName),dwFileAttributes);
}

BOOL apiCreateSymbolicLink(LPCWSTR lpSymlinkFileName,LPCWSTR lpTargetFileName,DWORD dwFlags)
{
	BOOL Result=FALSE;
	string strSymlinkFileName(NTPath(lpSymlinkFileName).Str);

	if (ifn.pfnCreateSymbolicLink)
	{
		Result=ifn.pfnCreateSymbolicLink(strSymlinkFileName,lpTargetFileName,dwFlags);
	}
	else
	{
		Result=CreateReparsePoint(lpTargetFileName,strSymlinkFileName,dwFlags&SYMBOLIC_LINK_FLAG_DIRECTORY?RP_SYMLINKDIR:RP_SYMLINKFILE);
	}

	return Result;
}

bool apiGetCompressedFileSize(LPCWSTR lpFileName,UINT64& Size)
{
	bool Result=false;
	DWORD High=0,Low=GetCompressedFileSize(NTPath(lpFileName),&High);

	if ((Low!=INVALID_FILE_SIZE)||(GetLastError()!=NO_ERROR))
	{
		ULARGE_INTEGER i={Low,High};
		Size=i.QuadPart;
		Result=true;
	}

	return Result;
}

BOOL apiCreateHardLink(LPCWSTR lpFileName,LPCWSTR lpExistingFileName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return CreateHardLink(NTPath(lpFileName),NTPath(lpExistingFileName),lpSecurityAttributes) ||
	       //bug in win2k: \\?\ fails
	       CreateHardLink(lpFileName,lpExistingFileName,lpSecurityAttributes);
}

BOOL apiSetFilePointerEx(HANDLE hFile,INT64 DistanceToMove,PINT64 NewFilePointer,DWORD dwMoveMethod)
{
	return SetFilePointerEx(hFile,*((PLARGE_INTEGER)&DistanceToMove),(PLARGE_INTEGER)NewFilePointer,dwMoveMethod);
}

HANDLE apiFindFirstStream(LPCWSTR lpFileName,STREAM_INFO_LEVELS InfoLevel,LPVOID lpFindStreamData,DWORD dwFlags)
{
	HANDLE Ret=INVALID_HANDLE_VALUE;

	if (ifn.pfnFindFirstStreamW)
	{
		Ret=ifn.pfnFindFirstStreamW(NTPath(lpFileName),InfoLevel,lpFindStreamData,dwFlags);
	}
	else
	{
		if (InfoLevel==FindStreamInfoStandard && ifn.pfnNtQueryInformationFile)
		{
			HANDLE hFile = apiCreateFile(lpFileName,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0);

			if (hFile!=INVALID_HANDLE_VALUE)
			{
				const size_t Size=sizeof(ULONG)+(64<<10);
				LPBYTE InfoBlock=static_cast<LPBYTE>(xf_malloc(Size));

				if (InfoBlock)
				{
					memset(InfoBlock,0,Size);
					PFILE_STREAM_INFORMATION pStreamInfo=reinterpret_cast<PFILE_STREAM_INFORMATION>(InfoBlock+sizeof(ULONG));
					IO_STATUS_BLOCK ioStatus;
					int res=ifn.pfnNtQueryInformationFile(hFile,&ioStatus,pStreamInfo,Size-sizeof(ULONG),FileStreamInformation);

					if (!res)
					{
						PWIN32_FIND_STREAM_DATA pFsd=reinterpret_cast<PWIN32_FIND_STREAM_DATA>(lpFindStreamData);
						*reinterpret_cast<PLONG>(InfoBlock)=pStreamInfo->NextEntryOffset;

						if (pStreamInfo->StreamNameLength)
						{
							memcpy(pFsd->cStreamName,pStreamInfo->StreamName,pStreamInfo->StreamNameLength);
							pFsd->cStreamName[pStreamInfo->StreamNameLength/sizeof(WCHAR)]=L'\0';
							pFsd->StreamSize=pStreamInfo->StreamSize;
							Ret=InfoBlock;
						}
					}

					if (Ret==INVALID_HANDLE_VALUE)
					{
						xf_free(InfoBlock);
					}
				}

				CloseHandle(hFile);
			}
		}
		else
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		}
	}

	return Ret;
}

BOOL apiFindNextStream(HANDLE hFindStream,LPVOID lpFindStreamData)
{
	BOOL Ret=FALSE;

	if (ifn.pfnFindNextStreamW)
	{
		Ret=ifn.pfnFindNextStreamW(hFindStream,lpFindStreamData);
	}
	else
	{
		if (ifn.pfnNtQueryInformationFile)
		{
			ULONG NextEntryOffset=*reinterpret_cast<PULONG>(hFindStream);

			if (NextEntryOffset)
			{
				PFILE_STREAM_INFORMATION pStreamInfo=reinterpret_cast<PFILE_STREAM_INFORMATION>(reinterpret_cast<LPBYTE>(hFindStream)+sizeof(ULONG)+NextEntryOffset);
				PWIN32_FIND_STREAM_DATA pFsd=reinterpret_cast<PWIN32_FIND_STREAM_DATA>(lpFindStreamData);
				*reinterpret_cast<PLONG>(hFindStream)=pStreamInfo->NextEntryOffset?NextEntryOffset+pStreamInfo->NextEntryOffset:0;

				if (pStreamInfo->StreamNameLength)
				{
					memcpy(pFsd->cStreamName,pStreamInfo->StreamName,pStreamInfo->StreamNameLength);
					pFsd->cStreamName[pStreamInfo->StreamNameLength/sizeof(WCHAR)]=L'\0';
					pFsd->StreamSize=pStreamInfo->StreamSize;
					Ret=TRUE;
				}
			}
		}
		else
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		}
	}

	return Ret;
}

BOOL apiFindStreamClose(HANDLE hFindFile)
{
	BOOL Ret=FALSE;

	if (ifn.pfnFindFirstStreamW && ifn.pfnFindNextStreamW)
	{
		Ret=apiFindClose(hFindFile);
	}
	else
	{
		xf_free(hFindFile);
		Ret=TRUE;
	}

	return Ret;
}

bool apiGetLogicalDriveStrings(string& DriveStrings)
{
	DWORD BufSize = MAX_PATH;
	DWORD Size = GetLogicalDriveStringsW(BufSize, DriveStrings.GetBuffer(BufSize));

	if (Size > BufSize)
	{
		BufSize = Size;
		Size = GetLogicalDriveStringsW(BufSize, DriveStrings.GetBuffer(BufSize));
	}

	if ((Size > 0) && (Size <= BufSize))
	{
		DriveStrings.ReleaseBuffer(Size);
		return true;
	}

	return false;
}

bool apiGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	if (!ifn.pfnGetFinalPathNameByHandle)
	{
		FinalFilePath.Clear();
		return false;
	}

	DWORD BufLen = NT_MAX_PATH;
	DWORD Len;

	// It is known that GetFinalPathNameByHandle crashes on Windows 7 with Ext2FSD
	__try
	{
		Len = ifn.pfnGetFinalPathNameByHandle(hFile, FinalFilePath.GetBuffer(BufLen+1), BufLen, VOLUME_NAME_GUID);
	}
	__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		Len = 0;
	}

	if (Len > BufLen)
	{
		BufLen = Len;
		__try
		{
			Len = ifn.pfnGetFinalPathNameByHandle(hFile, FinalFilePath.GetBuffer(BufLen+1), BufLen, VOLUME_NAME_GUID);
		}
		__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			Len = 0;
		}
	}

	if (Len <= BufLen)
		FinalFilePath.ReleaseBuffer(Len);
	else
		FinalFilePath.Clear();

	return Len != 0 && Len <= BufLen;
}
