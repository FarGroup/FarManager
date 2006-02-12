#ifndef __CRT_HPP__
#define __CRT_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memcpy(void * dst, const void * src, size_t count);
  void *memset(void *dst, int val, size_t count);
  void *memchr(const void *buf, int chr, size_t cnt);
  int isspace(int c);
  char *strchr(register const char *s,int c);
#ifdef __cplusplus
};
#endif

#endif
