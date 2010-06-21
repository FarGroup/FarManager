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
#include "ctrlobj.hpp"
#include "adminmode.hpp"

FindFile::FindFile(LPCWSTR Object, bool ScanSymLink):
	Handle(INVALID_HANDLE_VALUE),
	empty(false),
	admin(false)
{
	string strName(NTPath(Object).Str);
	Handle = FindFirstFile(strName, &W32FindData);

	if (Handle == INVALID_HANDLE_VALUE && ElevationRequired(ELEVATION_READ_REQUEST))
	{
		if(ScanSymLink)
		{
			string strReal;
			ConvertNameToReal(strName, strReal);
			strReal = NTPath(strReal);
			Handle = FindFirstFile(strReal, &W32FindData);
		}

		if (Handle == INVALID_HANDLE_VALUE && ElevationRequired(ELEVATION_READ_REQUEST))
		{
			Handle = Admin.fFindFirstFile(strName, &W32FindData);
			admin = Handle != INVALID_HANDLE_VALUE;
		}
	}
	empty = Handle == INVALID_HANDLE_VALUE;
}

FindFile::~FindFile()
{
	if(Handle != INVALID_HANDLE_VALUE)
	{
		admin?Admin.fFindClose(Handle):FindClose(Handle);
	}
}

bool FindFile::Get(FAR_FIND_DATA_EX& FindData)
{
	bool Result = false;
	if (!empty)
	{
		FindData.dwFileAttributes = W32FindData.dwFileAttributes;
		FindData.ftCreationTime = W32FindData.ftCreationTime;
		FindData.ftLastAccessTime = W32FindData.ftLastAccessTime;
		FindData.ftLastWriteTime = W32FindData.ftLastWriteTime;
		FindData.nFileSize = W32FindData.nFileSizeHigh*0x100000000ull+W32FindData.nFileSizeLow;
		FindData.nPackSize = 0;
		FindData.dwReserved0 = W32FindData.dwReserved0;
		FindData.dwReserved1 = W32FindData.dwReserved1;
		FindData.strFileName = W32FindData.cFileName;
		FindData.strAlternateFileName = W32FindData.cAlternateFileName;
		Result = true;
	}
	if(Result)
	{
		empty = admin?!Admin.fFindNextFile(Handle, &W32FindData):!FindNextFile(Handle, &W32FindData);
	}

	// skip ".." & "."
	if(Result && FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && FindData.strFileName.At(0) == L'.' &&
		// хитрый способ - у виртуальных папок не бывает SFN, в отличие от.
		FindData.strAlternateFileName.IsEmpty() &&
		((FindData.strFileName.At(1) == L'.' && !FindData.strFileName.At(2)) || !FindData.strFileName.At(1)))
	{
		Result = Get(FindData);
	}
	return Result;
}




File::File():
	Handle(INVALID_HANDLE_VALUE),
	eof(false),
	admin(false)
{
}

File::~File()
{
	Close();
}

bool File::Open(LPCWSTR Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile, bool ForceElevation)
{
	string strObject(NTPath(Object).Str);
	FlagsAndAttributes|=FILE_FLAG_BACKUP_SEMANTICS|(CreationDistribution==OPEN_EXISTING?FILE_FLAG_POSIX_SEMANTICS:0);
	Handle = CreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
	if(Handle == INVALID_HANDLE_VALUE)
	{
		DWORD Error=GetLastError();
		if(Error==ERROR_FILE_NOT_FOUND||Error==ERROR_PATH_NOT_FOUND)
		{
			FlagsAndAttributes&=~FILE_FLAG_POSIX_SEMANTICS;
			Handle = CreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		}
	}
	if((Handle == INVALID_HANDLE_VALUE && ElevationRequired(DesiredAccess&(GENERIC_ALL|GENERIC_WRITE|STANDARD_RIGHTS_ALL|STANDARD_RIGHTS_WRITE|WRITE_OWNER|WRITE_DAC|DELETE|FILE_GENERIC_WRITE)?ELEVATION_MODIFY_REQUEST:ELEVATION_READ_REQUEST)) || ForceElevation)
	{
		if(ForceElevation && Handle!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(Handle);
		}
		Handle = Admin.fCreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		if(Handle != INVALID_HANDLE_VALUE)
		{
			admin = true;
		}
	}
	return Handle != INVALID_HANDLE_VALUE;
}

