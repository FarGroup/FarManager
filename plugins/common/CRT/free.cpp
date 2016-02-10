#include "crt.hpp"
#pragma warning (disable : 4005)
#include <windows.h>

void __cdecl free(void *block)
{
  if (block)
    HeapFree(GetProcessHeap(),0,block);
}
