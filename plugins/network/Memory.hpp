#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__


void *my_malloc(size_t size);
void *my_realloc(void *block, size_t size);
void my_free(void *block);

#ifdef __cplusplus
void * cdecl operator new(size_t size);
void cdecl operator delete(void *block);
#endif

#endif //__MEMORY_HPP__