#include "memory.hpp"
#include <windows.h>

#define heapNEW GetProcessHeap()

void *malloc(size_t size)
{
  return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

void *realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(heapNEW,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY, size);
}

void free(void *block)
{
  if (block)
    HeapFree(heapNEW,0,block);
}

#ifdef __cplusplus
void * __cdecl operator new(size_t size)
{
  return malloc(size);
}

void __cdecl operator delete(void *block)
{
  free(block);
}

void *__cdecl operator new[] (size_t size)
{
  return ::operator new(size);
}

void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}
#endif
