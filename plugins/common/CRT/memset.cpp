#if defined(_MSC_VER) && _MSC_VER < 1900
#define __midl  // do not include inline implementation
#endif

#include "crt.hpp"
#ifdef _MSC_VER
#pragma function(memset)
#endif

void * __cdecl memset (void *dst, int val, size_t count)
{
  void *start = dst;

  while (count--)
  {
    *(char *)dst = (char)val;
    dst = (char *)dst + 1;
  }
  return(start);
}

WMEM* __cdecl
#ifdef __BORLANDC__
                    _wmemset
#else
                    wmemset
#endif
                            (WMEM *dst, WMINT val, size_t count)
{
  WMEM *start = dst;

  while (count--)
  {
    *(wchar_t *)dst = (wchar_t)val;
    dst = (wchar_t *)dst + 1;
  }
  return(start);
}
