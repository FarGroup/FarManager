#include "memchr.hpp"

void *memchr(const void *buf, int chr, size_t cnt)
{
  while (cnt && (*(unsigned char *)buf != (unsigned char)chr))
  {
    buf = (unsigned char *)buf + 1;
    cnt--;
  }
  return(cnt ? (void *)buf : NULL);
}
