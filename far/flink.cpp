/*
flink.cpp

Куча разных функций по обработке Link`ов - Hard&Sym

*/

/* Revision: 1.03 04.01.2001 $ */

/*
Modify:
  05.01.2000 OT 
    - Косметика, из-за которой не компилился под VC :) 
  04.01.2001 SVS
    + Заглушки для CreateJunctionPoint, DeleteJunctionPoint
    + GetJunctionPointInfo - получить инфу про Junc
  03.01.2001 SVS
    ! Выделение в качестве самостоятельного модуля
    + GetNumberOfLinks и MkLink переехали из mix.cpp
*/

#include "headers.hpp"
#pragma hdrstop

#include "internalheaders.hpp"


//#if defined(__BORLANDC__)
// current thread's ANSI code page
  #define CP_THREAD_ACP             3           

  #define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )
  // Predefined reparse tags.
  // These tags need to avoid conflicting with IO_REMOUNT defined in ntos\inc\io.h
  #define IO_REPARSE_TAG_RESERVED_ZERO             (0)
  #define IO_REPARSE_TAG_RESERVED_ONE              (1)

  // The value of the following constant needs to satisfy the following conditions:
  //  (1) Be at least as large as the largest of the reserved tags.
  //  (2) Be strictly smaller than all the tags in use.
  #define IO_REPARSE_TAG_RESERVED_RANGE            IO_REPARSE_TAG_RESERVED_ONE
  // The following constant represents the bits that are valid to use in
  // reparse tags.
  #define IO_REPARSE_TAG_VALID_VALUES     (0xE000FFFF)
  // Macro to determine whether a reparse tag is a valid tag.
  #define IsReparseTagValid(_tag) (                               \
                    !((_tag) & ~IO_REPARSE_TAG_VALID_VALUES) &&   \
                    ((_tag) > IO_REPARSE_TAG_RESERVED_RANGE)      \
                   )
  #define FILE_FLAG_OPEN_REPARSE_POINT    0x00200000
  #define FSCTL_GET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS) 
// REPARSE_DATA_BUFFER
//#endif
struct TMN_REPARSE_DATA_BUFFER
{
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;

  // IO_REPARSE_TAG_MOUNT_POINT specifics follow
  WORD   SubstituteNameOffset;
  WORD   SubstituteNameLength;
  WORD   PrintNameOffset;
  WORD   PrintNameLength;
  WCHAR  PathBuffer[1];

  // Some helper functions
  //BOOL Init(LPCSTR szJunctionPoint);
  //BOOL Init(LPCWSTR wszJunctionPoint);
  //int BytesForIoControl() const;
};


BOOL WINAPI CreateJunctionPoint(LPCTSTR szMountDir, LPCTSTR szDestDirArg)
{
  return TRUE;
}

BOOL WINAPI DeleteJunctionPoint(LPCTSTR szDir)
{
  return TRUE;
}

DWORD WINAPI GetJunctionPointInfo(LPCTSTR szMountDir,
              LPTSTR  szDestBuff,
              DWORD   dwBuffSize)
{
  const DWORD FileAttr = GetFileAttributes(szMountDir);
  if (FileAttr == 0xffffffff || !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return 0;
  }

  HANDLE hDir=CreateFile(szMountDir,GENERIC_READ|0,0,0,OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,0);

  if (hDir == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return 0;
  }

  char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;

  DWORD dwBytesReturned;
  if (!DeviceIoControl(hDir,
            FSCTL_GET_REPARSE_POINT,
            NULL,
            0,
            (LPVOID)&rdb,
            MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
            &dwBytesReturned,
            0) ||
    !IsReparseTagValid(rdb.ReparseTag))
  {
    CloseHandle(hDir);
    return 0;
  }

  CloseHandle(hDir);

  if (dwBuffSize < rdb.SubstituteNameLength / sizeof(TCHAR) + sizeof(TCHAR))
  {
    return rdb.SubstituteNameLength / sizeof(TCHAR) + sizeof(TCHAR);
  }

#ifdef UNICODE
  lstrcpy(szDestBuff, rdb.PathBuffer);
#else
  if (!WideCharToMultiByte(CP_THREAD_ACP,
              0,
              rdb.PathBuffer,
              rdb.SubstituteNameLength / sizeof(WCHAR) + 1,
              szDestBuff,
              dwBuffSize,
              "",
              FALSE))
  {
    //printf("WideCharToMultiByte failed (%d)\n", GetLastError());
    return 0;
  }
#endif
  return rdb.SubstituteNameLength / sizeof(TCHAR);
}


