#ifndef __MEMSET_HPP__
#define __MEMSET_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memset(void *dst, int val, size_t count);
#ifdef __cplusplus
};
#endif

#endif
