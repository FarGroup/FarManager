#include "crt.hpp"
#pragma warning (disable : 4005)
#include <windows.h>

void * __cdecl calloc(size_t nmemb, size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size*nmemb);
}