/* $ 07.09.2000 SVS
   Функция GetNumberOfLinks тоже доступна плагинам :-)
*/
int WINAPI GetNumberOfLinks(char *Name)
{
  HANDLE hFile=CreateFile(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(1);
  BY_HANDLE_FILE_INFORMATION bhfi;
  int GetCode=GetFileInformationByHandle(hFile,&bhfi);
  CloseHandle(hFile);
  return(GetCode ? bhfi.nNumberOfLinks:0);
}
/* SVS $*/


#if defined(__BORLANDC__)
#pragma option -a4
#endif
int WINAPI MkLink(char *Src,char *Dest)
{
  struct CORRECTED_WIN32_STREAM_ID
  {
    DWORD          dwStreamId ;
    DWORD          dwStreamAttributes ;
    LARGE_INTEGER  Size ;
    DWORD          dwStreamNameSize ;
    WCHAR          cStreamName[ ANYSIZE_ARRAY ] ;
  } StreamId;

  char FileSource[NM],FileDest[NM];
  WCHAR FileLink[NM];

  HANDLE hFileSource;

  DWORD dwBytesWritten;
  LPVOID lpContext;
  DWORD cbPathLen;
  DWORD StreamSize;

  BOOL bSuccess;

//  ConvertNameToFull(Src,FileSource, sizeof(FileSource));
  if (ConvertNameToFull(Src,FileSource, sizeof(FileSource)) >= sizeof(FileSource)){
    return FALSE;
  }
//  ConvertNameToFull(Dest,FileDest, sizeof(FileDest));
  if (ConvertNameToFull(Dest,FileDest, sizeof(FileDest)) >= sizeof(FileDest)){
    return FALSE;
  }
  MultiByteToWideChar(CP_OEMCP,0,FileDest,-1,FileLink,sizeof(FileLink)/sizeof(FileLink[0]));

  hFileSource = CreateFile(FileSource,FILE_WRITE_ATTRIBUTES,
                FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

  if(hFileSource == INVALID_HANDLE_VALUE)
    return(FALSE);

  lpContext = NULL;
  cbPathLen = (lstrlenW(FileLink) + 1) * sizeof(WCHAR);

  StreamId.dwStreamId = BACKUP_LINK;
  StreamId.dwStreamAttributes = 0;
  StreamId.dwStreamNameSize = 0;
  StreamId.Size.u.HighPart = 0;
  StreamId.Size.u.LowPart = cbPathLen;

  StreamSize=sizeof(StreamId)-sizeof(WCHAR **)+StreamId.dwStreamNameSize;

  bSuccess = BackupWrite(hFileSource,(LPBYTE)&StreamId,StreamSize,
             &dwBytesWritten,FALSE,FALSE,&lpContext);

  int LastError=0;

  if (bSuccess)
  {
    bSuccess = BackupWrite(hFileSource,(LPBYTE)FileLink,cbPathLen,
                &dwBytesWritten,FALSE,FALSE,&lpContext);
    if (!bSuccess)
      LastError=GetLastError();

    BackupWrite(hFileSource,NULL,0,&dwBytesWritten,TRUE,FALSE,&lpContext);
  }
  else
    LastError=GetLastError();

  CloseHandle(hFileSource);

  if (LastError)
    SetLastError(LastError);

  return(bSuccess);
}
#if defined(__BORLANDC__)
#pragma option -a.
#endif

