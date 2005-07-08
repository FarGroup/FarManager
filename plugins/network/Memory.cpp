/*
Eugene Kotlyarov <ek@oris.ru>
*/

//#include <windows.h>

#define heapNEW GetProcessHeap()

void *my_malloc(size_t size)
{
  return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

void *my_realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(heapNEW,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY, size);
}

void my_free(void *block)
{
  HeapFree(heapNEW,0,block);
}


#ifdef __cplusplus
void * cdecl operator new(size_t size)
{
  return my_malloc(size);
}

void cdecl operator delete(void *block)
{
  my_free(block);
}
#endif


void cdecl _pure_error_ ()
{
};
