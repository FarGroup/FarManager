/*
headers.cpp

Стандартные заголовки

*/

/* Revision: 1.04 19.07.2000 $ */

/*
Modify:
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
#ifndef __ALLOC_H
 #ifdef _MSC_VER
  #include <malloc.h>
 #else
  #include <alloc.h>
 #endif
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
  #define RAND_MAX 0x7fff
  #endif
  #define randomize() srand(67898)
  #define random(x) ((int) (((x) *  rand()) / RAND_MAX) )
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
