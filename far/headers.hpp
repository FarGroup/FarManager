/*
headers.cpp

Стандартные заголовки

*/

/* Revision: 1.01 07.07.2000 $ */

/*
Modify:
  27.06.2000 AT
    + Данный патч сделан для использования предкомпилированных заголовков
  07.07.2000 SVS
    + stdarg.h - Для FarAdvControl
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif

#include <winioctl.h>

#ifndef __DOS_H
#include <dos.h>	// FA_*
#endif
#ifndef __DIR_H
#include <dir.h>	// chdir
#endif
#if !defined(__NEW_H)
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif
#ifndef __ALLOC_H
#include <alloc.h>
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
