/*
flink.cpp

Куча разных функций по обработке Link`ов - Hard&Sym

*/

/* Revision: 1.24 01.10.2001 $ */

/*
Modify:
  01.10.2001 SVS
    ! FarGetRepasePointInfo -> FarGetRepa_R_sePointInfo
  27.09.2001 IS
    ! FarGetRepasePointInfo: выделяем столько памяти, сколько нужно
    - FarGetRepasePointInfo: использовали размер указателя, а не размер буфера
    - Левый размер при использовании strncpy
  26.09.2001 SVS
    ! В FarGetRepasePointInfo буфер выделется динамически (alloca)
  24.09.2001 SVS
    + FarGetRepasePointInfo - для FSF.
    ! уточнение для GetPathRoot(), если в качестве параметра передали
      полное наименование репасепоинта.
  11.09.2001 SVS
    ! для "Volume{" в функции GetPathRootOne() начнем просмотр с диска "A:"
  25.06.2001 IS
    ! Внедрение const
  01.06.2001 SVS
    + Добавки для вывода лога.
  30.05.2001 SVS
    + FarMkLink()
    ! Удалена за ненадобностью проверка на CREATE_JUNCTION (полностью)
  25.05.2001 SVS
    ! Удалена за ненадобностью проверка на CREATE_JUNCTION (осталась только
      для mount volume point
  22.05.2001 tran
    ! по результам прогона на CodeGuard
  06.05.2001 DJ
    ! перетрях #include
  28.04.2001 VVM
    ! GetSubstName() получает тип носителя
    + Обработка Opt.SubstNameRule
  25.04.2001 SVS
    + CreateVolumeMountPoint() - монтирование диска на файловую систему
  06.04.2001 SVS
    - В Win2K в корень папки "Program Files" не создавался HardLink (Alt-F6)
      Значит применяем "родную", готовую к употреблению функцию CreateHardLink()
    + CanCreateHardLinks() - проверка на вшивость.
  02.04.2001 SVS
    ! Уточнения для GetPathRoot[One]()
  14.03.2001 OT
    - В vc++ уже есть определение _REPARSE_GUID_DATA_BUFFER
  14.03.2001 SVS
    + Зарезервирован кусок кода для создания SymLink для каталогов
      в функции CreateJunctionPoint
  13.03.2001 SVS
    ! Добавлен необходимый код в функцию DeleteJunctionPoint()
  13.03.2001 SVS
    ! GetPathRoot переехала из strmix.cpp :-)
    + В функцию GetPathRoot добавлена обработка mounted volume
  01.02.2001 SKV
    - MAXPATH или _MAX_PATH вот в чём вопрос.
  26.01.2001 SVS
    - Бага в NT при удалении SUBST-дисков. В NT ЭТО должно выглядеть как
      '\??\K:\Foo'
  25.01.2001 SVS
    ! Функции GetSubstName и DelSubstDrive теперь нормально работают и для
      Windows98
  05.01.2001 SVS
    + Функция GetSubstName - переехала из mix.cpp
    + Функция DelSubstDrive - удаление Subst драйвера
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

#include "plugin.hpp"
#include "copy.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"

//#if defined(__BORLANDC__)
// current thread's ANSI code page
#define CP_THREAD_ACP             3

#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )
// Predefined reparse tags.
// These tags need to avoid conflicting with IO_REMOUNT defined in ntos\inc\io.h
#define IO_REPARSE_TAG_RESERVED_ZERO             (0)
#define IO_REPARSE_TAG_RESERVED_ONE              (1)
#define IO_REPARSE_TAG_MOUNT_POINT               (0xA0000003)

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
#define FSCTL_SET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // REPARSE_DATA_BUFFER,
#define FSCTL_GET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DELETE_REPARSE_POINT      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // REPARSE_DATA_BUFFER,

//#ifndef _MSC_VER
#ifndef REPARSE_GUID_DATA_BUFFER_HEADER_SIZE
typedef struct _REPARSE_GUID_DATA_BUFFER {
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;
  GUID   ReparseGuid;
  struct {
      BYTE   DataBuffer[1];
  } GenericReparseBuffer;
} REPARSE_GUID_DATA_BUFFER, *PREPARSE_GUID_DATA_BUFFER;

#define REPARSE_GUID_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, GenericReparseBuffer)
#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )
// REPARSE_DATA_BUFFER
//#endif
#endif // REPARSE_GUID_DATA_BUFFER_HEADER_SIZE

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

typedef BOOL (WINAPI *PDELETEVOLUMEMOUNTPOINT)(
          LPCTSTR lpszVolumeMountPoint);  // volume mount point path

typedef BOOL (WINAPI *PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)(
          LPCTSTR lpszVolumeMountPoint, // volume mount point or directory
          LPTSTR lpszVolumeName,        // volume name buffer
          DWORD cchBufferLength);       // size of volume name buffer

typedef BOOL (WINAPI *PSETVOLUMEMOUNTPOINT)(
          LPCTSTR lpszVolumeMountPoint, // mount point
          LPCTSTR lpszVolumeName);        // volume to be mounted

static PGETVOLUMENAMEFORVOLUMEMOUNTPOINT pGetVolumeNameForVolumeMountPoint=NULL;
static PDELETEVOLUMEMOUNTPOINT pDeleteVolumeMountPoint=NULL;
static PSETVOLUMEMOUNTPOINT pSetVolumeMountPoint=NULL;


/*
 SrcVolume
   Pointer to a string that indicates the volume mount point where the
   volume is to be mounted. This may be a root directory (X:\) or a
   directory on a volume (X:\mnt\). The string must end with a trailing
   backslash ('\').
*/
int WINAPI CreateVolumeMountPoint(LPCTSTR SrcVolume,LPCTSTR LinkFolder)
{
   BOOL bFlag;
   char Buf[1024];            // temporary buffer for volume name

   if(!pGetVolumeNameForVolumeMountPoint)
      pGetVolumeNameForVolumeMountPoint=(PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandle("KERNEL32"),"GetVolumeNameForVolumeMountPointA");
   if(!pSetVolumeMountPoint)
      pSetVolumeMountPoint=(PSETVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandle("KERNEL32"),"SetVolumeMountPointA");
   if(!pGetVolumeNameForVolumeMountPoint || !pSetVolumeMountPoint)
     return(3);

   // We should do some error checking on the inputs. Make sure
   // there are colons and backslashes in the right places, etc.
   if(pGetVolumeNameForVolumeMountPoint(
                SrcVolume, // input volume mount point or directory
                      Buf, // output volume name buffer
               sizeof(Buf) // size of volume name buffer
           ) != TRUE)
   {
      // printf( "Retrieving volume name for %s failed.\n", SrcVolume);
      return (1);
   }
   if(!pSetVolumeMountPoint(LinkFolder, // mount point
                               Buf)) // volume to be mounted
   {
     //printf ("Attempt to mount %s at %s failed.\n", SrcVolume, LinkFolder);
     return (2);
   }
   return (0);
}

