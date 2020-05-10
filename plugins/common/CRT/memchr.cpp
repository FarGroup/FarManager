#include "crt.hpp"
#if _MSC_VER >= 1925
 #pragma function(memchr)
#endif

#ifdef _MSC_VER
#pragma function(memchr)
#endif

_CONST_RETURN void * __cdecl memchr(const void *buf, int chr, size_t cnt)
{
  while (cnt && (*(unsigned char *)buf != (unsigned char)chr))
  {
    buf = (unsigned char *)buf + 1;
    cnt--;
  }
  return(cnt ? (void *)buf : NULL);
}
