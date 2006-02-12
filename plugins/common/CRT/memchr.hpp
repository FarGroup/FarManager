#ifndef __MEMCHR_HPP__
#define __MEMCHR_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memchr(const void *buf, int chr, size_t cnt);
#ifdef __cplusplus
};
#endif

#endif
