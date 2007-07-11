#include "crt.hpp"
#if defined(_MSC_VER) && !defined(UNICODE)
#pragma function(memset)
#endif

#ifndef UNICODE
  typedef void PTRTYP;
  void * __cdecl memset(void *dst, int val, size_t count)
#else
  typedef wchar_t PTRTYP;
  wchar_t * __cdecl _wmemset(wchar_t *dst, wchar_t val, size_t count)
#endif
{
  PTRTYP *start = dst;

  while (count--)
  {
    *(TCHAR *)dst = (TCHAR)val;
    dst = (TCHAR *)dst + 1;
  }
  return(start);
}