bool File::Read(LPVOID Buffer, DWORD NumberOfBytesToRead, LPDWORD NumberOfBytesRead, LPOVERLAPPED Overlapped)
{
	DWORD BytesRead=0;
	bool Result=admin?Admin.fReadFile(Handle, Buffer, NumberOfBytesToRead, &BytesRead, Overlapped):ReadFile(Handle, Buffer, NumberOfBytesToRead, &BytesRead, Overlapped) != FALSE;
	if(NumberOfBytesRead)
	{
		*NumberOfBytesRead=BytesRead;
	}
	eof = Result && !BytesRead;
	return Result;
}

bool File::Write(LPCVOID Buffer, DWORD NumberOfBytesToWrite, LPDWORD NumberOfBytesWritten, LPOVERLAPPED Overlapped) const
{
	return admin?Admin.fWriteFile(Handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Overlapped):WriteFile(Handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Overlapped) != FALSE;
}

bool File::SetPointer(INT64 DistanceToMove, PINT64 NewFilePointer, DWORD MoveMethod)
{
	return admin?Admin.fSetFilePointerEx(Handle, DistanceToMove, NewFilePointer, MoveMethod):SetFilePointerEx(Handle, *reinterpret_cast<PLARGE_INTEGER>(&DistanceToMove), reinterpret_cast<PLARGE_INTEGER>(NewFilePointer), MoveMethod) != FALSE;
}

bool File::SetEnd()
{
	return admin?Admin.fSetEndOfFile(Handle):SetEndOfFile(Handle) != FALSE;
}

bool File::GetTime(LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime)
{
	return admin?Admin.fGetFileTime(Handle, CreationTime, LastAccessTime, LastWriteTime):GetFileTime(Handle, CreationTime, LastAccessTime, LastWriteTime) != FALSE;
}

bool File::SetTime(const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime)
{
	return admin?Admin.fSetFileTime(Handle, CreationTime, LastAccessTime, LastWriteTime):SetFileTime(Handle, CreationTime, LastAccessTime, LastWriteTime) != FALSE;
}

bool File::GetSize(UINT64& Size)
{
	return admin?Admin.fGetFileSizeEx(Handle, Size):apiGetFileSizeEx(Handle, Size);
}

bool File::FlushBuffers()
{
	return admin?Admin.fFlushFileBuffers(Handle):FlushFileBuffers(Handle) != FALSE;
}

bool File::GetInformation(BY_HANDLE_FILE_INFORMATION& info)
{
	return admin?Admin.fGetFileInformationByHandle(Handle, info):GetFileInformationByHandle(Handle, &info) != FALSE;
}

bool File::IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped)
{
	return admin?Admin.fDeviceIoControl(Handle, IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped):DeviceIoControl(Handle, IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped) != FALSE;
}

bool File::Close()
{
	bool Result=true;
	if(Handle!=INVALID_HANDLE_VALUE)
	{
		Result = admin?Admin.fCloseHandle(Handle):CloseHandle(Handle) != FALSE;
		Handle = INVALID_HANDLE_VALUE;
	}
	return Result;
}

bool File::Eof()
{
	return eof;
}

NTSTATUS GetLastNtStatus()
{
	return ifn.pfnRtlGetLastNtStatus?ifn.pfnRtlGetLastNtStatus():STATUS_SUCCESS;
}

BOOL apiDeleteFile(const wchar_t *lpwszFileName)
{
	string strNtName(NTPath(lpwszFileName).Str);
	BOOL Result = DeleteFile(strNtName);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Admin.fDeleteFile(strNtName);
	}
	return Result;
}

