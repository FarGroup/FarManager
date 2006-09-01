/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций

*/

/* Revision: 1.11 28.08.2006 $ */

/*
Modify:
  28.08.2006 SVS
    ! уточнение GetFileWin32FindData - дабы не выскакивал гуевый диалог, если диск эжектед.
  10.04.2006 SVS
    + BOOL WINAPI FAR_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer)
  19.06.2005 SVS
    - BugZ#1348 - Не обновляется индикатор копирования при операциях с флешкой
  14.06.2005 SVS
    + GetFileWin32FindData(), FAR_CopyFile(), FAR_CopyFileEx(), FAR_MoveFile(), FAR_MoveFileEx(), MoveFileThroughTemp()
  21.06.2004 SVS
    ! FAR_GetDriveType и IsDriveTypeCDROM умчали в cddrv.cpp
  14.06.2004 SVS
    ! добавим вариант, когда не получилось определить (нехватка прав на доступ к девайсу) - в этом случае
      максимум, что сможем определить - это DVD или нет.
  09.06.2004 SVS
    ! Попался привод - DVD читает, но не писатель (не CD-RW) - изменена логика.
    + работаем в NT-based (проверено так же на NT4 SP6a)
  09.06.2004 SVS
    - Вот ить.... забыл, что у GetDriveType параметр может быть равен NULL.
  08.06.2004 SVS
    ! Вместо GetDriveType теперь вызываем FAR_GetDriveType().
    ! Вместо "DriveType==DRIVE_CDROM" вызываем IsDriveTypeCDROM()
  01.03.2004 SVS
    + Обертки FAR_OemTo* и FAR_CharTo* вокруг WinAPI
    + FAR_ANSI - руками не мацать (уж больно трудно синхронизацией заниматься)
      на "сейчас" влияния не окажет, зато потом...
  09.10.2003 SVS
    + SetFileApisTo() с параметром APIS2ANSI или APIS2OEM вместо SetFileApisToANSI() и SetFileApisToOEM()
  01.06.2003 SVS
    ! Выделение в качестве самостоятельного модуля
    ! FAR_DeleteFile и FAR_RemoveDirectory переехали из delete.cpp в farwinapi.cpp
    ! FAR_CreateFile переехал из farrtl.cpp в farwinapi.cpp
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"

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
#if defined(FAR_ANSI)
  if(Opt.FarAnsi)
  {
    if(lpszDst != lpszSrc)
      memmove(lpszDst,lpszSrc,cchDstLength);
    return TRUE;
  }
#endif
  return OemToCharBuff(lpszSrc,lpszDst,cchDstLength);
}

BOOL WINAPI FAR_CharToOemBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength)
{
#if defined(FAR_ANSI)
  if(Opt.FarAnsi)
  {
    if(lpszDst != lpszSrc)
      memmove(lpszDst,lpszSrc,cchDstLength);
    return TRUE;
  }
#endif
  return CharToOemBuff(lpszSrc,lpszDst,cchDstLength);
}


BOOL WINAPI FAR_OemToChar(LPCSTR lpszSrc,LPTSTR lpszDst)
{
#if defined(FAR_ANSI)
  if(Opt.FarAnsi)
  {
    if(lpszDst != lpszSrc)
      memmove(lpszDst,lpszSrc,strlen(lpszSrc)+1);
    return TRUE;
  }
#endif
  return OemToChar(lpszSrc,lpszDst);
}

BOOL WINAPI FAR_CharToOem(LPCSTR lpszSrc,LPTSTR lpszDst)
{
#if defined(FAR_ANSI)
  if(Opt.FarAnsi)
  {
    if(lpszDst != lpszSrc)
      memmove(lpszDst,lpszSrc,strlen(lpszSrc)+1);
    return TRUE;
  }
#endif
  return CharToOem(lpszSrc,lpszDst);
}

BOOL GetFileWin32FindData(const char *Name,WIN32_FIND_DATA *FInfo)
{
  WIN32_FIND_DATA WFD_Info;

  UINT  PrevErrMode;
  // дабы не выскакивал гуевый диалог, если диск эжектед.
  PrevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
  HANDLE FindHandle=FindFirstFile(Name,&WFD_Info);
  SetErrorMode(PrevErrMode);
  if(FindHandle!=INVALID_HANDLE_VALUE)
  {
    FindClose(FindHandle);
    if(FInfo)
      memmove(FInfo,&WFD_Info,sizeof(WIN32_FIND_DATA));
    return TRUE;
  }
  else if(FInfo)
  {
    memset(FInfo,0,sizeof(WIN32_FIND_DATA));
    FInfo->dwFileAttributes=(DWORD)-1;
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
