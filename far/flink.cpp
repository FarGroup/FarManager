/*
flink.cpp

���� ������ ������� �� ��������� Link`�� - Hard&Sym

*/

/* Revision: 1.62 25.08.2006 $ */

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
          const wchar_t *lpwszVolumeMountPoint);  // volume mount point path

typedef BOOL (WINAPI *PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)(
          const wchar_t *lpwszVolumeMountPoint, // volume mount point or directory
          wchar_t *lpwszVolumeName,        // volume name buffer
          DWORD cchBufferLength);       // size of volume name buffer

typedef BOOL (WINAPI *PSETVOLUMEMOUNTPOINT)(
          const wchar_t *lpwszVolumeMountPoint, // mount point
          const wchar_t *lpwszVolumeName);        // volume to be mounted


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

int WINAPI CreateVolumeMountPointW(const wchar_t *SrcVolume, const wchar_t *LinkFolder)
{
   wchar_t Buf[50]; //MS says 50           // temporary buffer for volume name

   if(!pGetVolumeNameForVolumeMountPoint)
      pGetVolumeNameForVolumeMountPoint=(PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandleW(L"KERNEL32"),"GetVolumeNameForVolumeMountPointW");

   if(!pSetVolumeMountPoint)
      pSetVolumeMountPoint=(PSETVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandleW(L"KERNEL32"),"SetVolumeMountPointW");

   if(!pGetVolumeNameForVolumeMountPoint || !pSetVolumeMountPoint)
     return(3);

   // We should do some error checking on the inputs. Make sure
   // there are colons and backslashes in the right places, etc.
   if( pGetVolumeNameForVolumeMountPoint(SrcVolume, Buf, 50) != TRUE)
      return (1);

   if(!pSetVolumeMountPoint(LinkFolder, Buf)) // volume to be mounted
     return (2);

   return (0);
}


BOOL WINAPI CreateJunctionPointW(const wchar_t *SrcFolder, const wchar_t *LinkFolder)
{
  if (!LinkFolder || !SrcFolder || !*LinkFolder || !*SrcFolder)
    return FALSE;

  string strDestDir;

  if (SrcFolder[0] == L'\\' && SrcFolder[1] == L'?')
     strDestDir = SrcFolder;
  else
  {
    string strFullDir;

    strDestDir = L"\\??\\";

    ConvertNameToFullW (SrcFolder, strFullDir); //??? ���� GetFullPathName

    if ( GetFileAttributesW (strFullDir) == -1 )
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return FALSE;
    }

    const wchar_t *PtrFullDir=(const wchar_t*)strFullDir;
    // �������� �� subst
    if(IsLocalPathW (strFullDir))
    {
      wchar_t LocalName[8];

      string strSubstName;

      swprintf (LocalName,L"%c:",*(const wchar_t*)strFullDir);

      if(GetSubstNameW(DRIVE_NOT_INIT,LocalName, strSubstName))
      {
        strDestDir += strSubstName;
        AddEndSlashW(strDestDir);
        PtrFullDir=(const wchar_t*)strFullDir+3;
      }
    }
    strDestDir += PtrFullDir;
  }

  wchar_t wszBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof (wchar_t)] = { 0 };
  TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)wszBuff;

  size_t nDestMountPointBytes = strDestDir.GetLength()*sizeof (wchar_t);
  rdb.ReparseTag              = IO_REPARSE_TAG_MOUNT_POINT;
  rdb.ReparseDataLength       = nDestMountPointBytes + 12;
  rdb.Reserved                = 0;
  rdb.SubstituteNameOffset    = 0;
  rdb.SubstituteNameLength    = nDestMountPointBytes;
  rdb.PrintNameOffset         = nDestMountPointBytes + 2;
  rdb.PrintNameLength         = 0;
  wcscpy (rdb.PathBuffer, strDestDir);


  HANDLE hDir=FAR_CreateFileW(LinkFolder,GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,
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
    FAR_DeleteFileW(LinkFolder); // � ����� �� �������, ����� ������� �������, �� ������� �� ������� ???
    SetLastError(LastErr);
    return 0;
  }
  CloseHandle(hDir);
  return TRUE;
}


