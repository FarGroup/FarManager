/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"

/* $ 26.01.2003 IS
    + FAR_DeleteFile вместо DeleteFile, FAR_RemoveDirectory вместо
      RemoveDirectory, просьба и впредь их использовать для удаления
      соответственно файлов и каталогов.
*/
// удалить файл, код возврата аналогичен DeleteFile
BOOL WINAPI FAR_DeleteFile(const char *FileName)
{
  // IS: сначала попробуем удалить стандартной функцией, чтобы
  // IS: не осуществлять лишние телодвижения
  BOOL rc=DeleteFile(FileName);
  if(!rc) // IS: вот тут лишние телодвижения и начнем...
  {
    SetLastError((_localLastError = GetLastError()));
    if(CheckErrorForProcessed(_localLastError))
    {
      char FullName[NM*2]="\\\\?\\";
      // IS: +4 - чтобы не затереть наши "\\?\"
      if(ConvertNameToFull(FileName, FullName+4, sizeof(FullName)-4) < sizeof(FullName)-4)
      {
        // IS: проверим, а вдруг уже есть есть нужные символы в пути
        if( (FullName[4]=='/' && FullName[5]=='/') ||
            (FullName[4]=='\\' && FullName[5]=='\\') )
          rc=DeleteFile(FullName+4);
        // IS: нужных символов в пути нет, поэтому используем наши
        else
          rc=DeleteFile(FullName);
      }
    }
  }
  return rc;
}

// удалить каталог, код возврата аналогичен RemoveDirectory
BOOL WINAPI FAR_RemoveDirectory(const char *DirName)
{
  // IS: сначала попробуем удалить стандартной функцией, чтобы
  // IS: не осуществлять лишние телодвижения
  BOOL rc=RemoveDirectory(DirName);
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
  return rc;
}
/* IS $ */

/* $ 26.01.2003 IS
     + FAR_CreateFile - обертка для CreateFile, просьба использовать именно
       ее вместо CreateFile
*/
// открыть файл, вод возврата аналогичен CreateFile
HANDLE WINAPI FAR_CreateFile(
    LPCTSTR lpFileName,     // pointer to name of the file
    DWORD dwDesiredAccess,  // access (read-write) mode
    DWORD dwShareMode,      // share mode
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
    DWORD dwCreationDistribution, // how to create
    DWORD dwFlagsAndAttributes,   // file attributes
    HANDLE hTemplateFile          // handle to file with attributes to copy
   )
{
  HANDLE hFile=CreateFile(lpFileName,dwDesiredAccess,dwShareMode,
    lpSecurityAttributes, dwCreationDistribution,dwFlagsAndAttributes,
    hTemplateFile);
  return hFile;
}
/* IS $ */

void WINAPI SetFileApisTo(int Type)
{
  switch(Type)
  {
    case APIS2OEM:
      SetFileApisToOEM();
      break;
    case APIS2ANSI:
      SetFileApisToANSI();
      break;
  }
}

BOOL WINAPI FAR_OemToCharBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength)
{
  return OemToCharBuff(lpszSrc,lpszDst,cchDstLength);
}

BOOL WINAPI FAR_CharToOemBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength)
{
  return CharToOemBuff(lpszSrc,lpszDst,cchDstLength);
}


BOOL WINAPI FAR_OemToChar(LPCSTR lpszSrc,LPTSTR lpszDst)
{
  return OemToChar(lpszSrc,lpszDst);
}

BOOL WINAPI FAR_CharToOem(LPCSTR lpszSrc,LPTSTR lpszDst)
{
  return CharToOem(lpszSrc,lpszDst);
}

HANDLE FAR_FindFirstFile(const char *FileName,LPWIN32_FIND_DATA lpFindFileData,bool ScanSymLink)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile=FindFirstFile(FileName,&FindFileData);
	if(hFindFile==INVALID_HANDLE_VALUE && ScanSymLink)
	{
		char RealName[1024];
		ConvertNameToReal(FileName,RealName,sizeof(RealName));
		hFindFile=FindFirstFile(RealName,&FindFileData);
	}
	if(hFindFile!=INVALID_HANDLE_VALUE)
		*lpFindFileData=FindFileData;
	return hFindFile;
}

BOOL FAR_FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData)
{
	return FindNextFile(hFindFile,lpFindFileData);
}

BOOL FAR_FindClose(HANDLE hFindFile)
{
	return FindClose(hFindFile);
}

