/*
Eugene Kotlyarov <ek@oris.ru>
*/

#include "Memory.hpp"
#ifdef _HEAP_ALLOC_MEM_
#include <windows.h>
#else
#include <stdlib.h>
#endif

#define heapNEW GetProcessHeap()

void *my_malloc(size_t size)
{
#ifdef _HEAP_ALLOC_MEM_
  return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
#else
  return malloc(size);
#endif
}

void *my_realloc(void *block, size_t size)
{
#ifdef _HEAP_ALLOC_MEM_
  if (block)
    return HeapReAlloc(heapNEW,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY, size);
#else
  return realloc(block, size);
#endif
}

void my_free(void *block)
{
#ifdef _HEAP_ALLOC_MEM_
  HeapFree(heapNEW,0,block);
#else
  free(block);
#endif
}


#ifdef __cplusplus
void * cdecl operator new(size_t size)
{
  return my_malloc(size);
}

void cdecl operator delete(void *block)
{
  my_free(block);
}
#endif


void cdecl _pure_error_ ()
{
};
