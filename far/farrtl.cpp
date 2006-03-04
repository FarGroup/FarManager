/*
farrtl.cpp

Переопределение функций работы с памятью: new/delete/malloc/realloc/free
*/

/* Revision: 1.23 02.03.2006 $ */

/*
Modify:
  02.03.2006 SVS
    + кусок кода из RTL strtoq.c для борманда (_strtoi64)
  24.04.2005 AY
    ! GCC
  09.10.2004 SVS
    - BugZ#1170 - Strange viewer behaviour if filesize > 0xFFFFFFFF
  09.08.2004 SKV
    - ftell64, fseek64 fix.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  01.06.2003 SVS
    ! FAR_CreateFile переехал из farrtl.cpp в farwinapi.cpp
  25.02.2003 SVS
    ! применим счетчик CallNewDelete/CallMallocFree для отладки
  20.02.2003 SVS
    ! xf_* будут __cdecl, т.к. юзаются в strdup.c, del.cpp и new.cpp
  26.01.2003 IS
    + FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  21.03.2002 SVS
    ! переезд функций FarBsearch, FarSscanf, FarSprintf, FarQsortEx,
      FarQsort, FarAtoi64, FarAtoi, FarItoa64, FarItoa из mix.cpp
      в farrtl.cpp
  27.02.2002 SVS
    ! кусок кода вынесен нафиг в cmem.cpp
  26.02.2002 SVS
    - Бага с RTL VC (теперь все нормально с V64 под MSVC!)
  22.02.2002 SVS
    ! Коррекция fseek64 и ftell64 (в т.ч. снесен модификатор WINAPI)
  10.07.2001 SVS
    - Забыли в свое время включить "fn.hpp" :-(
  31.10.2000 SVS
    ! Нормально можно и без объявления _nfile прожить (BC5.5.1 ругается)
  29.08.2000 SVS
    ! Уточнения для функций семейства seek под __int64
  14.08.2000 SVS
    + Функции семейства seek под __int64
  19.07.2000 SVS
    + Добавлена функция getdisk
      Из-за различий в реализации функции getdisk в BC & VC
      не работал AltFx если панель имела UNC путь
  13.07.2000 SVS (с подачи VVM)
    ! VVM> Где я выделил - если block = NULL, то 95/98 возвращают
      VVM> ошибку, а НТ проглатывает...
      VVM> if(!size)
      VVM>   HeapFree(FARHeapForNew,0,block);
  12.07.2000 IS
    ! В new заменил "void *p" на "void *p=NULL" (SVS забыл это сделать...)
    ! Проверка на NULL "переехала" из delete в free
  12.07.2000 SVS
    ! Увеличение MEM_DELTA до 4095
    + Включение операторов new/delete by IS
    + Включение cmem в качестве смотрелки памяти.
  11.07.2000 SVS
    ! Более разумное (с запасом) распределение памяти.
  05.07.2000 IS
    ! Добавил кучу проверок, почти заново написал все, но фар все равно
      рушится, если переопределить new/delete :-(((
  04.07.2000 SVS
    ! Выделение в качестве самодостаточного модуля!
  03.07.2000 IS
    ! Включение сего файла в проект
*/

#include "headers.hpp"
#pragma hdrstop
#include "fn.hpp"
#include "farconst.hpp"

/* $ 19.07.2000 SVS
  - Из-за различий в реализации функции getdisk в BC & VC
    не работал AltFx если панель имела UNC путь
    Сама функция находится в farrtl.cpp
*/
int _cdecl getdisk(void)
{
  unsigned drive;
  char    buf[MAX_PATH];

  /* Use GetCurrentDirectory to get the current directory path, then
   * parse the drive name.
   */
  GetCurrentDirectory(sizeof(buf), buf);    /* ignore errors */
  drive = buf[0] >= 'a' ? buf[0] - 'a' + 1 : buf[0] - 'A' + 1;
  return (int)drive - 1;
}
/* SVS $*/

/* $ 14.08.2000 SVS
    + Функции семейства seek под __int64
*/
#if defined(__BORLANDC__)
//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

int  __IOerror    (int  __doserror);   /* returns -1 */
int  __NTerror     (void);              /* returns -1 */

