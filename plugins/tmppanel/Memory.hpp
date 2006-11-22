#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#define hHeap GetProcessHeap()

void * my_malloc(size_t size);
void * my_realloc(void *block, size_t size);
void my_free(void *block);

#ifdef __cplusplus
void * __cdecl operator new(size_t size);
void __cdecl operator delete(void *block);
#endif

void _pure_error_ ();

void * my_memcpy(void *dst, const void *src, size_t count);
void * my_memset(void *dst, int val, size_t count);
int my_memcmp(const void *buf1, const void *buf2, size_t count);

#endif /* __MEMORY_HPP__ */