BOOL WINAPI CreateJunctionPoint(LPCTSTR SrcFolder,LPCTSTR LinkFolder)
{
  if (!LinkFolder || !SrcFolder || !*LinkFolder || !*SrcFolder)
    return FALSE;

_SVS(SysLog("SrcFolder=%s",SrcFolder));
_SVS(SysLog("LinkFolder=%s",LinkFolder));

  char szDestDir[1024];
  if (SrcFolder[0] == '\\' && SrcFolder[1] == '?')
    strcpy(szDestDir, SrcFolder);
  else
  {
    LPTSTR pFilePart;
    char szFullDir[1024];

    strcpy(szDestDir, "\\??\\");
    if (!GetFullPathName(SrcFolder, 1024, szFullDir, &pFilePart) ||
      GetFileAttributes(szFullDir) == -1)
    {
      return FALSE;
    }
    strcat(szDestDir, szFullDir);
  }
_SVS(SysLog("szLinkDir=%s",szDestDir));

  char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = { 0 };
  TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;

  size_t cchDest = strlen(szDestDir) + 1;
  if (cchDest > 512) {
    return FALSE;
  }
  wchar_t wszDestMountPoint[512];
  if (!MultiByteToWideChar(CP_THREAD_ACP,
              MB_PRECOMPOSED,
              szDestDir,
              cchDest,
              wszDestMountPoint,
              cchDest))
  {
    return FALSE;
  }

  size_t nDestMountPointBytes = lstrlenW(wszDestMountPoint) * 2;
  rdb.ReparseTag           = IO_REPARSE_TAG_MOUNT_POINT;
  rdb.ReparseDataLength    = nDestMountPointBytes + 12;
  rdb.Reserved             = 0;
  rdb.SubstituteNameOffset = 0;
  rdb.SubstituteNameLength = nDestMountPointBytes;
  rdb.PrintNameOffset      = nDestMountPointBytes + 2;
  rdb.PrintNameLength      = 0;
  lstrcpyW(rdb.PathBuffer, wszDestMountPoint);


  HANDLE hDir=CreateFile(LinkFolder,GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,0);

  if (hDir == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return FALSE;
  }
  DWORD dwBytes;
#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE \
      FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset)
  if(!DeviceIoControl(hDir,
            FSCTL_SET_REPARSE_POINT,
            (LPVOID)&rdb,
            rdb.ReparseDataLength + TMN_REPARSE_DATA_BUFFER_HEADER_SIZE,
            NULL,
            0,
            &dwBytes,
            0))
  {
    CloseHandle(hDir);
    return 0;
  }
  CloseHandle(hDir);
  return TRUE;
}