BOOL WINAPI DeleteJunctionPointW(const wchar_t *szDir)
{
  HANDLE hDir=FAR_CreateFileW(szDir,
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



DWORD WINAPI GetJunctionPointInfoW (const wchar_t *szMountDir, string &strDestBuff)
{
  const DWORD FileAttr = GetFileAttributesW(szMountDir);
  /* $ 14.06.2003 IS
     ��� ����������� ������ �������� ���������� ���������� � �����
     �� �������������� ���������
  */
  if (FileAttr == 0xffffffff || !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)
      || !IsLocalDriveW(szMountDir))
  /* IS $ */
  {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return 0;
  }

  HANDLE hDir=FAR_CreateFileW(szMountDir,GENERIC_READ|0,0,0,OPEN_EXISTING,
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

  strDestBuff = rdb.PathBuffer;
  return rdb.SubstituteNameLength / sizeof(TCHAR);
}



int IsLocalDriveW(const wchar_t *Path)
{
  DWORD DriveType = 0;
  wchar_t *lpwszRootDir,wszRootDir[8]=L"A:\\";

  if(IsLocalPathW(Path))
  {
    lpwszRootDir = wszRootDir;
    lpwszRootDir[0] = Path[0];

    DriveType = FAR_GetDriveTypeW(lpwszRootDir);
  }
  else
  {
    string strRootDir;
    ConvertNameToFullW(Path, strRootDir);

    if(IsLocalPathW(strRootDir))
    {
        lpwszRootDir = strRootDir.GetBuffer ();
        lpwszRootDir[3] = 0;
        strRootDir.ReleaseBuffer ();

      DriveType = FAR_GetDriveTypeW(lpwszRootDir);
    }
  }

  return (DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_FIXED ||
    IsDriveTypeCDROM(DriveType) || DriveType == DRIVE_RAMDISK);
}



int WINAPI GetNumberOfLinksW(const wchar_t *Name)
{
  HANDLE hFile=FAR_CreateFileW(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(1);
  BY_HANDLE_FILE_INFORMATION bhfi;
  int GetCode=GetFileInformationByHandle(hFile,&bhfi);
  CloseHandle(hFile);
  return(GetCode ? bhfi.nNumberOfLinks:0);
}

/* SVS $*/



int WINAPI MkLinkW(const wchar_t *Src,const wchar_t *Dest)
{
  string strFileSource,strFileDest;

  ConvertNameToFullW(Src,strFileSource);
  ConvertNameToFullW(Dest,strFileDest);

  BOOL bSuccess=FALSE;

  // ���� ����� ��� Win2K
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT &&
     WinVer.dwMajorVersion >= 5
     //&& FAR_GetDriveType(FileSource) == DRIVE_FIXED
    )
  {
    typedef BOOL (WINAPI *PCREATEHARDLINKW)(
       const wchar_t *lpFileName,                         // new file name
       const wchar_t *lpExistingFileName,                 // extant file name
       LPSECURITY_ATTRIBUTES lpSecurityAttributes  // SD
    );
    static PCREATEHARDLINKW PCreateHardLinkW=NULL;
    if(!PCreateHardLinkW)
      PCreateHardLinkW=(PCREATEHARDLINKW)GetProcAddress(GetModuleHandleW(L"KERNEL32"),"CreateHardLinkW");
    if(PCreateHardLinkW)
    {
      bSuccess=PCreateHardLinkW(strFileDest, strFileSource, NULL) != 0;
    }
  }

  if(bSuccess)
    return bSuccess;

  // ��� ��� ���� �������� � NT4/2000
  struct CORRECTED_WIN32_STREAM_ID
  {
    DWORD          dwStreamId ;
    DWORD          dwStreamAttributes ;
    LARGE_INTEGER  Size ;
    DWORD          dwStreamNameSize ;
    WCHAR          cStreamName[ ANYSIZE_ARRAY ] ;
  } StreamId;

  string strFileLink;

  HANDLE hFileSource;

  DWORD dwBytesWritten;
  LPVOID lpContext;
  DWORD cbPathLen;
  DWORD StreamSize;

  strFileLink = strFileDest;

  hFileSource = FAR_CreateFileW(strFileSource,FILE_WRITE_ATTRIBUTES,
                FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

  if(hFileSource == INVALID_HANDLE_VALUE)
    return(FALSE);

  lpContext = NULL;
  cbPathLen = (strFileLink.GetLength() + 1) * sizeof(WCHAR);

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
    bSuccess = BackupWrite(hFileSource,(LPBYTE)(const wchar_t*)strFileLink,cbPathLen,
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
  ������� EnumNTFSStreams ��������� �������� ���������� �������,
  ����������� � ����� FileName.
  ���������:
   FileName - ��� �����
   fpEnum   - �������, ���������� 2 ��������� - ����� ������ (Idx) � ���
              ������ � Unicode. ������������ �����������, ���� �������
              ������ FALSE. ���� �������� ����� ���� ����� NULL.
              ���������� ������� CALLBACK-�������������:

              BOOL WINAPI EnumFileStreams(int Idx,WCHAR *StreamName,const WIN32_STREAM_ID *sid)
              {
                char Buf[260];
                UnicodeToANSI(StreamName,Buf,sizeof(Buf));
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
   SizeStreams - ��������� �� ���������� ���� __int64, � ������� ����� �������
                 ����� ������ ���� ������� (��������� ��� ������!).
                 �������� ��������������.

  Return: ���������� ������� � �����.
          -1 - ������ �������� �����
           0 - FileName - ������� � � ��� ��� ��� �������, �� ���������
               �������� �� ����� ������� (����� ����� ���� - {Data})
           1 - ���� ����� �������� ����� ������...
          >1 - ... � ��������� ��������������.
*/

/*
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

*/
#if defined(__BORLANDC__)
#pragma option -a.
#endif

/* $ 05.01.2001 SVS
   ������� DelSubstDrive - �������� Subst ��������
   Return: -1 - ��� ���� �� SUBST-�������, ���� OS �� ��.
            0 - ��� ������� �� ���
            1 - ������ ��� ��������.
*/
int DelSubstDrive(const wchar_t *DosDeviceName)
{
  string strNtDeviceName;
  if(GetSubstNameW(DRIVE_NOT_INIT, DosDeviceName, strNtDeviceName))
  {
      if ( WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT )
          strNtDeviceName = (string)L"\\??\\"+strNtDeviceName;

    return !DefineDosDeviceW(DDD_RAW_TARGET_PATH|
                       DDD_REMOVE_DEFINITION|
                       DDD_EXACT_MATCH_ON_REMOVE,
                       DosDeviceName, strNtDeviceName)?1:0;
  }
  return(-1);
}
/* SVS $ */

BOOL GetSubstNameW (int DriveType,const wchar_t *LocalName, string &strSubstName)
{
  /* $28.04.2001 VVM
    + ��������� � ����������� �� Opt.SubstNameRule
      ������� �����:
      0 - ���� ����������, �� ���������� ������� �����
      1 - ���� ����������, �� ���������� ��� ��������� */
  if (DriveType!=DRIVE_NOT_INIT)
  {
    int DriveRemovable = (DriveType==DRIVE_REMOVABLE) || (DriveType==DRIVE_CDROM);
    if ((!(Opt.SubstNameRule & 1)) && (DriveRemovable))
      return(FALSE);
    if ((!(Opt.SubstNameRule & 2)) && (!DriveRemovable))
      return(FALSE);
  }

  string strLocalName = LocalName;

  /* VVM $ */
  wchar_t Name[NM*2]=L""; //BUGBUG

  strLocalName.Upper ();
  if ((strLocalName.At(0)>=L'A') && ((strLocalName.At(0)<=L'Z')))
  {
    // ��� �����������, ����� � WIN98 �������� �� �����!!!!

#if defined(_MSC_VER) || defined(__GNUC__)
    int SizeName=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?sizeof(Name)/sizeof (wchar_t):_MAX_PATH;
#else
    int SizeName=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?sizeof(Name)/sizeof (wchar_t):MAXPATH;
#endif
    /* skv$*/

    if (QueryDosDeviceW(strLocalName,Name,SizeName) >= 3)
    {
      /* Subst drive format API differences:
       *   WinNT: \??\qualified_path (e.g. \??\C:\WinNT)
       *   Win98: qualified_path (e.g. C:\ or C:\Win98) */
      if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      {
        if (!wcsncmp(Name,L"\\??\\",4))
        {
          strSubstName = Name+4;
          return TRUE;
        }
      }
      else
      {
        if(Name[1] == L':' && Name[2] == L'\\')
        {
          strSubstName = Name;
          //FAR_OemToChar(SubstName,SubstName); // Mantis#224 ???????????????????????
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

void GetPathRootOneW(const wchar_t *Path,string &strRoot)
{
  string strTempRoot;
  wchar_t *ChPtr;

  strTempRoot = Path;

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    if(!pGetVolumeNameForVolumeMountPoint)
      // �������� ������ ��� Win2000!
      pGetVolumeNameForVolumeMountPoint=(PGETVOLUMENAMEFORVOLUMEMOUNTPOINT)GetProcAddress(GetModuleHandleW(L"KERNEL32"),"GetVolumeNameForVolumeMountPointW");

    // ��������� mounted volume
    if(pGetVolumeNameForVolumeMountPoint && !wcsncmp(Path,L"Volume{",7))
    {
      wchar_t Drive[] = L"A:\\"; // \\?\Volume{...
      int I;
      for (I = L'A'; I <= L'Z';  I++ )
      {
        Drive[0] = (wchar_t)I;

        wchar_t *TempRoot = strTempRoot.GetBuffer (1024); //BUGBUGBUG

        if(pGetVolumeNameForVolumeMountPoint(
                  Drive, // input volume mount point or directory
                  TempRoot, // output volume name buffer
                  1024) &&       // size of volume name buffer
           !LocalStricmpW(TempRoot+4,Path))
        {
           strTempRoot.ReleaseBuffer ();

           strRoot = Drive;
           return;
        }

        strTempRoot.ReleaseBuffer ();
      }
      // Ops. ���� �� �� ����� �������
      strRoot = L"\\\\?\\";
      strRoot += Path;
      return;
    }
  }

  if ( strTempRoot.IsEmpty() )
    strTempRoot = L"\\";
  else
  {
    // ..2 <> ...\2
    if(!PathMayBeAbsoluteW(strTempRoot))
    {
      string strTemp;
      FarGetCurDirW(strTemp);
      AddEndSlashW(strTemp);
      strTemp += strTempRoot; //+(*TempRoot=='\\' || *TempRoot == '/'?1:0)); //??
      strTempRoot = strTemp;
    }

    wchar_t *TempRoot = strTempRoot.GetBuffer ();

    if (TempRoot[0]==L'\\' && TempRoot[1]==L'\\')
    {
      if ((ChPtr=wcschr(TempRoot+2,L'\\'))!=NULL)
        if ((ChPtr=wcschr(ChPtr+1,L'\\'))!=NULL)
          *(ChPtr+1)=0;
        else
          wcscat(TempRoot,L"\\");
    }
    else
    {
      if ((ChPtr=wcschr(TempRoot,'\\'))!=NULL)
        *(ChPtr+1)=0;
      else
        if ((ChPtr=wcschr(TempRoot,L':'))!=NULL)
          wcscpy(ChPtr+1,L"\\");
    }

    strTempRoot.ReleaseBuffer ();
  }
  strRoot = strTempRoot;
}


// ������ ������ ��!!!
static void _GetPathRootW(const wchar_t *Path, string &strRoot, int Reenter)
{
  string strTempRoot;
  string strNewPath;

  wchar_t *TmpPtr;

  int IsUNC = FALSE;
  int PathLen = wcslen(Path);

  strNewPath = Path;
  // �������� ��� �� UNC
  if (PathLen > 2 && Path[0] == L'\\' && Path[1] == L'\\')
  {
    if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        PathLen > 3 && Path[2] == L'?' && Path[3] == L'\\')
    { // �������� �� ������� UNC ��� ��� NT
      strNewPath = &Path[4];
      if (PathLen > 8 && wcsncmp(strNewPath, L"UNC\\", 4)==0)
      {
        IsUNC = TRUE;
        strNewPath = L"\\";
        strNewPath += &Path[7];
      }
    }
    else
      IsUNC = TRUE;
  }

  if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    DWORD FileAttr;
    string strJuncName;

    strTempRoot = strNewPath;
    TmpPtr=strTempRoot.GetBuffer ();
    wchar_t *TempRoot = TmpPtr;

    wchar_t *CtlChar = NULL; // ��������� �� ������ ��������� ���� � UNC. ��� ����� �������
    if (IsUNC)
      CtlChar = wcschr(TmpPtr+2,L'\\');
    if (!IsUNC || CtlChar)
    {
      wchar_t *Ptr=wcsrchr(TmpPtr,L'\\');
      while(Ptr >= CtlChar && wcslen(TempRoot) > 2)
      {
        FileAttr=GetFileAttributesW(TempRoot);

        if(FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
        {
          if(GetJunctionPointInfoW(TempRoot,strJuncName))
          {
             if(!Reenter)
               _GetPathRootW((const wchar_t*)strJuncName+4,strRoot,TRUE);
             else
               GetPathRootOneW((const wchar_t*)strJuncName+4,strRoot);

             strTempRoot.ReleaseBuffer ();
             return;
          }
        } /* if */
        if(Ptr) *Ptr=0; else break;
        Ptr=wcsrchr(TmpPtr,L'\\');
      } /* while */
    } /* if */

    strTempRoot.ReleaseBuffer ();
  } /* if */
  GetPathRootOneW(strNewPath, strRoot);
}


void WINAPI GetPathRootW(const wchar_t *Path,string &strRoot)
{
  _GetPathRootW(Path,strRoot,0);
}

/*
int WINAPI FarGetReparsePointInfo(const char *Src,char *Dest,int DestSize)
{
  _LOGCOPYR(CleverSysLog Clev("FarGetReparsePointInfo()"));
  _LOGCOPYR(SysLog("Params: Src='%s'",Src));
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5 && Src && *Src)
  {
      char Src2[2048];
      xstrncpy(Src2,Src,sizeof(Src2)-1);
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
      return Size;
  }
  return 0;
}
*/



BOOL WINAPI CanCreateHardLinksW(const wchar_t *TargetFile,const wchar_t *HardLinkName)
{
  if(!TargetFile)
    return FALSE;

  string strRoot1;
  string strRoot2;
  string strFSysName;

  GetPathRootW(TargetFile,strRoot1);

  if(HardLinkName)
    GetPathRootW(HardLinkName,strRoot2);
  else
    strRoot2 = strRoot1;

   // same drive?
  if( !wcscmp(strRoot1, strRoot2))
  {
    // NTFS drive?
    DWORD FileSystemFlags;
    if(apiGetVolumeInformation (strRoot1,NULL,NULL,NULL,&FileSystemFlags,&strFSysName))
    {
      if(!wcscmp(strFSysName,L"NTFS"))
        return TRUE;
    }
  }
  return FALSE;
}


int WINAPI FarMkLinkW(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
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
          if(CanCreateHardLinksW(Src,Dest))
            RetCode=MkLinkW(Src,Dest);
        break;

      case FLINK_SYMLINK:
      case FLINK_VOLMOUNT:
//        if(Delete)
//          RetCode=FAR_RemoveDirectory(Src);
//        else
          RetCode=ShellCopy::MkSymLinkW(Src,Dest,
             (Op==FLINK_VOLMOUNT?FCOPY_VOLMOUNT:FCOPY_LINK)|
             (Flags&FLINK_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
    }
  }

  if(RetCode && !(Flags&FLINK_DONOTUPDATEPANEL))
    ShellUpdatePanels(NULL,FALSE);

  return RetCode;
}
