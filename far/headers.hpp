#ifndef __HEADERS_HPP__
#define __HEADERS_HPP__
/*
headers.hpp

Стандартные заголовки

*/


#define STRICT

#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600
#if defined(__GNUC__)
#define WINVER 0x0500
#endif

#if defined(__BORLANDC__) && !defined(_WIN64)
#pragma option -a2
#endif


#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_) && !defined(_WINDOWS_H)
#if (defined(__GNUC__) || defined(_MSC_VER)) && !defined(_WIN64)
#define _WINCON_H
#define _WINCON_ // to prevent including wincon.h
#if defined(_MSC_VER)
#pragma pack(push,2)
#else
#pragma pack(2)
#endif
#include <windows.h>
#if defined(_MSC_VER)
#pragma pack(pop)
#else
#pragma pack()
#endif
#undef _WINCON_
#undef  _WINCON_H
#include <wincon.h> //this file wants 8-byte alignment
#else
#include <windows.h>
#endif
#endif

#include <winioctl.h>

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#undef FILE_ATTRIBUTE_ENCRYPTED
#define FILE_ATTRIBUTE_ENCRYPTED     0x00004000
//#define FILE_ATTRIBUTE_ENCRYPTED     0x00000040

#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100

#define FILE_NAMED_STREAMS              0x00040000
#define FILE_READ_ONLY_VOLUME           0x00080000
#define FILE_SUPPORTS_OBJECT_IDS        0x00010000
#define FILE_SUPPORTS_SPARSE_FILES      0x00000040
#define FILE_VOLUME_QUOTAS              0x00000020

#if !defined(FILE_SUPPORTS_ENCRYPTION)
#define FILE_SUPPORTS_ENCRYPTION     0x00020000
#endif
#if !defined(FS_FILE_ENCRYPTION)
#define FS_FILE_ENCRYPTION           FILE_SUPPORTS_ENCRYPTION
#endif

#if !defined(FILE_ATTRIBUTE_REPARSE_POINT)
#define FILE_ATTRIBUTE_REPARSE_POINT 0x400
#endif

#if !defined(FILE_SUPPORTS_REPARSE_POINTS)
#define FILE_SUPPORTS_REPARSE_POINTS    0x00000080
#endif

#if !defined(IO_REPARSE_TAG_MOUNT_POINT)
#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)
#endif
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)
#define IO_REPARSE_TAG_SIS                      (0x80000007L)
#define IO_REPARSE_TAG_DFS                      (0x8000000AL)
#if !defined(IO_REPARSE_TAG_SYMLINK)
#define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)
#endif
#define IO_REPARSE_TAG_DFSR                     (0x80000012L)

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#endif

#ifndef FSCTL_SET_SPARSE
#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#endif

#ifndef FSCTL_QUERY_ALLOCATED_RANGES
#define FSCTL_QUERY_ALLOCATED_RANGES    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 51,  METHOD_NEITHER, FILE_READ_DATA)  // FILE_ALLOCATED_RANGE_BUFFER, FILE_ALLOCATED_RANGE_BUFFER

typedef struct _FILE_ALLOCATED_RANGE_BUFFER
{

	LARGE_INTEGER FileOffset;
	LARGE_INTEGER Length;

} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;
#endif

#if !defined(FILE_ATTRIBUTE_VIRTUAL)
#define FILE_ATTRIBUTE_VIRTUAL              0x00010000
#endif

#if !defined(FILE_ATTRIBUTE_DEVICE)
#define FILE_ATTRIBUTE_DEVICE               0x00000040
#endif

