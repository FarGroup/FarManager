#include "crt.hpp"
#include <windows.h>

void * __cdecl calloc(size_t nmemb, size_t size)
{
  size *= nmemb;
  void *p = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
  if (p) memset(p, 0, size);
  return p;
}
