/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций

*/

/* Revision: 1.02 01.03.2004 $ */

/*
Modify:
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
