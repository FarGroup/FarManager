/*
cddrv.cpp

про сидюк

*/

/* Revision: 1.0 21.06.2004 $ */

/*
Modify:
  21.06.2004 SVS
    + добавлен в проект.
      Исходный  - EnumCD (http://support.microsoft.com/default.aspx?scid=kb;en-us;305184)
      Доработка - Alexander Kornienko <alexfh@mail.ru>
*/

#include "headers.hpp"
#pragma hdrstop

#include "farconst.hpp"
#include "fn.hpp"

#if defined(__BORLANDC__)
//#pragma option push -b -a4 -pc -A- /*P_O_Push*/
#pragma option -a4
#elif defined(_MSC_VER)
#pragma pack(push,4)
#endif



#define IOCTL_STORAGE_QUERY_PROPERTY    0x002D1400
#define SCSI_IOCTL_DATA_IN              1

#define SCSI_IOCTL_DATA_IN           1
#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

#undef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m)

typedef struct _STORAGE_DEVICE_DESCRIPTOR {
  ULONG  Version;
  ULONG  Size;
  UCHAR  DeviceType;
  UCHAR  DeviceTypeModifier;
  BYTE  RemovableMedia;
  BYTE  CommandQueueing;
  ULONG  VendorIdOffset;
  ULONG  ProductIdOffset;
  ULONG  ProductRevisionOffset;
  ULONG  SerialNumberOffset;
  BYTE  BusType;        //STORAGE_BUS_TYPE
  ULONG  RawPropertiesLength;
  UCHAR  RawDeviceProperties[1];
} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;

typedef struct _STORAGE_ADAPTER_DESCRIPTOR {
  ULONG  Version;
  ULONG  Size;
  ULONG  MaximumTransferLength;
  ULONG  MaximumPhysicalPages;
  ULONG  AlignmentMask;
  BYTE  AdapterUsesPio;
  BYTE  AdapterScansDown;
  BYTE  CommandQueueing;
  BYTE  AcceleratedTransfer;
  BYTE  BusType;        // STORAGE_BUS_TYPE
  USHORT  BusMajorVersion;
  USHORT  BusMinorVersion;
} STORAGE_ADAPTER_DESCRIPTOR, *PSTORAGE_ADAPTER_DESCRIPTOR;

typedef enum _STORAGE_PROPERTY_ID {
  StorageDeviceProperty = 0,
  StorageAdapterProperty,
  StorageDeviceIdProperty
} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

typedef enum _STORAGE_QUERY_TYPE {
  PropertyStandardQuery = 0,
  PropertyExistsQuery,
  PropertyMaskQuery,
  PropertyQueryMaxDefined
} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

typedef struct _STORAGE_PROPERTY_QUERY {
  STORAGE_PROPERTY_ID  PropertyId;
  STORAGE_QUERY_TYPE  QueryType;
  UCHAR  AdditionalParameters[1];
} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;


// no pack
typedef struct _SCSI_PASS_THROUGH {
  USHORT  Length;
  UCHAR  ScsiStatus;
  UCHAR  PathId;
  UCHAR  TargetId;
  UCHAR  Lun;
  UCHAR  CdbLength;
  UCHAR  SenseInfoLength;
  UCHAR  DataIn;
  ULONG  DataTransferLength;
  ULONG  TimeOutValue;
  ULONG_PTR DataBufferOffset;
  ULONG  SenseInfoOffset;
  UCHAR  Cdb[16];
}SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;


//
// Command Descriptor Block constants.
//

#define CDB6GENERIC_LENGTH         6
#define CDB10GENERIC_LENGTH        10


//
// SCSI CDB operation codes
//