#if !defined(__DOS_H) && !defined(_DOS_H_)
#include <dos.h>  // FA_*
#endif
#if !defined(__DIR_H) && !defined(_DIRECT_H_)
#if defined(_MSC_VER) || defined(__GNUC__)
#include <direct.h> // chdir
#else
#include <dir.h>  // chdir
#endif
#endif //__DIR_H
#ifdef __cplusplus
#if defined(__BORLANDC__) && !defined(_WIN64)
#pragma option -p-
#endif
#if (defined(__BORLANDC__) && !defined(_WIN64)) || (defined(_MSC_VER) && _MSC_VER < 1400)
#include <new.h>
#else
#include <new>
#endif
#if defined(__BORLANDC__) && !defined(_WIN64)
#pragma option -p.
#endif
#endif
#if !defined(__MALLOC_H) && !defined(_MALLOC_H_)
#include <malloc.h>
#endif
#if !defined(__FCNTL_H) && !defined(_FCNTL_H_)
#include <fcntl.h>
#endif
#if !defined(__IO_H) && !defined(_IO_H_)
#include <io.h>
#endif
#if !defined(__PROCESS_H) && !defined(_PROCESS_H_)
#include <process.h>
#endif
#if !defined(__STDIO_H) && !defined(_STDIO_H_)
#include <stdio.h>
#endif
#if !defined(__STDLIB_H) && !defined(_STDLIB_H_)
#include <stdlib.h>
#endif
#if !defined(__STRING_H) && !defined(_STRING_H_)
#include <string.h>
#endif
#if !defined(__STAT_H) && !defined(_STAT_H_)
#include <sys\stat.h> // S_IREAD...
#endif
#if !defined(__TIME_H) && !defined(_TIME_H_)
#include <time.h>
#endif
#if !defined(__STDARG_H) && !defined(_STDARG_H)
#include <stdarg.h>
#endif
#if !defined(__SHARE_H) && !defined(_SHARE_H_)
#include <share.h>
#endif
#if !defined(__SEARCH_H) && !defined(_SEARCH_H_)
#include <search.h>
#endif
#ifdef __GNUC__
#define ultoa _ultoa
#if !defined(_CTYPE_H_)
#include <ctype.h>
#endif
#endif
#if defined(_MSC_VER) || defined(__GNUC__)
#if !defined(_INC_WCHAR) && !defined(_WCHAR_H_)
#include <wchar.h>
#define _wmemset wmemset
#endif
#define _export
#define FA_DIREC _A_SUBDIR
#define FA_RDONLY _A_RDONLY
#define FA_HIDDEN _A_HIDDEN
#define FA_SYSTEM _A_SYSTEM
#define FA_RDONLY _A_RDONLY
#define FA_ARCH   _A_ARCH
#define setdisk(n) _chdrive((n)+1)
#define lfind  _lfind

/* $ 12.07.2000 OT
  - Исправление бага, из-за которго не работал ScreenSaver после
    компиляции VC++
*/
#ifndef RAND_MAX
#define RAND_MAX 0x7fffU
#endif
#define randomize() srand(67898)
#define random(x) ((int) (((x) *  rand()) / (RAND_MAX+1)) )
/* OT $ */

/* $ 19.07.2000 SVS
  - Из-за различий в реализации функции getdisk в BC & VC
    не работал AltFx если панель имела UNC путь
    Сама функция находится в farrtl.cpp
*/
#ifdef  __cplusplus
extern "C"
{
#endif
	int _cdecl getdisk(void);
#ifdef  __cplusplus
}
#endif
/* SVS $ */

#pragma warning (once:4018)
#endif

#if defined(__BORLANDC__) && !defined(_WIN64)
#pragma option -a.
#endif

//  The specified file could not be encrypted.
#define ERROR_ENCRYPTION_FAILED          6000L
//  The specified file could not be decrypted.
#define ERROR_DECRYPTION_FAILED          6001L
//  There is no valid encryption recovery policy configured for this system.
#define ERROR_NO_RECOVERY_POLICY         6003L
//  The specified file is not encrypted.
#define ERROR_FILE_NOT_ENCRYPTED         6007L

#if !defined(__USE_MCI)
#include <mmsystem.h>
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#define vsnprintf _vsnprintf
#if defined(__GNUC__)
#define _strtoi64 strtoll
#endif
#else
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define vsnprintf(a,b,c,d) vsprintf(a,c,d)
#endif
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define _snprintf(a,b,c,d) sprintf(a,c,d)
#endif

#ifdef __GNUC__
#define _i64(num)   num##ll
#define _ui64(num)  num##ull
#else
#define _i64(num)   num##i64
#define _ui64(num)  num##ui64
#endif

#ifndef REG_QWORD
#define REG_QWORD                   ( 11 )  // 64-bit number
#endif

