/*
headers.cpp

Стандартные заголовки

*/

/* Revision: 1.12 27.02.2002 $ */

/*
Modify:
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


#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
 #if defined(_MSC_VER)
  #define _WINCON_ // to prevent including wincon.h
  #pragma pack(push,2)
  #include <windows.h>
  #undef _WINCON_
  #pragma pack(pop)
  #include <wincon.h> //this file wants 8-byte alignment
 #else
  #include <windows.h>
 #endif
#endif

#include <winioctl.h>

#undef FILE_ATTRIBUTE_ENCRYPTED
#define FILE_ATTRIBUTE_ENCRYPTED     0x00004000
//#define FILE_ATTRIBUTE_ENCRYPTED     0x00000040

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

#ifndef __DOS_H
#include <dos.h>	// FA_*
#endif
#ifndef __DIR_H
 #ifdef _MSC_VER
  #include <direct.h>	// chdir
 #else
  #include <dir.h>	// chdir
 #endif
#endif //__DIR_H
#if !defined(__NEW_H)
 #if defined(__BORLANDC__)
  #pragma option -p-
 #endif
  #include <new.h>
 #if defined(__BORLANDC__)
  #pragma option -p.
 #endif
#endif  //!defined(__NEW_H)
#ifndef __MALLOC_H
#include <malloc.h>
#endif
#ifndef __FCNTL_H
#include <fcntl.h>
#endif
#ifndef __IO_H
#include <io.h>
#endif
#ifndef __PROCESS_H
#include <process.h>
#endif
#ifndef __STDIO_H
#include <stdio.h>
#endif
#ifndef __STDLIB_H
#include <stdlib.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __STAT_H
#include <sys\stat.h>	// S_IREAD...
#endif
#ifndef __TIME_H
#include <time.h>
#endif
#ifndef __STDARG_H
#include <stdarg.h>
#endif
#ifndef __SHARE_H
#include <share.h>
#endif
#if _MSC_VER
  #define _export
  #define FA_DIREC _A_SUBDIR
  #define FA_RDONLY _A_RDONLY
  #define FA_HIDDEN _A_HIDDEN
  #define FA_SYSTEM _A_SYSTEM
  #define FA_RDONLY _A_RDONLY
  #define FA_ARCH   _A_ARCH
  #define setdisk(n) _chdrive((n)+1)

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
