/*
flink.cpp

Куча разных функций по обработке Link`ов - Hard&Sym

*/

/* Revision: 1.49 05.07.2006 $ */

/*
Modify:
  05.07.2006 IS
    - warnings
  06.05.2005 SVS
    ! У GetSubstName() предпоследний параметр может быть равен NULL
  24.04.2005 AY
    ! GCC
  04.11.2004 SVS
    ! избавимся от варнинга под VC
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  08.06.2004 SVS
    ! Вместо GetDriveType теперь вызываем FAR_GetDriveType().
    ! Вместо "DriveType==DRIVE_CDROM" вызываем IsDriveTypeCDROM()
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  25.02.2004 SVS
    + Новые сведения про _REPARSE_DATA_BUFFER ;-) (from DDK)
    - BugZ#581 - Неверно показывается свободное место
  10.10.2003 SVS
    - BugZ#970 - Сообщение об ошибке при создании хардлинка
  14.06.2003 IS
    ! CheckParseJunction -> IsLocalDrive (по смыслу содержимого функции)
      Функция проверяет - является ли диск локальным,
      возвращает 0 для нелокальных дисков.
      Если нет буквы диска, то диск считаем нелокальным.
      Функции неважно, полный ей путь передан или относительный.
    ! GetJunctionPointInfo, FarGetReparsePointInfo - для нелокальных
      дисков возвращает ошибку
  06.03.2003 SVS
    + _LOGCOPYR() - детальный лог процесса копирования
    ! наработки по вопросу о символических связях
  26.01.2003 IS
    ! FAR_DeleteFile вместо DeleteFile, FAR_RemoveDirectory вместо
      RemoveDirectory, просьба и впредь их использовать для удаления
      соответственно файлов и каталогов.
    ! FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
  12.07.2002 SVS
    ! Применяем CreateHardLink только для случая DRIVE_FIXED
      В остальных случаях - по старинке. ;-)
  13.06.2002 SVS
    - Если делать симлинк из-под SUBST-диска - траблы с именем
      (забыл в прошлый раз выставить разделитель '\')
  06.06.2002 VVM
    ! В функции GetPathRoot учтем UNC пути.
  31.05.2002 SVS
    - Бага при создании симлинка с subst-диска (проверим и поправим)
    + SetLastError в нужных местах
  30.05.2002 SVS
    - Ошибка создания симлинка ("Атака клоунов") - добавим обновление панелей
  25.04.2002 SVS
    - BugZ#466 - Ошибка создания симлинка (продолжение эпопеи)
  18.04.2002 SVS
    - BugZ#466 - Ошибка создания симлинка
      Так же, если не удавалось создать символическую связь... оставался
      созданный пустой каталог. Теперь этот каталог удаляется.
  22.03.2002 SVS
    - strcpy - Fuck!
  20.03.2002 SVS
    ! GetCurrentDirectory -> FarGetCurDir
  12.03.2002 SVS
    - Неверная работа GetPathRootOne в плане получения рута для пути типа "1\"
  14.12.2001 IS
    ! stricmp -> LocalStricmp
  17.10.2001 SVS
    ! Внедрение const
  16.10.2001 SVS
    + EnumNTFSStreams() - получить информацию о потоках
    ! У функции CanCreateHardLinks второй параметр можно не указывать (это
      позволяет проверять "а не NTFS ли ЭТО?"
    ! немного const-модификаторов
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

#if !defined(__GNUC__)

#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )
// Predefined reparse tags.
// These tags need to avoid conflicting with IO_REMOUNT defined in ntos\inc\io.h
#define IO_REPARSE_TAG_RESERVED_ZERO             (0)
#define IO_REPARSE_TAG_RESERVED_ONE              (1)
#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT               (0xA0000003)
#endif

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

#endif //!defined(__GNUC__)

//#ifndef _MSC_VER
#ifndef REPARSE_GUID_DATA_BUFFER_HEADER_SIZE
//   For non-Microsoft reparse point drivers, a fixed header must be
//used, with the remainder of the reparse point tag belonging to the
//driver itself. The format for this information is:
typedef struct _REPARSE_GUID_DATA_BUFFER {
  DWORD  ReparseTag;
  WORD   ReparseDataLength;
  WORD   Reserved;
/*
#define IO_REPARSE_TAG_IFSTEST_CONGRUENT        (0x00000009L)
#define IO_REPARSE_TAG_ARKIVIO                  (0x0000000CL)
#define IO_REPARSE_TAG_SOLUTIONSOFT             (0x2000000DL)
#define IO_REPARSE_TAG_COMMVAULT                (0x0000000EL)
*/
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

