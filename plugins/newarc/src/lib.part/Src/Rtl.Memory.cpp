#include <Rtl.Base.h>
#include <stddef.h>

#define heapNEW GetProcessHeap()

void *malloc (size_t size)
{
	return HeapAlloc (heapNEW, HEAP_ZERO_MEMORY, size);
}

void *realloc (void *block, size_t size)
{
	if (block)
		return HeapReAlloc (heapNEW, HEAP_ZERO_MEMORY, block, size);
	else
		return HeapAlloc (heapNEW, HEAP_ZERO_MEMORY, size);
}

void free (void *block)
{
	if (block)
		HeapFree (heapNEW, 0, block);
}

#ifdef __cplusplus
void * __cdecl operator new (size_t size)
{
	return malloc (size);
}

void __cdecl operator delete (void *block)
{
	free (block);
}

void *__cdecl operator new[] (size_t size)
{
	return ::operator new (size);
}

void __cdecl operator delete[] (void *ptr)
{
	::operator delete (ptr);
}
#endif

#ifdef __GNUC__
void __cxa_pure_virtual(void)
{
}
#endif

#if 0

void *RtlAllocateMemory (unsigned int dwSize)
{
	void *pBlock = malloc (dwSize);
	memset (pBlock, 0, dwSize);

	return pBlock;
}

void RtlFreeMemory (void *pBlock)
{
	free (pBlock);
}

void *RtlReAllocateMemory (void *pBlock, unsigned int dwSize)
{
	return realloc (pBlock, dwSize);
}

/*void * operator new(size_t size)
{
	return RtlAllocateMemory (size);
}

void operator delete(void *block)
{
	RtlFreeMemory (block);
}*/

#endif
