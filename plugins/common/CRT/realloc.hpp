#ifndef __REALLOC_HPP__
#define __REALLOC_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *realloc(void *block, size_t size);
#ifdef __cplusplus
};
#endif

#endif
