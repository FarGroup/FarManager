#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *malloc(size_t size);
  void *realloc(void *block, size_t size);
  void free(void *block);
#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
void *__cdecl operator new(size_t size);
void *__cdecl operator new[] (size_t size);
void __cdecl operator delete(void *p);
void __cdecl operator delete[] (void *ptr);
#endif

#endif