#if defined(__BORLANDC__) || defined(_DEBUG)
// (defined(_MSC_VER) && _MSC_VER < 1300)
#ifdef  __cplusplus
extern "C"
{
#endif
	__int64 _cdecl _strtoi64(const char *nptr,char **endptr,int ibase);
#ifdef  __cplusplus
}
#endif
#endif


#if defined(__BORLANDC__) || (defined(_MSC_VER) && _MSC_VER <= 1200)
#if (defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)) || (defined(_MSC_VER) && _MSC_VER <= 1200)
//#if !defined(__midl) && defined(_X86_)
//#define _W64 __w64
//#else
#define _W64
//#endif
#endif

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif
#endif


#if (defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)) || (defined(_MSC_VER) && _MSC_VER <= 1200)
//#if (__BORLANDC__ < 0x0550)
#if defined(_WIN64)
#if defined(__BORLANDC__)
typedef __int64 INT_PTR, *PINT_PTR;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
#endif

typedef __int64 LONG_PTR, *PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
typedef unsigned __int64    UINT64, *PUINT64;
#else
#if defined(__BORLANDC__)
typedef _W64 int INT_PTR, *PINT_PTR;
typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;
#endif

typedef _W64 long LONG_PTR, *PLONG_PTR;
typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
typedef _W64 unsigned __int64    UINT64, *PUINT64;
#endif
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
#endif


#if (defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)) || (defined(_MSC_VER) && _MSC_VER <= 1200) // defined(_DEBUG)
#ifndef _DWORDLONG_
typedef unsigned __int64 DWORDLONG;
typedef DWORDLONG *PDWORDLONG;
#endif // !_DWORDLONG_

typedef struct _MEMORYSTATUSEX
{
	DWORD dwLength;
	DWORD dwMemoryLoad;
	DWORDLONG ullTotalPhys;
	DWORDLONG ullAvailPhys;
	DWORDLONG ullTotalPageFile;
	DWORDLONG ullAvailPageFile;
	DWORDLONG ullTotalVirtual;
	DWORDLONG ullAvailVirtual;
	DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
typedef struct _IMAGE_OPTIONAL_HEADER IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_OPTIONAL_HEADER64
{
	WORD        Magic;
	BYTE        MajorLinkerVersion;
	BYTE        MinorLinkerVersion;
	DWORD       SizeOfCode;
	DWORD       SizeOfInitializedData;
	DWORD       SizeOfUninitializedData;
	DWORD       AddressOfEntryPoint;
	DWORD       BaseOfCode;
	ULONGLONG   ImageBase;
	DWORD       SectionAlignment;
	DWORD       FileAlignment;
	WORD        MajorOperatingSystemVersion;
	WORD        MinorOperatingSystemVersion;
	WORD        MajorImageVersion;
	WORD        MinorImageVersion;
	WORD        MajorSubsystemVersion;
	WORD        MinorSubsystemVersion;
	DWORD       Win32VersionValue;
	DWORD       SizeOfImage;
	DWORD       SizeOfHeaders;
	DWORD       CheckSum;
	WORD        Subsystem;
	WORD        DllCharacteristics;
	ULONGLONG   SizeOfStackReserve;
	ULONGLONG   SizeOfStackCommit;
	ULONGLONG   SizeOfHeapReserve;
	ULONGLONG   SizeOfHeapCommit;
	DWORD       LoaderFlags;
	DWORD       NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107
#endif

#if (defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)) || defined(__GNUC__)
/*
 * Bits in wParam of WM_INPUTLANGCHANGEREQUEST message
 */
#define INPUTLANGCHANGE_SYSCHARSET 0x0001
#define INPUTLANGCHANGE_FORWARD    0x0002
#define INPUTLANGCHANGE_BACKWARD   0x0004
#endif

#define __USE_MCI    1

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define VK_SLEEP          0x5F
#endif

#include <setupapi.h>

#if !(defined(__BORLANDC__) || defined(__GNUC__))
#include <shobjidl.h>
#else
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
#endif

#if defined(__BORLANDC__)
#define SEE_MASK_NOZONECHECKS      0x00800000
#endif

#endif // __HEADERS_HPP__