BOOL WINAPI DeleteJunctionPoint(LPCTSTR szDir)
{
  HANDLE hDir=CreateFile(szDir,
          GENERIC_READ | GENERIC_WRITE,
          0,
          0,
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
          0);
  if (hDir == INVALID_HANDLE_VALUE)
  {
    return FALSE;
  }

  REPARSE_GUID_DATA_BUFFER rgdb = { 0 };
  rgdb.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
  DWORD dwBytes;
  const BOOL bOK =
    DeviceIoControl(hDir,
            FSCTL_DELETE_REPARSE_POINT,
            &rgdb,
            REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,
            NULL,
            0,
            &dwBytes,
            0);
  CloseHandle(hDir);
  return bOK != 0;
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

int WINAPI FarGetReparsePointInfo(const char *Src,char *Dest,int DestSize)
{
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT &&
     WinVer.dwMajorVersion >= 5 &&
     Src && *Src)
  {
    /* $ 27.09.2001 IS
       ! Выделяем столько памяти, сколько нужно.
       - Использовали размер указателя, а не размер буфера.
       - Указывали не верный размер для strncpy
    */
    int TempSize=Max((int)(strlen(Src)+1),DestSize);
    char *TempDest=(char *)alloca(TempSize);
    strcpy(TempDest,Src);
    AddEndSlash(TempDest);
    DWORD Size=GetJunctionPointInfo(TempDest,TempDest,TempSize);
    if(Size && Dest)
      strncpy(Dest,TempDest,DestSize-1);
    /* IS $ */
    return Size;
  }
  return 0;
}

/* $ 07.09.2000 SVS
   Функция GetNumberOfLinks тоже доступна плагинам :-)
*/
int WINAPI GetNumberOfLinks(const char *Name)
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
  char FileSource[NM],FileDest[NM];

//  ConvertNameToFull(Src,FileSource, sizeof(FileSource));
  if (ConvertNameToFull(Src,FileSource, sizeof(FileSource)) >= sizeof(FileSource)){
    return FALSE;
  }