/*
// http://www.osr.com/ntinsider/2003/reparse.htm
// The format for Microsoft-defined reparse points (from ntifs.h) is:

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

*/
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

//_SVS(SysLog("SrcFolder ='%s'",SrcFolder));
//_SVS(SysLog("LinkFolder='%s'",LinkFolder));

  char szDestDir[1024];
  if (SrcFolder[0] == '\\' && SrcFolder[1] == '?')
    xstrncpy(szDestDir, SrcFolder,sizeof(szDestDir)-1);
  else
  {
    LPTSTR pFilePart;
    char szFullDir[1024];

    strcpy(szDestDir, "\\??\\");
    if (!GetFullPathName(SrcFolder, sizeof(szFullDir), szFullDir, &pFilePart) ||
      GetFileAttributes(szFullDir) == -1)
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return FALSE;
    }

    char *PtrFullDir=szFullDir;
    // проверка на subst
    if(IsLocalPath(szFullDir))
    {
      char LocalName[8], SubstName[NM];
      sprintf(LocalName,"%c:",*szFullDir);
      if(GetSubstName(DRIVE_NOT_INIT,LocalName,SubstName,sizeof(SubstName)))
      {
        strcat(szDestDir, SubstName);
        AddEndSlash(szDestDir);
        PtrFullDir=szFullDir+3;
      }
    }
    strcat(szDestDir, PtrFullDir);
  }

  char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = { 0 };
  TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;

  size_t cchDest = strlen(szDestDir) + 1;
  if (cchDest > 512) {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  FAR_OemToChar(szDestDir,szDestDir); // !!!
  wchar_t wszDestMountPoint[512];
  if (!MultiByteToWideChar(CP_THREAD_ACP,           // code page
                           MB_PRECOMPOSED,          // character-type options
                           szDestDir,               // string to map
                           cchDest,                 // number of bytes in string
                           wszDestMountPoint,       // wide-character buffer
                           cchDest))                // size of buffer

  {
    return FALSE;
  }

  //_SVS(SysLog("szDestDir ='%s'",szDestDir));
  size_t nDestMountPointBytes = lstrlenW(wszDestMountPoint) * 2;
  rdb.ReparseTag              = IO_REPARSE_TAG_MOUNT_POINT;
  rdb.ReparseDataLength       = nDestMountPointBytes + 12;
  rdb.Reserved                = 0;
  rdb.SubstituteNameOffset    = 0;
  rdb.SubstituteNameLength    = nDestMountPointBytes;
  rdb.PrintNameOffset         = nDestMountPointBytes + 2;
  rdb.PrintNameLength         = 0;
  lstrcpyW(rdb.PathBuffer, wszDestMountPoint);
  //_SVS(SysLogDump("rdb",0,szBuff,MAXIMUM_REPARSE_DATA_BUFFER_SIZE/3,0));


  HANDLE hDir=FAR_CreateFile(LinkFolder,GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,
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
    DWORD LastErr=GetLastError();
    CloseHandle(hDir);
    FAR_DeleteFile(LinkFolder); // А нужно ли убивать, когда создали каталог, но симлинк не удалось ???
    SetLastError(LastErr);
    return 0;
  }
  CloseHandle(hDir);
  return TRUE;
}

BOOL WINAPI DeleteJunctionPoint(LPCTSTR szDir)
{
  HANDLE hDir=FAR_CreateFile(szDir,
          GENERIC_READ | GENERIC_WRITE,
          0,
          0,
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
          0);
  if (hDir == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
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
  /* $ 14.06.2003 IS
     Для нелокальных дисков получить корректную информацию о связи
     не представляется возможным
  */
  if (FileAttr == 0xffffffff || !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)
      || !IsLocalDrive(szMountDir))
  /* IS $ */
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return 0;
  }

  HANDLE hDir=FAR_CreateFile(szMountDir,GENERIC_READ|0,0,0,OPEN_EXISTING,
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
  FAR_CharToOemBuff(szDestBuff,szDestBuff,dwBuffSize); // !!!
#endif
  return rdb.SubstituteNameLength / sizeof(TCHAR);
}

