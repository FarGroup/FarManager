#include "crt.hpp"
#if defined(_MSC_VER) && !defined(UNICODE)
#pragma function(memset)
#endif

#if defined(_MSC_VER) && defined(UNICODE)
typedef wchar_t PTRTYP;
#else
typedef void    PTRTYP;
#endif

PTRTYP * __cdecl
#ifndef UNICODE
               memset
#else
               _wmemset
#endif
                        (PTRTYP *dst, int val, size_t count)
{
  PTRTYP *start = dst;

  while (count--)
  {
    *(TCHAR *)dst = (TCHAR)val;
    dst = (TCHAR *)dst + 1;
  }
  return(start);
}