//  ConvertNameToFull(Dest,FileDest, sizeof(FileDest));
  if (ConvertNameToFull(Dest,FileDest, sizeof(FileDest)) >= sizeof(FileDest)){
    return FALSE;
  }

  // этот кусок для Win2K
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    typedef BOOL (WINAPI *PCREATEHARDLINK)(
       LPCTSTR lpFileName,                         // new file name
       LPCTSTR lpExistingFileName,                 // extant file name
       LPSECURITY_ATTRIBUTES lpSecurityAttributes  // SD
    );
    static PCREATEHARDLINK PCreateHardLinkA=NULL;
    if(!PCreateHardLinkA)
      PCreateHardLinkA=(PCREATEHARDLINK)GetProcAddress(GetModuleHandle("KERNEL32"),"CreateHardLinkA");
    if(PCreateHardLinkA)
    {
      return PCreateHardLinkA(FileDest, FileSource, NULL) != 0;
    }
  }

  // все что ниже работает в NT4/2000
  struct CORRECTED_WIN32_STREAM_ID
  {
    DWORD          dwStreamId ;
    DWORD          dwStreamAttributes ;
    LARGE_INTEGER  Size ;
    DWORD          dwStreamNameSize ;
    WCHAR          cStreamName[ ANYSIZE_ARRAY ] ;
  } StreamId;

  WCHAR FileLink[NM];

  HANDLE hFileSource;

  DWORD dwBytesWritten;
  LPVOID lpContext;
  DWORD cbPathLen;
  DWORD StreamSize;

  BOOL bSuccess;

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

/* $ 05.01.2001 SVS
   Функция DelSubstDrive - удаление Subst драйвера
   Return: -1 - это либо не SUBST-драйвер, либо OS не та.
            0 - все удалено на ура
            1 - ошибка при удалении.
*/
int DelSubstDrive(char *DosDeviceName)
{
  char NtDeviceName[512]="\\??\\";
  int Pos=(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?4:0);
  if(GetSubstName(DRIVE_NOT_INIT, DosDeviceName,
                  NtDeviceName+Pos, sizeof(NtDeviceName)-Pos))
  {
    return !DefineDosDevice(DDD_RAW_TARGET_PATH|
                       DDD_REMOVE_DEFINITION|
                       DDD_EXACT_MATCH_ON_REMOVE,
                       DosDeviceName, NtDeviceName)?1:0;
  }
  return(-1);
}
/* SVS $ */

