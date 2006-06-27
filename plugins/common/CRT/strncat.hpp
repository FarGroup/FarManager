#ifndef __STRNCAT_HPP__
#define __STRNCAT_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  char *strncat(char *first, const char *last, size_t count);
#ifdef __cplusplus
};
#endif

#endif
