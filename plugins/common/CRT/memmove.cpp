#if defined(_MSC_VER) && _MSC_VER < 1900
#define __midl  // do not include inline implementation
#endif

#include "crt.hpp"
#if _MSC_VER >= 1925
 #pragma function(memmove)
#endif

#if !(defined(_MSC_VER) && _MSC_VER >= 1900 && defined(UNICODE))

#if defined(UNICODE) && !defined(__BORLANDC__)
typedef wchar_t PTRTYP;
#else
typedef void  PTRTYP;
#endif

PTRTYP * __cdecl
#ifndef UNICODE
               memmove
#else
#ifdef __BORLANDC__
               _wmemmove
#else
               wmemmove
#endif
#endif
                        (PTRTYP *dst, const PTRTYP *src, size_t count)
{
  PTRTYP *ret = dst;
  if (dst <= src || (TCHAR *)dst >= ((TCHAR *)src + count))
  {
    while (count--)
    {
      *(TCHAR *)dst = *(TCHAR *)src;
      dst = (TCHAR *)dst + 1;
      src = (TCHAR *)src + 1;
    }
  }
  else
  {
    dst = (TCHAR *)dst + count - 1;
    src = (TCHAR *)src + count - 1;

    while (count--)
    {
      *(TCHAR *)dst = *(TCHAR *)src;
      dst = (TCHAR *)dst - 1;
      src = (TCHAR *)src - 1;
    }
  }
  return(ret);
}

#endif