BOOL GetSubstName(int DriveType,char *LocalName,char *SubstName,int SubstSize)
{
  /* $28.04.2001 VVM
    + Обработка в зависимости от Opt.SubstNameRule
      битовая маска:
      0 - если установлен, то опрашивать сменные диски
      1 - если установлен, то опрашивать все остальные */
  if (DriveType!=DRIVE_NOT_INIT)
  {
    int DriveRemovable = (DriveType==DRIVE_REMOVABLE) || (DriveType==DRIVE_CDROM);
    if ((!(Opt.SubstNameRule & 1)) && (DriveRemovable))
      return(FALSE);
    if ((!(Opt.SubstNameRule & 2)) && (!DriveRemovable))
      return(FALSE);
  }
  /* VVM $ */
  char Name[NM*2]="";
  LocalName=CharUpper((LPTSTR)LocalName);
  if ((LocalName[0]>='A') && ((LocalName[0]<='Z')))
  {
    // ЭТО ОБЯЗАТЕЛЬНО, ИНАЧЕ В WIN98 РАБОТАТЬ НЕ БУДЕТ!!!!
/*$ 01.02.2001 skv
  Хоцца компилятся и под ВЦ++ однако.
*/
#ifdef _MSC_VER
    int SizeName=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?sizeof(Name):_MAX_PATH;
#else
    int SizeName=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?sizeof(Name):MAXPATH;
#endif
    /* skv$*/

    if (QueryDosDevice(LocalName,Name,SizeName) >= 3)
    {
      /* Subst drive format API differences:
       *   WinNT: \??\qualified_path (e.g. \??\C:\WinNT)
       *   Win98: qualified_path (e.g. C:\ or C:\Win98) */
      if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      {
        if (!strncmp(Name,"\\??\\",4))
        {
          strncpy(SubstName,Name+4,SubstSize-1);
          return TRUE;
        }
      }
      else
      {
        if(Name[1] == ':' && Name[2] == '\\')
        {
          strncpy(SubstName,Name,SubstSize-1);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

// просмотр одной позиции :-)
void GetPathRootOne(const char *Path,char *Root)
{
  char TempRoot[1024],*ChPtr;
  strncpy(TempRoot,Path,NM-1);

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    if(!pGetVolumeNameForVolumeMountPoint)
      // работает только под Win2000!
      pGetVolumeNameForVolumeMountPoint=(PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandle("KERNEL32"),"GetVolumeNameForVolumeMountPointA");

    // обработка mounted volume
    if(pGetVolumeNameForVolumeMountPoint && !strncmp(Path,"Volume{",7))
    {
      char Drive[] = "A:\\"; // \\?\Volume{...
      BOOL Res;
      int I;
      for (I = 'A'; I <= 'Z';  I++ )
      {
        Drive[0] = (char)I;
        if(pGetVolumeNameForVolumeMountPoint(
                  Drive, // input volume mount point or directory
                  TempRoot, // output volume name buffer
                  sizeof(TempRoot)) &&       // size of volume name buffer
           !stricmp(TempRoot+4,Path))
        {
           strcpy(Root,Drive);
           return;
        }
      }
      // Ops. Диск то не имеет буковки
      strcpy(Root,"\\\\?\\");
      strcat(Root,Path);
      return;
    }
  }

  if (*TempRoot==0)
    strcpy(TempRoot,"\\");
  else
    if (TempRoot[0]=='\\' && TempRoot[1]=='\\')
    {
      if ((ChPtr=strchr(TempRoot+2,'\\'))!=NULL)
        if ((ChPtr=strchr(ChPtr+1,'\\'))!=NULL)
          *(ChPtr+1)=0;
        else
          strcat(TempRoot,"\\");
    }
    else
      if ((ChPtr=strchr(TempRoot,'\\'))!=NULL)
        *(ChPtr+1)=0;
      else
        if ((ChPtr=strchr(TempRoot,':'))!=NULL)
          strcpy(ChPtr+1,"\\");
  strncpy(Root,TempRoot,NM-1);
}

// полный проход ПО!!!
void WINAPI GetPathRoot(const char *Path,char *Root)
{
  char TempRoot[1024], *TmpPtr;
  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    DWORD FileAttr;
    char JuncName[NM];

    strncpy(TempRoot,Path,sizeof(TempRoot)-1);
    TmpPtr=TempRoot;
    while(strlen(TempRoot) > 2)
    {
      FileAttr=GetFileAttributes(TempRoot);
      if(FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        if(GetJunctionPointInfo(TempRoot,JuncName,sizeof(JuncName)))
        {
           GetPathRootOne(JuncName+4,Root);
           return;
        }
      }
      char *Ptr=strrchr(TmpPtr,'\\');
      if(Ptr) *Ptr=0; else break;
    }
  }
  //GetPathRootOne(Path,Root);
  // Хмм... а ведь здесь может быть \\?\ и еже с ним
  GetPathRootOne(Path+((strlen(Path) > 4 && Path[0]=='\\' && Path[2]=='?' && Path[3]=='\\')?4:0),Root);
}

// Verify that both files are on the same NTFS disk
BOOL WINAPI CanCreateHardLinks(char *TargetFile,char *HardLinkName)
{
  char Root[2][512],FSysName[NM];
  GetPathRoot(TargetFile,Root[0]);
  GetPathRoot(HardLinkName,Root[1]);

   // same drive?
  if(!strcmp(Root[0],Root[1]))
  {
    // NTFS drive?
    DWORD FileSystemFlags;
    if(GetVolumeInformation(Root[0],NULL,0,NULL,NULL,&FileSystemFlags,FSysName,sizeof(FSysName)))
    {
      if(!strcmp(FSysName,"NTFS"))
        return TRUE;
    }
  }
  return FALSE;
}

int WINAPI FarMkLink(char *Src,char *Dest,DWORD Flags)
{
  if(Src && *Src && Dest && *Dest)
  {
//    int Delete=Flags&FLINK_DELETE;
    int Op=Flags&0xFFFF;

    switch(Op)
    {
      case FLINK_HARDLINK:
//        if(Delete)
//          return DeleteFile(Src);
//        else
          if(CanCreateHardLinks(Src,Dest))
            return MkLink(Src,Dest);
        break;

      case FLINK_SYMLINK:
      case FLINK_VOLMOUNT:
//        if(Delete)
//          return RemoveDirectory(Src);
//        else
          return ShellCopy::MkSymLink(Src,Dest,
             (Op==FLINK_VOLMOUNT?FCOPY_VOLMOUNT:FCOPY_LINK)|
             (Flags&FLINK_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
    }
  }
  return 0;
}
