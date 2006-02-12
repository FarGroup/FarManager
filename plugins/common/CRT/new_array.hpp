#ifndef __NEW_ARRAY_HPP__
#define __NEW_ARRAY_HPP__
#include <stddef.h>

#ifdef __cplusplus
void *__cdecl operator new[] (size_t size);
#endif

#endif
