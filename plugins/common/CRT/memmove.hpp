#ifndef __MEMMOVE_HPP__
#define __MEMMOVE_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memmove(void *dst, const void *src, size_t count);
#ifdef __cplusplus
};
#endif

#endif
