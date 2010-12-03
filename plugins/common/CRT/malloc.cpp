#include "crt.hpp"
#include <windows.h>

void * __cdecl malloc(size_t size)
{
  return HeapAlloc(GetProcessHeap(),0,size);
}
