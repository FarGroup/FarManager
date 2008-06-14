#include "crt.hpp"
#include <windows.h>

void * __cdecl calloc(size_t nmemb, size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size*nmemb);
}
