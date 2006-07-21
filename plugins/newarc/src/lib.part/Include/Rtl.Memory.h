#pragma once
#include "Rtl.Base.h"

//extern void *RtlAllocateMemory (unsigned int dwSize);
//extern void  RtlFreeMemory (void *pBlock);
//extern void *RtlReAllocateMemory (void *pBlock, unsigned int dwNewSize);

#ifdef __cplusplus
extern "C" {
#endif
	void *malloc (size_t size);
	void *realloc (void *block, size_t size);
	void free (void *block);
#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
void *__cdecl operator new      (size_t size);
void *__cdecl operator new[]    (size_t size);
void __cdecl  operator delete   (void *p);
void __cdecl  operator delete[] (void *ptr);
#endif

#ifdef __GNUC__
#ifdef __cplusplus
extern "C"{
#endif
	void __cxa_pure_virtual(void);
#ifdef __cplusplus
};
#endif
#endif
