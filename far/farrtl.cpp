/*
farrtl.cpp

Переопределение функций работы с памятью: new/delete/malloc/realloc/free
*/

/* Revision: 1.03 11.07.2000 $ */

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

*/

#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned size_t;

void *malloc(size_t size);
void *realloc(void *block, size_t size);
void free(void *block);

#ifdef __cplusplus
}
#endif

#include "headers.hpp"
#pragma hdrstop

#define MEM_DELTA	1023

static HANDLE FARHeapForNew=NULL;

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
      HeapFree(FARHeapForNew,0,block);
    else if (!block) // В первый раз даем столько, сколько
                     // просят. Что-бы небыло лишнего расхода при единичном вызове
      p=HeapAlloc(FARHeapForNew,HEAP_ZERO_MEMORY, size);
    else
    {
      p=block;
      // Required size, alligned to MEM_DELTA
      size  = (size + MEM_DELTA) & (~MEM_DELTA);
      // Current allocated size
      size2 = HeapSize(FARHeapForNew,0,block);
      if (size  > size2 ||          // Запросили больше, чем реально выделено
          size2 - size > MEM_DELTA) // Надо освободить блок размером с MEM_DELTA или больше
        p=HeapReAlloc(FARHeapForNew,HEAP_ZERO_MEMORY,block,size);
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
  if(!FARHeapForNew) FARHeapForNew=GetProcessHeap();
  if(FARHeapForNew) HeapFree(FARHeapForNew,0,block);
}

#if 0 //пока без этого поработаем, ибо рушится фар по непонятным причинам
void *operator new(size_t sz)
 {
  extern new_handler _new_handler;
  void *p;
  sz=sz?sz:1;
  while((p=malloc(sz))==NULL)
   {
    if(_new_handler!=NULL)_new_handler();
    else break;
   }
  return p;
 }
void operator delete(void *v)
 {
  if(v)free(v);
 }
void *operator new[](size_t sz)
 {
  extern new_handler _new_handler;
  void *p;
  sz=sz?sz:1;
  while((p=malloc(sz))==NULL)
   {
    if(_new_handler!=NULL)_new_handler();
    else break;
   }
  return p;
 }
void operator delete[](void *v)
 {
  if(v)free(v);
 }
#endif
