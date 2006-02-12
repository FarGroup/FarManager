#ifndef __CRT_HPP__
#define __CRT_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memcpy(void * dst, const void * src, size_t count);
  void *memset(void *dst, int val, size_t count);
  int memcmp(const void * buf1, const void * buf2, size_t count);
#ifdef __cplusplus
};
#endif

#endif