#ifdef __cplusplus
};
#endif

#define RETURN(code)    {rc=(code); goto exit;}

//extern unsigned  _nfile;

/* Array of open file flags.
 */
extern unsigned int _openfd[];

/* Array of open file handles (not used on OS/2).
 */
extern long _handles[];

static int Displacement (FILE *fp)
{
  register    level;
  int         disp;
  register    unsigned char *P;

  if (fp->level < 0)
    disp = level = fp->bsize + fp->level + 1;
  else
    disp = level = fp->level;

  if (fp->flags & _F_BIN)
    return disp;

  P = fp->curp;

  if (fp->level < 0)          /* If writing */
  {
    while (level--)
     if ('\n' == *--P)
       disp++;
  }
  else                        /* Else reading */
  {
    while (level--)
     if ('\n' == *P++)
       disp++;
  }

  return  disp;
}


static __int64 __lseek64(int fd, __int64 offset, int kind)
{
  FAR_INT64 Number;

  DWORD  method;

  if ((unsigned)fd >= _nfile)
    return (__int64)__IOerror(ERROR_INVALID_HANDLE);

  /* Translate the POSIX seek type to the NT method.
   */
  switch (kind)
  {
    case SEEK_SET:
      method = FILE_BEGIN;
      break;
    case SEEK_CUR:
      method = FILE_CURRENT;
      break;
    case SEEK_END:
      method = FILE_END;
      break;
    default:
      return ((__int64)__IOerror(ERROR_INVALID_FUNCTION));
  }

  _openfd[fd] &= ~_O_EOF;      /* forget about ^Z      */

  Number.Part.HighPart=offset>>32;

  Number.Part.LowPart = SetFilePointer((HANDLE)_handles[fd], (DWORD)offset, &Number.Part.HighPart, method);
  if (Number.Part.LowPart == -1 && GetLastError() != NO_ERROR)
  {
    __NTerror();        /* set errno */
    Number.i64=-1;
  }

  return Number.i64;
}

int fseek64 (FILE *fp, __int64 offset, int whence)
{
  if (fflush (fp))
    return (EOF);

  if (SEEK_CUR == whence && fp->level > 0)
    offset -= Displacement (fp);

  fp->flags &= ~(_F_OUT | _F_IN | _F_EOF);
  fp->level = 0;
  fp->curp = fp->buffer;

  return (__lseek64 (fp->fd, offset, whence) == -1L) ? EOF : 0;
}

__int64 ftell64(FILE *fp)
{
  __int64  oldpos, rc;

  if ((rc = __lseek64( fp->fd, 0, SEEK_CUR )) != -1)
  {
    if (fp->level < 0)  /* if writing */
    {
      if (_openfd[fp->fd] & O_APPEND)
      {
        /* In append mode, find out how big the file is,
         * and add the number of buffered bytes to that.
         */
        oldpos = rc;
        if ((rc = __lseek64( fp->fd, 0, SEEK_END )) == -1)
          return rc;
        if (__lseek64( fp->fd, oldpos, SEEK_SET ) == -1)
          return -1;
      }
      rc += Displacement(fp);
    }
    else
      rc -= Displacement(fp);
  }
  return rc;
}

#else

#include <io.h>
#if !defined(FAR_MSVCRT) && defined(_MSC_VER)
extern "C"{
_CRTIMP __int64 __cdecl _ftelli64 (FILE *str);
_CRTIMP int __cdecl _fseeki64 (FILE *stream, __int64 offset, int whence);
};
#endif

__int64 ftell64(FILE *fp)
{
#ifndef FAR_MSVCRT
  #ifdef __GNUC__
    return ftello64(fp);
  #else
    return _ftelli64(fp);
  #endif
#else
  fpos_t pos;
  if(fgetpos(fp,&pos)!=0)return 0;
  return pos;
#endif
}

