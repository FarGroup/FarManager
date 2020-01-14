#include "crt.hpp"

#ifdef _MSC_VER
#pragma function(memcpy)
#endif

void * __cdecl memcpy (void *dst, const void *src, size_t count)
{
  void *ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

WMEM* __cdecl
#ifdef __BORLANDC__
         _wmemcpy
#else
         wmemcpy
#endif
                 (WMEM *dst, const WMEM *src, size_t count)
{
    return (WMEM*)memcpy(dst, src, count*sizeof(WMEM));
}
