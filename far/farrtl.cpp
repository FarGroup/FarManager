/*
farrtl.cpp

Переопределение функций работы с памятью: new/delete/malloc/realloc/free
*/

/* Revision: 1.07 29.08.2000 $ */

/*
Modify:
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

#if defined(__BORLANDC__)
#ifdef ALLOC

#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned size_t;

void *malloc(size_t size);
void *realloc(void *block, size_t size);
void *calloc(size_t nitems, size_t size);
void free(void *block);

#ifdef __cplusplus
}
#endif

#define MEM_DELTA	4095

static HANDLE FARHeapForNew=NULL;

void *calloc(size_t nitems, size_t size)
{
  return realloc(NULL,size*nitems);
}


void *malloc(size_t size)
{
  return realloc(NULL,size);
}


void *realloc(void *block, size_t size)
{
  void *p=NULL;
  int size2;

  if(!FARHeapForNew) FARHeapForNew=GetProcessHeap();

  if(FARHeapForNew)
  {
    if(!size)
    {
      if(block)
        HeapFree(FARHeapForNew,0,block);
    }
    else if (!block) // В первый раз даем столько, сколько
    {
                     // просят. Что-бы небыло лишнего расхода при единичном вызове
      p=HeapAlloc(FARHeapForNew,HEAP_ZERO_MEMORY, size);
    }
    else
    {
      p=block;
      // Required size, alligned to MEM_DELTA
      size  = (size + MEM_DELTA) & (~MEM_DELTA);
      // Current allocated size
      size2 = HeapSize(FARHeapForNew,0,block);
      if (size  > size2 ||          // Запросили больше, чем реально выделено
          size2 - size > MEM_DELTA) // Надо освободить блок размером с MEM_DELTA или больше
      {
        p=HeapReAlloc(FARHeapForNew,HEAP_ZERO_MEMORY,block,size);
      }
      // else
      /*
          Ничего. Память в хипе уже выделена в прошлый раз и мы не трогаем его.
          А программа считает, что произошли изменения...
      */
    }
  }
  return p;
}

void free(void *block)
{
 if(block!=NULL)
 {
  if(!FARHeapForNew) FARHeapForNew=GetProcessHeap();
  if(FARHeapForNew) HeapFree(FARHeapForNew,0,block);
 }
}

#ifdef CMEM_INCLUDE
#include "cmem.cpp"
#else //у _меня_ - работает, но тестировать еще надо...

#if 1
/*
    ! Свершилось!!! Народ, rtfm - рулезЪ forever :-)))
      Скачал я таки стандарт по C++:
         ftp://ftp.ldz.lv/pub/doc/ansi_iso_iec_14882_1998.pdf (размер 2860601)
      А там все черным по белому... Короче, переопределил я new/delete как надо
      (если быть точным, то пару способов не осуществил, т.к. мы без исключений
      работаем), в крайнем случае, фар у _меня_ больше не грохается! Кому
      интересны подробности, смотрите в указанном стандарте параграф 18.4
*/
void *operator new(size_t size)
 {
  extern new_handler _new_handler;
  void *p=NULL;
  size=size?size:1;
  while((p=malloc(size))==NULL)
   {
    if(_new_handler!=NULL)_new_handler();
    else break;
   }
  return p;
 }
void *operator new[](size_t size) {return ::operator new(size);}
void *operator new(size_t size, void *p) {return p;}
void operator delete(void *p) {free(p);}
void operator delete[](void *ptr) {::operator delete(ptr);}
#endif

#endif // CMEM_INCLUDE

#endif // ALLOC
#endif // defined(__BORLANDC__)



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

extern unsigned  _nfile;

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
    LONG   NewPtr,MoveHigh;
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

    MoveHigh=offset>>32;
    if ((NewPtr = SetFilePointer((HANDLE)_handles[fd], (DWORD)offset, &MoveHigh, method))
                == -1)
        __NTerror();        /* set errno */

    return (((__int64)MoveHigh)<<32)|NewPtr;
}

int WINAPI fseek64 (FILE *fp, __int64 offset, int whence)
{
    int rc;


    if (fflush (fp))
        return (EOF);

    if (SEEK_CUR == whence && fp->level > 0)
        offset -= Displacement (fp);

    fp->flags &= ~(_F_OUT | _F_IN | _F_EOF);
    fp->level = 0;
    fp->curp = fp->buffer;

    rc = (__lseek64 (fp->fd, offset, whence) == -1L) ? EOF : 0;

exit:
    return rc;

}

__int64 WINAPI ftell64(FILE *fp)
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
                    RETURN(rc);
                if (__lseek64( fp->fd, oldpos, SEEK_SET ) == -1)
                    RETURN(-1);
            }
            rc += Displacement(fp);
        }
        else
            rc -= Displacement(fp);
    }
exit:
    return rc;
}

#else
_CRTIMP __int64 __cdecl _lseeki64(int, __int64, int);
_CRTIMP __int64 __cdecl _telli64(int);

__int64 WINAPI ftell64(FILE *fp)
{
  return _telli64(fileno(fp));
}

int WINAPI fseek64 (FILE *fp, __int64 offset, int whence)
{
  return (int)_lseeki64(fileno(fp),offset,whence);
}

#endif
/* SVS $*/
