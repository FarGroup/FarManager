#ifndef __SDKPATCHES_HPP__
#define __SDKPATCHES_HPP__
/*
sdkpatches.hpp

Типы и определения, отсутствующие в поддерживаемых SDK.
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef __GNUC__
// winioctl.h
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

typedef enum _STORAGE_MEDIA_TYPE {
    DDS_4mm = 0x20,            // Tape - DAT DDS1,2,... (all vendors)
    MiniQic,                   // Tape - miniQIC Tape
    Travan,                    // Tape - Travan TR-1,2,3,...
    QIC,                       // Tape - QIC
    MP_8mm,                    // Tape - 8mm Exabyte Metal Particle
    AME_8mm,                   // Tape - 8mm Exabyte Advanced Metal Evap
    AIT1_8mm,                  // Tape - 8mm Sony AIT1
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
    STK_EAGLE,                 // STK Eagle
    LTO_Ultrium,               // IBM, HP, Seagate LTO Ultrium
    LTO_Accelis                // IBM, HP, Seagate LTO Accelis
} STORAGE_MEDIA_TYPE, *PSTORAGE_MEDIA_TYPE;

typedef struct _DEVICE_MEDIA_INFO {
    union {
        struct {
            LARGE_INTEGER Cylinders;
            STORAGE_MEDIA_TYPE MediaType;
            DWORD TracksPerCylinder;
            DWORD SectorsPerTrack;
            DWORD BytesPerSector;
            DWORD NumberMediaSides;
            DWORD MediaCharacteristics; // Bitmask of MEDIA_XXX values.
        } DiskInfo;

        struct {
            LARGE_INTEGER Cylinders;
            STORAGE_MEDIA_TYPE MediaType;
            DWORD TracksPerCylinder;
            DWORD SectorsPerTrack;
            DWORD BytesPerSector;
            DWORD NumberMediaSides;
            DWORD MediaCharacteristics; // Bitmask of MEDIA_XXX values.
        } RemovableDiskInfo;

        struct {
            STORAGE_MEDIA_TYPE MediaType;
            DWORD   MediaCharacteristics; // Bitmask of MEDIA_XXX values.
            DWORD   CurrentBlockSize;
            STORAGE_BUS_TYPE BusType;

            //
            // Bus specific information describing the medium supported.
            //

            union {
                struct {
                    BYTE  MediumType;
                    BYTE  DensityCode;
                } ScsiInformation;
            } BusSpecificData;

        } TapeInfo;
    } DeviceSpecific;
} DEVICE_MEDIA_INFO, *PDEVICE_MEDIA_INFO;

typedef struct _GET_MEDIA_TYPES {
    DWORD DeviceType;              // FILE_DEVICE_XXX values
    DWORD MediaInfoCount;
    DEVICE_MEDIA_INFO MediaInfo[1];
} GET_MEDIA_TYPES, *PGET_MEDIA_TYPES;

typedef struct _FILE_ALLOCATED_RANGE_BUFFER {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER Length;

} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;

typedef struct _FILE_SET_SPARSE_BUFFER
{
  BOOLEAN SetSparse;
}
FILE_SET_SPARSE_BUFFER, *PFILE_SET_SPARSE_BUFFER;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DecryptFile
 WINBASEAPI BOOL WINAPI DecryptFileW(LPCWSTR,DWORD);
 #define DecryptFile DecryptFileW
#endif

#ifndef GetConsoleAlias
 WINBASEAPI DWORD WINAPI GetConsoleAliasW(LPWSTR,LPWSTR,DWORD,LPWSTR);
 #define GetConsoleAlias GetConsoleAliasW
#endif

#ifdef __cplusplus
}
#endif

typedef enum tagASSOCIATIONLEVEL
{
	AL_MACHINE,
	AL_EFFECTIVE,
	AL_USER,
} ASSOCIATIONLEVEL;

typedef enum tagASSOCIATIONTYPE
{
	AT_FILEEXTENSION,
	AT_URLPROTOCOL,
	AT_STARTMENUCLIENT,
	AT_MIMETYPE,
} ASSOCIATIONTYPE;

EXTERN_C const IID IID_IApplicationAssociationRegistration;
#define INTERFACE IApplicationAssociationRegistration
DECLARE_INTERFACE_(IApplicationAssociationRegistration,IUnknown)
{
  STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS) PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;
  STDMETHOD(QueryCurrentDefault)(THIS_ LPCWSTR, ASSOCIATIONTYPE, ASSOCIATIONLEVEL, LPWSTR *) PURE;
  STDMETHOD(QueryAppIsDefault)(THIS_ LPCWSTR, ASSOCIATIONTYPE, ASSOCIATIONLEVEL, LPCWSTR, BOOL *) PURE;
  STDMETHOD(QueryAppIsDefaultAll)(THIS_ ASSOCIATIONLEVEL, LPCWSTR, BOOL *) PURE;
  STDMETHOD(SetAppAsDefault)(THIS_ LPCWSTR, LPCWSTR, ASSOCIATIONTYPE) PURE;
  STDMETHOD(SetAppAsDefaultAll)(THIS_ LPCWSTR) PURE;
  STDMETHOD(ClearUserAssociations)(THIS) PURE;
};
#undef INTERFACE

typedef enum _STREAM_INFO_LEVELS
{
	FindStreamInfoStandard,
}
STREAM_INFO_LEVELS;

typedef struct _WIN32_FIND_STREAM_DATA
{
	LARGE_INTEGER StreamSize;
	WCHAR cStreamName[MAX_PATH+36];
}
WIN32_FIND_STREAM_DATA,*PWIN32_FIND_STREAM_DATA;

#endif // __GNUC__

#ifndef FSCTL_QUERY_ALLOCATED_RANGES
#define FSCTL_QUERY_ALLOCATED_RANGES    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 51,  METHOD_NEITHER, FILE_READ_DATA)  // FILE_ALLOCATED_RANGE_BUFFER, FILE_ALLOCATED_RANGE_BUFFER
#endif

#ifndef CM_DRP_FRIENDLYNAME
#define CM_DRP_FRIENDLYNAME 0x0000000D
#endif

#ifndef CM_DRP_DEVICEDESC
#define CM_DRP_DEVICEDESC 0x00000001
#endif

#ifndef CM_DRP_CAPABILITIES
#define CM_DRP_CAPABILITIES 0x00000010
#endif

#ifndef CM_DEVCAP_REMOVABLE
#define CM_DEVCAP_REMOVABLE 0x00000004
#endif

#ifndef CM_DEVCAP_SURPRISEREMOVALOK
#define CM_DEVCAP_SURPRISEREMOVALOK 0x00000080
#endif

#ifndef CM_DEVCAP_DOCKDEVICE
#define CM_DEVCAP_DOCKDEVICE 0x00000008
#endif

#ifndef CM_DEVCAP_UNIQUEID
#define CM_DEVCAP_UNIQUEID 0x00000010
#endif

#ifdef _MSC_VER
#pragma pack(push,4)
#endif

// winioctl.h
#ifndef IOCTL_STORAGE_QUERY_PROPERTY
#define IOCTL_STORAGE_QUERY_PROPERTY 0x002D1400

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

#endif //IOCTL_STORAGE_QUERY_PROPERTY

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

#define CDB6GENERIC_LENGTH 6
#define CDB10GENERIC_LENGTH 10

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH Spt;
    ULONG             Filler;      // realign buffers to double word boundary
    UCHAR             SenseBuf[32];
    UCHAR             DataBuf[512];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifndef __GCC__
 // winnt.h
 #ifndef IO_REPARSE_TAG_VALID_VALUES
 #define IO_REPARSE_TAG_VALID_VALUES 0xE000FFFF
 #endif

 #ifndef IsReparseTagValid
 #define IsReparseTagValid(x) (!((x)&~IO_REPARSE_TAG_VALID_VALUES)&&((x)>IO_REPARSE_TAG_RESERVED_RANGE))
 #endif //IsReparseTagValid
#endif //__GCC__

// wincon.h
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif

// winnt.h
#ifndef FILE_ATTRIBUTE_VIRTUAL
#define FILE_ATTRIBUTE_VIRTUAL 0x00010000
#endif

#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK 0xA000000CL
#endif

#ifndef IO_REPARSE_TAG_DFSR
#define IO_REPARSE_TAG_DFSR 0x80000012L
#endif

// winuser.h
#ifndef INPUTLANGCHANGE_FORWARD
#define INPUTLANGCHANGE_FORWARD 0x0002
#endif

#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#endif

#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#endif

// winbase.h
#ifndef COPY_FILE_ALLOW_DECRYPTED_DESTINATION
#define COPY_FILE_ALLOW_DECRYPTED_DESTINATION 0x00000008
#endif

#ifndef FS_FILE_ENCRYPTION
#define FS_FILE_ENCRYPTION FILE_SUPPORTS_ENCRYPTION
#endif

#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif

// scsi.h
#ifndef SCSIOP_MODE_SENSE
#define SCSIOP_MODE_SENSE 0x1A
#endif

#ifndef MODE_PAGE_CAPABILITIES
#define MODE_PAGE_CAPABILITIES 0x2A
#endif

// ntddscsi.h
#ifndef SCSI_IOCTL_DATA_IN
#define SCSI_IOCTL_DATA_IN 1
#endif

#ifndef IOCTL_SCSI_BASE
#define IOCTL_SCSI_BASE FILE_DEVICE_CONTROLLER
#endif

#ifndef IOCTL_SCSI_PASS_THROUGH
#define IOCTL_SCSI_PASS_THROUGH CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#endif

typedef struct _FILE_STREAM_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG StreamNameLength;
	LARGE_INTEGER StreamSize;
	LARGE_INTEGER StreamAllocationSize;
	WCHAR StreamName[1];
}
FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _IO_STATUS_BLOCK
{
	union
	{
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
}
IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#define FileStreamInformation 22

#endif // __SDKPATCHES_HPP__
