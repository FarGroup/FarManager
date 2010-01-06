#include <Rtl.Base.h>
#include <malloc.h>

#ifdef _DEBUG

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