/* $ 14.06.2003 IS
   функция проверяет - является ли диск локальным,
   возвращает 0 для нелокальных дисков.
   Если нет буквы диска, то диск считаем нелокальным.
   Функции неважно, полный ей путь передан или относительный.
*/
int IsLocalDrive(const char *Path)
{
  // попытаемся получить полный путь
  char RootDir[MAX_PATH]="A:\\";
  DWORD DriveType = 0;

  if(IsLocalPath(Path))
  {
    RootDir[0] = Path[0];
    DriveType = FAR_GetDriveType(RootDir);
  }
  else
  {
    if(ConvertNameToFull(Path, RootDir, sizeof(RootDir)) >= sizeof(RootDir))
      return 0; // получили слишком длинный путь, будем считать, что диск нелокальный!
    if(IsLocalPath(RootDir))
    {
      RootDir[3] = 0;
      DriveType = FAR_GetDriveType(RootDir);
    }
  }

  return (DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_FIXED ||
    IsDriveTypeCDROM(DriveType) || DriveType == DRIVE_RAMDISK);
}
/* IS $ */

/* $ 07.09.2000 SVS
   Функция GetNumberOfLinks тоже доступна плагинам :-)
*/
int WINAPI GetNumberOfLinks(const char *Name)
{
  HANDLE hFile=FAR_CreateFile(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
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
int WINAPI MkLink(const char *Src,const char *Dest)
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

  BOOL bSuccess=FALSE;

  // этот кусок для Win2K
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT &&
     WinVer.dwMajorVersion >= 5
     //&& FAR_GetDriveType(FileSource) == DRIVE_FIXED
    )
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
      bSuccess=PCreateHardLinkA(FileDest, FileSource, NULL) != 0;
    }
  }

  if(bSuccess)
    return bSuccess;

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

  MultiByteToWideChar(CP_OEMCP,0,FileDest,-1,FileLink,sizeof(FileLink)/sizeof(FileLink[0]));

  hFileSource = FAR_CreateFile(FileSource,FILE_WRITE_ATTRIBUTES,
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

/*
  Функция EnumNTFSStreams позволяет получить количество потоков,
  привязанных к файлу FileName.
  Параметры:
   FileName - имя файла
   fpEnum   - функция, получающая 2 параметра - номер потока (Idx) и имя
              потока в Unicode. Перечисление прерывается, если функция
              вернет FALSE. Этот параметр может быть равен NULL.
              Простейший вариант CALLBACK-перечислителя:

              BOOL WINAPI EnumFileStreams(int Idx,WCHAR *StreamName,const WIN32_STREAM_ID *sid)
              {
                char Buf[260];
                UnicodeToAscii(StreamName,Buf,sizeof(Buf));
                printf("%2d) '%s' StreamId=%d",Idx,Buf,sid->dwStreamId);
                switch(sid->dwStreamId)
                {
                  case BACKUP_DATA: printf(" (Standard data)\n");break;
                  case BACKUP_EA_DATA: printf(" (Extended attribute data)\n");break;
                  case BACKUP_SECURITY_DATA: printf(" (Windows NT security descriptor data)\n");break;
                  case BACKUP_ALTERNATE_DATA: printf(" (Alternative data streams)\n");break;
                  case BACKUP_LINK: printf(" (Hard link information)\n");break;
                }
                return TRUE;
              }
   SizeStreams - указатель на паременную типа __int64, в которую будет помещен
                 общий размер всех потоков (актуально для копира!).
                 Параметр необязательный.

  Return: Количество потоков в файле.
          -1 - ошибка открытия файла
           0 - FileName - каталог и в нем еще нет потоков, по умолчанию
               каталоги не имеют потоков (файлы имеют один - {Data})
           1 - файл имеет основной поток данных...
          >1 - ... и несколько дополнительных.
*/
int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams)
{
  int StreamsCount=-1;

  HANDLE hFile = FAR_CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    // Prepare for execution
    LPVOID lpContext = NULL;
    WIN32_STREAM_ID sid;
    DWORD dwRead;
    WCHAR wszStreamName[MAX_PATH];

    StreamsCount=0;

    // Enumerate the streams...
    BOOL bContinue = TRUE;
    while (bContinue)
    {
      // Calculate the size of the variable length
      // WIN32_STREAM_ID structure
      memset(&sid,0,sizeof(WIN32_STREAM_ID));
      memset(wszStreamName,0,MAX_PATH);
      DWORD dwStreamHeaderSize = (LPBYTE)&sid.cStreamName -
           (LPBYTE)&sid+ sid.dwStreamNameSize;

      bContinue = BackupRead(hFile, (LPBYTE) &sid,
        dwStreamHeaderSize, &dwRead, FALSE, FALSE,
        &lpContext);

      if (!dwRead)
        break;
      if (dwRead != dwStreamHeaderSize)
        break;
      // At this point, we've read the header of the i.th
      // stream. What follows is the name of the stream and
      // next its content.

      // Read the stream name
      BackupRead(hFile,(LPBYTE)wszStreamName,sid.dwStreamNameSize,&dwRead,FALSE,FALSE,&lpContext);
      if (dwRead != sid.dwStreamNameSize)
        break;

      // A stream name is stored enclosed in a pair of colons
      // plus a $DATA default trailer. If the stream name is
      // VersionInfo it's stored (and retrieved) as:
      // :VersionInfo:$DATA
      if (wcslen(wszStreamName))
      {
        WCHAR Wsz[MAX_PATH], *pwsz=Wsz;
        lstrcpyW(pwsz, wszStreamName + sizeof(CHAR));
        LPWSTR wp = wcsstr(pwsz, L":");
        pwsz[wp-pwsz] = 0;
        lstrcpyW(wszStreamName, pwsz);
      }

      StreamsCount++;
      if(SizeStreams)
        *SizeStreams+=sid.Size.QuadPart;
      if(fpEnum && !fpEnum(StreamsCount,wszStreamName,&sid))
        break;

      // Skip the stream body
      DWORD dw1, dw2;
      if(!BackupSeek(hFile, sid.Size.u.LowPart, sid.Size.u.HighPart,&dw1, &dw2, &lpContext))
         break;
    }

    CloseHandle(hFile);
  }
  return StreamsCount;
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
#if defined(_MSC_VER) || defined(__GNUC__)
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
          if(SubstName)
            xstrncpy(SubstName,Name+4,SubstSize-1);
          return TRUE;
        }
      }
      else
      {
        if(Name[1] == ':' && Name[2] == '\\')
        {
          if(SubstName)
            xstrncpy(SubstName,Name,SubstSize-1);
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
  char TempRoot[2048],*ChPtr;
  xstrncpy(TempRoot,Path,NM-1);

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    if(!pGetVolumeNameForVolumeMountPoint)
      // работает только под Win2000!
      pGetVolumeNameForVolumeMountPoint=(PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandle("KERNEL32"),"GetVolumeNameForVolumeMountPointA");

    // обработка mounted volume
    if(pGetVolumeNameForVolumeMountPoint && !strncmp(Path,"Volume{",7))
    {
      char Drive[] = "A:\\"; // \\?\Volume{...
      int I;
      for (I = 'A'; I <= 'Z';  I++ )
      {
        Drive[0] = (char)I;
        if(pGetVolumeNameForVolumeMountPoint(
                  Drive, // input volume mount point or directory
                  TempRoot, // output volume name buffer
                  sizeof(TempRoot)) &&       // size of volume name buffer
           !LocalStricmp(TempRoot+4,Path))
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
  {
    // ..2 <> ...\2
    if(!PathMayBeAbsolute(TempRoot))
    {
      char Temp[2048];
      FarGetCurDir(sizeof(Temp)-2,Temp);
      AddEndSlash(Temp);
      strcat(Temp,TempRoot); //+(*TempRoot=='\\' || *TempRoot == '/'?1:0)); //??
      xstrncpy(TempRoot,Temp,sizeof(TempRoot)-1);
    }

    if (TempRoot[0]=='\\' && TempRoot[1]=='\\')
    {
      if ((ChPtr=strchr(TempRoot+2,'\\'))!=NULL)
        if ((ChPtr=strchr(ChPtr+1,'\\'))!=NULL)
          *(ChPtr+1)=0;
        else
          strcat(TempRoot,"\\");
    }
    else
    {
      if ((ChPtr=strchr(TempRoot,'\\'))!=NULL)
        *(ChPtr+1)=0;
      else
        if ((ChPtr=strchr(TempRoot,':'))!=NULL)
          strcpy(ChPtr+1,"\\");
    }
  }
  xstrncpy(Root,TempRoot,NM-1);
}

// полный проход ПО!!!
static void _GetPathRoot(const char *Path,char *Root,int Reenter)
{
  _LOGCOPYR(CleverSysLog Clev("GetPathRoot()"));
  _LOGCOPYR(SysLog("Params: Path='%s'",Path));
  char TempRoot[1024], *TmpPtr;
  char NewPath[2048];
  /* $ 06.06.2002 VVM
    ! Учтем UNC пути */
  int IsUNC = FALSE;
  int PathLen = strlen(Path);
  xstrncpy(NewPath, Path, sizeof(NewPath)-1);
  // Проверим имя на UNC
  if (PathLen > 2 && Path[0] == '\\' && Path[1] == '\\')
  {
    if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        PathLen > 3 && Path[2] == '?' && Path[3] == '\\')
    { // Проверим на длинное UNC имя под NT
      xstrncpy(NewPath, &Path[4], sizeof(NewPath)-1);
      if (PathLen > 8 && strncmp(NewPath, "UNC\\", 4)==0)
      {
        IsUNC = TRUE;
        xstrncpy(NewPath, "\\",  sizeof(NewPath)-1);
        strncat(NewPath, &Path[7], sizeof(NewPath)-1);
      }
    }
    else
      IsUNC = TRUE;
  }

  _LOGCOPYR(SysLog("%d NewPath='%s', IsUNC=%d",__LINE__,NewPath,IsUNC));
  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    _LOGCOPYR(CleverSysLog Clev("VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5"));
    DWORD FileAttr;
    char JuncName[NM];

    xstrncpy(TempRoot,NewPath,sizeof(TempRoot)-1);
    TmpPtr=TempRoot;

    char *CtlChar = NULL; // Указатель на начало реального пути в UNC. Без имени сервера
    if (IsUNC)
      CtlChar = strchr(TmpPtr+2,'\\');
    if (!IsUNC || CtlChar)
    {
      char *Ptr=strrchr(TmpPtr,'\\');
      while(Ptr >= CtlChar && strlen(TempRoot) > 2)
      {
        FileAttr=GetFileAttributes(TempRoot);
        _LOGCOPYR(SysLog("GetFileAttributes('%s')=0x%08X",TempRoot,FileAttr));
        if(FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
        {
          if(GetJunctionPointInfo(TempRoot,JuncName,sizeof(JuncName)))
          {
             if(!Reenter)
               _GetPathRoot(JuncName+4,Root,TRUE);
             else
               GetPathRootOne(JuncName+4,Root);
#if 0
             _LOGCOPYR(SysLog("afret  GetPathRootOne() Root='%s', JuncName='%s'",Root,JuncName));
               //CheckParseJunction('\\vr-srv002\userhome$\vskirdin\wwwroot')=2
               //return -> 952 Root='F:\', JuncName='\??\F:\wwwroot'
             if(TempRoot[0] == '\\' && TempRoot[1] == '\\' && IsLocalPath(Root))
             {
               char *Ptr=strchr(TempRoot+2,'\\');
               if(Ptr)
               {
                 JuncName[5]='$';
                 strcpy(Ptr+1,JuncName+4);
                 strcpy(Root,TempRoot);
               }
             }
#endif
             _LOGCOPYR(SysLog("return -> %d Root='%s', JuncName='%s'",__LINE__,Root,JuncName));
             return;
          }
        } /* if */
        if(Ptr) *Ptr=0; else break;
        Ptr=strrchr(TmpPtr,'\\');
      } /* while */
    } /* if */
  } /* if */
  _LOGCOPYR(SysLog("%d NewPath='%s'",__LINE__,NewPath));
  GetPathRootOne(NewPath,Root);
  _LOGCOPYR(SysLog("return -> %d Root='%s'",__LINE__,Root));
  // Хмм... а ведь здесь может быть \\?\ и еже с ним
  //GetPathRootOne(Path+((strlen(Path) > 4 && Path[0]=='\\' && Path[2]=='?' && Path[3]=='\\')?4:0),Root);
  /* VVM $ */
}

void WINAPI GetPathRoot(const char *Path,char *Root)
{
  _GetPathRoot(Path,Root,0);
}

int WINAPI FarGetReparsePointInfo(const char *Src,char *Dest,int DestSize)
{
  _LOGCOPYR(CleverSysLog Clev("FarGetReparsePointInfo()"));
  _LOGCOPYR(SysLog("Params: Src='%s'",Src));
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5 && Src && *Src)
  {
      char Src2[2048];
      xstrncpy(Src2,Src,sizeof(Src2)-1);
      /* $ 27.09.2001 IS
         ! Выделяем столько памяти, сколько нужно.
         - Использовали размер указателя, а не размер буфера.
         - Указывали не верный размер для xstrncpy
      */
      int TempSize=Max((int)(strlen(Src2)+1),DestSize);
      char *TempDest=(char *)alloca(TempSize);
      strcpy(TempDest,Src2);
      AddEndSlash(TempDest);
      DWORD Size=GetJunctionPointInfo(TempDest,TempDest,TempSize);
      // Src2='\\vr-srv002\userhome$\vskirdin\wwwroot', TempDest='\??\F:\wwwroot'
      _LOGCOPYR(SysLog("return -> %d Src2='%s', TempDest='%s'",__LINE__,Src2,TempDest));
#if 0
      if(Src2[0] == '\\' && Src2[1] == '\\' && IsLocalPath(TempDest+4))
      {
        char *Ptr=strchr(Src2+2,'\\');
        if(Ptr)
        {
          TempDest[5]='$';
          strcpy(Ptr+1,TempDest+4);
          strcpy(TempDest,"\\??\\");
          strcat(TempDest,Src2);
        }
      }
#endif
      if(Size && Dest)
        xstrncpy(Dest,TempDest,DestSize-1);
      /* IS $ */
//      _LOGCOPYR(SysLog("return -> %d Dest='%s'",__LINE__,Dest));
      return Size;
  }
  return 0;
}


// Verify that both files are on the same NTFS disk
BOOL WINAPI CanCreateHardLinks(const char *TargetFile,const char *HardLinkName)
{
  if(!TargetFile)
    return FALSE;

  char Root[2][512],FSysName[NM];
  GetPathRoot(TargetFile,Root[0]);
  if(HardLinkName)
    GetPathRoot(HardLinkName,Root[1]);
  else
    strcpy(Root[1],Root[0]);

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

int WINAPI FarMkLink(const char *Src,const char *Dest,DWORD Flags)
{
  int RetCode=0;

  if(Src && *Src && Dest && *Dest)
  {
//    int Delete=Flags&FLINK_DELETE;
    int Op=Flags&0xFFFF;

    switch(Op)
    {
      case FLINK_HARDLINK:
//        if(Delete)
//          RetCode=FAR_DeleteFile(Src);
//        else
          if(CanCreateHardLinks(Src,Dest))
            RetCode=MkLink(Src,Dest);
        break;

      case FLINK_SYMLINK:
      case FLINK_VOLMOUNT:
//        if(Delete)
//          RetCode=FAR_RemoveDirectory(Src);
//        else
          RetCode=ShellCopy::MkSymLink(Src,Dest,
             (Op==FLINK_VOLMOUNT?FCOPY_VOLMOUNT:FCOPY_LINK)|
             (Flags&FLINK_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
    }
  }

  if(RetCode && !(Flags&FLINK_DONOTUPDATEPANEL))
    ShellUpdatePanels(NULL,FALSE);

  return RetCode;
}
