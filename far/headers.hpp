#ifndef __HEADERS_HPP__
#define __HEADERS_HPP__
/*
headers.hpp

Стандартные заголовки

*/

/* Revision: 1.24 10.04.2006 $ */

/*
Modify:
  10.04.2006 SVS
    + MEMORYSTATUSEX для Borland < 5.5
  06.04.2006 AY
    ! GCC
  12.03.2006 SVS
    ! _strtoi64 кроме борманда так же компилим в дебажной MSVC.
  02.03.2006 SVS
    + инфа для борманда про _strtoi64 и REG_QWORD
  22.12.2005 SVS
    + FILE_ATTRIBUTE_TEMPORARY
  07.12.2005 SVS
    + FILE_ATTRIBUTE_SPARSE_FILE
  27.10.2005 SVS
    + FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
  30.09.2005 SVS
    + несколько констант из SDK для совместимости с ранними версиями компиляторов
  23.07.2005 SVS
    + про vsnprintf()
  24.04.2005 AY
    ! GCC
  06.05.2003 SVS
    - Проблемы с VC после 1645
  04.04.2002 SVS
    - Хе... lfind видите ли им не понравился ;-((
  27.02.2002 SVS
    + mmsystem.h при условии, что выставлен флаг __USE_MCI, который
      компилит ФАР с юзанием константами для mciSendCommand
  22.10.2001 SVS
    + По поводу шифрования в Win2K (борманд 5.02 об этих константах не ведает!)
  25.04.2001 SVS
    + FILE_SUPPORTS_REPARSE_POINTS
  08.04.2001 SVS
    ! вместо alloc.h просто вызываем malloc.h
  24.10.2000 SVS
    + share.h
  20.10.2000 SVS
    + FILE_SUPPORTS_ENCRYPTION,
      FILE_ATTRIBUTE_ENCRYPTED,
      FS_FILE_ENCRYPTION
    ! FILE_ATTRIBUTE_REPARSE_POINT перенесен из farconst.hpp в headers.hpp
  11.10.2000 SVS
    ! В BC RAND_MAX = 0x7fffU, а не 0x7fff
  19.09.2000 SVS
    + выравнивание на 2 байта
  19.07.2000 SVS
    - Из-за различий в реализации функции getdisk в BC & VC
      не работал AltFx если панель имела UNC путь
  12.07.2000 OT
    - Исправление бага, из-за которго не работал ScreenSaver после
      компиляции VC++
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  07.07.2000 SVS
    + stdarg.h - Для FarAdvControl
  27.06.2000 AT
    + Данный патч сделан для использования предкомпилированных заголовков
*/

#define STRICT

#if defined(__BORLANDC__)
  #pragma option -a2
#endif


#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_) && !defined(_WINDOWS_H)
 #if defined(__GNUC__) || defined(_MSC_VER)
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
  #if defined(__GNUC__)
    #define _strtoi64 strtoll
  #endif
#else
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
#define vsnprintf(a,b,c,d) vsprintf(a,c,d)
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

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)

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
