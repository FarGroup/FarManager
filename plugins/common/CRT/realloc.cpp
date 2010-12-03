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
    return HeapReAlloc(GetProcessHeap(),0,block,size);
  else
    return HeapAlloc(GetProcessHeap(),0,size);
}

void * __cdecl _recalloc(void *block, size_t num, size_t size)
{
  size_t tot = num*size;
  if(tot < num || tot < size) return 0;
  block = realloc(block, tot);
  if(block && tot) memset(block, 0, tot);
  return block;
}
