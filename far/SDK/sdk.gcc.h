#pragma once

/*
sdk.gcc.h

Типы и определения, отсутствующие в SDK (GCC).
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

// ntddstor.h
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

#ifndef MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
#endif

// winuser.h
#ifndef INPUTLANGCHANGE_FORWARD
#define INPUTLANGCHANGE_FORWARD 0x0002
#endif

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

typedef enum _OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectAllTypesInformation,
	ObjectHandleInformation
}
OBJECT_INFORMATION_CLASS;

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

#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC 0
#endif

// wincon.h
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
#ifndef CONSOLE_FULLSCREEN_HARDWARE
#define CONSOLE_FULLSCREEN_HARDWARE 2
#endif

// winnt.h
#ifndef FILE_ATTRIBUTE_VIRTUAL
#define FILE_ATTRIBUTE_VIRTUAL 0x00010000
#endif

// winbase.h
#ifndef COPY_FILE_ALLOW_DECRYPTED_DESTINATION
#define COPY_FILE_ALLOW_DECRYPTED_DESTINATION 0x00000008
#endif

#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif

#ifndef GetComputerNameEx
#define GetComputerNameEx GetComputerNameExW
#endif

#ifndef FIND_FIRST_EX_LARGE_FETCH
#define FIND_FIRST_EX_LARGE_FETCH 0x00000002
#endif

//ntddstor.h
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

typedef enum _FILE_INFORMATION_CLASS
{
	FileStreamInformation=22,
}
FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_BUFFER_OVERFLOW
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#endif

#ifndef VOLUME_NAME_GUID
#define VOLUME_NAME_GUID 0x1
#endif

// ShObjIdl.h
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


const IID IID_IApplicationAssociationRegistration = { 0x4E530B0A, 0xE611, 0x4C77, 0xA3, 0xAC, 0x90, 0x31, 0xD0, 0x22, 0x28, 0x1B };
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


#ifndef __ITaskbarList3_INTERFACE_DEFINED__
#define __ITaskbarList3_INTERFACE_DEFINED__

typedef enum TBPFLAG
{
	TBPF_NOPROGRESS    = 0,
	TBPF_INDETERMINATE = 0x1,
	TBPF_NORMAL        = 0x2,
	TBPF_ERROR         = 0x4,
	TBPF_PAUSED        = 0x8
}
TBPFLAG;

typedef enum TBATFLAG
{
	TBATF_USEMDITHUMBNAIL   = 0x1,
	TBATF_USEMDILIVEPREVIEW = 0x2
}
TBATFLAG;

const IID IID_ITaskbarList3  = { 0xEA1AFB91, 0x9E28, 0x4B86, 0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF };

DECLARE_INTERFACE_(ITaskbarList3,IUnknown) //BUGBUG, ITaskbarList2
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetProgressValue(HWND hwnd,ULONGLONG ullCompleted,ULONGLONG ullTotal)=0;
	virtual HRESULT STDMETHODCALLTYPE SetProgressState(HWND hwnd,TBPFLAG tbpFlags)=0;
	virtual HRESULT STDMETHODCALLTYPE RegisterTab(HWND hwndTab,HWND hwndMDI)=0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterTab(HWND hwndTab)=0;
	virtual HRESULT STDMETHODCALLTYPE SetTabOrder(HWND hwndTab,HWND hwndInsertBefore)=0;
	virtual HRESULT STDMETHODCALLTYPE SetTabActive(HWND hwndTab,HWND hwndMDI,TBATFLAG tbatFlags)=0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons(HWND hwnd,UINT cButtons,/*LPTHUMBBUTTON*/LPVOID pButton)=0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons(HWND hwnd,UINT cButtons,/*LPTHUMBBUTTON*/LPVOID pButton)=0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList(HWND hwnd,HIMAGELIST himl)=0;
	virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon(HWND hwnd,HICON hIcon,LPCWSTR pszDescription)=0;
	virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip(HWND hwnd,LPCWSTR pszTip)=0;
	virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip(HWND hwnd,RECT *prcClip)=0;
};