BOOL apiRemoveDirectory(const wchar_t *DirName)
{
	string strNtName(NTPath(DirName).Str);
	BOOL Result = RemoveDirectory(strNtName);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Admin.fRemoveDirectory(strNtName);
	}
	return Result;
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
	string strFrom(NTPath(lpwszExistingFileName).Str), strTo(NTPath(lpwszNewFileName).Str);
	BOOL Result = CopyFileEx(strFrom, strTo, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
	{
		Result = Admin.fCopyFileEx(strFrom, strTo, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
	return Result;
}

BOOL apiMoveFile(
    const wchar_t *lpwszExistingFileName, // address of name of the existing file
    const wchar_t *lpwszNewFileName   // address of new name for the file
)
{
	string strFrom(NTPath(lpwszExistingFileName).Str), strTo(NTPath(lpwszNewFileName).Str);
	BOOL Result = MoveFile(strFrom, strTo);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
	{
		Result = Admin.fMoveFileEx(strFrom, strTo, 0);
	}
	return Result;
}

BOOL apiMoveFileEx(
    const wchar_t *lpwszExistingFileName, // address of name of the existing file
    const wchar_t *lpwszNewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
)
{
	string strFrom(NTPath(lpwszExistingFileName).Str), strTo(NTPath(lpwszNewFileName).Str);
	BOOL Result = MoveFileEx(strFrom, strTo, dwFlags);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
	{
		Result = Admin.fMoveFileEx(strFrom, strTo, dwFlags);
	}
	return Result;
}


BOOL apiMoveFileThroughTemp(const wchar_t *Src, const wchar_t *Dest)
{
	string strTemp;
	BOOL rc = FALSE;

	if (FarMkTempEx(strTemp, nullptr, FALSE))
	{
		if (apiMoveFile(Src, strTemp))
			rc = apiMoveFile(strTemp, Dest);
	}

	return rc;
}

DWORD apiGetEnvironmentVariable(const wchar_t *lpwszName, string &strBuffer)
{
	int nSize = GetEnvironmentVariable(lpwszName, nullptr, 0);

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

void InitCurrentDirectory()
{
	//get real curdir:
	DWORD Size=GetCurrentDirectory(0,nullptr);
	string strInitCurDir;
	LPWSTR InitCurDir=strInitCurDir.GetBuffer(Size);
	GetCurrentDirectory(Size,InitCurDir);
	strInitCurDir.ReleaseBuffer(Size-1);
	//set virtual curdir:
	apiSetCurrentDirectory(strInitCurDir);
}

DWORD apiGetCurrentDirectory(string &strCurDir)
{
	//never give outside world a direct pointer to our internal string
	//who knows what they gonna do
	strCurDir.Copy(strCurrentDirectory().CPtr(),strCurrentDirectory().GetLength());
	return static_cast<DWORD>(strCurDir.GetLength());
}

BOOL apiSetCurrentDirectory(LPCWSTR lpPathName, bool Validate)
{
	// correct path to our standard
	string strDir=lpPathName;
	ReplaceSlashToBSlash(strDir);
	DeleteEndSlash(strDir);
	LPCWSTR CD=strDir;
	int Offset=HasPathPrefix(CD)?4:0;
	if ((CD[Offset] && CD[Offset+1]==L':' && !CD[Offset+2]) || IsLocalVolumeRootPath(CD))
		AddEndSlash(strDir);

	if (strDir == strCurrentDirectory())
		return TRUE;

	if (Validate)
	{
		string strDir=lpPathName;
		AddEndSlash(strDir);
		strDir+=L"*";
		FAR_FIND_DATA_EX fd;
		if (!apiGetFindDataEx(strDir, fd))
		{
			DWORD LastError = GetLastError();
			if(!(LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES))
				return FALSE;
		}
	}

	strCurrentDirectory()=strDir;

	// try to synchronize far cur dir with process cur dir
	if(CtrlObject && CtrlObject->Plugins.GetOemPluginsCount())
	{
		SetCurrentDirectory(strCurrentDirectory());
	}

	return TRUE;
}

DWORD apiGetTempPath(string &strBuffer)
{
	DWORD dwSize = GetTempPath(0, nullptr);
	wchar_t *lpwszBuffer = strBuffer.GetBuffer(dwSize);
	dwSize = GetTempPath(dwSize, lpwszBuffer);
	strBuffer.ReleaseBuffer();
	return dwSize;
};


DWORD apiGetModuleFileName(HMODULE hModule, string &strFileName)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszFileName = nullptr;

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
	DWORD length = ExpandEnvironmentStrings(strSrc, nullptr, 0);

	if (length)
	{
		wchar_t *lpwszDest = strDest.GetBuffer(length);
		ExpandEnvironmentStrings(strSrc, lpwszDest, length);
		strDest.ReleaseBuffer();
		length = (DWORD)strDest.GetLength();
	}

	return length;
}

DWORD apiWNetGetConnection(const wchar_t *lpwszLocalName, string &strRemoteName)
{
	DWORD dwRemoteNameSize = 0;
	DWORD dwResult = WNetGetConnection(lpwszLocalName, nullptr, &dwRemoteNameSize);

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
	wchar_t *lpwszVolumeName = pVolumeName?pVolumeName->GetBuffer(MAX_PATH+1):nullptr;  //MSDN!
	wchar_t *lpwszFileSystemName = pFileSystemName?pFileSystemName->GetBuffer(MAX_PATH+1):nullptr;
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

BOOL apiGetFindDataEx(const wchar_t *lpwszFileName, FAR_FIND_DATA_EX& FindData,bool ScanSymLink)
{
	FindFile Find(lpwszFileName, ScanSymLink);
	if(Find.Get(FindData))
	{
		return TRUE;
	}
	else if (!wcspbrk(lpwszFileName,L"*?"))
	{
		DWORD dwAttr=apiGetFileAttributes(lpwszFileName);

		if (dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			// Ага, значит файл таки есть. Заполним структуру ручками.
			FindData.Clear();
			FindData.dwFileAttributes=dwAttr;
			File file;
			if(file.Open(lpwszFileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
			{
				file.GetTime(&FindData.ftCreationTime,&FindData.ftLastAccessTime,&FindData.ftLastWriteTime);
				file.GetSize(FindData.nFileSize);
				file.Close();
			}

			if (FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
			{
				string strTmp;
				GetReparsePointInfo(lpwszFileName,strTmp,&FindData.dwReserved0); //MSDN
			}
			else
			{
				FindData.dwReserved0=0;
			}

			FindData.dwReserved1=0;
			FindData.strFileName=PointToName(lpwszFileName);
			ConvertNameToShort(lpwszFileName,FindData.strAlternateFileName);
			return TRUE;
		}
	}

	FindData.Clear();
	FindData.dwFileAttributes=INVALID_FILE_ATTRIBUTES; //BUGBUG

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

		if (DeviceIoControl(hFile,IOCTL_DISK_GET_LENGTH_INFO,nullptr,0,&gli,sizeof(gli),&BytesReturned,nullptr))
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
		ExitCode=RegEnumKeyEx(hKey,dwIndex,Name,&Size0,nullptr,nullptr,nullptr,lpftLastWriteTime);
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
	BOOL Res = apiGetVolumeInformation(strDrive, &strVolName, nullptr, &MaxComSize, &Flags, &strFS);
	return Res;
}

int apiGetFileTypeByName(const wchar_t *Name)
{
	HANDLE hFile=apiCreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0);

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

HANDLE apiFindFirstFileName(LPCWSTR lpFileName,DWORD dwFlags,string& strLinkName)
{
	HANDLE hRet=INVALID_HANDLE_VALUE;

	if (ifn.pfnFindFirstFileNameW)
	{
		DWORD StringLength=0;

		if (ifn.pfnFindFirstFileNameW(NTPath(lpFileName),0,&StringLength,nullptr)==INVALID_HANDLE_VALUE && GetLastError()==ERROR_MORE_DATA)
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

		if (!ifn.pfnFindNextFileNameW(hFindStream,&StringLength,nullptr) && GetLastError()==ERROR_MORE_DATA)
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
	string strNtName(NTPath(lpPathName).Str);
	BOOL Result = CreateDirectory(strNtName,lpSecurityAttributes);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Admin.fCreateDirectory(strNtName,lpSecurityAttributes);
	}
	return Result;
}

DWORD apiGetFileAttributes(LPCWSTR lpFileName)
{
	string strNtName(NTPath(lpFileName).Str);
	DWORD Result = GetFileAttributes(strNtName);
	if(Result == INVALID_FILE_ATTRIBUTES && ElevationRequired(ELEVATION_READ_REQUEST))
	{
		Result = Admin.fGetFileAttributes(strNtName);
	}
	return Result;
}

BOOL apiSetFileAttributes(LPCWSTR lpFileName,DWORD dwFileAttributes)
{
	string strNtName(NTPath(lpFileName).Str);
	BOOL Result = SetFileAttributes(strNtName, dwFileAttributes);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Admin.fSetFileAttributes(strNtName, dwFileAttributes);
	}
	return Result;

}

BOOL CreateSymbolicLinkInternal(LPCWSTR Object,LPCWSTR Target, DWORD dwFlags)
{
	return ifn.pfnCreateSymbolicLink?
		ifn.pfnCreateSymbolicLink(Object, Target, dwFlags):
		CreateReparsePoint(Target, Object, dwFlags&SYMBOLIC_LINK_FLAG_DIRECTORY?RP_SYMLINKDIR:RP_SYMLINKFILE);
}

BOOL apiCreateSymbolicLink(LPCWSTR lpSymlinkFileName,LPCWSTR lpTargetFileName,DWORD dwFlags)
{
	BOOL Result=FALSE;
	string strSymlinkFileName(NTPath(lpSymlinkFileName).Str);
	Result=CreateSymbolicLinkInternal(strSymlinkFileName, lpTargetFileName, dwFlags);
	if (!Result)
	{
		if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
		{
			Result=Admin.fCreateSymbolicLink(strSymlinkFileName, lpTargetFileName, dwFlags);
		}
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

bool CreateHardLinkInternal(LPCWSTR Object,LPCWSTR Target,LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	bool Result = CreateHardLink(Object, Target, SecurityAttributes) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Admin.fCreateHardLink(Object, Target, SecurityAttributes);
	}
	return Result;
}

BOOL apiCreateHardLink(LPCWSTR lpFileName,LPCWSTR lpExistingFileName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return CreateHardLinkInternal(NTPath(lpFileName),NTPath(lpExistingFileName),lpSecurityAttributes) ||
	       //bug in win2k: \\?\ fails
	       CreateHardLinkInternal(lpFileName,lpExistingFileName,lpSecurityAttributes);
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
			HANDLE hFile = apiCreateFile(lpFileName,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0);

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

				if (pStreamInfo->StreamNameLength && pStreamInfo->StreamNameLength < sizeof(pFsd->cStreamName))
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
		Ret=FindClose(hFindFile);
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

bool internalNtQueryGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	if (ifn.pfnNtQueryObject)
	{
		ULONG RetLen;
		ULONG BufSize = NT_MAX_PATH;
		OBJECT_NAME_INFORMATION* oni = reinterpret_cast<OBJECT_NAME_INFORMATION*>(xf_malloc(BufSize));
		NTSTATUS Res = ifn.pfnNtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);

		if (Res == STATUS_BUFFER_OVERFLOW || Res == STATUS_BUFFER_TOO_SMALL)
		{
			BufSize = RetLen;
			oni = reinterpret_cast<OBJECT_NAME_INFORMATION*>(xf_realloc_nomove(oni, BufSize));
			Res = ifn.pfnNtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);
		}

		string NtPath;

		if (Res == STATUS_SUCCESS)
		{
			NtPath.Copy(oni->Name.Buffer, oni->Name.Length / sizeof(WCHAR));
		}

		xf_free(oni);

		FinalFilePath.Clear();

		if (Res == STATUS_SUCCESS)
		{
			// try to convert NT path (\Device\HarddiskVolume1) to drive letter
			string DriveStrings;

			if (apiGetLogicalDriveStrings(DriveStrings))
			{
				wchar_t DiskName[3] = L"A:";
				const wchar_t* Drive = DriveStrings.CPtr();

				while (*Drive)
				{
					DiskName[0] = *Drive;
					int Len = MatchNtPathRoot(NtPath, DiskName);

					if (Len)
					{
						FinalFilePath = NtPath.Replace(0, Len, DiskName);
						break;
					}

					Drive += StrLength(Drive) + 1;
				}
			}

			if (FinalFilePath.IsEmpty())
			{
				// try to convert NT path (\Device\HarddiskVolume1) to \\?\Volume{...} path
				wchar_t VolumeName[cVolumeGuidLen + 1 + 1];
				HANDLE hEnum = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));
				BOOL Res = hEnum != INVALID_HANDLE_VALUE;

				while (Res)
				{
					if (StrLength(VolumeName) >= (int)cVolumeGuidLen)
					{
						DeleteEndSlash(VolumeName);
						int Len = MatchNtPathRoot(NtPath, VolumeName + 4 /* w/o prefix */);

						if (Len)
						{
							FinalFilePath = NtPath.Replace(0, Len, VolumeName);
							break;
						}
					}

					Res = FindNextVolumeW(hEnum, VolumeName, ARRAYSIZE(VolumeName));
				}

				if (hEnum != INVALID_HANDLE_VALUE)
					FindVolumeClose(hEnum);
			}
		}

		return !FinalFilePath.IsEmpty();
	}

	FinalFilePath.Clear();
	return false;
}

bool apiGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	if (ifn.pfnGetFinalPathNameByHandle)
	{
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

		return Len  && Len <= BufLen;
	}

	return internalNtQueryGetFinalPathNameByHandle(hFile, FinalFilePath);
}

bool apiSearchPath(const wchar_t *Path, const wchar_t *FileName, const wchar_t *Extension, string &strDest)
{
	DWORD dwSize = SearchPath(Path,FileName,Extension,0,nullptr,nullptr);

	if (dwSize)
	{
		wchar_t *lpwszFullName=strDest.GetBuffer(dwSize);
		dwSize = SearchPath(Path,FileName,Extension,dwSize,lpwszFullName,nullptr);
		strDest.ReleaseBuffer(dwSize);
		return true;
	}

	return false;
}

bool apiQueryDosDevice(const wchar_t *DeviceName, string &Path) {
	SetLastError(NO_ERROR);
	DWORD Res = QueryDosDeviceW(DeviceName, Path.GetBuffer(MAX_PATH), MAX_PATH);
	if (!Res && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		SetLastError(NO_ERROR);
		Res = QueryDosDeviceW(DeviceName, Path.GetBuffer(NT_MAX_PATH), NT_MAX_PATH);
	}
	if (!Res || GetLastError() != NO_ERROR)
		return false;	
	Path.ReleaseBuffer();
	return true;
}

bool apiGetVolumeNameForVolumeMountPoint(LPCWSTR VolumeMountPoint,string& strVolumeName)
{
	bool Result=false;
	WCHAR VolumeName[50];
	string strVolumeMountPoint(NTPath(VolumeMountPoint).Str);
	AddEndSlash(strVolumeMountPoint);
	if(GetVolumeNameForVolumeMountPoint(strVolumeMountPoint,VolumeName,ARRAYSIZE(VolumeName)))
	{
		strVolumeName=VolumeName;
		Result=true;
	}
	return Result;
}
