#include "crt.hpp"
#include <windows.h>

void * __cdecl realloc(void *block, size_t size)
{
  if (!size)
  {
    if (block)
      HeapFree(GetProcessHeap(),0,block);
    return NULL;
  }

  if (block)
    return HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}
