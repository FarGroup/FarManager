/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций

*/

/* Revision: 1.04 09.06.2004 $ */

/*
Modify:
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


UINT FAR_GetDriveType(LPCTSTR RootDir)
{
  UINT DrvType = GetDriveType(RootDir);

  // анализ CD-привода
  if (RootDir && IsLocalPath(RootDir) && DrvType == DRIVE_CDROM && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
#if defined(__BORLANDC__)
//#pragma option push -b -a4 -pc -A- /*P_O_Push*/
#pragma option -a4
#elif defined(_MSC_VER)
#pragma pack(push,4)
#endif

    typedef long LONG_PTR, *PLONG_PTR;
    typedef unsigned long ULONG_PTR, *PULONG_PTR;

    //#ifndef _NTDDSCSIH_
    typedef struct _SCSI_PASS_THROUGH {
        USHORT Length;
        UCHAR ScsiStatus;
        UCHAR PathId;
        UCHAR TargetId;
        UCHAR Lun;
        UCHAR CdbLength;
        UCHAR SenseInfoLength;
        UCHAR DataIn;
        ULONG DataTransferLength;
        ULONG TimeOutValue;
        ULONG_PTR DataBufferOffset;
        ULONG SenseInfoOffset;
        UCHAR Cdb[16];
    } SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;

    #define SCSI_IOCTL_DATA_IN           1
    #define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER

    //
    // NtDeviceIoControlFile IoControlCode values for this device.
    //
    // Warning:  Remember that the low two bits of the code specify how the
    //           buffers are passed to the driver!
    //

    #define IOCTL_SCSI_PASS_THROUGH         CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
    #define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
    //#endif

    typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
        SCSI_PASS_THROUGH Spt;
        ULONG             Filler;      // realign buffers to double word boundary
        UCHAR             SenseBuf[32];
        UCHAR             DataBuf[512];
    } SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;


    // Command Descriptor Block constants.
    #define CDB6GENERIC_LENGTH         6

    // SCSI CDB operation codes
    #define SCSIOP_INQUIRY             0x12
    #define SCSIOP_MODE_SENSE          0x1A
    #define MODE_PAGE_CAPABILITIES  0x2A

    #ifndef _INC_STDDEF

    #undef offsetof
    #define offsetof(s,m)   (size_t)&(((s *)0)->m)

    #endif

#if defined(__BORLANDC__)
//#pragma option pop /*P_O_Push*/
#pragma option -a.
#elif defined(_MSC_VER)
#pragma pack(pop)
#endif


    char szVolumeName[20]="\\\\.\\ :";
    szVolumeName[4]=*RootDir;

    //get a handle to the device
    HANDLE hDevice = CreateFile(szVolumeName,
                           GENERIC_READ|GENERIC_WRITE,
                           FILE_SHARE_READ|FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, 0, NULL);//get the media types IO call

    if (hDevice != INVALID_HANDLE_VALUE)
    {
      ULONG dwBytesReturned=0;

      SCSI_PASS_THROUGH_WITH_BUFFERS      sptwb;
      BOOL                                status;
      ULONG                               length;

      ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

      sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
      //sptwb.Spt.PathId = 0;
      sptwb.Spt.TargetId = 1;
      //sptwb.Spt.Lun = 0;
      sptwb.Spt.CdbLength = CDB6GENERIC_LENGTH;
      sptwb.Spt.SenseInfoLength = 24;
      sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
      sptwb.Spt.DataTransferLength = 192;
      sptwb.Spt.TimeOutValue = 2;
      sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf);
      sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,SenseBuf);
//      sptwb.Spt.Cdb[0] = SCSIOP_INQUIRY;
      sptwb.Spt.Cdb[4] = 192;
      length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf) + sptwb.Spt.DataTransferLength;
/*
      status = DeviceIoControl(hDevice,
                               IOCTL_SCSI_PASS_THROUGH,
                               &sptwb,
                               sizeof(SCSI_PASS_THROUGH),
                               &sptwb,
                               length,
                               &dwBytesReturned,
                               FALSE);

      _SVS(if(!status)Message(MSG_ERRORTYPE,1,"FAR_GetDriveType()","","Ok"));

      //PrintStatusResults(status, returned, &sptwb);

  */
      // If device supports SCSI-3, then we can get the CD drive capabilities, i.e. ability to
      // read/write to CD-ROM/R/RW or/and read/write to DVD-ROM/R/RW.
      // Use the previous spti structure, only modify the command to "mode sense"

      sptwb.Spt.Cdb[0] = SCSIOP_MODE_SENSE;
      sptwb.Spt.Cdb[1] = 0x08;                    // target shall not return any block descriptors
      sptwb.Spt.Cdb[2] = MODE_PAGE_CAPABILITIES;

      status = DeviceIoControl(hDevice,
                               IOCTL_SCSI_PASS_THROUGH,
                               &sptwb,
                               sizeof(SCSI_PASS_THROUGH),
                               &sptwb,
                               length,
                               &dwBytesReturned,
                               FALSE);
      CloseHandle(hDevice);

      if (status)
      {
        if (((sptwb.DataBuf[7] & 0x10)||(sptwb.DataBuf[7] & 0x20)))  //DVRRW
        {
          DrvType = DRIVE_DVD_RW;
/*
          if ((sptwb.DataBuf[7] & 0x10))
            DrvType = DRIVE_DVD_R;
          else if ((sptwb.DataBuf[7] & 0x20))
            DrvType = DRIVE_DVD_RAM;
*/
        }
/*
        else if (                                                             //DVD Combo
                  (
                    (sptwb.DataBuf[6] & 0x08)||   //  DVDROM
                    (sptwb.DataBuf[6] & 0x10)||   //  DVDR
                    (sptwb.DataBuf[6] & 0x20)     //  DVDRAM
                  )
                    &&
                  (
                    (sptwb.DataBuf[7] & 0x01)||   //CDR
                    (sptwb.DataBuf[7] & 0x02)     //CDRW
                  )
                )
        {
          DrvType = DRIVE_DVD_COMBO;
        }
*/
        else if ((sptwb.DataBuf[6] & 0x08)||   //  DVDROM
                 (sptwb.DataBuf[6] & 0x10)||   //  DVDR
                 (sptwb.DataBuf[6] & 0x20)     //  DVDRAM
                )
        {
          DrvType = DRIVE_DVD_ROM;
        }
        else if(((sptwb.DataBuf[7] & 0x01)||(sptwb.DataBuf[7] & 0x02)))   //  CDRW
        {
          if ((sptwb.DataBuf[7] & 0x02))
            DrvType = DRIVE_CD_RW;
/*
          else if ((sptwb.DataBuf[7] & 0x01))
            DrvType = DRIVE_CD_R;
*/
        }
      }
      _SVS(else Message(MSG_ERRORTYPE,1,"FAR_GetDriveType()","","Ok"));

    }
  }

  return DrvType;
}

BOOL IsDriveTypeCDROM(UINT DriveType)
{
  return DriveType == DRIVE_CDROM || DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_DVD_RW;
}
