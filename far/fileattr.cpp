/*
fileattr.cpp

Работа с атрибутами файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"

typedef BOOL (WINAPI *PEncryptFileW)(const wchar_t *lpwszFileName);
typedef BOOL (WINAPI *PDecryptFileW)(const wchar_t *lpwszFileName, DWORD dwReserved);

static PEncryptFileW pEncryptFileW=NULL;
static PDecryptFileW pDecryptFileW=NULL;

static int SetFileEncryptionW(const wchar_t *Name,int State);
static int SetFileCompressionW(const wchar_t *Name,int State);

// получим функции криптования
int GetEncryptFunctions(void)
{
  if(!pEncryptFileW)
  {
    // работает только под Win2000! Если не 2000, то не надо и показывать эту опцию.
    pEncryptFileW = (PEncryptFileW)GetProcAddress(GetModuleHandleW(L"KERNEL32.DLL"), "EncryptFileW");
    if(!pEncryptFileW)
      pEncryptFileW = (PEncryptFileW)GetProcAddress(GetModuleHandleW(L"ADVAPI32.DLL"), "EncryptFileW");
  }

  if(!pDecryptFileW)
  {
    pDecryptFileW = (PDecryptFileW)GetProcAddress(GetModuleHandleW(L"KERNEL32.DLL"), "DecryptFileW");
    if(!pDecryptFileW)
      pDecryptFileW = (PDecryptFileW)GetProcAddress(GetModuleHandleW(L"ADVAPI32.DLL"), "DecryptFileW");
  }

  if(pDecryptFileW && pEncryptFileW)
    IsCryptFileASupport=TRUE;

  return IsCryptFileASupport;
}

// Возвращает 0 - ошибка, 1 - Ок, 2 - Skip
int ESetFileAttributesW(const wchar_t *Name,int Attr)
{
//_SVS(SysLog(L"Attr=0x%08X",Attr));
  while (!SetFileAttributesW(Name,Attr))
  {
    int Code=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),
             UMSG(MSetAttrCannotFor),Name,UMSG(MHRetry),UMSG(MHSkip),UMSG(MHCancel));
    if (Code==1 || Code<0)
      return 2;
    if (Code==2)
      return 0;
  }
  return 1;
}


static int SetFileCompressionW(const wchar_t *Name,int State)
{
  HANDLE hFile=FAR_CreateFileW(Name,FILE_READ_DATA|FILE_WRITE_DATA,
                 FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                 FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FALSE);
  USHORT NewState=State ? COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
  UDWORD Result;
  int RetCode=DeviceIoControl(hFile,FSCTL_SET_COMPRESSION,&NewState,
                              sizeof(NewState),NULL,0,&Result,NULL);
  CloseHandle(hFile);
  return(RetCode);
}


int ESetFileCompressionW(const wchar_t *Name,int State,int FileAttr)
{
  if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
    return 1;

  int Ret=1;
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr & ~(FA_RDONLY|FILE_ATTRIBUTE_SYSTEM));

  // Drop Encryption
  if ((FileAttr & FILE_ATTRIBUTE_ENCRYPTED) && State)
    SetFileEncryptionW(Name,0);

  while (!SetFileCompressionW(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
    {
      Ret=1;
      break;
    }
    int Code=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),
                UMSG(MSetAttrCompressedCannotFor),Name,UMSG(MHRetry),
                UMSG(MHSkip),UMSG(MHCancel));
    if (Code==1 || Code<0)
    {
      Ret=2;
      break;
    }
    if (Code==2)
    {
      Ret=0;
      break;
    }
  }
  // Set ReadOnly
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr);
  return(Ret);
}


static int SetFileEncryptionW(const wchar_t *Name,int State)
{
  // заодно и проверяется успешность получения адреса API...
  if(State)
     return pEncryptFileW ? (*pEncryptFileW)(Name) : FALSE;
  else
     return pDecryptFileW ? (*pDecryptFileW)(Name, 0) : FALSE;
}


int ESetFileEncryptionW(const wchar_t *Name,int State,int FileAttr,int Silent)
{
  if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
    return 1;

  if(!IsCryptFileASupport)
    return 1;

  int Ret=1;

  // Drop ReadOnly
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr & ~(FA_RDONLY|FILE_ATTRIBUTE_SYSTEM));

  while (!SetFileEncryptionW(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
      break;

    if(Silent)
    {
      Ret=0;
      break;
    }

    int Code=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),
                UMSG(MSetAttrEncryptedCannotFor),Name,UMSG(MHRetry), //BUGBUG
                UMSG(MHSkip),UMSG(MHCancel));
    if (Code==1 || Code<0)
    {
      Ret=2;
      break;
    }
    if (Code==2)
    {
      Ret=0;
      break;
    }
  }

  // Set ReadOnly
  if (FileAttr & (FA_RDONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr);

  return(Ret);
}


int ESetFileTimeW(const wchar_t *Name,FILETIME *LastWriteTime,FILETIME *CreationTime,
                  FILETIME *LastAccessTime,int FileAttr)
{
  if (LastWriteTime==NULL && CreationTime==NULL && LastAccessTime==NULL ||
      ((FileAttr & FA_DIREC) && WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT))
    return 1;

  while (1)
  {
    if (FileAttr & FA_RDONLY)
      SetFileAttributesW(Name,FileAttr & ~FA_RDONLY);

    HANDLE hFile=FAR_CreateFileW(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                 NULL,OPEN_EXISTING,
                 (FileAttr & FA_DIREC) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
    int SetTime;
    DWORD LastError=0;
    if (hFile==INVALID_HANDLE_VALUE)
    {
      SetTime=FALSE;
      LastError=GetLastError();
    }
    else
    {
      SetTime=SetFileTime(hFile,CreationTime,LastAccessTime,LastWriteTime);
      LastError=GetLastError();
      CloseHandle(hFile);
    }

    if (FileAttr & FA_RDONLY)
      SetFileAttributesW(Name,FileAttr);
    SetLastError(LastError);

    if (SetTime)
      break;
    int Code=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),
                UMSG(MSetAttrTimeCannotFor),Name,UMSG(MHRetry), //BUGBUG
                UMSG(MHSkip),UMSG(MHCancel));
    if (Code<0)
      return 0; //???
    if(Code == 1)
      return 2;
    if(Code == 2)
      return 0;
  }
  return 1;
}