int fseek64 (FILE *fp, __int64 offset, int whence)
{
#ifndef FAR_MSVCRT
  #ifdef __GNUC__
    return fseeko64(fp,offset,whence);
  #else
    return _fseeki64(fp,offset,whence);
  #endif
#else
  switch(whence)
  {
    case SEEK_SET:return fsetpos(fp,&offset);
    case SEEK_CUR:
    {
      fpos_t pos;
      fgetpos(fp,&pos);
      pos+=offset;
      return fsetpos(fp,&pos);
    }
    case SEEK_END:
    {
      fpos_t pos;
      fseek(fp,0,SEEK_END);
      fgetpos(fp,&pos);
      pos+=offset;
      return fsetpos(fp,&pos);
    }
  }
  return -1;
#endif
}

#endif
/* SVS $*/

/* $ 25.07.2000 SVS
   Оболочки вокруг вызовов стандартных функцйи, приведенных к WINAPI
*/
char *WINAPI FarItoa(int value, char *string, int radix)
{
  if(string)
    return itoa(value,string,radix);
  return NULL;
}
/* $ 28.08.2000 SVS
  + FarItoa64
*/
char *WINAPI FarItoa64(__int64 value, char *string, int radix)
{
  if(string)
    return _i64toa(value, string, radix);
  return NULL;
}
/* SVS $ */
int WINAPI FarAtoi(const char *s)
{
  if(s)
    return atoi(s);
  return 0;
}
__int64 WINAPI FarAtoi64(const char *s)
{
  if(s)
    return _atoi64(s);
  return _i64(0);
}

void WINAPI FarQsort(void *base, size_t nelem, size_t width,
                     int (__cdecl *fcmp)(const void *, const void *))
{
  if(base && fcmp)
    far_qsort(base,nelem,width,fcmp);
}

/* $ 24.03.2001 tran
   новая фишка...*/
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width,
                     int (__cdecl *fcmp)(const void *, const void *,void *user),void *user)
{
  if(base && fcmp)
    qsortex((char*)base,nelem,width,fcmp,user);
}
/* tran $ */

int WINAPIV FarSprintf(char *buffer,const char *format,...)
{
  int ret=0;
  if(buffer && format)
  {
    va_list argptr;
    va_start(argptr,format);
    ret=vsprintf(buffer,format,argptr);
    va_end(argptr);
  }
  return ret;
}

#ifndef FAR_MSVCRT
/* $ 29.08.2000 SVS
   - Неверно отрабатывала функция FarSscanf
   Причина - т.к. у VC нету vsscanf, то пришлось смоделировать (взять из
   исходников VC sscanf и "нарисовать" ее сюда
*/
#if defined(_MSC_VER)
extern "C" {
int __cdecl _input (FILE *stream,const unsigned char *format,va_list arglist);
};
#endif

int WINAPIV FarSscanf(const char *buffer, const char *format,...)
{
  if(!buffer || !format)
    return 0;
#if defined(_MSC_VER)
  // полная копия внутренностей sscanf :-)
  va_list arglist;
  FILE str;
  FILE *infile = &str;
  int retval;

  va_start(arglist, format);

  infile->_flag = _IOREAD|_IOSTRG|_IOMYBUF;
  infile->_ptr = infile->_base = (char *) buffer;
  infile->_cnt = strlen(buffer);

  retval = (_input(infile,(const unsigned char *)format,arglist));

  return(retval);
#else
  va_list argptr;
  va_start(argptr,format);
  int ret=vsscanf(buffer,format,argptr);
  va_end(argptr);
  return ret;
#endif
}
/* 29.08.2000 SVS $ */
/* SVS $ */

#endif


/* $ 07.09.2000 SVS
   Оболочка FarBsearch для плагинов (функция bsearch)
*/
void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
  if(key && fcmp && base)
    return bsearch(key,base,nelem,width,fcmp);
  return NULL;
}
/* SVS $ */

#if defined(SYSLOG)
extern long CallMallocFree;
#endif

void __cdecl  xf_free(void *__block)
{
#if defined(SYSLOG)
  CallMallocFree--;
#endif
  free(__block);
}

void *__cdecl xf_malloc(size_t __size)
{
  void *Ptr=malloc(__size);
#if defined(SYSLOG)
  CallMallocFree++;
#endif
  return Ptr;
}

