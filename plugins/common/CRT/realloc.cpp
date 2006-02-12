#include "realloc.hpp"
#include <windows.h>

void *realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}
