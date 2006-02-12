#ifndef __MEMCMP_HPP__
#define __MEMCMP_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  int memcmp(const void *buf1, const void *buf2, size_t count);
#ifdef __cplusplus
};
#endif

#endif
