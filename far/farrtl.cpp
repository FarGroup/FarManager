/*
farrtl.cpp

Переопределение функций работы с памятью: new/delete/malloc/realloc/free
*/

/* Revision: 1.05 19.07.2000 $ */

/*
Modify:
  03.07.2000 IS
    ! Включение сего файла в проект
  04.07.2000 SVS
    ! Выделение в качестве самодостаточного модуля!
  05.07.2000 IS
    ! Добавил кучу проверок, почти заново написал все, но фар все равно
      рушится, если переопределить new/delete :-(((
  11.07.2000 SVS
    ! Более разумное (с запасом) распределение памяти.
  12.07.2000 SVS
    ! Увеличение MEM_DELTA до 4095
    + Включение операторов new/delete by IS
    + Включение cmem в качестве смотрелки памяти.
  12.07.2000 IS
    ! В new заменил "void *p" на "void *p=NULL" (SVS забыл это сделать...)
    ! Проверка на NULL "переехала" из delete в free
  13.07.2000 SVS (с подачи VVM)
    ! VVM> Где я выделил - если block = NULL, то 95/98 возвращают
      VVM> ошибку, а НТ проглатывает...
      VVM> if(!size)
      VVM>   HeapFree(FARHeapForNew,0,block);
  19.07.2000 SVS
    + Добавлена функция getdisk
      Из-за различий в реализации функции getdisk в BC & VC
      не работал AltFx если панель имела UNC путь
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


