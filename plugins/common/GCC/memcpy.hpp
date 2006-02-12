#ifndef __MEMCPY_HPP__
#define __MEMCPY_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memcpy(void *dst, const void *src, size_t count);
#ifdef __cplusplus
};
#endif

#endif
