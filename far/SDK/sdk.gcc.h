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

#ifndef _W32API_OLD
typedef struct _FILE_ALLOCATED_RANGE_BUFFER {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER Length;

} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;

typedef struct _FILE_SET_SPARSE_BUFFER
{
  BOOLEAN SetSparse;
}
FILE_SET_SPARSE_BUFFER, *PFILE_SET_SPARSE_BUFFER;

#ifndef DecryptFile
EXTERN_C WINBASEAPI BOOL WINAPI DecryptFileW(LPCWSTR,DWORD);
 #define DecryptFile DecryptFileW
#endif

#ifndef GetConsoleAlias
EXTERN_C WINBASEAPI DWORD WINAPI GetConsoleAliasW(LPWSTR,LPWSTR,DWORD,LPWSTR);
 #define GetConsoleAlias GetConsoleAliasW
#endif

// winuser.h
#ifndef INPUTLANGCHANGE_FORWARD
#define INPUTLANGCHANGE_FORWARD 0x0002
#endif

#ifndef INPUTLANGCHANGE_BACKWARD
#define INPUTLANGCHANGE_BACKWARD 0x0004
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
#endif //_W32API_OLD

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

#ifndef MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
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

#ifndef JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK
#define JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK 0x00001000
#endif

#ifndef IO_REPARSE_TAG_DFS
#define IO_REPARSE_TAG_DFS (0x8000000AL)
#endif

#ifndef IO_REPARSE_TAG_DFSR
#define IO_REPARSE_TAG_DFSR (0x80000012L)
#endif

#ifndef IO_REPARSE_TAG_HSM
#define IO_REPARSE_TAG_HSM (0xC0000004L)
#endif

#ifndef IO_REPARSE_TAG_HSM2
#define IO_REPARSE_TAG_HSM2 (0x80000006L)
#endif

#ifndef IO_REPARSE_TAG_SIS
#define IO_REPARSE_TAG_SIS (0x80000007L)
#endif

#ifndef IO_REPARSE_TAG_WIM
#define IO_REPARSE_TAG_WIM (0x80000008L)
#endif

#ifndef IO_REPARSE_TAG_CSV
#define IO_REPARSE_TAG_CSV (0x80000009L)
#endif

// winbase.h
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
	FileBothDirectoryInformation=3,
	FileBasicInformation=4,
	FileStreamInformation=22,
	FileIdBothDirectoryInformation=37,
}
FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

#if !defined(_W32API_OLD) || (_GCC_VER < GCC_VER_(4,5,3))
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

// shobjidl.h

