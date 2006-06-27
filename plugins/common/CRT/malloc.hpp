#ifndef __MALLOC_HPP__
#define __MALLOC_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *malloc(size_t size);
#ifdef __cplusplus
};
#endif

#endif
