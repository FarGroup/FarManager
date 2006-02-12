#include "crt.hpp"

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

void *memset(void *dst, int val, size_t count)
{
  void *start = dst;

  while (count--)
  {
    *(char *)dst = (char)val;
    dst = (char *)dst + 1;
  }
  return(start);
}

void *memmove(void *dst, const void *src, size_t count)
{
  void *ret = dst;
  if (dst <= src || (char *)dst >= ((char *)src + count))
  {
    while (count--)
    {
      *(char *)dst = *(char *)src;
      dst = (char *)dst + 1;
      src = (char *)src + 1;
    }
  }
  else
  {
    dst = (char *)dst + count - 1;
    src = (char *)src + count - 1;

    while (count--)
    {
      *(char *)dst = *(char *)src;
      dst = (char *)dst - 1;
      src = (char *)src - 1;
    }
  }
  return(ret);
}
