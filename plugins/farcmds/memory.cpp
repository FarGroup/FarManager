#ifndef __memory_cpp
#define __memory_cpp

HANDLE heap;

void *malloc(size_t size)
{
  return HeapAlloc(heap,HEAP_ZERO_MEMORY,size);
}

void *realloc(void *block, size_t size)
{
  void *p=NULL;
  if(heap!=NULL)

  if(!size) HeapFree(heap,0,block);
  else
   {
    if(block==NULL) p=HeapAlloc(heap,HEAP_ZERO_MEMORY,size);
    else p=HeapReAlloc(heap,HEAP_ZERO_MEMORY,block,size);
   }

  return p;
}

void free(void *block)
{
 if(block!=NULL && heap) HeapFree(heap,0,block);
}

#endif