const IID IID_IApplicationAssociationRegistration = {0x4E530B0A, 0xE611, 0x4C77, {0xA3, 0xAC, 0x90, 0x31, 0xD0, 0x22, 0x28, 0x1B}};
DECLARE_INTERFACE_(IApplicationAssociationRegistration,IUnknown)
{
	virtual HRESULT STDMETHODCALLTYPE QueryCurrentDefault(PCWSTR pszQuery, ASSOCIATIONTYPE atQueryType, ASSOCIATIONLEVEL alQueryLevel, LPWSTR *ppszAssociation) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryAppIsDefault(LPCWSTR pszQuery, ASSOCIATIONTYPE atQueryType, ASSOCIATIONLEVEL alQueryLevel, LPCWSTR pszAppRegistryName, BOOL *pfDefault) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryAppIsDefaultAll(ASSOCIATIONLEVEL alQueryLevel, LPCWSTR pszAppRegistryName, BOOL *pfDefault) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetAppAsDefault(LPCWSTR pszAppRegistryName, LPCWSTR pszSet, ASSOCIATIONTYPE atSetType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetAppAsDefaultAll(LPCWSTR pszAppRegistryName) = 0;
	virtual HRESULT STDMETHODCALLTYPE ClearUserAssociations() = 0;
};
#endif

#if !defined(_W32API_OLD)
const CLSID CLSID_TaskbarList = {0x56FDF344, 0xFD6D, 0x11d0, {0x95, 0x8A, 0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90}};

const IID IID_ITaskbarList = {0x56FDF342, 0xFD6D, 0x11d0, {0x95, 0x8A, 0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90}};
DECLARE_INTERFACE_(ITaskbarList,IUnknown)
{
public:
	virtual HRESULT STDMETHODCALLTYPE HrInit() = 0;
	virtual HRESULT STDMETHODCALLTYPE AddTab(HWND hwnd) = 0;
	virtual HRESULT STDMETHODCALLTYPE DeleteTab(HWND hwnd) = 0;
	virtual HRESULT STDMETHODCALLTYPE ActivateTab(HWND hwnd) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetActiveAlt(HWND hwnd) = 0;
};

const IID IID_ITaskbarList2 = {0x602D4995, 0xB13A, 0x429b, {0xA6, 0x6E, 0x19, 0x35, 0xE4, 0x4F, 0x43, 0x17}};
DECLARE_INTERFACE_(ITaskbarList2,ITaskbarList)
{
public:
	virtual HRESULT STDMETHODCALLTYPE MarkFullscreenWindow(HWND hwnd,BOOL fFullscreen)=0;
};
#endif

#if !defined(_W32API_OLD) || (_GCC_VER < GCC_VER_(4,5,3))
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

const IID IID_ITaskbarList3 = {0xEA1AFB91, 0x9E28, 0x4B86, {0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF}};
DECLARE_INTERFACE_(ITaskbarList3,ITaskbarList2)
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

// will create a compiler error if wrong level of indirection is used.
template<typename T>
void** IID_PPV_ARGS_Helper(T** pp)
{
	// make sure everyone derives from IUnknown
	IUnknown* I = static_cast<IUnknown*>(*pp); I = 0; (void)I;
	return reinterpret_cast<void**>(pp);
}
#endif //_W32API_OLD

#ifndef OF_CAP_CANSWITCHTO
typedef enum FILE_USAGE_TYPE
{
	FUT_PLAYING = 0,
	FUT_EDITING = (FUT_PLAYING + 1),
	FUT_GENERIC = (FUT_EDITING + 1),
}
FILE_USAGE_TYPE;

#define OF_CAP_CANSWITCHTO 0x0001
#define OF_CAP_CANCLOSE    0x0002

const IID IID_IFileIsInUse = {0x64A1CBF0, 0x3A1A, 0x4461, {0x91, 0x58, 0x37, 0x69, 0x69, 0x69, 0x39, 0x50}};
DECLARE_INTERFACE_(IFileIsInUse,IUnknown)
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetAppName(LPWSTR *ppszName) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetUsage(FILE_USAGE_TYPE *pfut) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCapabilities(DWORD *pdwCapFlags) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSwitchToHWND(HWND *phwnd) = 0;
	virtual HRESULT STDMETHODCALLTYPE CloseFile() = 0;
};
#endif //OF_CAP_CANSWITCHTO

template <typename T, size_t N>
char (*RtlpNumberOf(T(&)[N]))[N];
#undef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(*RtlpNumberOf(A)))

// shellapi.h
#ifndef SEE_MASK_NOASYNC
#define SEE_MASK_NOASYNC 0x00000100
#endif

// WinIoCtl.h
#ifndef VolumeClassGuid
DEFINE_GUID(GUID_DEVINTERFACE_VOLUME, 0x53f5630dL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#endif

// virtdisk.h
#if !defined(VIRT_DISK_API_DEF) && !defined(_INC_VIRTDISK)
#define VIRT_DISK_API_DEF
#define _INC_VIRTDISK
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

#define VIRTUAL_STORAGE_TYPE_DEVICE_VHD 2

typedef enum _OPEN_VIRTUAL_DISK_VERSION
{
	OPEN_VIRTUAL_DISK_VERSION_UNSPECIFIED = 0,
	OPEN_VIRTUAL_DISK_VERSION_1           = 1,
}
OPEN_VIRTUAL_DISK_VERSION;

typedef struct _OPEN_VIRTUAL_DISK_PARAMETERS
{
	OPEN_VIRTUAL_DISK_VERSION Version;
	union
	{
		struct
		{
			ULONG RWDepth;
		}
		Version1;
	};
}
OPEN_VIRTUAL_DISK_PARAMETERS, *POPEN_VIRTUAL_DISK_PARAMETERS;

typedef enum _VIRTUAL_DISK_ACCESS_MASK
{
	VIRTUAL_DISK_ACCESS_ATTACH_RO = 0x00010000,
	VIRTUAL_DISK_ACCESS_ATTACH_RW = 0x00020000,
	VIRTUAL_DISK_ACCESS_DETACH    = 0x00040000,
	VIRTUAL_DISK_ACCESS_GET_INFO  = 0x00080000,
	VIRTUAL_DISK_ACCESS_CREATE    = 0x00100000,
	VIRTUAL_DISK_ACCESS_METAOPS   = 0x00200000,
	VIRTUAL_DISK_ACCESS_READ      = 0x000d0000,
	VIRTUAL_DISK_ACCESS_ALL       = 0x003f0000,
	VIRTUAL_DISK_ACCESS_WRITABLE  = 0x00320000
}
VIRTUAL_DISK_ACCESS_MASK;

typedef enum _OPEN_VIRTUAL_DISK_FLAG
{
	OPEN_VIRTUAL_DISK_FLAG_NONE       = 0x00000000,
	OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS = 0x00000001,
	OPEN_VIRTUAL_DISK_FLAG_BLANK_FILE = 0x00000002,
	OPEN_VIRTUAL_DISK_FLAG_BOOT_DRIVE = 0x00000004,
}
OPEN_VIRTUAL_DISK_FLAG;

typedef enum _DETACH_VIRTUAL_DISK_FLAG
{
	DETACH_VIRTUAL_DISK_FLAG_NONE = 0x00000000,
}
DETACH_VIRTUAL_DISK_FLAG;

DEFINE_GUID(VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT, 0xec984aec, 0xa0f9, 0x47e9, 0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b);

#endif

#ifdef _W32API_OLD
/* Object Attributes */
typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

/* Helper Macro */
#define InitializeObjectAttributes(p,n,a,r,s) { \
  (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
  (p)->RootDirectory = (r); \
  (p)->Attributes = (a); \
  (p)->ObjectName = (n); \
  (p)->SecurityDescriptor = (s); \
  (p)->SecurityQualityOfService = NULL; \
}

typedef struct _REPARSE_DATA_BUFFER {
  ULONG ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  _ANONYMOUS_UNION union {
    struct {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      ULONG Flags;
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
      UCHAR DataBuffer[1];
    } GenericReparseBuffer;
  } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)

/* Reserved reparse tags */
#define IO_REPARSE_TAG_RESERVED_ZERO            (0)
#define IO_REPARSE_TAG_RESERVED_ONE             (1)
#define IO_REPARSE_TAG_RESERVED_RANGE           IO_REPARSE_TAG_RESERVED_ONE

#define IO_REPARSE_TAG_VALID_VALUES             (0xF000FFFF)

#define IsReparseTagValid(tag) (                               \
                  !((tag) & ~IO_REPARSE_TAG_VALID_VALUES) &&   \
                  ((tag) > IO_REPARSE_TAG_RESERVED_RANGE)      \
                )
