/*
Eugene Kotlyarov <ek@oris.ru>
*/

//#define heapNEW GetProcessHeap()

//extern "C" void *          __cdecl _alloca(size_t);
//#define alloca  _alloca
//#pragma intrinsic(_alloca)

void * __cdecl malloc(size_t size)
{
  return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

void * __cdecl realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(heapNEW,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY, size);
}

void __cdecl free(void *block)
{
  HeapFree(heapNEW,0,block);
}


#ifdef __cplusplus
void * __cdecl operator new(size_t size)
{
  return malloc(size);
}

void __cdecl operator delete(void *block)
{
  free(block);
}
#endif


void _pure_error_ () {};
