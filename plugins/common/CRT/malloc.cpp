#include "malloc.hpp"
#include <windows.h>

void *malloc(size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}
