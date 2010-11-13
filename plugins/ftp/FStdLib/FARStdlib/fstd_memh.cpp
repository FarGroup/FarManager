#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static HANDLE Heap = NULL;

#define CHH  if ( !Heap ) Heap = HeapCreate(0,0x400000,0);

BOOL WINAPI __HeapCheck_H(void)
{
	BOOL rc;
	CHH
#ifndef __GNU
	__try
	{
#endif
		rc = HeapValidate(Heap,0,NULL);
#ifndef __GNU
	} __except(EXCEPTION_EXECUTE_HANDLER)
	{
		rc = FALSE;
	}
#endif
	return rc;
}

LPVOID WINAPI __Alloc_H(SIZE_T sz)              { CHH return HeapAlloc(Heap,0,sz); }
LPVOID WINAPI __Realloc_H(LPVOID ptr,SIZE_T sz) { CHH return (!ptr) ? HeapAlloc(Heap,0,sz) : HeapReAlloc(Heap,0,ptr,sz); }
void   WINAPI __Del_H(LPVOID ptr)               { CHH if(ptr) HeapFree(Heap,0,ptr); }
SIZE_T WINAPI __PtrSize_H(LPVOID ptr)           { CHH return ptr ? HeapSize(Heap,0,ptr) : 0; }
