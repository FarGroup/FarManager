/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций

*/

/* Revision: 1.06 14.06.2004 $ */

/*
Modify:
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


UINT FAR_GetDriveType(LPCTSTR RootDir)
{
  UINT DrvType = GetDriveType(RootDir);

  // анализ CD-привода
  if (RootDir && IsLocalPath(RootDir) && DrvType == DRIVE_CDROM && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    ULONG dwBytesReturned=0;

    char szVolumeName[20]="\\\\.\\ :";
    szVolumeName[4]=*RootDir;

    //get a handle to the device
    HANDLE hDevice = CreateFile(szVolumeName,
                           GENERIC_READ|GENERIC_WRITE,
                           FILE_SHARE_READ|FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, 0, NULL);//get the media types IO call

    if (hDevice != INVALID_HANDLE_VALUE)
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
      if (status)
      {
        int CdRW=0,CdR=0,DvdR=0, DvdRW=0;
        if (((sptwb.DataBuf[7] & 0x10)||(sptwb.DataBuf[7] & 0x20)))  //DVR-RW
          DvdRW=1;
        if ((sptwb.DataBuf[7] & 0x01)||(sptwb.DataBuf[7] & 0x02))     //CD-RW
          CdRW=1;
        if ((sptwb.DataBuf[6] & 0x08)||(sptwb.DataBuf[6] & 0x10)||(sptwb.DataBuf[6] & 0x20)) // DVD-ROM
          DvdR=1;
        if ((sptwb.DataBuf[6] & 0x01)||(sptwb.DataBuf[6] & 0x02))     //CD-ROM
          CdR=1;

        if(CdRW)
        {
          if(DvdRW)
            DrvType = DRIVE_DVD_RW;
          else if(DvdR)
            DrvType = DRIVE_CD_RWDVD;
          else
            DrvType = DRIVE_CD_RW;
        }
        else
        {
          if(DvdRW)
            DrvType = DRIVE_DVD_RW;
          else if(DvdR)
            DrvType = DRIVE_DVD_ROM;
        }
      }
      else // вариант, когда не получилось определить (нехватка прав на доступ к девайсу)
      {    // в этом случае максимум, что сможем определить - это DVD или нет.
        //#ifndef _NTDDSTOR_H_

        #if defined(__BORLANDC__)
        //#pragma option push -b -a4 -pc -A- /*P_O_Push*/
        #pragma option -a4
        #elif defined(_MSC_VER)
        #pragma pack(push,4)
        #endif

        typedef enum _STORAGE_MEDIA_TYPE {
            //
            // Following are defined in ntdddisk.h in the MEDIA_TYPE enum
            //
            // Unknown,                // Format is unknown
            // F5_1Pt2_512,            // 5.25", 1.2MB,  512 bytes/sector
            // F3_1Pt44_512,           // 3.5",  1.44MB, 512 bytes/sector
            // F3_2Pt88_512,           // 3.5",  2.88MB, 512 bytes/sector
            // F3_20Pt8_512,           // 3.5",  20.8MB, 512 bytes/sector
            // F3_720_512,             // 3.5",  720KB,  512 bytes/sector
            // F5_360_512,             // 5.25", 360KB,  512 bytes/sector
            // F5_320_512,             // 5.25", 320KB,  512 bytes/sector
            // F5_320_1024,            // 5.25", 320KB,  1024 bytes/sector
            // F5_180_512,             // 5.25", 180KB,  512 bytes/sector
            // F5_160_512,             // 5.25", 160KB,  512 bytes/sector
            // RemovableMedia,         // Removable media other than floppy
            // FixedMedia,             // Fixed hard disk media
            // F3_120M_512,            // 3.5", 120M Floppy
            // F3_640_512,             // 3.5" ,  640KB,  512 bytes/sector
            // F5_640_512,             // 5.25",  640KB,  512 bytes/sector
            // F5_720_512,             // 5.25",  720KB,  512 bytes/sector
            // F3_1Pt2_512,            // 3.5" ,  1.2Mb,  512 bytes/sector
            // F3_1Pt23_1024,          // 3.5" ,  1.23Mb, 1024 bytes/sector
            // F5_1Pt23_1024,          // 5.25",  1.23MB, 1024 bytes/sector
            // F3_128Mb_512,           // 3.5" MO 128Mb   512 bytes/sector
            // F3_230Mb_512,           // 3.5" MO 230Mb   512 bytes/sector
            // F8_256_128,             // 8",     256KB,  128 bytes/sector
            // F3_200Mb_512,           // 3.5",   200M Floppy (HiFD)
            //

            DDS_4mm = 0x20,            // Tape - DAT DDS1,2,... (all vendors)
            MiniQic,                   // Tape - miniQIC Tape
            Travan,                    // Tape - Travan TR-1,2,3,...
            QIC,                       // Tape - QIC
            MP_8mm,                    // Tape - 8mm Exabyte Metal Particle
            AME_8mm,                   // Tape - 8mm Exabyte Advanced Metal Evap
            AIT1_8mm,                  // Tape - 8mm Sony AIT
            DLT,                       // Tape - DLT Compact IIIxt, IV
            NCTP,                      // Tape - Philips NCTP
            IBM_3480,                  // Tape - IBM 3480
            IBM_3490E,                 // Tape - IBM 3490E
            IBM_Magstar_3590,          // Tape - IBM Magstar 3590
            IBM_Magstar_MP,            // Tape - IBM Magstar MP
            STK_DATA_D3,               // Tape - STK Data D3
            SONY_DTF,                  // Tape - Sony DTF
            DV_6mm,                    // Tape - 6mm Digital Video
            DMI,                       // Tape - Exabyte DMI and compatibles
            SONY_D2,                   // Tape - Sony D2S and D2L
            CLEANER_CARTRIDGE,         // Cleaner - All Drive types that support Drive Cleaners
            CD_ROM,                    // Opt_Disk - CD
            CD_R,                      // Opt_Disk - CD-Recordable (Write Once)
            CD_RW,                     // Opt_Disk - CD-Rewriteable
            DVD_ROM,                   // Opt_Disk - DVD-ROM
            DVD_R,                     // Opt_Disk - DVD-Recordable (Write Once)
            DVD_RW,                    // Opt_Disk - DVD-Rewriteable
            MO_3_RW,                   // Opt_Disk - 3.5" Rewriteable MO Disk
            MO_5_WO,                   // Opt_Disk - MO 5.25" Write Once
            MO_5_RW,                   // Opt_Disk - MO 5.25" Rewriteable (not LIMDOW)
            MO_5_LIMDOW,               // Opt_Disk - MO 5.25" Rewriteable (LIMDOW)
            PC_5_WO,                   // Opt_Disk - Phase Change 5.25" Write Once Optical
            PC_5_RW,                   // Opt_Disk - Phase Change 5.25" Rewriteable
            PD_5_RW,                   // Opt_Disk - PhaseChange Dual Rewriteable
            ABL_5_WO,                  // Opt_Disk - Ablative 5.25" Write Once Optical
            PINNACLE_APEX_5_RW,        // Opt_Disk - Pinnacle Apex 4.6GB Rewriteable Optical
            SONY_12_WO,                // Opt_Disk - Sony 12" Write Once
            PHILIPS_12_WO,             // Opt_Disk - Philips/LMS 12" Write Once
            HITACHI_12_WO,             // Opt_Disk - Hitachi 12" Write Once
            CYGNET_12_WO,              // Opt_Disk - Cygnet/ATG 12" Write Once
            KODAK_14_WO,               // Opt_Disk - Kodak 14" Write Once
            MO_NFR_525,                // Opt_Disk - Near Field Recording (Terastor)
            NIKON_12_RW,               // Opt_Disk - Nikon 12" Rewriteable
            IOMEGA_ZIP,                // Mag_Disk - Iomega Zip
            IOMEGA_JAZ,                // Mag_Disk - Iomega Jaz
            SYQUEST_EZ135,             // Mag_Disk - Syquest EZ135
            SYQUEST_EZFLYER,           // Mag_Disk - Syquest EzFlyer
            SYQUEST_SYJET,             // Mag_Disk - Syquest SyJet
            AVATAR_F2,                 // Mag_Disk - 2.5" Floppy
            MP2_8mm,                   // Tape - 8mm Hitachi
            DST_S,                     // Ampex DST Small Tapes
            DST_M,                     // Ampex DST Medium Tapes
            DST_L,                     // Ampex DST Large Tapes
            VXATape_1,                 // Ecrix 8mm Tape
            VXATape_2,                 // Ecrix 8mm Tape
            STK_9840,                  // STK 9840
            LTO_Ultrium,               // IBM, HP, Seagate LTO Ultrium
            LTO_Accelis,               // IBM, HP, Seagate LTO Accelis
            DVD_RAM,                   // Opt_Disk - DVD-RAM
            AIT_8mm,                   // AIT2 or higher
            ADR_1,                     // OnStream ADR Mediatypes
            ADR_2
        } STORAGE_MEDIA_TYPE, *PSTORAGE_MEDIA_TYPE;
        //
        // Define the different storage bus types
        // Bus types below 128 (0x80) are reserved for Microsoft use
        //
        typedef enum _STORAGE_BUS_TYPE {
            BusTypeUnknown = 0x00,
            BusTypeScsi,
            BusTypeAtapi,
            BusTypeAta,
            BusType1394,
            BusTypeSsa,
            BusTypeFibre,
            BusTypeUsb,
            BusTypeRAID,
            BusTypeMaxReserved = 0x7F
        } STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;

        typedef struct _DEVICE_MEDIA_INFO {
            union {
                struct {
                    LARGE_INTEGER Cylinders;
                    STORAGE_MEDIA_TYPE MediaType;
                    ULONG TracksPerCylinder;
                    ULONG SectorsPerTrack;
                    ULONG BytesPerSector;
                    ULONG NumberMediaSides;
                    ULONG MediaCharacteristics; // Bitmask of MEDIA_XXX values.
                } DiskInfo;

                struct {
                    LARGE_INTEGER Cylinders;
                    STORAGE_MEDIA_TYPE MediaType;
                    ULONG TracksPerCylinder;
                    ULONG SectorsPerTrack;
                    ULONG BytesPerSector;
                    ULONG NumberMediaSides;
                    ULONG MediaCharacteristics; // Bitmask of MEDIA_XXX values.
                } RemovableDiskInfo;

                struct {
                    STORAGE_MEDIA_TYPE MediaType;
                    ULONG   MediaCharacteristics; // Bitmask of MEDIA_XXX values.
                    ULONG   CurrentBlockSize;
                    STORAGE_BUS_TYPE BusType;

                    //
                    // Bus specific information describing the medium supported.
                    //

                    union {
                        struct {
                            UCHAR MediumType;
                            UCHAR DensityCode;
                        } ScsiInformation;
                    } BusSpecificData;

                } TapeInfo;
            } DeviceSpecific;
        } DEVICE_MEDIA_INFO, *PDEVICE_MEDIA_INFO;

        typedef struct _GET_MEDIA_TYPES {
            ULONG DeviceType;              // FILE_DEVICE_XXX values
            ULONG MediaInfoCount;
            DEVICE_MEDIA_INFO MediaInfo[1];
        } GET_MEDIA_TYPES, *PGET_MEDIA_TYPES;

        #define MEDIA_ERASEABLE         0x00000001
        #define MEDIA_WRITE_ONCE        0x00000002
        #define MEDIA_READ_ONLY         0x00000004
        #define MEDIA_READ_WRITE        0x00000008
        #define MEDIA_WRITE_PROTECTED   0x00000100
        #define MEDIA_CURRENTLY_MOUNTED 0x80000000

        #if !defined(IOCTL_STORAGE_GET_MEDIA_TYPES_EX)
        #define IOCTL_STORAGE_GET_MEDIA_TYPES_EX      CTL_CODE(IOCTL_STORAGE_BASE, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)
        #endif

        #if defined(__BORLANDC__)
        //#pragma option pop /*P_O_Push*/
        #pragma option -a.
        #elif defined(_MSC_VER)
        #pragma pack(pop)
        #endif

        //#endif  // _NTDDSTOR_H_

        GET_MEDIA_TYPES mediaTypes;

        //  Простой как песня способ - даже плачу, ремируя его (блин малоинформативен)
        if (DeviceIoControl(hDevice,IOCTL_STORAGE_GET_MEDIA_TYPES_EX,
                NULL, 0, &mediaTypes, (DWORD)sizeof(mediaTypes), &dwBytesReturned, NULL) != 0)
        {
          #define FILE_DEVICE_DVD                 0x00000033
          if(mediaTypes.DeviceType == FILE_DEVICE_DVD)
            DrvType = DRIVE_DVD_ROM;
        }
      }
      CloseHandle(hDevice);
    }
  }

  return DrvType;
}

BOOL IsDriveTypeCDROM(UINT DriveType)
{
  return DriveType == DRIVE_CDROM || DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_DVD_RW;
}