#define SCSIOP_TEST_UNIT_READY     0x00
#define SCSIOP_REZERO_UNIT         0x01
#define SCSIOP_REWIND              0x01
#define SCSIOP_REQUEST_BLOCK_ADDR  0x02
#define SCSIOP_REQUEST_SENSE       0x03
#define SCSIOP_FORMAT_UNIT         0x04
#define SCSIOP_READ_BLOCK_LIMITS   0x05
#define SCSIOP_REASSIGN_BLOCKS     0x07
#define SCSIOP_READ6               0x08
#define SCSIOP_RECEIVE             0x08
#define SCSIOP_WRITE6              0x0A
#define SCSIOP_PRINT               0x0A
#define SCSIOP_SEND                0x0A
#define SCSIOP_SEEK6               0x0B
#define SCSIOP_TRACK_SELECT        0x0B
#define SCSIOP_SLEW_PRINT          0x0B
#define SCSIOP_SEEK_BLOCK          0x0C
#define SCSIOP_PARTITION           0x0D
#define SCSIOP_READ_REVERSE        0x0F
#define SCSIOP_WRITE_FILEMARKS     0x10
#define SCSIOP_FLUSH_BUFFER        0x10
#define SCSIOP_SPACE               0x11
#define SCSIOP_INQUIRY             0x12
#define SCSIOP_VERIFY6             0x13
#define SCSIOP_RECOVER_BUF_DATA    0x14
#define SCSIOP_MODE_SELECT         0x15
#define SCSIOP_RESERVE_UNIT        0x16
#define SCSIOP_RELEASE_UNIT        0x17
#define SCSIOP_COPY                0x18
#define SCSIOP_ERASE               0x19
#define SCSIOP_MODE_SENSE          0x1A
#define SCSIOP_START_STOP_UNIT     0x1B
#define SCSIOP_STOP_PRINT          0x1B
#define SCSIOP_LOAD_UNLOAD         0x1B
#define SCSIOP_RECEIVE_DIAGNOSTIC  0x1C
#define SCSIOP_SEND_DIAGNOSTIC     0x1D
#define SCSIOP_MEDIUM_REMOVAL      0x1E
#define SCSIOP_READ_CAPACITY       0x25
#define SCSIOP_READ                0x28
#define SCSIOP_WRITE               0x2A
#define SCSIOP_SEEK                0x2B
#define SCSIOP_LOCATE              0x2B
#define SCSIOP_WRITE_VERIFY        0x2E
#define SCSIOP_VERIFY              0x2F
#define SCSIOP_SEARCH_DATA_HIGH    0x30
#define SCSIOP_SEARCH_DATA_EQUAL   0x31
#define SCSIOP_SEARCH_DATA_LOW     0x32
#define SCSIOP_SET_LIMITS          0x33
#define SCSIOP_READ_POSITION       0x34
#define SCSIOP_SYNCHRONIZE_CACHE   0x35
#define SCSIOP_COMPARE             0x39
#define SCSIOP_COPY_COMPARE        0x3A
#define SCSIOP_WRITE_DATA_BUFF     0x3B
#define SCSIOP_READ_DATA_BUFF      0x3C
#define SCSIOP_CHANGE_DEFINITION   0x40
#define SCSIOP_READ_SUB_CHANNEL    0x42
#define SCSIOP_READ_TOC            0x43
#define SCSIOP_READ_HEADER         0x44
#define SCSIOP_PLAY_AUDIO          0x45
#define SCSIOP_PLAY_AUDIO_MSF      0x47
#define SCSIOP_PLAY_TRACK_INDEX    0x48
#define SCSIOP_PLAY_TRACK_RELATIVE 0x49
#define SCSIOP_PAUSE_RESUME        0x4B
#define SCSIOP_LOG_SELECT          0x4C
#define SCSIOP_LOG_SENSE           0x4D

#define MODE_PAGE_CAPABILITIES  0x2A

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH Spt;
    ULONG             Filler;      // realign buffers to double word boundary
    UCHAR             SenseBuf[32];
    UCHAR             DataBuf[512];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

#if defined(__BORLANDC__)
//#pragma option pop /*P_O_Push*/
#pragma option -a.
#elif defined(_MSC_VER)
#pragma pack(pop)
#endif


static CDROM_DeviceCaps getCapsUsingMediaType(HANDLE hDevice)
{

    UCHAR               buffer[2048];   // Must be big enough hold DEVICE_MEDIA_INFO
    ULONG               returned;

    if(!DeviceIoControl(hDevice, IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0,
                        buffer, sizeof(buffer), &returned, FALSE))
    {
       return CDDEV_CAPS_NONE;
    }

    switch(((PGET_MEDIA_TYPES)buffer)->DeviceType)
    {
        case FILE_DEVICE_CD_ROM:
            return CDDEV_CAPS_GENERIC_CD;

        case FILE_DEVICE_DVD:
            return CDDEV_CAPS_GENERIC_DVD;

        default:
            return CDDEV_CAPS_NONE;
    }
}

