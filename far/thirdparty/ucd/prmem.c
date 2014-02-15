#include "prmem.h"

void* PR_Malloc(PRUint32 size)
{
	return malloc(size);
}


void* PR_Calloc(PRUint32 nelem, PRUint32 elsize)
{
	return calloc(nelem, elsize);
}

void* PR_Realloc(void *ptr, PRUint32 size)
{
	return realloc(ptr, size);
}

void PR_Free(void *ptr)
{
	free(ptr);
}
