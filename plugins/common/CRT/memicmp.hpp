#ifndef __MEMICMP_HPP__
#define __MEMICMP_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  int memicmp(const void *first, const void *last, size_t count);
#ifdef __cplusplus
};
#endif

#endif
