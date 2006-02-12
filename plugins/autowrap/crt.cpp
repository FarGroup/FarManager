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

void *memchr(const void *buf, int chr, size_t cnt)
{
  while (cnt && (*(unsigned char *)buf != (unsigned char)chr))
  {
    buf = (unsigned char *)buf + 1;
    cnt--;
  }
  return(cnt ? (void *)buf : NULL);
}

int isspace(int c)
{
  return (c==0x20 || (c>=0x09 && c<=0x0D));
}

char *strchr(register const char *s,int c)
{
  do
  {
    if(*s==c)
      return (char*)s;
  } while (*s++);
  return 0;
}
