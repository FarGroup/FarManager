#ifndef __STRNCMP_HPP__
#define __STRNCMP_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  int strncmp(const char *first, const char *last, size_t count);
#ifdef __cplusplus
};
#endif

#endif