BOOL GetFileWin32FindData(const char *Name,WIN32_FIND_DATA *FInfo,bool ScanSymLink)
{
  WIN32_FIND_DATA WFD_Info;

  //UINT  PrevErrMode;
  // дабы не выскакивал гуевый диалог, если диск эжектед.
  //PrevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
  HANDLE FindHandle=FAR_FindFirstFile(Name,&WFD_Info,ScanSymLink);
  //SetErrorMode(PrevErrMode);
  if(FindHandle!=INVALID_HANDLE_VALUE)
  {
    FAR_FindClose(FindHandle);
    if(FInfo)
      *FInfo=WFD_Info;
    return TRUE;
  }
	else
	{
		DWORD dwAttr=GetFileAttributes(Name);
		if(dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			// Ага, значит файл таки есть. Заполним структуру ручками.
			if(FInfo)
			{
				memset(FInfo,0,sizeof(WIN32_FIND_DATA));
				FInfo->dwFileAttributes=dwAttr;
				HANDLE hFile=FAR_CreateFile(Name,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,(FInfo->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
				if(hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile,&FInfo->ftCreationTime,&FInfo->ftLastAccessTime,&FInfo->ftLastWriteTime);
					unsigned __int64 Size=0;
					FAR_GetFileSize (hFile,&Size);
					FInfo->nFileSizeHigh=(DWORD)(Size>>32);
					FInfo->nFileSizeLow=(DWORD)(Size&0xffffffff);
					CloseHandle(hFile);
				}
				if(FInfo->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
					GetReparsePointInfo(Name,NULL,0,&FInfo->dwReserved0); //MSDN
				else
					FInfo->dwReserved0=0;
				FInfo->dwReserved1=0;
				xstrncpy(FInfo->cFileName,PointToName(Name),sizeof(FInfo->cFileName));
				char ShortName[NM];
				ConvertNameToShort(Name,ShortName,sizeof(ShortName));
				xstrncpy(FInfo->cAlternateFileName,PointToName(ShortName),sizeof(FInfo->cAlternateFileName));
				return TRUE;
			}
		}
	}
	if(FInfo)
	{
		memset(FInfo,0,sizeof(WIN32_FIND_DATA));
		FInfo->dwFileAttributes=INVALID_FILE_ATTRIBUTES;
	}
  return FALSE;
}


BOOL FAR_CopyFile(
    LPCTSTR lpExistingFileName, // pointer to name of an existing file
    LPCTSTR lpNewFileName,  // pointer to filename to copy to
    BOOL bFailIfExists  // flag for operation if file exists
   )
{
  return CopyFile(lpExistingFileName,lpNewFileName,bFailIfExists);
}

typedef BOOL (WINAPI *COPYFILEEX)(LPCTSTR lpExistingFileName,
            LPCTSTR lpNewFileName,void *lpProgressRoutine,
            LPVOID lpData,LPBOOL pbCancel,DWORD dwCopyFlags);
static COPYFILEEX pCopyFileEx=NULL;

BOOL Init_CopyFileEx(void)
{
  static int LoadAttempt=FALSE;

  if (!LoadAttempt && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
  {
    HMODULE hKernel=GetModuleHandle("KERNEL32.DLL");
    if (hKernel)
      pCopyFileEx=(COPYFILEEX)GetProcAddress(hKernel,"CopyFileExA");
    IsFn_FAR_CopyFileEx=pCopyFileEx != NULL;
    LoadAttempt=TRUE;
  }
  return IsFn_FAR_CopyFileEx;
}

BOOL FAR_CopyFileEx(LPCTSTR lpExistingFileName,
            LPCTSTR lpNewFileName,void *lpProgressRoutine,
            LPVOID lpData,LPBOOL pbCancel,DWORD dwCopyFlags)
{
  if(pCopyFileEx)
    return pCopyFileEx(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
  return FALSE;
}

BOOL FAR_MoveFile(
    LPCTSTR lpExistingFileName, // address of name of the existing file
    LPCTSTR lpNewFileName   // address of new name for the file
   )
{
  return MoveFile(lpExistingFileName,lpNewFileName);
}

BOOL FAR_MoveFileEx(
    LPCTSTR lpExistingFileName, // address of name of the existing file
    LPCTSTR lpNewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
   )
{
  return MoveFileEx(lpExistingFileName,lpNewFileName,dwFlags);
}

BOOL MoveFileThroughTemp(const char *Src, const char *Dest)
{
  char Temp[NM];
  BOOL rc = FALSE;
  if(FarMkTempEx(Temp, NULL, FALSE))
  {
    if(MoveFile(Src, Temp))
      rc = MoveFile(Temp, Dest);
  }
  return rc;
}

BOOL WINAPI FAR_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer)
{
  typedef BOOL (WINAPI *PGlobalMemoryStatusEx)(LPMEMORYSTATUSEX lpBuffer);
  static PGlobalMemoryStatusEx pGlobalMemoryStatusEx=NULL;
  BOOL Ret=FALSE;

  if(!pGlobalMemoryStatusEx)
    pGlobalMemoryStatusEx = (PGlobalMemoryStatusEx)GetProcAddress(GetModuleHandle("KERNEL32"),"GlobalMemoryStatusEx");

  if(pGlobalMemoryStatusEx)
  {
    MEMORYSTATUSEX ms;
    ms.dwLength=sizeof(ms);
    Ret=pGlobalMemoryStatusEx(&ms);
    if(Ret)
      memcpy(lpBuffer,&ms,sizeof(ms));
  }
  else
  {
    MEMORYSTATUS ms;
    ms.dwLength=sizeof(ms);
    GlobalMemoryStatus(&ms);
    lpBuffer->dwLength=sizeof(MEMORYSTATUSEX);
    lpBuffer->dwMemoryLoad=ms.dwMemoryLoad;
    lpBuffer->ullTotalPhys           =(DWORDLONG)ms.dwTotalPhys;
    lpBuffer->ullAvailPhys           =(DWORDLONG)ms.dwAvailPhys;
    lpBuffer->ullTotalPageFile       =(DWORDLONG)ms.dwTotalPageFile;
    lpBuffer->ullAvailPageFile       =(DWORDLONG)ms.dwAvailPageFile;
    lpBuffer->ullTotalVirtual        =(DWORDLONG)ms.dwTotalVirtual;
    lpBuffer->ullAvailVirtual        =(DWORDLONG)ms.dwAvailVirtual;
    lpBuffer->ullAvailExtendedVirtual=0;
    Ret=TRUE;
  }
  return Ret;
}

BOOL FAR_GetFileSize (HANDLE hFile, unsigned __int64 *pSize)
{
  DWORD dwHi, dwLo;

  dwLo = GetFileSize (hFile, &dwHi);

  int nError = GetLastError();
  SetLastError (nError);

  if ( (dwLo == INVALID_FILE_SIZE) && (nError != NO_ERROR) )
  {
    if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      #if defined(__BORLANDC__)
      #define IOCTL_DISK_GET_LENGTH_INFO          CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)
      // The structure GET_LENGTH_INFORMATION is used with the ioctl
      // IOCTL_DISK_GET_LENGTH_INFO to obtain the length, in bytes, of the
      // disk, partition, or volume.
      //

      typedef struct _GET_LENGTH_INFORMATION {
          LARGE_INTEGER   Length;
      } GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;
      #endif
      GET_LENGTH_INFORMATION gli;
      DWORD BytesReturned;
      if(DeviceIoControl(hFile,IOCTL_DISK_GET_LENGTH_INFO,NULL,0,&gli,sizeof(gli),&BytesReturned,NULL))
      {
        if ( pSize )
          *pSize=gli.Length.QuadPart;
        return TRUE;
      }
    }
    return FALSE;
  }
  else
  {
    if ( pSize )
    *pSize = dwHi*_ui64(0x100000000)+dwLo;

    return TRUE;
  }
}

BOOL WINAPI FAR_SetFilePointerEx(HANDLE hFile,LARGE_INTEGER liDistanceToMove,PLARGE_INTEGER lpNewFilePointer,DWORD dwMoveMethod)
{
  typedef BOOL (WINAPI *PSetFilePointerEx)(HANDLE hFile,LARGE_INTEGER liDistanceToMove,PLARGE_INTEGER lpNewFilePointer,DWORD dwMoveMethod);
  static PSetFilePointerEx pSetFilePointerEx=NULL;

  if(!pSetFilePointerEx)
    pSetFilePointerEx=(PSetFilePointerEx)GetProcAddress(GetModuleHandleW(L"KERNEL32.DLL"),"SetFilePointerEx");

  if(pSetFilePointerEx)
  {
    return pSetFilePointerEx(hFile,liDistanceToMove,lpNewFilePointer,dwMoveMethod);
  }
  else
  {
    LONG HighPart=liDistanceToMove.u.HighPart;
    DWORD LowPart=SetFilePointer(hFile,liDistanceToMove.u.LowPart,&HighPart,dwMoveMethod);
    if(LowPart==INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR)
      return FALSE;
    if(lpNewFilePointer)
    {
      lpNewFilePointer->u.HighPart=HighPart;
      lpNewFilePointer->u.LowPart=LowPart;
    }
    return TRUE;
  }
}

BOOL WINAPI FAR_GetUserNameEx(int NameFormat,LPSTR lpNameBuffer,PULONG nSize)
{
  typedef BOOL (WINAPI * PGETUSERNAMEEX)(int NameFormat,LPSTR lpNameBuffer,PULONG nSize);
  static PGETUSERNAMEEX pGetUserNameEx=NULL;

  if(!pGetUserNameEx)
    pGetUserNameEx=(PGETUSERNAMEEX)GetProcAddress(GetModuleHandle("secur32.dll"),"GetUserNameExA");

  BOOL Result=TRUE;
  if(pGetUserNameEx)
    Result=pGetUserNameEx(NameFormat,lpNameBuffer,nSize);

  if(!pGetUserNameEx || !Result)
    Result=GetUserName(lpNameBuffer,nSize);

  return Result;
}
