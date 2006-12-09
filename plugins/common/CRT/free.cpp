#include "crt.hpp"
#include <windows.h>

void __cdecl free(void *block)
{
  if (block)
    HeapFree(GetProcessHeap(),0,block);
}
