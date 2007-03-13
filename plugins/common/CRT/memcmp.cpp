#include "crt.hpp"
#ifdef _MSC_VER
#pragma function(memcmp)
#endif

int __cdecl memcmp(const void *buf1, const void *buf2, size_t count)
{
  if (!count)
    return(0);

  while (--count && *(char *)buf1 == *(char *)buf2)
  {
    buf1 = (char *)buf1 + 1;
    buf2 = (char *)buf2 + 1;
  }

  return(*((unsigned char *)buf1) - *((unsigned char *)buf2));
}