void *__cdecl xf_realloc(void *__block, size_t __size)
{
  void *Ptr=realloc(__block,__size);
#if defined(SYSLOG)
  if(!__block)
    CallMallocFree++;
#endif
  return Ptr;
}

#if defined(__BORLANDC__)

#define _UI64_MAX     0xffffffffffffffffui64
#define _I64_MIN      (-9223372036854775807i64 - 1)
#define _I64_MAX      9223372036854775807i64
#define ERANGE        34

#define FL_UNSIGNED   1       /* strtouq called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

static unsigned __int64 strtoxq (
    const char *nptr,
    const char **endptr,
    int ibase,
    int flags
    )
{
    const char *p;
    char c;
    unsigned __int64 number;
    unsigned digval;
    unsigned __int64 maxval;

    p = nptr;            /* p is our scanning pointer */
    number = 0;            /* start with zero */

    c = *p++;            /* read char */
    while ( isspace((int)(unsigned char)c) )
        c = *p++;        /* skip whitespace */

    if (c == '-') {
        flags |= FL_NEG;    /* remember minus sign */
        c = *p++;
    }
    else if (c == '+')
        c = *p++;        /* skip sign */

    if (ibase < 0 || ibase == 1 || ibase > 36) {
        /* bad base! */
        if (endptr)
            /* store beginning of string in endptr */
            *endptr = nptr;
        return 0L;        /* return 0 */
    }
    else if (ibase == 0) {
        /* determine base free-lance, based on first two chars of
           string */
        if (c != '0')
            ibase = 10;
        else if (*p == 'x' || *p == 'X')
            ibase = 16;
        else
            ibase = 8;
    }

    if (ibase == 16) {
        /* we might have 0x in front of number; remove if there */
        if (c == '0' && (*p == 'x' || *p == 'X')) {
            ++p;
            c = *p++;    /* advance past prefix */
        }
    }

    /* if our number exceeds this, we will overflow on multiply */
    maxval = _UI64_MAX / ibase;


    for (;;) {    /* exit in middle of loop */
        /* convert c to value */
        if ( isdigit((int)(unsigned char)c) )
            digval = c - '0';
        else if ( isalpha((int)(unsigned char)c) )
            digval = toupper(c) - 'A' + 10;
        else
            break;
        if (digval >= (unsigned)ibase)
            break;        /* exit loop if bad digit found */

        /* record the fact we have read one digit */
        flags |= FL_READDIGIT;

        /* we now need to compute number = number * base + digval,
           but we need to know if overflow occured.  This requires
           a tricky pre-check. */

        if (number < maxval || (number == maxval &&
        (unsigned __int64)digval <= _UI64_MAX % ibase)) {
            /* we won't overflow, go ahead and multiply */
            number = number * ibase + digval;
        }
        else {
            /* we would have overflowed -- set the overflow flag */
            flags |= FL_OVERFLOW;
        }

        c = *p++;        /* read next digit */
    }

    --p;                /* point to place that stopped scan */

    if (!(flags & FL_READDIGIT)) {
        /* no number there; return 0 and point to beginning of
           string */
        if (endptr)
            /* store beginning of string in endptr later on */
            p = nptr;
        number = 0L;        /* return 0 */
    }
    else if ( (flags & FL_OVERFLOW) ||
              ( !(flags & FL_UNSIGNED) &&
                ( ( (flags & FL_NEG) && (number > -_I64_MIN) ) ||
                  ( !(flags & FL_NEG) && (number > _I64_MAX) ) ) ) )
    {
        /* overflow or signed overflow occurred */
        errno = ERANGE;
        if ( flags & FL_UNSIGNED )
            number = _UI64_MAX;
        else if ( flags & FL_NEG )
            number = _I64_MIN;
        else
            number = _I64_MAX;
    }
    if (endptr != NULL)
        /* store pointer to char that stopped the scan */
        *endptr = p;

    if (flags & FL_NEG)
        /* negate result if there was a neg sign */
        number = (unsigned __int64)(-(__int64)number);

    return number;            /* done. */
}

__int64 _cdecl _strtoi64(const char *nptr,char **endptr,int ibase)
{
    return (__int64) strtoxq(nptr, (const char **)endptr, ibase, 0);
}

#endif