#endif

#ifndef ENABLE_EXTENDED_FLAGS
# define ENABLE_QUICK_EDIT_MODE 64
# define ENABLE_EXTENDED_FLAGS  128
#endif

#if !defined(_W32API_OLD)
typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX
{
	ULONG cbSize;
	COORD dwSize;
	COORD dwCursorPosition;
	WORD wAttributes;
	SMALL_RECT srWindow;
	COORD dwMaximumWindowSize;
	WORD wPopupAttributes;
	BOOL bFullscreenSupported;
	COLORREF ColorTable[16];
}
CONSOLE_SCREEN_BUFFER_INFOEX, *PCONSOLE_SCREEN_BUFFER_INFOEX;
#endif

#define _WIN32_WINNT_WIN2K    0x0500
#define _WIN32_WINNT_WINXP    0x0501
#define _WIN32_WINNT_WS03     0x0502
#define _WIN32_WINNT_WIN6     0x0600
#define _WIN32_WINNT_VISTA    0x0600
#define _WIN32_WINNT_WS08     0x0600
#define _WIN32_WINNT_LONGHORN 0x0600
#define _WIN32_WINNT_WIN7     0x0601

#define RM_SESSION_KEY_LEN  sizeof(GUID)
#define CCH_RM_SESSION_KEY  RM_SESSION_KEY_LEN*2
#define CCH_RM_MAX_APP_NAME 255
#define CCH_RM_MAX_SVC_NAME 63

typedef enum _RM_APP_TYPE
{
	RmUnknownApp = 0,
	RmMainWindow = 1,
	RmOtherWindow = 2,
	RmService = 3,
	RmExplorer = 4,
	RmConsole = 5,
	RmCritical = 1000,
}
RM_APP_TYPE;

typedef struct _RM_UNIQUE_PROCESS
{
	DWORD dwProcessId;
	FILETIME ProcessStartTime;
}
RM_UNIQUE_PROCESS, *PRM_UNIQUE_PROCESS;

typedef struct _RM_PROCESS_INFO
{
	RM_UNIQUE_PROCESS Process;
	WCHAR strAppName[CCH_RM_MAX_APP_NAME+1];
	WCHAR strServiceShortName[CCH_RM_MAX_SVC_NAME+1];
	RM_APP_TYPE ApplicationType;
	ULONG AppStatus;
	DWORD TSSessionId;
	BOOL bRestartable;
}
RM_PROCESS_INFO, *PRM_PROCESS_INFO;

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#define PROCESS_QUERY_LIMITED_INFORMATION  0x1000
#endif
