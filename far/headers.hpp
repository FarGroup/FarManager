#ifndef __HEADERS_HPP__
#define __HEADERS_HPP__
/*
headers.hpp

����������� ���������

*/

/* Revision: 1.29 28.06.2006 $ */

#define STRICT

#if defined(__BORLANDC__)
  #pragma option -a2
#endif

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
 #if defined(__GNUC__) || defined(_MSC_VER)
  #if !defined(_WINCON_H) && !defined(_WINCON_)
    #define _WINCON_H
    #define _WINCON_ // to prevent including wincon.h
    #if defined(_MSC_VER)
     #pragma pack(push,2)
    #else
     #pragma pack(2)
    #endif
    #include<windows.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
    #undef _WINCON_
    #undef  _WINCON_H

    #if defined(_MSC_VER)
     #pragma pack(push,8)
    #else
     #pragma pack(8)
    #endif
    #include<wincon.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
  #endif
  #define _WINCON_
 #else
   #include<windows.h>
 #endif
#endif


#include <winioctl.h>

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
#if !defined(__NEW_H) && !defined(_NEW)
 #if defined(__BORLANDC__)
  #pragma option -p-
 #endif
  #include <new.h>
 #if defined(__BORLANDC__)
  #pragma option -p.
 #endif
#endif  //!defined(__NEW_H)
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
    - ����������� ����, ��-�� ������� �� ������� ScreenSaver �����
      ���������� VC++
  */
  #ifndef RAND_MAX
  #define RAND_MAX 0x7fffU
  #endif
  #define randomize() srand(67898)
  #define random(x) ((int) (((x) *  rand()) / (RAND_MAX+1)) )
  /* OT $ */

  /* $ 19.07.2000 SVS
    - ��-�� �������� � ���������� ������� getdisk � BC & VC
      �� ������� AltFx ���� ������ ����� UNC ����
      ���� ������� ��������� � farrtl.cpp
  */
  #ifdef  __cplusplus
  extern "C" {
  #endif
  int _cdecl getdisk(void);
  #ifdef  __cplusplus
  }
  #endif
  /* SVS $ */

  #pragma warning (once:4018)
#endif

#if defined(__BORLANDC__)
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
#define vsnwprintf _vsnwprintf
  #if defined(__GNUC__)
    #define _strtoi64 strtoll
    #define _wcstoi64 wcstoll
  #endif
#else
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define vsnprintf(a,b,c,d) vsprintf(a,c,d)
#define vsnwprintf(a,b,c,d) vswprintf(a,c,d)
#endif
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
  extern "C" {
  #endif
  __int64 _cdecl _strtoi64(const char *nptr,char **endptr,int ibase);
  #ifdef  __cplusplus
  }
  #endif
#endif

#if defined(__GNUC__) || (defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)) || (defined(_MSC_VER) && _MSC_VER <= 1200) // defined(_DEBUG)

#ifndef _DWORDLONG_
typedef unsigned __int64 DWORDLONG;
typedef DWORDLONG *PDWORDLONG;
#endif // !_DWORDLONG_

typedef struct _MEMORYSTATUSEX {
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

#endif // __HEADERS_HPP__
