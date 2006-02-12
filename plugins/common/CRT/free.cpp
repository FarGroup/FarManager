#include "free.hpp"
#include <windows.h>

void free(void *block)
{
  if (block)
    HeapFree(GetProcessHeap(),0,block);
}