static CDROM_DeviceCaps getCapsUsingProductId(const char *prodID)
{
    char productID[1024];
    int idx = 0;

    for(int i = 0; prodID[i]; i++)
    {
        char c = prodID[i];
        if(c >= 'A' && c <= 'Z')
            productID[idx++] = c;
        else if(c >= 'a' && c <= 'z')
            productID[idx++] = c - 'a' + 'A';
    }

    productID[idx] = 0;


    int caps = CDDEV_CAPS_NONE;

    if(strstr(productID, "CD"))
    {
        caps |= CDDEV_CAPS_GENERIC_CD;
    }

    if(strstr(productID, "CDRW"))
    {
        caps |= CDDEV_CAPS_GENERIC_CDRW;
    }

    if(strstr(productID, "DVD"))
    {
        caps |= CDDEV_CAPS_GENERIC_DVD;
    }

    if(strstr(productID, "DVDRW"))
    {
        caps |= CDDEV_CAPS_GENERIC_DVDRW;
    }

    if(strstr(productID, "DVDRAM"))
    {
        caps |= CDDEV_CAPS_GENERIC_DVDRAM;
    }

    return (CDROM_DeviceCaps)caps;
}


static CDROM_DeviceCaps getCapsUsingSCSIPassThrough(HANDLE hDevice)
{
    SCSI_PASS_THROUGH_WITH_BUFFERS      sptwb;
    ULONG                               length;
    DWORD                               returned;
    BOOL                                status;


    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.Spt.PathId = 0;
    sptwb.Spt.TargetId = 1;
    sptwb.Spt.Lun = 0;
    sptwb.Spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.Spt.SenseInfoLength = 24;
    sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.Spt.DataTransferLength = 192;
    sptwb.Spt.TimeOutValue = 2;
    sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf);
    sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,SenseBuf);
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf) + sptwb.Spt.DataTransferLength;

    // If device supports SCSI-3, then we can get the CD drive capabilities, i.e. ability to
    // read/write to CD-ROM/R/RW or/and read/write to DVD-ROM/R/RW.
    // Use the previous spti structure, only modify the command to "mode sense"

    sptwb.Spt.Cdb[0] = SCSIOP_MODE_SENSE;
    sptwb.Spt.Cdb[1] = 0x08;                    // target shall not return any block descriptors
    sptwb.Spt.Cdb[2] = MODE_PAGE_CAPABILITIES;
    sptwb.Spt.Cdb[4] = 192;

    status = DeviceIoControl(hDevice,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    if(status)
    {
        if(!sptwb.Spt.ScsiStatus)
        {
            // Notes:
            // 1. The header of 6-byte MODE commands is 4 bytes long.
            // 2. No Block Descriptors returned before parameter page as was specified when building the Mode command.
            // 3. First two bytes of a parameter page are the Page Code and Page Length bytes.
            // Therefore, our useful data starts at the 7th byte in the data buffer.

            int caps = CDDEV_CAPS_READ_CDROM;
//char BBBB[1024];
//sprintf(BBBB,"6=%X, 7=%X",sptwb.DataBuf[6],sptwb.DataBuf[7]);
//MessageBox(0,BBBB,BBBB,0);

            if(sptwb.DataBuf[6] & 0x01)
                caps |= CDDEV_CAPS_READ_CDR;

            if(sptwb.DataBuf[6] & 0x02)
                caps |= CDDEV_CAPS_READ_CDRW;

            if(sptwb.DataBuf[6] & 0x08)
                caps |= CDDEV_CAPS_READ_DVDROM;

            if(sptwb.DataBuf[6] & 0x10)
                caps |= CDDEV_CAPS_READ_DVDR | CDDEV_CAPS_READ_DVDRW;

            if(sptwb.DataBuf[6] & 0x20)
                caps |= CDDEV_CAPS_READ_DVDRAM;

            if(sptwb.DataBuf[7] & 0x01)
                caps |= CDDEV_CAPS_WRITE_CDR;

            if(sptwb.DataBuf[7] & 0x02)
                caps |= CDDEV_CAPS_WRITE_CDRW;

            if(sptwb.DataBuf[7] & 0x10)
                caps |= CDDEV_CAPS_WRITE_DVDR | CDDEV_CAPS_WRITE_DVDRW;

            if(sptwb.DataBuf[7] & 0x20)
                caps |= CDDEV_CAPS_WRITE_DVDRAM;

            if(caps != CDDEV_CAPS_NONE)
                return (CDROM_DeviceCaps)caps;
        }
    }




    sptwb.Spt.Cdb[0] = SCSIOP_INQUIRY;
    sptwb.Spt.Cdb[1] = 0;
    sptwb.Spt.Cdb[2] = 0;
    sptwb.Spt.Cdb[4] = 192;

    status = DeviceIoControl(hDevice,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    if(status)
    {
        if(!sptwb.Spt.ScsiStatus)
        {
            char productID[17];
            int idx = 0;

            for(int i = 16; i <= 31; i++)
            {
                productID[idx++] = sptwb.DataBuf[i];
            }

            productID[idx] = 0;

            return getCapsUsingProductId(productID);
        }
    }

    return CDDEV_CAPS_NONE;
}

static CDROM_DeviceCaps getCapsUsingDeviceProps(HANDLE hDevice)
{
    PSTORAGE_DEVICE_DESCRIPTOR      devDesc;
    BOOL                            status;
    char                            outBuf[512];
    DWORD                           returnedLength;
    STORAGE_PROPERTY_QUERY          query;

    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    status = DeviceIoControl(
                        hDevice,
                        IOCTL_STORAGE_QUERY_PROPERTY,
                        &query,
                        sizeof( STORAGE_PROPERTY_QUERY ),
                        &outBuf,
                        512,
                        &returnedLength,
                        NULL
                        );
    if(status)
    {
        devDesc = (PSTORAGE_DEVICE_DESCRIPTOR) outBuf;

        if(devDesc->ProductIdOffset && outBuf[devDesc->ProductIdOffset])
        {
            char productID[1024];
            int idx = 0;

            for(int i = devDesc->ProductIdOffset; outBuf[i] && i < returnedLength; i++)
            {
                productID[idx++] = outBuf[i];
            }
            productID[idx] = 0;

            return getCapsUsingProductId(productID);
        }
    }

    return CDDEV_CAPS_NONE;
}


CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice)
{
    CDROM_DeviceCaps caps = CDDEV_CAPS_NONE;
    if((caps = getCapsUsingSCSIPassThrough(hDevice)) !=  CDDEV_CAPS_NONE)
        return caps;

    if((caps = getCapsUsingDeviceProps(hDevice)) != CDDEV_CAPS_NONE)
        return caps;

    return getCapsUsingMediaType(hDevice);
}

UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps)
{
    if(caps & CDDEV_CAPS_WRITE_DVDRAM)
        return DRIVE_DVD_RAM;

    if(caps & CDDEV_CAPS_WRITE_DVDRW)
        return DRIVE_DVD_RW;

    if((caps & CDDEV_CAPS_WRITE_CDRW) && (caps & CDDEV_CAPS_READ_DVDROM))
        return DRIVE_CD_RWDVD;

    if(caps & CDDEV_CAPS_READ_DVDROM)
        return DRIVE_DVD_ROM;

    if(caps & CDDEV_CAPS_WRITE_CDRW)
        return DRIVE_CD_RW;

    if(caps & CDDEV_CAPS_READ_CDROM)
        return DRIVE_CDROM;

    return DRIVE_UNKNOWN;
}

BOOL IsDriveTypeCDROM(UINT DriveType)
{
  return DriveType == DRIVE_CDROM || DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_DVD_RAM;
}

UINT FAR_GetDriveType(LPCTSTR RootDir,CDROM_DeviceCaps *Caps)
{
  CDROM_DeviceCaps caps=CDDEV_CAPS_NONE;
  UINT DrvType = GetDriveType(RootDir);

  // анализ CD-привода
  if (RootDir && IsLocalPath(RootDir) && DrvType == DRIVE_CDROM)// && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    char szVolumeName[20]="\\\\.\\ :";
    szVolumeName[4]=*RootDir;

    //get a handle to the device
    HANDLE hDevice = CreateFile(szVolumeName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

    if (hDevice != INVALID_HANDLE_VALUE)
    {
      CDROM_DeviceCaps caps=GetCDDeviceCaps(hDevice);
      DrvType=GetCDDeviceTypeByCaps(caps);
      CloseHandle(hDevice);
    }

    if(DrvType == DRIVE_UNKNOWN) // фигня могла кака-нить произойти, посему...
      DrvType=DRIVE_CDROM;       // ...вертаем в зад сидюк.
  }

  if(Caps)
    *Caps=caps;

  return DrvType;
}
