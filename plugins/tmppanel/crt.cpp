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

int memcmp(const void * buf1, const void * buf2, size_t count)
{
  if (!count)
    return(0);

  while ( --count && *(char *)buf1 == *(char *)buf2 )
  {
    buf1 = (char *)buf1 + 1;
    buf2 = (char *)buf2 + 1;
  }

  return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
}
