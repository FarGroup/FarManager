#include "stdafx.h"
#include "Memory.hpp"

void * my_malloc(size_t size)
{
  return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
}

void * my_realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(hHeap,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(hHeap,HEAP_ZERO_MEMORY, size);
}

void my_free(void *block)
{
  HeapFree(hHeap,0,block);
}


#ifdef __cplusplus
void * __cdecl operator new(size_t size)
{
  return my_malloc(size);
}

void __cdecl operator delete(void *block)
{
  my_free(block);
}
#endif


void _pure_error_ () {};

void * my_memcpy(void *dst, const void *src, size_t count)
{
  void *ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

void * __cdecl my_memset(void *dst, int val, size_t count)
{
  void *start = dst;

  while (count--)
  {
    *(char *)dst = (char)val;
    dst = (char *)dst + 1;
  }
  return(start);
}

int my_memcmp(const void *buf1, const void *buf2, size_t count)
{
  if (!count)
    return(0);

  while (--count && *(char *)buf1 == *(char *)buf2)
  {
    buf1 = (char *)buf1 + 1;
    buf2 = (char *)buf2 + 1;
  }

  return(*((unsigned char *)buf1) - *((unsigned char *)buf2));
}
