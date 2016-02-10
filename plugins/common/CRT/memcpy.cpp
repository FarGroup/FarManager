#if defined(_MSC_VER) && _MSC_VER < 1900
#define __midl  // do not include inline implementation
#endif

#include "crt.hpp"
#if defined(_MSC_VER) && !defined(UNICODE)
#pragma function(memcpy)
#endif

#if !(defined(_MSC_VER) && _MSC_VER >= 1900 && defined(UNICODE))

#if defined(UNICODE) && !defined(__BORLANDC__)
typedef wchar_t PTRTYP;
#else
typedef void  PTRTYP;
#endif

PTRTYP * __cdecl
#ifndef UNICODE
               memcpy
#else
#ifdef __BORLANDC__
               _wmemcpy
#else
               wmemcpy
#endif
#endif
                        (PTRTYP *dst, const PTRTYP *src, size_t count)
{
  PTRTYP *ret = dst;

  while (count--)
  {
    *(TCHAR *)dst = *(TCHAR *)src;
    dst = (TCHAR *)dst + 1;
    src = (TCHAR *)src + 1;
  }
  return(ret);
}

#endif