// shellapi.h
#ifndef SEE_MASK_NOASYNC
#define SEE_MASK_NOASYNC 0x00000100
#endif

#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))

#endif // __ITaskbarList3_INTERFACE_DEFINED__

// virtdisk.h
typedef enum _GET_STORAGE_DEPENDENCY_FLAG
{
	GET_STORAGE_DEPENDENCY_FLAG_NONE         = 0x00000000,
	GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES = 0x00000001,
	GET_STORAGE_DEPENDENCY_FLAG_DISK_HANDLE  = 0x00000002
}
GET_STORAGE_DEPENDENCY_FLAG;

typedef enum _STORAGE_DEPENDENCY_INFO_VERSION
{
	STORAGE_DEPENDENCY_INFO_VERSION_UNSPECIFIED = 0,
	STORAGE_DEPENDENCY_INFO_VERSION_1           = 1,
	STORAGE_DEPENDENCY_INFO_VERSION_2           = 2
}
STORAGE_DEPENDENCY_INFO_VERSION;

typedef enum _DEPENDENT_DISK_FLAG
{
	DEPENDENT_DISK_FLAG_NONE                 = 0x00000000,
	DEPENDENT_DISK_FLAG_MULT_BACKING_FILES   = 0x00000001,
	DEPENDENT_DISK_FLAG_FULLY_ALLOCATED      = 0x00000002,
	DEPENDENT_DISK_FLAG_READ_ONLY            = 0x00000004,
	DEPENDENT_DISK_FLAG_REMOTE               = 0x00000008,
	DEPENDENT_DISK_FLAG_SYSTEM_VOLUME        = 0x00000010,
	DEPENDENT_DISK_FLAG_SYSTEM_VOLUME_PARENT = 0x00000020,
	DEPENDENT_DISK_FLAG_REMOVABLE            = 0x00000040,
	DEPENDENT_DISK_FLAG_NO_DRIVE_LETTER      = 0x00000080,
	DEPENDENT_DISK_FLAG_PARENT               = 0x00000100,
	DEPENDENT_DISK_FLAG_NO_HOST_DISK         = 0x00000200,
	DEPENDENT_DISK_FLAG_PERMANENT_LIFETIME   = 0x00000400 
}
DEPENDENT_DISK_FLAG;

typedef struct _VIRTUAL_STORAGE_TYPE
{
	ULONG DeviceId;
	GUID VendorId;
}
VIRTUAL_STORAGE_TYPE, *PVIRTUAL_STORAGE_TYPE;

typedef struct _STORAGE_DEPENDENCY_INFO_TYPE_1
{
	DEPENDENT_DISK_FLAG DependencyTypeFlags;
	ULONG ProviderSpecificFlags;
	VIRTUAL_STORAGE_TYPE VirtualStorageType;
}
STORAGE_DEPENDENCY_INFO_TYPE_1, *PSTORAGE_DEPENDENCY_INFO_TYPE_1;

typedef struct _STORAGE_DEPENDENCY_INFO_TYPE_2
{
	DEPENDENT_DISK_FLAG DependencyTypeFlags;
	ULONG ProviderSpecificFlags;
	VIRTUAL_STORAGE_TYPE VirtualStorageType;
	ULONG AncestorLevel;
	PWSTR DependencyDeviceName;
	PWSTR HostVolumeName;
	PWSTR DependentVolumeName;
	PWSTR DependentVolumeRelativePath;
}
STORAGE_DEPENDENCY_INFO_TYPE_2, *PSTORAGE_DEPENDENCY_INFO_TYPE_2;

typedef struct _STORAGE_DEPENDENCY_INFO
{
	STORAGE_DEPENDENCY_INFO_VERSION Version;
	ULONG NumberEntries;
	union
	{
		STORAGE_DEPENDENCY_INFO_TYPE_1 Version1Entries[];
		STORAGE_DEPENDENCY_INFO_TYPE_2 Version2Entries[];
	};
}
STORAGE_DEPENDENCY_INFO, *PSTORAGE_DEPENDENCY_INFO;
